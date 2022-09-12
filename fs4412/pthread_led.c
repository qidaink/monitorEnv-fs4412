/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : pthread_led.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    : 
 * Description: led 设备控制线程
 * Others     : 
 * Log        : 
 * ======================================================
 */

#include "common.h"
/* 宏定义 */
#define LED_DEV       "/dev/led_dev0" /* LED驱动加载后的设备节点名称 */
#define LED_DEV_MAGIC 'a'
#define LED_DEV_ON    _IO(LED_DEV_MAGIC, 0)  /* 打开LED */
#define LED_DEV_OFF   _IO(LED_DEV_MAGIC, 1)  /* 关闭LED */
#define LED_STATUS(x) _IO(LED_DEV_MAGIC, x)  /* 根据x的值决定LED状态 */

/* 变量声明 */
/* 声明条件变量和互斥锁 */
extern pthread_mutex_t mutex_led; /* LED互斥锁 */
extern pthread_cond_t cond_led;   /* LED条件变量 */

/* 变量与结构体定义 */
typedef struct led_desc /* LED属性结构体定义 */
{
    int num;    /* LED编号：2 3 4 5 */
    int status; /* LED状态：0 or 1 */
} led_desc_t;

unsigned char led_cmd;           /* LED命令 */
unsigned char seg_cmd;           /* seg命令 */
char function_flag;              /* 区分LED命令和seg命令 */

int close_all_led(int led_fd);   /* 关闭所有LED灯 */

/**
 * @Function   : pthread_led
 * @brief      : LED设备控制线程
 * @param arg  : 线程创建时传递过来的参数
 * @return     : void *类型
 * @Description: 
 */
void *pthread_led(void *arg)
{
    printf("[INFO ]pthread_led is running!\n");
	/* 0.相关变量定义 */
	int i, j;
	int led_fd;
	led_desc_t led;
	/* 1.打开LED设备 */
	led_fd = open(LED_DEV, O_RDWR);
	if (led_fd < 0)
	{
		printf("[ERROR]open %s failed!\n", LED_DEV);
		exit(-1);
	}
	printf("open %s success! led_fd:%d.\n", LED_DEV, led_fd);
	/* 2.LED命令处理 */
	while (1)
	{
		pthread_mutex_lock(&mutex_led);
		pthread_cond_wait(&cond_led, &mutex_led);
		printf("============led ioctl=============\n");
        if(function_flag == 1)
        {
            /* 2.1获取LED控制状态 */
            led.status = !((led_cmd >> 0) & 0x01);/* xxxx xxx0 | 0000 0001 = 0000 0000; xxxx xxx1 | 0000 0001 = 0000 0001 */
            /* 2.2获取LED编号 */
            led.num = (led_cmd >> 1) & 0x7;       
            printf("\nled_status=%d, led_num=%d\n", led.status, led.num);
            switch(led.num )
            {
                case 2: /* LED2 */
                case 3: /* LED3 */
                case 4: /* LED4 */
                case 5: /* LED5 */
                    ioctl(led_fd, LED_STATUS(led.status), led.num);
                    break;
                case 6: /* 全部LED */
                    for(i = 2; i < 6; i++)
                    {
                        ioctl(led_fd, LED_STATUS(led.status), i);
                    }
                    break;
                case 7: /* 流水灯 */
                    close_all_led(led_fd);
                    for (j = 0; j < 3; j++)
                    {
                        for (i = 2; i < 6; i++)
                        {
                            ioctl(led_fd, LED_DEV_ON, i);
                            usleep(500000);
                            ioctl(led_fd, LED_DEV_OFF, i);
                            usleep(500000);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            /* 2.3seg对LED的控制部分 */
            unsigned char temp = (seg_cmd >> 4) & 0x2;/* 0x2 = 0000 0010 */
            led_desc_t seg;
            unsigned char seg_cmd_num;
            if(temp == 0x2)/* s说明是seg模拟数码管命令 */
            {
                close_all_led(led_fd);
                seg_cmd_num = seg_cmd & 0x0f;/* 获取seg命令的低4位 */
                for(i = 2; i < 6; i++)
                {
                    seg.status = !((seg_cmd_num >> (i - 2)) & 0x1);/* 1111 */
                    ioctl(led_fd, LED_STATUS(seg.status), i);
                    usleep(50000);
                }
            }
        }
		pthread_mutex_unlock(&mutex_led);
	}
    close(led_fd);
    printf("[INFO ]pthread_led will exit!\n");
    exit(0);
}

/**
 * @Function     : close_all_led
 * @brief        : 关闭所有LED灯
 * @param led_fd : int类型，led设备的文件描述符
 * @return       : int类型，操作结束返回0
 * @Description  : 
 */
int close_all_led(int led_fd)
{
    /* 0.定义相关变量 */
    int i = 0;
    /* 1.关闭所有LED */
    for (i = 2; i < 6; i++)
    {
        ioctl(led_fd, LED_DEV_OFF, i);
        usleep(50000);
    }

    return 0;
}