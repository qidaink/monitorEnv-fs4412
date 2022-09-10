/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : buzzer_dev.c
 * Author     : qidaink
 * Date       : 2022-08-28
 * Version    :
 * Description: buzzer驱动模块
 * Others     :
 * Log        :
 * ======================================================
 */

/** 
 * BEEP -- XpwmTOUT0/LCD_FRM/GPD0_0 -- AE24
 */

/* 头文件 */
#include <linux/module.h> /* MODULE_LICENSE */
#include <linux/kernel.h>
#include <linux/init.h>    /* module_init module_exit */
#include <linux/fs.h>      /* register_chrdev_region alloc_chrdev_region unregister_chrdev_region */
#include <linux/cdev.h>    /* cdev_init cdev_add cdev_del */
#include <linux/uaccess.h> /* copy_from_user copy_to_user */
#include <linux/device.h>  /* class_create class_destroy device_create device_destroy */
#include <linux/export.h>  /* THIS_MODULE */
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/io.h>      /* writel readl */

#include "buzzer_dev.h"
//=======================================================
#define MODULE_NAME "buzzer_dev"    /* 设备名称： /proc/devices 文件中与该设备对应的名字，方便用户层查询主设备号 */
#define MAX_LEN     4               /* 驱动内部读写缓冲区字节数 */

/** buzzer寄存器地址宏定义 */
#define GPD0CON       (0x114000a0)  
#define PWM_BASE      (0x139D0000)           
#define TCFG0         (0x0000)                 
#define TCFG1         (0x0004)                              
#define TCON          (0x0008)               
#define TCNTB0        (0x000C)            
#define TCMPB0        (0x0010)

/* 字符设备的属性 */
struct __my_dev
{
    /* 1.设备号与设备数量 */
    int major;   /* 主设备号：占高12位，用来表示驱动程序相同的一类设备，Linux系统中主设备号范围为 0~4095 */
    int minor;   /* 次设备号：占低20位，用来表示被操作的哪个具体设备 */
    dev_t devno; /* 新字符设备的设备号：由主设备号和次设备号组成     */
    int dev_num; /* 申请的设备数量 */
    /* 2.字符设备对象 */
    struct cdev mybuzzer;            /* 定义一个字符设备对象设备 */
    /* 3.自动创建设备节点相关成员变量 */
    struct class *class;             /* 类 */
    struct device *device;           /* 设备 /dev/dev_name */
    /* 4.驱动中的缓冲区数组定义 */
    char buzzer_buf[MAX_LEN];       /* 读写缓冲区 */
    /* 5.buzzer寄存器虚拟地址 */
    volatile unsigned int *gpd0con;
    volatile void *pwm_base;        /* 注意这里是 void * 类型，我用unsigned int * 的话，配置会有问题 */
};

struct __my_dev *p_gmybuzzer_dev; /* 定义一个字符设备指针变量 */

/* 新字符设备操作函数声明 */
int mybuzzer_open(struct inode *pnode, struct file *pfile);                                      /* 打开设备 */
int mybuzzer_close(struct inode *pnode, struct file *pfile);                                     /* 关闭设备 */
ssize_t mybuzzer_read(struct file *pfile, char __user *pbuf, size_t count, loff_t *ppos);        /* 读取数据 */
ssize_t mybuzzer_write(struct file *pfile, const char __user *pbuf, size_t count, loff_t *ppos); /* 写数据 */
long mybuzzer_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg);                    /* 设备控制 */

void buzzer_init(struct __my_dev *p_my_dev);                                          /* 蜂鸣器初始化 */
void buzzer_on(struct __my_dev *p_my_dev);                                            /* 打开buzzer */
void buzzer_off(struct __my_dev *p_my_dev);                                           /* 关闭buzzer */
static void buzzer_freq(struct __my_dev *p_my_dev, int buzzer_tcnt, int buzzer_tcmp); /* 设置蜂鸣器的占空比 */
void ioremap_buzzer_reg(struct __my_dev *p_my_dev);                                   /* buzzer寄存器地址映射 */
void iounmap_buzzer_reg(struct __my_dev *p_my_dev);                                   /* 取消buzzer寄存器地址映射 */

