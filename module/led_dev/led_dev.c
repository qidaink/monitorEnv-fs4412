/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : led_dev.c
 * Author     : qidaink
 * Date       : 2022-08-28
 * Version    :
 * Description: LED驱动模块
 * Others     :
 * Log        :
 * ======================================================
 */
//=======================================================
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

#include "led_dev.h"
//=======================================================
#define MODULE_NAME "led_dev"    /* 设备名称： /proc/devices 文件中与该设备对应的名字，方便用户层查询主设备号 */
#define BUF_LEN 100              /* 数据读写缓冲区大小 */

/** LED寄存器地址宏定义
 * LED2 -- GPX2_7   GPX2CON[31:28] -- 0x11000C40   GPX2DAT[7] -- 0x11000C44
 * LED3 -- GPX1_0   GPX1CON[3:0]   -- 0x11000C20   GPX1DAT[0] -- 0x11000C24
 * LED4 -- GPF3_4   GPF3CON[19:16] -- 0x114001E0   GPF3DAT[4] -- 0x114001E4
 * LED4 -- GPF3_5   GPF3CON[23:20] -- 0x114001E0   GPF3DAT[5] -- 0x114001E4
 */
#define GPX1CON (0x11000C20)
#define GPX1DAT (0x11000C24)

#define GPX2CON (0x11000C40)
#define GPX2DAT (0x11000C44)

#define GPF3CON (0x114001E0)
#define GPF3DAT (0x114001E4)

/* 字符设备的属性 */
struct __my_dev
{
    /* 1.设备号与设备数量 */
    int major;   /* 主设备号：占高12位，用来表示驱动程序相同的一类设备，Linux系统中主设备号范围为 0~4095 */
    int minor;   /* 次设备号：占低20位，用来表示被操作的哪个具体设备 */
    dev_t devno; /* 新字符设备的设备号：由主设备号和次设备号组成     */
    int dev_num; /* 申请的设备数量 */
    /* 2.字符设备对象 */
    struct cdev myled; /* 定义一个字符设备对象设备 */
    /* 3.自动创建设备节点相关成员变量 */
    struct class *class;   /* 类 */
    struct device *device; /* 设备 /dev/dev_name */
    /* 4.LED寄存器虚拟地址 */
    volatile unsigned long *pled2_con;
    volatile unsigned long *pled2_dat;

    volatile unsigned long *pled3_con;
    volatile unsigned long *pled3_dat;

    volatile unsigned long *pled4_con;
    volatile unsigned long *pled4_dat;

    volatile unsigned long *pled5_con;
    volatile unsigned long *pled5_dat;
};
struct __my_dev *p_gmyled_dev; /* 定义一个字符设备指针变量 */

/* 新字符设备操作函数声明 */
int myled_open(struct inode *pnode, struct file *pfile);                   /* 打开设备 */
int myled_close(struct inode *pnode, struct file *pfile);                  /* 关闭设备 */
void led_on(struct __my_dev *p_my_dev, int ledno);                         /* 打开LED */
void led_off(struct __my_dev *p_my_dev, int ledno);                        /* 关闭LED */
long myled_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg); /* 设备控制 */
void ioremap_ledreg(struct __my_dev *p_my_dev);                            /* LED寄存器映射 */
void set_output_ledconreg(struct __my_dev *p_my_dev);                      /* LED寄存器设置为输出 */
void iounmap_ledreg(struct __my_dev *p_my_dev);                            /* 取消LED寄存器映射 */

/* 操作函数集定义 */
struct file_operations myops = {
    /* data */
    .owner = THIS_MODULE,
    .open = myled_open,
    .release = myled_close,
    .unlocked_ioctl = myled_ioctl,
};

/**
 * @Function    : myled_open
 * @brief       : 打开设备
 * @param pnode : struct inode * 类型，内核中记录文件元信息的结构体
 * @param pfile : struct file *  类型，读写文件内容过程中用到的一些控制性数据组合而成的对象
 * @return      : 0,操作完成
 * @Description : 函数名初始化给 struct file_operations 的成员 .open
 */
int myled_open(struct inode *pnode, struct file *pfile)
{
    /* 1.获取 struct __my_dev p_gmyled_dev 地址 */
    pfile->private_data = (void *)(container_of(pnode->i_cdev, struct __my_dev, myled));
    printk("[INFO ]myled_open is called!\n");
    return 0;
}

