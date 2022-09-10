/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : key_input.c
 * Author     : qidaink
 * Date       : 2022-08-28
 * Version    :
 * Description: 按键驱动模块
 * Others     :
 * Log        :
 * ======================================================
 */

/** 
 * key2 -- UART_RING  -- XEINT9/KP_COL1/ALV_DBG5/GPX1_1    -- H5
 * key3 -- SIM_DET    -- XEINT10/KP_COL2/ALV_DBG6/GPX1_2   -- H6
 * key4 -- 6260_GPIO2 -- XEINT26/KP_ROW10/ALV_DBG22/GPX3_2 -- G9
 */

/* 头文件 */
#include <linux/kernel.h>
#include <linux/module.h>    /* MODULE_LICENSE */
#include <linux/init.h>      /* module_init module_exit */
#include <linux/interrupt.h> /* request_irq */
#include <linux/of.h>        /* of_find_node_by_path */
#include <linux/of_irq.h>    /* irq_of_parse_and_map */
#include <linux/of_gpio.h>   /* of_get_named_gpio */
#include <linux/input.h>
#include <asm/io.h>
#include "key_input.h"


#define   KEY_NUMS   2       /* 按键数量定义 */

/* 每个按键的属性结构体 */
struct key_desc
{
    const char *name;         /* 按键名称,const可以防止 of_property_read_string 产生警告 */
    int irqno;                /* 中断号 */
    int key_code;             /* 按键编码 */
    int gpionum;              /* GPIO引脚 */
    void *reg_base;           /* 寄存器地址，暂未使用 */
    struct device_node *key_node;  /* 可以随时去获取设备树节点各个信息 */
};
struct key_desc all_key[KEY_NUMS];/* 按键数组，包括了多个按键的属性 */
struct input_dev *inputdev = NULL;         /* 输入设备指针变量 */
/**
 * @Function   : get_all_child_from_node
 * @brief      : 获取按键父节点下所有的子节点
 * @param path : char *类型，父节点的路径，如 "/fs4412-keys"
 * @return     : none
 * @Description: 
 */
void get_all_child_from_node(const char *path)
{
    /* 0.相关变量定义 */
    int i = 0;
	struct device_node *current_node = NULL;  /* 当前子节点地址 */
	struct device_node *prev = NULL;	      /* 前一个子节点地址 */
	struct device_node *parent_node = NULL;   /* 暂存父节点地址 */
    /* 1.查找到父节点地址 */
    parent_node = of_find_node_by_path(path); 
    if (parent_node)
        printk("[ OK  ]find node "BLUE"%s"CLS" ok!\n", path);
    else
        printk("[ERROR]find node "BLUE"%s"CLS" failed!\n", path);
    /* 2.获取子节点 */
    do{
        current_node = of_get_next_child(parent_node, prev);/* 函数将会跳过属性，直接找到一个子节点 */
        if (current_node != NULL)
        {
            all_key[i++].key_node = current_node; /* 将当前的节点记录下来 */
            printk("[INFO ]child_node "BLUE"%s"CLS" is found!\n", current_node->name);
        }
        prev = current_node; /* 把当前的节点地址赋值给prev，便于查找下一个节点 */
    }while(of_get_next_child(parent_node, prev) != NULL);
}

/**
 * @Function    : input_key_irq_handler
 * @brief       : 中断处理函数
 * @param no    : int 类型
 * @param arg   ：void * 类型
 * @return      : 返回
 * @Description :
 */