/* 操作函数集定义 */
struct file_operations myops = {
    /* data */
    .owner = THIS_MODULE,
    .open = mybuzzer_open,
    .release = mybuzzer_close,
    .read = mybuzzer_read,
    .write = mybuzzer_write,
    .unlocked_ioctl = mybuzzer_ioctl,
};

/**
 * @Function    : mybuzzer_open
 * @brief       : 打开设备
 * @param pnode : struct inode * 类型，内核中记录文件元信息的结构体
 * @param pfile : struct file *  类型，读写文件内容过程中用到的一些控制性数据组合而成的对象
 * @return      : 0,操作完成
 * @Description : 函数名初始化给 struct file_operations 的成员 .open
 */
int mybuzzer_open(struct inode *pnode, struct file *pfile)
{
    /* 1.获取 struct __my_dev p_gmybuzzer_dev 地址 */
    pfile->private_data = (void *)(container_of(pnode->i_cdev, struct __my_dev, mybuzzer));
    printk("[INFO ]mybuzzer_open is called!\n");
    return 0;
}

/**
 * @Function    : mybuzzer_close
 * @brief       : 关闭设备
 * @param pnode : struct inode * 类型，内核中记录文件元信息的结构体
 * @param pfile : struct file *  类型，读写文件内容过程中用到的一些控制性数据组合而成的对象
 * @return      : 0,操作完成
 * @Description : 函数名初始化给 struct file_operations 的成员 .close
 */
int mybuzzer_close(struct inode *pnode, struct file *pfile)
{
    /* 0.相关变量定义 */
    struct __my_dev *p_my_dev = (struct __my_dev *)(pfile->private_data); /* 通过私有数据方式获取 p_gmyled_dev设备属性 */
    buzzer_off(p_my_dev);
    printk("[INFO ]mybuzzer_close is called!\n");
    return 0;
}
/**
 * @Function    : mybuzzer_read
 * @brief       : 读设备
 * @param pfile : struct file *  类型，指向open产生的struct file类型的对象，表示本次read对应的那次open。
 *                                    (读写文件内容过程中用到的一些控制性数据组合而成的对象)
 * @param pbuf  ：__user * 类型，指向用户空间一块内存，用来保存读到的数据。
 * @param count ：size_t   类型，用户期望读取的字节数。
 * @param ppos  ：loff_t * 类型，对于需要位置指示器控制的设备操作有用，用来指示读取的起始位置，读完后也需要变更位置指示器的指示位置。
 * @return      : 成功返回本次成功读取的字节数，失败返回-1
 * @Description : 函数名初始化给 struct file_operations 的成员 .read
 */
ssize_t mybuzzer_read(struct file *pfile, char __user *pbuf, size_t count, loff_t *ppos)
{
    /* 0.相关变量定义 */
    int bytes = 0;
    struct __my_dev *p_my_dev = (struct __my_dev *)(pfile->private_data); /* 通过私有数据方式获取 p_gmyled_dev设备属性 */
    /* 1.读取数据 */
    bytes = copy_to_user(pbuf, p_my_dev->buzzer_buf, count);
    if (bytes > 0)
    {
        printk("[ERROR]copy_to_user failed!\n");
        return -1;
    }
    return 0;
}
/**
 * @Function    : mybuzzer_write
 * @brief       : 写设备
 * @param pfile : struct file *  类型，指向open产生的struct file类型的对象，表示本次write对应的那次open。
 *                                    (读写文件内容过程中用到的一些控制性数据组合而成的对象)
 * @param pbuf  ：__user * 类型，指向用户空间一块内存，用来保存被写的数据。
 * @param count ：size_t   类型，用户期望写入的字节数。
 * @param ppos  ：loff_t * 类型，对于需要位置指示器控制的设备操作有用，用来指示写入的起始位置，写完后也需要变更位置指示器的指示位置。
 * @return      : 成功返回本次成功写入的字节数，失败返回-1
 * @Description : 函数名初始化给 struct file_operations 的成员 .write
 */
