/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : mpu6050_driver.c
 * Author     : qidaink
 * Date       : 2022-08-29
 * Version    :
 * Description:  MPU6050 驱动模块
 * Others     :
 * Log        :
 * ======================================================
 */

/** 头文件 */
#include <linux/module.h> /* MODULE_LICENSE */
#include <linux/kernel.h>
#include <linux/init.h>            /* module_init module_exit */
#include <linux/platform_device.h> /* platform_get_resource */
#include <linux/export.h>          /* THIS_MODULE */
#include <linux/device.h>          /* class_create class_destroy device_create device_destroy */
#include <linux/fs.h>              /* register_chrdev_region alloc_chrdev_region unregister_chrdev_region */
#include <linux/cdev.h>            /* cdev_init cdev_add cdev_del */
#include <linux/io.h>              /* writel readl */
#include <linux/uaccess.h>         /* copy_from_user copy_to_user */
#include <linux/i2c.h>             /* i2c_transfer */

#include <linux/slab.h>
#include <linux/mm.h>

#include "mpu6050_dev.h"
//=======================================================//
/** 宏定义 */
#define MODULE_NAME "mpu6050" /* 模块名称 */

/* 宏定义 */
#define	SMPLRT_DIV	  0x19	/* 陀螺仪采样率，典型值：0x07(125Hz) */
#define	CONFIG		  0x1A	/* 低通滤波频率，典型值：0x06(5Hz) */
#define	GYRO_CONFIG	  0x1B	/* 陀螺仪自检及测量范围，典型值：0xF8(不自检，+/-2000deg/s) */
#define	ACCEL_CONFIG  0x1C	/* 加速计自检、测量范围及高通滤波频率，加速计自检、测量范围，典型值：0x19(不自检，+/-G) */

#define ACCEL_XOUT_H  0x3B
#define ACCEL_XOUT_L  0x3C
#define ACCEL_YOUT_H  0x3D
#define ACCEL_YOUT_L  0x3E
#define ACCEL_ZOUT_H  0x3F
#define ACCEL_ZOUT_L  0x40
#define TEMP_OUT_H    0x41
#define TEMP_OUT_L    0x42
#define GYRO_XOUT_H   0x43
#define GYRO_XOUT_L   0x44
#define GYRO_YOUT_H   0x45
#define GYRO_YOUT_L   0x46
#define GYRO_ZOUT_H   0x47
#define GYRO_ZOUT_L   0x48
#define SLAVE_ADDRESS 0x68
#define PWR_MGMT_1    0x6B    /* 电源管理，典型值：0x00(正常启用) */

//=======================================================//
/** 设备相关 */
/* 字符设备的属性 */
struct __my_dev
{
    /* 1.设备号与设备数量 */
    int major;   /* 主设备号：占高12位，用来表示驱动程序相同的一类设备，Linux系统中主设备号范围为 0~4095 */
    int minor;   /* 次设备号：占低20位，用来表示被操作的哪个具体设备 */
    dev_t devno; /* 新字符设备的设备号：由主设备号和次设备号组成     */
    int dev_num; /* 申请的设备数量 */
    /* 2.字符设备对象 */
    struct cdev mympu6050; /* 定义一个字符设备对象设备 */
    /* 3.自动创建设备节点相关成员变量 */
    struct class *class;   /* 类 */
    struct device *device; /* 设备 /dev/dev_name */
    /* 4.I2C用 */
    struct i2c_client *pclt;
};
struct __my_dev *p_mpu6050_dev = NULL; /* 定义一个字符设备指针变量 */

/* 设备操作函数声明 */
int mympu6050_open(struct inode *pnode, struct file *pfile);                   /* 打开设备 */
int mympu6050_close(struct inode *pnode, struct file *pfile);                  /* 关闭设备 */
long mympu6050_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg); /* 设备控制 */

int mympu6050_read_byte(struct i2c_client *pclt, unsigned char reg);                     /* 从mpu6050指定寄存器读取一个字节数据 */
int mympu6050_write_byte(struct i2c_client *pclt, unsigned char reg, unsigned char val); /* 向mpu6050指定寄存器写入一个字节数据 */
void mympu6050_init(struct i2c_client *pclt);                                            /* mpu6050初始化 */