/**
 * @Function    : myled_close
 * @brief       : 关闭设备
 * @param pnode : struct inode * 类型，内核中记录文件元信息的结构体
 * @param pfile : struct file *  类型，读写文件内容过程中用到的一些控制性数据组合而成的对象
 * @return      : 0,操作完成
 * @Description : 函数名初始化给 struct file_operations 的成员 .close
 */
int myled_close(struct inode *pnode, struct file *pfile)
{
    printk("[INFO ]myled_close is called!\n");
    return 0;
}
//=======================================================
/**
 * @Function       : led_on
 * @brief          : 打开指定LED
 * @param p_my_dev : struct __my_dev * 类型
 * @param ledno    : int 类型, LED灯序号(2,3,4,5)
 * @return         : none
 * @Description    :
 */
void led_on(struct __my_dev *p_my_dev, int ledno)
{
    switch (ledno)
    {
        case 2:
            writel(readl(p_my_dev->pled2_dat) | (0x1 << 7), p_my_dev->pled2_dat);
            break;
        case 3:
            writel(readl(p_my_dev->pled3_dat) | (0x1 << 0), p_my_dev->pled3_dat);
            break;
        case 4:
            writel(readl(p_my_dev->pled4_dat) | (0x1 << 4), p_my_dev->pled4_dat);
            break;
        case 5:
            writel(readl(p_my_dev->pled5_dat) | (0x1 << 5), p_my_dev->pled5_dat);
            break;
        default:
            break;
    }
}
/**
 * @Function   : led_off
 * @brief      : 关闭指定LED
 * @param p_my_dev : struct __my_dev * 类型
 * @param ledno    : int 类型, LED灯序号(2,3,4,5)
 * @return     : none
 * @Description: 
 */
void led_off(struct __my_dev *p_my_dev, int ledno)
{
    switch (ledno)
    {
        case 2:
            writel(readl(p_my_dev->pled2_dat) & (~(0x1 << 7)), p_my_dev->pled2_dat);
            break;
        case 3:
            writel(readl(p_my_dev->pled3_dat) & (~(0x1 << 0)), p_my_dev->pled3_dat);
            break;
        case 4:
            writel(readl(p_my_dev->pled4_dat) & (~(0x1 << 4)), p_my_dev->pled4_dat);
            break;
        case 5:
            writel(readl(p_my_dev->pled5_dat) & (~(0x1 << 5)), p_my_dev->pled5_dat);
            break;
        default:
            break;
    }
}

/**
 * @Function    : myled_ioctl
 * @brief       : 设备控制
 * @param pfile : struct file *  类型，指向open产生的struct file类型的对象，表示本次ioctl对应的那次open。
 *                                    (读写文件内容过程中用到的一些控制性数据组合而成的对象)
 * @param cmd   ：unsigned int  类型，用来表示做的是哪一个操作。
 * @param arg   ：unsigned long 类型，和cmd配合用的参数。
 * @return      : 成功返回0，失败返回-1
 * @Description : 函数名初始化给 struct file_operations 的成员 .unlocked_ioctl
 */
long myled_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
    struct __my_dev *p_my_dev = (struct __my_dev *)(pfile->private_data); /* 通过私有数据方式获取 p_gmyled_dev设备属性 */
    /* 1.判断参数是否合法 */
    if (arg < 2 || arg > 5)
    {
        printk("[ERROR]myled_ioctl arg(2-5) is invalid!\n");
        return -1;
    }
    /* 2.执行命令 */
    switch (cmd)
    {
        case LED_DEV_ON:
            led_on(p_my_dev, arg);
            break;
        case LED_DEV_OFF:
            led_off(p_my_dev, arg);
            break;
        default:
            printk("[WARN ]The cmd is unknown!\n");
            return -1;
    }
    return 0;
}

/**
 * @Function       : ioremap_ledreg
 * @brief          : LED寄存器地址映射
 * @param p_my_dev : struct __my_dev * 类型
 * @return         : none
 * @Description    :
 */
void ioremap_ledreg(struct __my_dev *p_my_dev)
{
    /* LED2 寄存器映射 */
    p_my_dev->pled2_con = ioremap(GPX2CON, 4); /* 4 表示要映射的内存空间大小 */
    p_my_dev->pled2_dat = ioremap(GPX2DAT, 4);
    /* LED3 寄存器映射 */
    p_my_dev->pled3_con = ioremap(GPX1CON, 4);
    p_my_dev->pled3_dat = ioremap(GPX1DAT, 4);
    /* LED4 寄存器映射 */
    p_my_dev->pled4_con = ioremap(GPF3CON, 4);
    p_my_dev->pled4_dat = ioremap(GPF3DAT, 4);
    /* LED5 寄存器映射 */
    p_my_dev->pled5_con = p_my_dev->pled4_con;
    p_my_dev->pled5_dat = p_my_dev->pled4_dat;
}