ssize_t mybuzzer_write(struct file *pfile, const char __user *pbuf, size_t count, loff_t *ppos)
{
    /* 0.相关变量定义 */
    int bytes = 0;
    struct __my_dev *p_my_dev = (struct __my_dev *)(pfile->private_data); /* 通过私有数据方式获取 p_gmyled_dev设备属性 */
    /* 1.写入数据 */
    bytes = copy_from_user(p_my_dev->buzzer_buf, pbuf, count);
    if (bytes > 0)
    {
        printk("[ERROR]copy_from_user failed!\n");
        return -1;
    }
    printk("copy_from_user pbuf:%c\n", p_my_dev->buzzer_buf[0]);
    return 0;
}

/**
 * @Function    : mybuzzer_ioctl
 * @brief       : 设备控制
 * @param pfile : struct file *  类型，指向open产生的struct file类型的对象，表示本次ioctl对应的那次open。
 *                                    (读写文件内容过程中用到的一些控制性数据组合而成的对象)
 * @param cmd   ：unsigned int  类型，用来表示做的是哪一个操作。
 * @param arg   ：unsigned long 类型，和cmd配合用的参数。
 * @return      : 成功返回0，失败返回-1
 * @Description : 函数名初始化给 struct file_operations 的成员 .unlocked_ioctl
 */
long mybuzzer_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
    /* 0.相关变量定义 */
	buzzer_desc_t *buzzer = (buzzer_desc_t *)arg;
    struct __my_dev *p_my_dev = (struct __my_dev *)(pfile->private_data); /* 通过私有数据方式获取 p_gmyled_dev设备属性 */
    /* 1.执行命令 */
    switch(cmd)  
    {  
        case BUZZER_ON:  
            buzzer_on(p_my_dev);  
            break;
        case BUZZER_OFF:  
            buzzer_off(p_my_dev);  
            break;
        case BUZZER_FREQ:  
            buzzer_freq(p_my_dev, buzzer->tcnt, buzzer->tcmp);  
            break;
        default: 
            printk("[WARN ]The cmd is unknown!\n"); 
            return -EINVAL;  
    }  
    return 0;
}

//=======================================================
/**
 * @Function       : buzzer_init
 * @brief          : 蜂鸣器初始化
 * @param p_my_dev : struct __my_dev * 类型
 * @return         : none
 * @Description    : 
 */
