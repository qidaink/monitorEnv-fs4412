/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : adc_dev.c
 * Author     : qidaink
 * Date       : 2022-08-28
 * Version    :
 * Description: adc驱动模块
 * Others     : 注意，Analog Input Range: 0 ~ 1.8V
 * Log        :
 * ======================================================
 */
/**
 * ADC0 -- XadcAIN0 -- AF10
 * ADC1 -- XadcAIN1 -- AE10
 * ADC2 -- XadcAIN2 -- AD10
 * ADC3 -- XadcAIN3 -- AB11 -- 板子使用的是这个
 */

/* 头文件 */
#include <linux/module.h> /* MODULE_LICENSE */
#include <linux/kernel.h>
#include <linux/init.h>    /* module_init module_exit */
#include <linux/platform_device.h>
#include <linux/fs.h>      /* register_chrdev_region alloc_chrdev_region unregister_chrdev_region */
#include <linux/cdev.h>    /* cdev_init cdev_add cdev_del */
#include <linux/uaccess.h> /* copy_from_user copy_to_user */
#include <linux/device.h>  /* class_create class_destroy device_create device_destroy */
#include <linux/export.h>  /* THIS_MODULE */
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/io.h>      /* writel readl */
#include <linux/of.h>       /* of_find_node_by_path */
#include <linux/wait.h>     /* wait_event_interruptible wake_up_interruptible */
#include <linux/interrupt.h>/* request_irq */
#include <linux/of_irq.h>   /* irq_of_parse_and_map */
#include <linux/sched.h> 

#include "adc_dev.h"

//=======================================================
#define MODULE_NAME "adc_dev"    /* 设备名称： /proc/devices 文件中与该设备对应的名字，方便用户层查询主设备号 */
#define MAX_LEN     4               /* 驱动内部读写缓冲区字节数 */
#define DEBUG_FLAG  1

/** adc寄存器地址宏定义（基地址+偏移） */
#define ADC_BASE  (0x126C0000)  /* 基地址：Base Address，直接从设备树读取，这里不需要 */
#define ADCCON    (0x0000)      /* ADC Control Register  0x0000_3FC4 */
#define ADCDLY    (0x0008)      /* ADC Start or Interval Delay Register  0x0000_00FF */
#define ADCDAT    (0x000C)      /* ADC Conversion Data Register  Undefined */
#define CLRINTADC (0x0018)      /* Clear ADC Interrupt  Undefined */
#define ADCMUX    (0x001C)      /* Specifies the Analog input channel selection  0x0000_0000 */

/* 字符设备的属性 */
struct __my_dev
{
    /* 1.设备号与设备数量 */
    int major;   /* 主设备号：占高12位，用来表示驱动程序相同的一类设备，Linux系统中主设备号范围为 0~4095 */
    int minor;   /* 次设备号：占低20位，用来表示被操作的哪个具体设备 */
    dev_t devno; /* 新字符设备的设备号：由主设备号和次设备号组成     */
    int dev_num; /* 申请的设备数量 */
    /* 2.字符设备对象 */
    struct cdev myadc; /* 定义一个字符设备对象设备 */
    /* 3.自动创建设备节点相关成员变量 */
    struct class *class;   /* 类 */
    struct device *device; /* 设备 /dev/dev_name */
    /* 4.驱动中的缓冲区数组定义 */
    char adc_buf[MAX_LEN]; /* 读写缓冲区 */
    /* 5.阻塞非阻塞支持 */
    wait_queue_head_t wq;
    /* 6.adc相关成员 */
    int have_data;           /* 是否有数据可读 */
    int fs4412_adc;          /* 保存adc数据 */
    struct resource *res1;   /* 保存中断信息地址 */
    struct resource *res2;   /* 保存寄存器基地址信息地址 */
    volatile void *adc_base; /* 注意这里是 void * 类型，我用unsigned int * 的话，配置会有问题 */
};

struct __my_dev *p_gmyadc_dev; /* 定义一个字符设备指针变量 */

/* 函数声明 */
static int myadc_open(struct inode *pnode, struct file *pfile);                                      /* 打开设备 */
static int myadc_close(struct inode *pnode, struct file *pfile);                                     /* 关闭设备 */
static ssize_t myadc_read(struct file *pfile, char __user *pbuf, size_t count, loff_t *ppos);        /* 读取数据 */