/* 操作函数集定义 */
struct file_operations mympu6050_ops = {
    /* data */
    .owner = THIS_MODULE,
    .open = mympu6050_open,
    .release = mympu6050_close,
    .unlocked_ioctl = mympu6050_ioctl,
};

/**
 * @Function    : mympu6050_open
 * @brief       : 打开设备
 * @param pnode : struct inode * 类型，内核中记录文件元信息的结构体
 * @param pfile : struct file *  类型，读写文件内容过程中用到的一些控制性数据组合而成的对象
 * @return      : 0,操作完成
 * @Description : 函数名初始化给 struct file_operations 的成员 .open
 */
int mympu6050_open(struct inode *pnode, struct file *pfile)
{
    /* 获取 struct __my_dev p_mpu6050_dev 地址 */
    pfile->private_data = (void *)(container_of(pnode->i_cdev, struct __my_dev, mympu6050));
    printk("[INFO]mympu6050_open is called!\n");
    return 0;
}

/**
 * @Function    : mympu6050_close
 * @brief       : 关闭设备
 * @param pnode : struct inode * 类型，内核中记录文件元信息的结构体
 * @param pfile : struct file *  类型，读写文件内容过程中用到的一些控制性数据组合而成的对象
 * @return      : 0,操作完成
 * @Description : 函数名初始化给 struct file_operations 的成员 .close
 */
int mympu6050_close(struct inode *pnode, struct file *pfile)
{
    printk("[INFO]mympu6050_close is called!\n");
    return 0;
}

/**
 * @Function    : mympu6050_ioctl
 * @brief       : 设备控制
 * @param pfile : struct file *  类型，指向open产生的struct file类型的对象，表示本次ioctl对应的那次open。
 *                                    (读写文件内容过程中用到的一些控制性数据组合而成的对象)
 * @param cmd   ：unsigned int  类型，用来表示做的是哪一个操作。
 * @param arg   ：unsigned long 类型，和cmd配合用的参数。
 * @return      : 成功返回0，失败返回-1
 * @Description : 函数名初始化给 struct file_operations 的成员 .unlocked_ioctl
 */
long mympu6050_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
    /* 0.相关变量定义 */
    struct __my_dev *p_my_dev = (struct __my_dev *)(pfile->private_data); /* 通过私有数据方式获取 p_mpu6050_dev设备属性 */
    union mpu6050_data data; /* 存储从mpu6050获取的数据 */
    /* 1.判断参数是否合法 */
    switch (cmd)
    {
        case GET_ACCEL:
            data.accel.x = mympu6050_read_byte(p_my_dev->pclt, ACCEL_XOUT_L);
            data.accel.x |= mympu6050_read_byte(p_my_dev->pclt, ACCEL_XOUT_H) << 8;

            data.accel.y = mympu6050_read_byte(p_my_dev->pclt, ACCEL_YOUT_L);
            data.accel.y |= mympu6050_read_byte(p_my_dev->pclt, ACCEL_YOUT_H) << 8;

            data.accel.z = mympu6050_read_byte(p_my_dev->pclt, ACCEL_ZOUT_L);
            data.accel.z |= mympu6050_read_byte(p_my_dev->pclt, ACCEL_ZOUT_H) << 8;
            break;
        case GET_GYRO:
            data.gyro.x = mympu6050_read_byte(p_my_dev->pclt, GYRO_XOUT_L);
            data.gyro.x |= mympu6050_read_byte(p_my_dev->pclt, GYRO_XOUT_H) << 8;

            data.gyro.y = mympu6050_read_byte(p_my_dev->pclt, GYRO_YOUT_L);
            data.gyro.y |= mympu6050_read_byte(p_my_dev->pclt, GYRO_YOUT_H) << 8;

            data.gyro.z = mympu6050_read_byte(p_my_dev->pclt, GYRO_ZOUT_L);
            data.gyro.z |= mympu6050_read_byte(p_my_dev->pclt, GYRO_ZOUT_H) << 8;
            break;
        case GET_TEMP:
            data.temp = mympu6050_read_byte(p_my_dev->pclt, TEMP_OUT_L);
            data.temp |= mympu6050_read_byte(p_my_dev->pclt, TEMP_OUT_H) << 8;
            break;
        default:
            return -EINVAL;
    }

    if (copy_to_user((void *)arg, &data, sizeof(data)))
    {
        return -EFAULT;
    }

    return sizeof(data);
}