void buzzer_init(struct __my_dev *p_my_dev)
{
    /* 1.虚拟地址映射 */
    ioremap_buzzer_reg(p_my_dev);
    
    /* 2.设置GPD0_0的引脚功能 */
    writel((readl(p_my_dev->gpd0con) & ~(0xf << 0)) | (0x2 << 0), p_my_dev->gpd0con); /* GPD0_0 : 0x2表示 TOUT_0，用作PWM */
    /* 3.设置分频 */
    /* 3.1设置PWM0的一级分频, 一级分频倍数设置为250倍：
     *    一级分频: PWM.TCFG0 = 299 = 0xF9 ---> PCLK/(249 + 1) = 100MHZ/250 = 400 000Hz
     *    TCFG0[Prescaler 0(7:0)]  —— Prescaler 0 value for timer 0 and 1
     *    TCFG0[Prescaler 1(15:8)] —— Prescaler 1 value for Timer 2, 3, and 4
     */
    writel((readl(p_my_dev->pwm_base + TCFG0) & ~(0xff << 0)) | (0Xf9 << 0), p_my_dev->pwm_base + TCFG0); /* 设置默认值0XF9 = 249分频 */
    /* 3.2设置PWM0的二级分频，二级分频倍数设置为4倍，递减计数器递减频率 = PLCK / (249 + 1) / 4 = 100M / 1000 = 100 000Hz = 100KHz
     * TCFG1[Divider MUX0(3:0)] —— Selects Mux input for PWM timer 0
     */
    writel((readl(p_my_dev->pwm_base + TCFG1) & ~(0xf << 0)) | (0x2 << 0), p_my_dev->pwm_base + TCFG1);   /* 4分频  */

    /* 4.设置占空比 */
    /* 4.1设置PWM0的频率为 1000 HZ
     * TCNTB0[Timer 0 count buffer(31:0)] —— 就是每个周期开始递减的初始值
     * 递减计数器减一次所用时间为 1/100K s, 这里设置重载值为 2000 ，计数 100次所用时间为 100*(1/100000) = 1/1000 s
     * 故递减频率为 1/(1/1000) = 1000Hz = 1KHz
     */
    writel(100, p_my_dev->pwm_base + TCNTB0); /* 计数值 100次 */
    /* 4.2设置PWM0的占空比 
     * TCMPB0[Timer 0 compare buffer(31:0)] —— 就是每个周期中开始是高电平的计数值
     * 综述：占空比 = tcmp / tcnt 决定声音的大小
     */
    writel(80, p_my_dev->pwm_base + TCMPB0); /* 比较值 80次 */
    /* 5.设置PWM0为自动重装载，使其能够产生连续的脉冲信号
     * TCON[Timer 0 auto reload on/off(3)] —— 设置PWM0计数递减到0后自动重装起始值
     */
    writel(readl(p_my_dev->pwm_base + TCON) | (0x1 << 3), p_my_dev->pwm_base + TCON);  /* 设置自动重装载 */
    /* 6.将TCNTB0中的值手动装载到递减计数器(主要是完成第一个周期的开启)
     * TCMPB0[Timer 0 manual update(1)] ——  Updates TCNTB0 andTCMPB0
     */
    writel(readl(p_my_dev->pwm_base + TCON) | (0x1 << 1), p_my_dev->pwm_base + TCON);  /* 设置手动重装 */
    /* 7.关闭手动更新，后续进行自动装载 (主要是完成第一个周期的开启)
     * TCMPB0[Timer 0 manual update(1)] ——  Updates TCNTB0 andTCMPB0
     */
    writel(readl(p_my_dev->pwm_base + TCON) & (~(1 << 1)), p_my_dev->pwm_base + TCON); /* 清除手动装载 */
}

/**
 * @Function       : buzzer_on
 * @brief          : 打开指定buzzer
 * @param p_my_dev : struct __my_dev * 类型
 * @return         : none
 * @Description    :
 */
void buzzer_on(struct __my_dev *p_my_dev)
{
    /* 1.使能/关闭PWM0，递减计数器开始递减
     * TCON[Timer 0 start/stop(0)] —— 使能或者关闭 PWM0
     */
    writel(readl(p_my_dev->pwm_base + TCON) | (0x1 << 0), p_my_dev->pwm_base + TCON);
}
/**
 * @Function       : buzzer_off
 * @brief          : 关闭buzzer
 * @param p_my_dev : struct __my_dev * 类型
 * @return         : none
 * @Description    : 
 */
void buzzer_off(struct __my_dev *p_my_dev)
{
    /* 1.使能/关闭PWM0，递减计数器开始递减
     * TCON[Timer 0 start/stop(0)] —— 使能或者关闭 PWM0
     */
    writel(readl(p_my_dev->pwm_base + TCON) & (~(1 << 0)), p_my_dev->pwm_base + TCON);
}
/**
 * @Function          : buzzer_freq
 * @brief             : 设置蜂鸣器的占空比
 * @param buzzer_tcnt : int类型，决定了周期
 * @param buzzer_tcmp : int类型，决定了占空比
 * @return            : none
 * @Description       : 
 */
static void buzzer_freq(struct __my_dev *p_my_dev, int buzzer_tcnt, int buzzer_tcmp)
{
     /* 1.设置PWM0的频率，tcnt决定了周期
      * TCNTB0[Timer 0 count buffer(31:0)] —— 就是每个周期开始递减的初始值
      */
    writel(buzzer_tcnt, p_my_dev->pwm_base + TCNTB0); /* 计数值 buzzer_tcnt 次 */
    /* 2.设置PWM0的占空比 
     * TCMPB0[Timer 0 compare buffer(31:0)] —— 就是每个周期中开始是高电平的计数值
     * 占空比 = tcmp / tcnt 决定声音的大小
     */
    writel(buzzer_tcmp, p_my_dev->pwm_base + TCMPB0); /* 比较值 buzzer_tcmp 次 */
}