void iounmap_adc_reg(struct __my_dev *p_my_dev);
void ioremap_adc_reg(struct __my_dev *p_my_dev);

/* 操作函数集定义 */
struct file_operations myadc_ops = {
    /* data */
    .owner = THIS_MODULE,
    .open = myadc_open,
    .release = myadc_close,
    .read = myadc_read,
};

/**
 * @Function    : myadc_open
 * @brief       : 打开设备
 * @param pnode : struct inode * 类型，内核中记录文件元信息的结构体
 * @param pfile : struct file *  类型，读写文件内容过程中用到的一些控制性数据组合而成的对象
 * @return      : 0,操作完成
 * @Description : 函数名初始化给 struct file_operations 的成员 .open
 */
static int myadc_open(struct inode *pnode, struct file *pfile)
{
    /* 1.获取 struct __my_dev p_gmyadc_dev 地址 */
    pfile->private_data = (void *)(container_of(pnode->i_cdev, struct __my_dev, myadc));
    DEBUG_PRINTK("[INFO ]myadc_open is called!\n", DEBUG_FLAG);
    return 0;
}

/**
 * @Function    : myadc_close
 * @brief       : 关闭设备
 * @param pnode : struct inode * 类型，内核中记录文件元信息的结构体
 * @param pfile : struct file *  类型，读写文件内容过程中用到的一些控制性数据组合而成的对象
 * @return      : 0,操作完成
 * @Description : 函数名初始化给 struct file_operations 的成员 .close
 */
static int myadc_close(struct inode *pnode, struct file *pfile)
{
    /* 0.相关变量定义 */
    // struct __my_dev *p_my_dev = (struct __my_dev *)(pfile->private_data); /* 通过私有数据方式获取 p_gmyled_dev设备属性 */
    DEBUG_PRINTK("[INFO ]myadc_close is called!\n", DEBUG_FLAG);
    return 0;
}
/**
 * @Function    : myadc_read
 * @brief       : 读设备
 * @param pfile : struct file *  类型，指向open产生的struct file类型的对象，表示本次read对应的那次open。
 *                                    (读写文件内容过程中用到的一些控制性数据组合而成的对象)
 * @param pbuf  ：__user * 类型，指向用户空间一块内存，用来保存读到的数据。
 * @param count ：size_t   类型，用户期望读取的字节数。
 * @param ppos  ：loff_t * 类型，对于需要位置指示器控制的设备操作有用，用来指示读取的起始位置，读完后也需要变更位置指示器的指示位置。
 * @return      : 成功返回本次成功读取的字节数，失败返回-1
 * @Description : 函数名初始化给 struct file_operations 的成员 .read
 */
static ssize_t myadc_read(struct file *pfile, char __user *pbuf, size_t count, loff_t *ppos)
{
    /* 0.相关变量定义 */
    struct __my_dev *p_my_dev = (struct __my_dev *)(pfile->private_data); /* 通过私有数据方式获取 p_gmyled_dev设备属性 */
    /* 1.选择要读的通道
     * ADCMUX[SEL_MUX(3:0)]：选择转换通道。
     */
    writel(0x3, p_my_dev->adc_base + ADCMUX);
    /* 2.设置ADC分频并启动一次转换
     * ADCCON[RES(16)]：ADC 精度。1，12位；0，10位。
     * ADCCON[PRSCEN(14)]：分频器使能(必须使用)。为1，表示开启分频器。
     * ADCCON[PRSCVL(13:6)]：分频值（19 ~ 255）。ADC_CLK = PLCK/(255 +1) = 100MHz / 256 = 390625HZ, ADC转换频率=390625HZ/5=78125HZ
     * ADCCON[ENABLE_START(0)]：启动一次转换
     */
    writel(0X1 << 16 | 1 << 14 | 0XFF << 6 | 1 << 0, p_my_dev->adc_base + ADCCON);
    /* 3.等待有数据可以读取 */
    wait_event_interruptible(p_my_dev->wq, p_my_dev->have_data == 1); /* 等待，直到 have_data == 1 */
    /* 4.只获取 bit[11:0] 的值 */
    p_my_dev->fs4412_adc = readl(p_my_dev->adc_base + ADCDAT) & 0xfff;
    /* 5.读取数据到用户空间 */
    if( copy_to_user(pbuf, &p_my_dev->fs4412_adc, count) > 0) /* 驱动层不进行运算，直接将数据拷贝给应用层 */
    {
        printk("[ERROR]copy_to_user failed!\n");
        return -EFAULT;
    }
    /* 6.数据标志清0 */
    p_my_dev->have_data = 0; 
    return count;
}