/**
 * @Function   : mympu6050_read_byte
 * @brief      : 从mpu6050指定寄存器读取一个字节数据
 * @param pclt : struct i2c_client * 类型
 * @param reg  : unsigned char 类型，要读的寄存器
 * @return     : int 类型，表示读取到的一个字节数据
 * @Description: 
 */
int mympu6050_read_byte(struct i2c_client *pclt, unsigned char reg)
{
    /* 0.相关变量定义 */
    int ret = 0;
    char txbuf[1] = {reg};
    char rxbuf[1] = {0};
    /* 1.定义发送数据用的变量和接收数据用的变量并初始化 */
    struct i2c_msg msg[2] = { /* 该结构体定义在 linux/i2c.h */
        {pclt->addr, 0, 1, txbuf},
        {pclt->addr, I2C_M_RD, 1, rxbuf},
    };
    /* 2.读取相应寄存器数据 */
    ret = i2c_transfer(pclt->adapter, msg, ARRAY_SIZE(msg));
    if (ret < 0)
    {
        printk("ret = %d,in mpu6050_read_byte\n", ret);
        return ret;
    }

    return rxbuf[0];
}

/**
 * @Function   : mympu6050_write_byte
 * @brief      : 向mpu6050指定寄存器写入一个字节数据
 * @param pclt : struct i2c_client * 类型
 * @param reg  : unsigned char 类型，要写的寄存器
 * @param val  : unsigned char 类型，要写的数据
 * @return     : int 类型
 * @Description: 
 */
int mympu6050_write_byte(struct i2c_client *pclt, unsigned char reg, unsigned char val)
{
    /* 0.相关变量定义 */
    int ret = 0;
    char txbuf[2] = {reg, val};
     /* 1.定义发送数据用的变量并初始化 */
    struct i2c_msg msg[1] = {
        {pclt->addr, 0, 2, txbuf},
    };
    /* 2.写入数据 */
    ret = i2c_transfer(pclt->adapter, msg, ARRAY_SIZE(msg));
    if (ret < 0)
    {
        printk("ret = %d,in mpu6050_write_byte\n", ret);
        return ret;
    }

    return 0;
}

/**
 * @Function   : mympu6050_init
 * @brief      : mpu6050初始化
 * @param pclt : struct i2c_client * 类型
 * @return     : none
 * @Description: 
 */
void mympu6050_init(struct i2c_client *pclt)
{
    mympu6050_write_byte(pclt, PWR_MGMT_1, 0x00);
    mympu6050_write_byte(pclt, SMPLRT_DIV, 0x07);
    mympu6050_write_byte(pclt, CONFIG, 0x06);
    mympu6050_write_byte(pclt, GYRO_CONFIG, 0xF8);
    mympu6050_write_byte(pclt, ACCEL_CONFIG, 0x19);
}

//=======================================================//
/** 平台驱动相关 */
/**
 * @Function   : mpu6050_driver_probe
 * @brief      : 设备和驱动匹配成功之后的调用函数
 * @param pclt : struct i2c_client * 类型
 * @param pid  : struct i2c_device_id * 类型
 * @return     : int 类型
 * @Description: 函数名赋值给 struct i2c_driver 结构体的 .probe 成员
 */