irqreturn_t input_key_irq_handler(int irqno, void *devid)
{
    /* 1.区分不同的按键 */
    struct key_desc *pdesc = (struct key_desc *)devid; /* 获取按键属性参数 */
    int gpionum = of_get_named_gpio(pdesc->key_node, "gpio", 0);/* 从子节点中获取按键的gpio引脚 */
    int value = gpio_get_value(gpionum);                        /* 读取gpio引脚电平，默认为高电平 */
    // printk("gpionum :%d----value :%d.\n", gpionum, value);

    /* 直接通过gpio获取按键状态 */
    if (value) /* 抬起，高点平 */
    {
        input_report_key(inputdev, pdesc->key_code, 0);
        input_sync(inputdev); /* 上报数据结束 */
    }
    else /* 按下 */
    {
        input_report_key(inputdev, pdesc->key_code, 1);
        input_sync(inputdev); /* 上报数据结束 */
    }

    return IRQ_HANDLED;
}
//=======================================================
// 模块相关
/* 驱动入口函数 */
int __init key_input_init(void)
{
    /* 0.相关变量定义 */
    int ret;
    int i;
    /* 1.创建一个 input_dev 对象 */
    inputdev = input_allocate_device();
    if (inputdev == NULL)
    {
        printk("[ERROR]input_allocate_device error!\n");
        return -ENOMEM;
    }
    /* 2.读取设备树中代表每个按键的子节点 */
    get_all_child_from_node("/fs4412-keys");

    /* 3.添加设备信息--/sys/class/input/eventx/device/ */
    inputdev->name = "simple input key"; /* 输入设备的名字 */
    inputdev->phys = "key/input/input0";
    inputdev->uniq = "simple key for fs4412";
    inputdev->id.bustype = BUS_HOST; /* 输入设备id,总线类型*/
    inputdev->id.vendor = 0x1234;    /* 输入设备id,制造商id*/
    inputdev->id.product = 0x8888;   /* 输入设备id,产品id */
    inputdev->id.version = 0x0001;   /* 输入设备id,版本号 */

    /* 4.当前设备能够产生按键数据--将某个bit置1 */
    __set_bit(EV_KEY, inputdev->evbit);
    __set_bit(KEY_2, inputdev->evbit);
    __set_bit(KEY_3, inputdev->evbit);
    /* 5.从设备树循环获取每个按键的具体参数 */
    for (i = 0; i < KEY_NUMS; i++)
    {
        /* 5.1获取按键编号 */
        of_property_read_u32(all_key[i].key_node, "key_code", &all_key[i].key_code);
        __set_bit(all_key[i].key_code, inputdev->keybit);/* 设置上报的按键编号 */
        /* 5.2获取按键对应的中断号 */
        all_key[i].irqno= irq_of_parse_and_map(all_key[i].key_node, 0);
        /* 5.3获取按键的名称 */
        of_property_read_string(all_key[i].key_node, "key_name", &all_key[i].name);
        /* 5.4申请中断号 */
        ret = request_irq(all_key[i].irqno, input_key_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, all_key[i].name, &all_key[i]);
        if (ret != 0)
        {
            printk("[ERROR]request_irq error!\n");
            goto err_0;
        }
    }
    /* 6.向内核注册设备 */
    ret = input_register_device(inputdev);
    if (ret != 0)
    {
        printk("[ERROR]input_register_device error\n");
        goto err_1;
    }
    /* 打印提示信息 */
    printk("=======================================\n");
    for (i = 0; i < KEY_NUMS; i++)
    {
        printk("[INFO ]key:"BLUE"key_name=%s,key_code=%d,key_irq=%d"CLS"\n", all_key[i].name, all_key[i].key_code, all_key[i].irqno);
    }
    printk("[INFO]This module has been loaded!\n");
    printk("=======================================\n");
    return 0;

err_1:
    input_unregister_device(inputdev);
err_0:
    input_free_device(inputdev);
    return ret;
}

//=======================================================
/* 驱动出口函数 */
void __exit key_input_exit(void)
{
    /* 0.定义相关变量 */
    int i;
    /* 1.释放IRQ */
    for (i = 0; i < KEY_NUMS; i++)
        free_irq(all_key[i].irqno, &all_key[i]);
    /* 2.注销input_dev设备 */
    input_unregister_device(inputdev);
    /* 3.释放字符设备的存储空间 */
    input_free_device(inputdev);
    /* 4.打印卸载模块的提示信息 */
    printk("This module is exited!\n");
}

/* 将上面两个函数指定为驱动的入口和出口函数 */
module_init(key_input_init);
module_exit(key_input_exit);

/* 模块信息(通过 modinfo key_input 查看) */
MODULE_LICENSE("GPL");               /* 源码的许可证协议 */
MODULE_AUTHOR("qidaink");            /* 字符串常量内容为模块作者说明 */
MODULE_DESCRIPTION("Description");   /* 字符串常量内容为模块功能说明 */
MODULE_ALIAS("module's other name"); /* 字符串常量内容为模块别名 */