/**
 * @Function       : ioremap_adc_reg
 * @brief          : adc寄存器地址映射
 * @param p_my_dev : struct __my_dev * 类型
 * @return         : none
 * @Description    :
 */
void ioremap_adc_reg(struct __my_dev *p_my_dev)
{
    /* 1.虚拟地址映射 */
    p_my_dev->adc_base = ioremap(p_my_dev->res2->start, p_my_dev->res2->end - p_my_dev->res2->start);
    
}

/**
 * @Function       : iounmap_adc_reg
 * @brief          : 取消adc寄存器地址映射
 * @param p_my_dev : struct __my_dev * 类型
 * @return         : none
 * @Description    :
 */
void iounmap_adc_reg(struct __my_dev *p_my_dev)
{
    /* 1.取消地址映射 */
    iounmap(p_my_dev->adc_base);
}

//=======================================================
// platform平台驱动相关
/* 函数声明 */
static int adc_driver_probe(struct platform_device *p_pltdev);
static int adc_driver_remove(struct platform_device *p_pltdev);

/**
 * @Function       : adc_handler
 * @brief          : ADC中断处理函数
 * @param p_my_dev : struct __my_dev * 类型
 * @param irqno    : int类型
 * @param dev      : void *类型
 * @return         : irqreturn_t类型
 * @Description    : 
 */
static irqreturn_t adc_handler(int irqno, void *arg)  
{  
    /* 0.相关变量定义 */
    struct __my_dev *p_my_dev = (struct __my_dev *)arg;
    /* 1.重置唤醒条件 */
    p_my_dev->have_data = 1;   //唤醒条件
    /* 2. 清中断*/
    writel(0x12, p_my_dev->adc_base + CLRINTADC);
    /* 3.唤醒读取任务 */
    wake_up_interruptible(&p_my_dev->wq);  
    return IRQ_HANDLED;  
} 

/**
 * @Function       : adc_driver_probe
 * @brief          : 设备和驱动匹配成功之后的调用函数
 * @param p_pltdev : struct device * 类型
 * @return         : int 类型
 * @Description    : 函数名赋值给 struct platform_driver 结构体的 .probe 成员
 */