static int mpu6050_driver_probe(struct i2c_client *pclt, const struct i2c_device_id *pid)
{
    /* 0.相关变量定义 */

    /* 1.申请设备的内存空间 */
    p_mpu6050_dev = (struct __my_dev *)kmalloc(sizeof(struct __my_dev), GFP_KERNEL); /* 为设备属性结构体分配内存空间 */
    if (p_mpu6050_dev == NULL)
    {
        printk("[ERROR]p_mpu6050_dev kmalloc faimpu6050!\n");
        return -1;
    }
    memset(p_mpu6050_dev, 0, sizeof(struct __my_dev)); /* 申请的内存空间清 0 */
    /* 2.分配设备号 */
    /* 2.1尝试指定设备的主设备号和次设备号 */
    p_mpu6050_dev->major = 999;                                             /* 手动指定主设备号 */
    p_mpu6050_dev->minor = 0;                                               /* 大部分驱动次设备号都选择0 */
    p_mpu6050_dev->dev_num = 1;                                             /* 申请的设备数量 */
    p_mpu6050_dev->devno = MKDEV(p_mpu6050_dev->major, p_mpu6050_dev->minor); /* MKDEV宏用来将主设备号和次设备号组合成32位完整的设备号 */
    if (register_chrdev_region(p_mpu6050_dev->devno, p_mpu6050_dev->dev_num, MODULE_NAME))
    {
        /* 手动指定设备号分配失败，申请自动分配设备号 */
        if (alloc_chrdev_region(&p_mpu6050_dev->devno, 0, p_mpu6050_dev->dev_num, MODULE_NAME))
        {
            printk("[ERROR]get devno faimpu6050!\n");
            return -1;
        }
        /* 重新获取主设备号和次设备号 */
        p_mpu6050_dev->major = MAJOR(p_mpu6050_dev->devno); /* 自动分配设备号后，分离出主设备号 */
        p_mpu6050_dev->minor = MINOR(p_mpu6050_dev->minor); /* 自动分配设备号后，分离出次设备号 */
    }
    /* 3.向Linux内核注册新的字符设备 */
    /* 3.1初始化 mympu6050 对象, 给该对象添加操作函数集 */
    cdev_init(&p_mpu6050_dev->mympu6050, &mympu6050_ops);
    /* 3.2向 Linux 系统添加字符设备 (cdev结构体变量,将会被添加到一个hash链表中) */
    p_mpu6050_dev->mympu6050.owner = THIS_MODULE;
    cdev_add(&p_mpu6050_dev->mympu6050, p_mpu6050_dev->devno, p_mpu6050_dev->dev_num); /* 会在 /proc/devices 中创建设备号和对应的名称记录 */
    /* 4.自动创建设备节点(省去 mknod 命令在 /dev/ 下创建设备节点步骤) */
    /* 4.1创建类（创建成功时，名称可以在 /sys/class 中查到） */
    p_mpu6050_dev->class = class_create(THIS_MODULE, MODULE_NAME);
    if (IS_ERR(p_mpu6050_dev->class))
    {
        printk("[ERROR]class_create faimpu6050!\n");
        cdev_del(&p_mpu6050_dev->mympu6050);                                       /* 删除字符设备对象 cdev */
        unregister_chrdev_region(p_mpu6050_dev->devno, p_mpu6050_dev->dev_num); /* 注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
        return PTR_ERR(p_mpu6050_dev->class);
    }
    /* 4.2创建设备(创建的设备为 /dev/dev_name) */
    p_mpu6050_dev->device = device_create(p_mpu6050_dev->class, NULL, p_mpu6050_dev->devno, NULL, "%s%d", MODULE_NAME, p_mpu6050_dev->minor);
    if (IS_ERR(p_mpu6050_dev->device))
    {
        printk("[ERROR]device create faimpu6050!\n");
        class_destroy(p_mpu6050_dev->class);                                   /* 删除类 */
        cdev_del(&p_mpu6050_dev->mympu6050);                                       /* 删除字符设备对象 cdev */
        unregister_chrdev_region(p_mpu6050_dev->devno, p_mpu6050_dev->dev_num); /* 注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
        return PTR_ERR(p_mpu6050_dev->device);
    }
    /* 5.mpu6050初始化 */
    p_mpu6050_dev->pclt = pclt;
    mympu6050_init(p_mpu6050_dev->pclt);
    /* 打印提示信息 */
    printk("=======================================\n");
    printk("[INFO ]%s(%d):major=%d,minor=%d\n", MODULE_NAME, p_mpu6050_dev->devno, p_mpu6050_dev->major, p_mpu6050_dev->minor);
    printk("[INFO ]"BLUE"%s%d"CLS" has been created!\n", MODULE_NAME, p_mpu6050_dev->minor);
    printk("[INFO]mpu6050_driver_probe is called!\n");
    printk("=======================================\n");
    return 0;
}
/**
 * @Function   : mpu6050_driver_remove
 * @brief      : 卸载设备（rmmod 删除 driver 模块的时候被调用）
 * @param pclt : struct i2c_client * 类型
 * @return     : int 类型
 * @Description: 函数名赋值给 struct i2c_driver 结构体的 .remove 成员
 */
static int mpu6050_driver_remove(struct i2c_client *pclt)
{
    /* 1.重新获取设备号(有可能是动态分配了的设备号) */
    p_mpu6050_dev->devno = MKDEV(p_mpu6050_dev->major, p_mpu6050_dev->minor); /* MKDEV宏用来将主设备号和次设备号组合成32位完整的设备号 */

    /* 3.删除设备节点 */
    /* 3.1删除设备(/dev/dev_name 将会被删除) */
    device_destroy(p_mpu6050_dev->class, p_mpu6050_dev->devno);
    printk("[INFO]p_mpu6050_dev[%d]: [/dev/%s%d] has been deleted!\n", p_mpu6050_dev->devno, MODULE_NAME, p_mpu6050_dev->minor);
    /* 3.2删除类（将会删除 /sys/class 中的名称） */
    class_destroy(p_mpu6050_dev->class);
    /* 4.删除字符设备对象 cdev */
    cdev_del(&p_mpu6050_dev->mympu6050);
    /* 5.注销设备号(将会删除 /proc/devices 中的设备号和对应的名称记录)*/
    unregister_chrdev_region(p_mpu6050_dev->devno, p_mpu6050_dev->dev_num);
    /* 6.释放字符设备的存储空间 */
    kfree(p_mpu6050_dev);
    p_mpu6050_dev = NULL;
    printk("[INFO]mpu6050_driver_remove is called!\n");
    return 0;
}

/* 设备树匹配数组 */
struct of_device_id mpu6050_dt[] = {
    {.compatible = "invensense,mpu6050"},
    {},
};

/* ID匹配数组 */
struct i2c_device_id mpu6050_id_table[] = {
	{},
	{},
};

/** 定义I2C驱动 */
struct i2c_driver mpu6050_driver = {
    .driver = {
        .name = MODULE_NAME,   /* 必须初始化 name 成员 */
        .owner = THIS_MODULE,
        .of_match_table = mpu6050_dt,
    },
    .probe = mpu6050_driver_probe,   /* 设备和驱动匹配成功之后的调用函数 */
    .remove = mpu6050_driver_remove, /* 卸载设备 */
    .id_table =mpu6050_id_table,     /* 可以不使用，但是要有，不然似乎不能匹配成功 */
};

//=======================================================//
/** 整个模块相关 */
#if 1
/* 驱动入口函数 */
int __init mpu6050_driver_init(void)
{
    /* 注册平台设备驱动 */
    i2c_add_driver(&mpu6050_driver);
    /* 打印加载模块的提示信息 */
    printk("[INFO]This module [%s](driver) is loaded!\n", MODULE_NAME);
    return 0;
}

/* 驱动出口函数 */
void __exit mpu6050_driver_exit(void)
{
    /* 注销平台设备驱动 */
    i2c_del_driver(&mpu6050_driver);
    /* 打印卸载模块的提示信息 */
    printk("[INFO]This module [%s](driver) is exited!\n", MODULE_NAME);
}

/* 将上面两个函数指定为驱动的入口和出口函数 */
module_init(mpu6050_driver_init);
module_exit(mpu6050_driver_exit);
#else
module_i2c_driver(mpu6050_driver); /* 这其实是一个宏，展开后与上边一样 */
#endif
/* 模块信息(通过 modinfo module_name 查看) */
MODULE_LICENSE("GPL");                        /* 源码的许可证协议 */
MODULE_AUTHOR("qidaink <2038035593@qq.com>"); /* 字符串常量内容为模块作者说明 */
MODULE_DESCRIPTION("Description");            /* 字符串常量内容为模块功能说明 */
MODULE_ALIAS("module's other name");          /* 字符串常量内容为模块别名 */