/**
 * @Function       : ioremap_buzzer_reg
 * @brief          : buzzer寄存器地址映射
 * @param p_my_dev : struct __my_dev * 类型
 * @return         : none
 * @Description    :
 */
void ioremap_buzzer_reg(struct __my_dev *p_my_dev)
{
     /* 1.虚拟地址映射 */
    p_my_dev->gpd0con = ioremap(GPD0CON, 4);
    p_my_dev->pwm_base = ioremap(PWM_BASE, 4);
}

/**
 * @Function       : iounmap_buzzer_reg
 * @brief          : 取消buzzer寄存器地址映射
 * @param p_my_dev : struct __my_dev * 类型
 * @return         : none
 * @Description    :
 */
void iounmap_buzzer_reg(struct __my_dev *p_my_dev)
{
    /* 1.取消地址映射 */
    iounmap(p_my_dev->gpd0con);
	iounmap(p_my_dev->pwm_base);
}

//=======================================================
/* 驱动入口函数 */
int __init buzzer_dev_init(void)
{
    /* 1.申请设备的内存空间 */
    p_gmybuzzer_dev = (struct __my_dev *)kmalloc(sizeof(struct __my_dev), GFP_KERNEL); /* 为设备属性结构体分配内存空间 */
    if (p_gmybuzzer_dev == NULL)
    {
        printk("[ERROR]p_gmybuzzer_dev kmalloc failed!\n");
        return -1;
    }
    memset(p_gmybuzzer_dev, 0, sizeof(struct __my_dev)); /* 申请的内存空间清0 */
    /* 2.分配设备号 */
    p_gmybuzzer_dev->major = 1;                                               /* 手动指定主设备号 */
    p_gmybuzzer_dev->minor = 0;                                               /* 大部分驱动次设备号都选择0 */
    p_gmybuzzer_dev->dev_num = 1;                                             /* 申请的设备数量 */
    p_gmybuzzer_dev->devno = MKDEV(p_gmybuzzer_dev->major, p_gmybuzzer_dev->minor); /* MKDEV宏用来将主设备号和次设备号组合成32位完整的设备号 */
    if (register_chrdev_region(p_gmybuzzer_dev->devno, p_gmybuzzer_dev->dev_num, MODULE_NAME))
    {
        /* 手动指定设备号分配失败，申请自动分配设备号 */
        if (alloc_chrdev_region(&p_gmybuzzer_dev->devno, 0, p_gmybuzzer_dev->dev_num, MODULE_NAME))
        {
            printk("[ERROR]get devno failed!\n");
            return -1;
        }
        /* 重新获取主设备号和次设备号 */
        p_gmybuzzer_dev->major = MAJOR(p_gmybuzzer_dev->devno); /* 自动分配设备号后，分离出主设备号 */
        p_gmybuzzer_dev->minor = MINOR(p_gmybuzzer_dev->minor); /* 自动分配设备号后，分离出次设备号 */
    }
    /* 3.向Linux内核注册新的字符设备 */
    /* 3.1初始化 mybuzzer 对象, 给该对象添加操作函数集 */
    cdev_init(&p_gmybuzzer_dev->mybuzzer, &myops);
    /* 3.2向 Linux 系统添加字符设备 (cdev结构体变量,将会被添加到一个hash链表中) */
    p_gmybuzzer_dev->mybuzzer.owner = THIS_MODULE;
    cdev_add(&p_gmybuzzer_dev->mybuzzer, p_gmybuzzer_dev->devno, p_gmybuzzer_dev->dev_num); /* 会在 /proc/devices 中创建设备号和对应的名称记录 */
    /* 4.自动创建设备节点(省去 mknod 命令在 /dev/ 下创建设备节点步骤) */
    /* 4.1创建类 */
    p_gmybuzzer_dev->class = class_create(THIS_MODULE, MODULE_NAME);
    if (IS_ERR(p_gmybuzzer_dev->class))
    {
        printk("[ERROR]class_create failed!\n");
        cdev_del(&p_gmybuzzer_dev->mybuzzer);                                       /* 删除字符设备对象 cdev */
        unregister_chrdev_region(p_gmybuzzer_dev->devno, p_gmybuzzer_dev->dev_num); /* 注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
        return PTR_ERR(p_gmybuzzer_dev->class);
    }
    /* 4.2创建设备(创建的设备为 /dev/dev_name) */
    p_gmybuzzer_dev->device = device_create(p_gmybuzzer_dev->class, NULL, p_gmybuzzer_dev->devno, NULL, "%s%d", MODULE_NAME, p_gmybuzzer_dev->minor);
    if (IS_ERR(p_gmybuzzer_dev->device))
    {
        printk("[ERROR]device create failed!\n");
        class_destroy(p_gmybuzzer_dev->class);                                   /* 删除类 */
        cdev_del(&p_gmybuzzer_dev->mybuzzer);                                       /* 删除字符设备对象 cdev */
        unregister_chrdev_region(p_gmybuzzer_dev->devno, p_gmybuzzer_dev->dev_num); /* 注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
        return PTR_ERR(p_gmybuzzer_dev->device);
    }
    /* 5.buzzer初始化 */
    buzzer_init(p_gmybuzzer_dev);
    buzzer_off(p_gmybuzzer_dev);
    /* 打印提示信息 */
    printk("=======================================\n");
    printk("[INFO ]%s(%d):major=%d,minor=%d\n", MODULE_NAME, p_gmybuzzer_dev->devno, p_gmybuzzer_dev->major, p_gmybuzzer_dev->minor);
    printk("[INFO ]"BLUE"%s%d"CLS" has been created!\n", MODULE_NAME, p_gmybuzzer_dev->minor);
    printk("=======================================\n");
    return 0;
}

/* 驱动出口函数 */
void __exit buzzer_dev_exit(void)
{
    /* 1.重新获取设备号(有可能是动态分配了的设备号) */
    p_gmybuzzer_dev->devno = MKDEV(p_gmybuzzer_dev->major, p_gmybuzzer_dev->minor); /* MKDEV宏用来将主设备号和次设备号组合成32位完整的设备号 */
    /* 2.buzzer设备寄存器设置 */
    /* 2.1取消寄存器映射 iounmap */
    iounmap_buzzer_reg(p_gmybuzzer_dev);
    /* 3.删除设备节点 */
    /* 3.1删除设备(/dev/dev_name 将会被删除) */
    device_destroy(p_gmybuzzer_dev->class, p_gmybuzzer_dev->devno);
    /* 3.2删除类 */
    class_destroy(p_gmybuzzer_dev->class);
    /* 4.删除字符设备对象 cdev */
    cdev_del(&p_gmybuzzer_dev->mybuzzer);
    /* 5.注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
    unregister_chrdev_region(p_gmybuzzer_dev->devno, p_gmybuzzer_dev->dev_num);
    /* 6.释放字符设备的存储空间 */
    kfree(p_gmybuzzer_dev);
    p_gmybuzzer_dev = NULL;
    /* 7.打印卸载模块的提示信息 */
    printk("[INFO ]This module is exited!\n");
}

/* 将上面两个函数指定为驱动的入口和出口函数 */
module_init(buzzer_dev_init);
module_exit(buzzer_dev_exit);

/* 模块信息(通过 modinfo buzzer_dev 查看) */
MODULE_LICENSE("GPL");               /* 源码的许可证协议 */
MODULE_AUTHOR("qidaink");            /* 字符串常量内容为模块作者说明 */
MODULE_DESCRIPTION("Description");   /* 字符串常量内容为模块功能说明 */
MODULE_ALIAS("module's other name"); /* 字符串常量内容为模块别名 */