static int adc_driver_probe(struct platform_device *p_pltdev)
{
    /* 1.申请设备的内存空间 */
    p_gmyadc_dev = (struct __my_dev *)kmalloc(sizeof(struct __my_dev), GFP_KERNEL); /* 为设备属性结构体分配内存空间 */
    if (p_gmyadc_dev == NULL)
    {
        printk("[error]p_gmyadc_dev kmalloc failed!\n");
        return -1;
    }
    memset(p_gmyadc_dev, 0, sizeof(struct __my_dev)); /* 申请的内存空间清 0 */
    /* 2.分配设备号 */
    /* 2.1尝试指定设备的主设备号和次设备号 */
    p_gmyadc_dev->major = 1;                                               /* 手动指定主设备号 */
    p_gmyadc_dev->minor = 0;                                               /* 大部分驱动次设备号都选择0 */
    p_gmyadc_dev->dev_num = 1;                                             /* 申请的设备数量 */
    p_gmyadc_dev->devno = MKDEV(p_gmyadc_dev->major, p_gmyadc_dev->minor); /* MKDEV宏用来将主设备号和次设备号组合成32位完整的设备号 */
    if (register_chrdev_region(p_gmyadc_dev->devno, p_gmyadc_dev->dev_num, MODULE_NAME))
    {
        /* 手动指定设备号分配失败，申请自动分配设备号 */
        if (alloc_chrdev_region(&p_gmyadc_dev->devno, 0, p_gmyadc_dev->dev_num, MODULE_NAME))
        {
            printk("[error]get devno failed!\n");
            return -1;
        }
        /* 重新获取主设备号和次设备号 */
        p_gmyadc_dev->major = MAJOR(p_gmyadc_dev->devno); /* 自动分配设备号后，分离出主设备号 */
        p_gmyadc_dev->minor = MINOR(p_gmyadc_dev->minor); /* 自动分配设备号后，分离出次设备号 */
    }
    /* 3.向Linux内核注册新的字符设备 */
    /* 3.1初始化 myadc 对象, 给该对象添加操作函数集 */
    cdev_init(&p_gmyadc_dev->myadc, &myadc_ops);
    /* 3.2向 Linux 系统添加字符设备 (cdev结构体变量,将会被添加到一个hash链表中) */
    p_gmyadc_dev->myadc.owner = THIS_MODULE;
    cdev_add(&p_gmyadc_dev->myadc, p_gmyadc_dev->devno, p_gmyadc_dev->dev_num); /* 会在 /proc/devices 中创建设备号和对应的名称记录 */
    /* 4.自动创建设备节点(省去 mknod 命令在 /dev/ 下创建设备节点步骤) */
    /* 4.1创建类（创建成功时，名称可以在 /sys/class 中查到） */
    p_gmyadc_dev->class = class_create(THIS_MODULE, MODULE_NAME);
    if (IS_ERR(p_gmyadc_dev->class))
    {
        printk("[error]class_create failed!\n");
        cdev_del(&p_gmyadc_dev->myadc);                                       /* 删除字符设备对象 cdev */
        unregister_chrdev_region(p_gmyadc_dev->devno, p_gmyadc_dev->dev_num); /* 注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
        return PTR_ERR(p_gmyadc_dev->class);
    }
    /* 4.2创建设备(创建的设备为 /dev/dev_name) */
    p_gmyadc_dev->device = device_create(p_gmyadc_dev->class, NULL, p_gmyadc_dev->devno, NULL, "%s%d", MODULE_NAME, p_gmyadc_dev->minor);
    if (IS_ERR(p_gmyadc_dev->device))
    {
        printk("[error]device create failed!\n");
        class_destroy(p_gmyadc_dev->class);                                   /* 删除类 */
        cdev_del(&p_gmyadc_dev->myadc);                                       /* 删除字符设备对象 cdev */
        unregister_chrdev_region(p_gmyadc_dev->devno, p_gmyadc_dev->dev_num); /* 注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
        return PTR_ERR(p_gmyadc_dev->device);
    }
    /* 5.初始化读写阻塞等待队列 */
    init_waitqueue_head(&p_gmyadc_dev->wq); /* 初始化等待队列头 */
    /* 6.ADC相关配置 */
    /* 6.1获取设备树节点资源 */
    p_gmyadc_dev->res1 = platform_get_resource(p_pltdev, IORESOURCE_IRQ, 0); /* 中断信息 */
    p_gmyadc_dev->res2 = platform_get_resource(p_pltdev, IORESOURCE_MEM, 0); /* 寄存器基地址 */
    /* 6.2寄存器映射 */
    ioremap_adc_reg(p_gmyadc_dev);
    /* 6.3申请中断 */
    if(request_irq(p_gmyadc_dev->res1->start, adc_handler, IRQF_DISABLED, "adc1", p_gmyadc_dev) < 0)
    {
        printk("[error]device create failed!\n");
        class_destroy(p_gmyadc_dev->class);                                   /* 删除类 */
        cdev_del(&p_gmyadc_dev->myadc);                                       /* 删除字符设备对象 cdev */
        unregister_chrdev_region(p_gmyadc_dev->devno, p_gmyadc_dev->dev_num); /* 注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
        return PTR_ERR(p_gmyadc_dev->device);
    }
    /* 打印提示信息 */
    printk("=======================================\n");
    printk("[INFO ]res1->start :%d.\n", p_gmyadc_dev->res1->start);
    printk("[INFO ]res2->start=%#x,res2->end - res2->start :%#x.\n", p_gmyadc_dev->res2->start, p_gmyadc_dev->res2->end - p_gmyadc_dev->res2->start);
    printk("[INFO ]%s(%d):major=%d,minor=%d\n", MODULE_NAME, p_gmyadc_dev->devno, p_gmyadc_dev->major, p_gmyadc_dev->minor);
    printk("[INFO ]"BLUE"%s%d"CLS" has been created!\n", MODULE_NAME, p_gmyadc_dev->minor);
    printk("[INFO ]adc_driver_probe is called!\n");
    printk("=======================================\n");

    return 0;
}
/**
 * @Function       : adc_driver_remove
 * @brief          : 卸载设备（rmmod 删除 driver 模块的时候被调用）
 * @param p_pltdev : struct platform_device * 类型
 * @return         : int 类型
 * @Description    : 函数名赋值给 struct platform_driver 结构体的 .remove 成员
 */
static int adc_driver_remove(struct platform_device *p_pltdev)
{
    /* 1.重新获取设备号(有可能是动态分配了的设备号) */
    p_gmyadc_dev->devno = MKDEV(p_gmyadc_dev->major, p_gmyadc_dev->minor); /* MKDEV宏用来将主设备号和次设备号组合成32位完整的设备号 */
    /* 2.取消寄存器映射和中断 */
    iounmap_adc_reg(p_gmyadc_dev);
    free_irq(p_gmyadc_dev->res1->start, p_gmyadc_dev);
    /* 3.删除设备节点 */
    /* 3.1删除设备(/dev/dev_name 将会被删除) */
    device_destroy(p_gmyadc_dev->class, p_gmyadc_dev->devno);
    printk("[INFO]p_gmyadc_dev[%d]: [/dev/%s%d] has been deleted!\n", p_gmyadc_dev->devno, MODULE_NAME, p_gmyadc_dev->minor);
    /* 3.2删除类（将会删除 /sys/class 中的名称） */
    class_destroy(p_gmyadc_dev->class);
    /* 4.删除字符设备对象 cdev */
    cdev_del(&p_gmyadc_dev->myadc);
    /* 5.注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
    unregister_chrdev_region(p_gmyadc_dev->devno, p_gmyadc_dev->dev_num);
    /* 6.释放字符设备的存储空间 */
    kfree(p_gmyadc_dev);
    p_gmyadc_dev = NULL;

    printk("[INFO]adc_driver_remove is caladc!\n");
    return 0;
}

/** 定义匹配设备树数组
 *  与数组中含有的名称相同的设备都可以匹配到该驱动
 *  结构体变量名赋值给 struct platform_driver 结构体的 ..driver.of_match_table 成员
 */
struct of_device_id adc_driver_dt_ids[] = {
    [0] = {.compatible = "fs4412,adc" },
    [1] = {}, /* means ending */
};

/** 定义platform平台驱动 */
struct platform_driver adc_driver = {
    .driver = {
        .name = MODULE_NAME, /* 必须初始化 name 成员 */
        // .of_match_table = adc_driver_dt_ids,
        .of_match_table = of_match_ptr(adc_driver_dt_ids), /* 直接使用的话，加载驱动报错，暂未研究 */
    },
    .probe = adc_driver_probe,   /* 设备和驱动匹配成功之后的调用函数 */
    .remove = adc_driver_remove, /* 卸载设备 */
};


//=======================================================
// 整个模块相关
/* 驱动入口函数 */
int __init adc_dev_init(void)
{
    /* 注册平台设备驱动 */
    platform_driver_register(&adc_driver);  
    /* 打印加载模块的提示信息 */
    printk("[INFO]This module [%s](driver) is loaded!\n", MODULE_NAME);
    return 0;
}

/* 驱动出口函数 */
void __exit adc_dev_exit(void)
{
    /* 注销平台设备驱动 */
    platform_driver_unregister(&adc_driver);
    /* 打印卸载模块的提示信息 */
    printk("[INFO]This module [%s](driver) is exited!\n", MODULE_NAME);
}

/* 将上面两个函数指定为驱动的入口和出口函数 */
module_init(adc_dev_init);
module_exit(adc_dev_exit);

/* 模块信息(通过 modinfo adc_dev 查看) */
MODULE_LICENSE("GPL");               /* 源码的许可证协议 */
MODULE_AUTHOR("qidaink");            /* 字符串常量内容为模块作者说明 */
MODULE_DESCRIPTION("Description");   /* 字符串常量内容为模块功能说明 */
MODULE_ALIAS("module's other name"); /* 字符串常量内容为模块别名 */