/**
 * @Function       : set_output_ledconreg
 * @brief          : LED寄存器设置为输出
 * @param p_my_dev : struct __my_dev * 类型
 * @return         : none
 * @Description    :
 */
void set_output_ledconreg(struct __my_dev *p_my_dev)
{
    /* 1.设置IO模式，0001表示输出 */
    writel((readl(p_my_dev->pled2_con) & (~(0xF << 28))) | (0x1 << 28), p_my_dev->pled2_con);
    writel((readl(p_my_dev->pled3_con) & (~(0xF << 0))) | (0x1 << 0), p_my_dev->pled3_con);
    writel((readl(p_my_dev->pled4_con) & (~(0xF << 16))) | (0x1 << 16), p_my_dev->pled4_con);
    writel((readl(p_my_dev->pled5_con) & (~(0xF << 20))) | (0x1 << 20), p_my_dev->pled5_con);
    /* 2.设置初始IO电平 */
    writel(readl(p_my_dev->pled2_dat) & (~(0x1 << 7)), p_my_dev->pled2_dat);
    writel(readl(p_my_dev->pled3_dat) & (~(0x1 << 0)), p_my_dev->pled3_dat);
    writel(readl(p_my_dev->pled4_dat) & (~(0x1 << 4)), p_my_dev->pled4_dat);
    writel(readl(p_my_dev->pled5_dat) & (~(0x1 << 5)), p_my_dev->pled5_dat);
}

/**
 * @Function       : iounmap_ledreg
 * @brief          : 取消LED寄存器地址映射
 * @param p_my_dev : struct __my_dev * 类型
 * @return         : none
 * @Description    :
 */
void iounmap_ledreg(struct __my_dev *p_my_dev)
{
    /* LED2 */
    iounmap(p_my_dev->pled2_con);
    p_my_dev->pled2_con = NULL;
    iounmap(p_my_dev->pled2_dat);
    p_my_dev->pled2_dat = NULL;
    /* LED3 */
    iounmap(p_my_dev->pled3_con);
    p_my_dev->pled3_con = NULL;
    iounmap(p_my_dev->pled3_dat);
    p_my_dev->pled3_dat = NULL;
    /* LED4 */
    iounmap(p_my_dev->pled4_con);
    p_my_dev->pled4_con = NULL;
    iounmap(p_my_dev->pled4_dat);
    p_my_dev->pled4_dat = NULL;
    /* LED5 */
    p_my_dev->pled5_con = NULL;
    p_my_dev->pled5_dat = NULL;
}

//=======================================================
/* 驱动入口函数 */
int __init led_dev_init(void)
{
    /* 1.申请设备的内存空间 */
    p_gmyled_dev = (struct __my_dev *)kmalloc(sizeof(struct __my_dev), GFP_KERNEL); /* 为设备属性结构体分配内存空间 */
    if (p_gmyled_dev == NULL)
    {
        printk("[ERROR]p_gmyled_dev kmalloc failed!\n");
        return -1;
    }
    memset(p_gmyled_dev, 0, sizeof(struct __my_dev)); /* 申请的内存空间清0 */
    /* 2.分配设备号 */
    p_gmyled_dev->major = 200;                                             /* 手动指定主设备号 */
    p_gmyled_dev->minor = 0;                                               /* 大部分驱动次设备号都选择0 */
    p_gmyled_dev->dev_num = 1;                                             /* 申请的设备数量 */
    p_gmyled_dev->devno = MKDEV(p_gmyled_dev->major, p_gmyled_dev->minor); /* MKDEV宏用来将主设备号和次设备号组合成32位完整的设备号 */
    if (register_chrdev_region(p_gmyled_dev->devno, p_gmyled_dev->dev_num, MODULE_NAME))
    {
        /* 手动指定设备号分配失败，申请自动分配设备号 */
        if (alloc_chrdev_region(&p_gmyled_dev->devno, 0, p_gmyled_dev->dev_num, MODULE_NAME))
        {
            printk("[ERROR]get devno failed!\n");
            return -1;
        }
        /* 重新获取主设备号和次设备号 */
        p_gmyled_dev->major = MAJOR(p_gmyled_dev->devno); /* 自动分配设备号后，分离出主设备号 */
        p_gmyled_dev->minor = MINOR(p_gmyled_dev->minor); /* 自动分配设备号后，分离出次设备号 */
    }
    /* 3.向Linux内核注册新的字符设备 */
    /* 3.1初始化 myled 对象, 给该对象添加操作函数集 */
    cdev_init(&p_gmyled_dev->myled, &myops);
    /* 3.2向 Linux 系统添加字符设备 (cdev结构体变量,将会被添加到一个hash链表中) */
    p_gmyled_dev->myled.owner = THIS_MODULE;
    cdev_add(&p_gmyled_dev->myled, p_gmyled_dev->devno, p_gmyled_dev->dev_num); /* 会在 /proc/devices 中创建设备号和对应的名称记录 */
    /* 4.自动创建设备节点(省去 mknod 命令在 /dev/ 下创建设备节点步骤) */
    /* 4.1创建类 */
    p_gmyled_dev->class = class_create(THIS_MODULE, MODULE_NAME);
    if (IS_ERR(p_gmyled_dev->class))
    {
        printk("[ERROR]class_create failed!\n");
        cdev_del(&p_gmyled_dev->myled);                                       /* 删除字符设备对象 cdev */
        unregister_chrdev_region(p_gmyled_dev->devno, p_gmyled_dev->dev_num); /* 注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
        return PTR_ERR(p_gmyled_dev->class);
    }
    /* 4.2创建设备(创建的设备为 /dev/dev_name) */
    p_gmyled_dev->device = device_create(p_gmyled_dev->class, NULL, p_gmyled_dev->devno, NULL, "%s%d", MODULE_NAME, p_gmyled_dev->minor);
    if (IS_ERR(p_gmyled_dev->device))
    {
        printk("[ERROR]device create failed!\n");
        class_destroy(p_gmyled_dev->class);                                   /* 删除类 */
        cdev_del(&p_gmyled_dev->myled);                                       /* 删除字符设备对象 cdev */
        unregister_chrdev_region(p_gmyled_dev->devno, p_gmyled_dev->dev_num); /* 注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
        return PTR_ERR(p_gmyled_dev->device);
    }
    /* 5.LED寄存器相关配置 */
    /* 5.1寄存器地址映射 ioremap */
    ioremap_ledreg(p_gmyled_dev);
    /* 5.2控制寄存器设置为输出 con-register set output */
    set_output_ledconreg(p_gmyled_dev);
    /* 打印提示信息 */
    printk("=======================================\n");
    printk("[INFO ]%s(%d):major=%d,minor=%d\n", MODULE_NAME, p_gmyled_dev->devno, p_gmyled_dev->major, p_gmyled_dev->minor);
    printk("[INFO ]"BLUE"%s%d"CLS" has been created!\n", MODULE_NAME, p_gmyled_dev->minor);
    printk("=======================================\n");
    return 0;
}

/* 驱动出口函数 */
void __exit led_dev_exit(void)
{
    /* 1.重新获取设备号(有可能是动态分配了的设备号) */
    p_gmyled_dev->devno = MKDEV(p_gmyled_dev->major, p_gmyled_dev->minor); /* MKDEV宏用来将主设备号和次设备号组合成32位完整的设备号 */
    /* 2.LED设备寄存器设置 */
    /* 2.1取消寄存器映射 iounmap */
    iounmap_ledreg(p_gmyled_dev);
    /* 3.删除设备节点 */
    /* 3.1删除设备(/dev/dev_name 将会被删除) */
    device_destroy(p_gmyled_dev->class, p_gmyled_dev->devno);
    /* 3.2删除类 */
    class_destroy(p_gmyled_dev->class);
    /* 4.删除字符设备对象 cdev */
    cdev_del(&p_gmyled_dev->myled);
    /* 5.注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
    unregister_chrdev_region(p_gmyled_dev->devno, p_gmyled_dev->dev_num);
    /* 6.释放字符设备的存储空间 */
    kfree(p_gmyled_dev);
    p_gmyled_dev = NULL;
    /* 7.打印卸载模块的提示信息 */
    printk("[INFO ]This module is exited!\n");
}

/* 将上面两个函数指定为驱动的入口和出口函数 */
module_init(led_dev_init);
module_exit(led_dev_exit);

/* 模块信息(通过 modinfo led_dev 查看) */
MODULE_LICENSE("GPL");               /* 源码的许可证协议 */
MODULE_AUTHOR("qidaink");            /* 字符串常量内容为模块作者说明 */
MODULE_DESCRIPTION("Description");   /* 字符串常量内容为模块功能说明 */
MODULE_ALIAS("module's other name"); /* 字符串常量内容为模块别名 */
