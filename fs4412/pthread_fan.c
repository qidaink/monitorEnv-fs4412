/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : pthread_fan.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    : 
 * Description: fan 设备控制线程
 * Others     : 
 * Log        : 
 * ======================================================
 */

#include "common.h"
#include "linux_uart.h"
/* 宏定义 */
#define FAN_DEV       "/dev/ttyUSB0"  /* USB驱动加载后的设备节点名称 */
#define FAN_ON        "fan_on"
#define FAN_OFF       "fan_off"
/* 变量声明 */
/* 声明条件变量和互斥锁 */
pthread_mutex_t mutex_zigbee;
pthread_cond_t cond_zigbee;
unsigned char fan_cmd;         /* FAN命令 */

/**
 * @Function   : pthread_fan
 * @brief      : fan设备控制线程
 * @param arg  : 线程创建时传递过来的参数
 * @return     : void *类型
 * @Description: 
 */
void *pthread_fan(void *arg)
{
    printf("[INFO ]pthread_fan is running!\n");
	/* 0.相关变量定义 */
	int fan_fd;
	/* 1.打开USB串口 */
	// fan_fd = open_port(FAN_DEV);
	// if (fan_fd < 0)
	// {
	// 	printf("[ERROR]open %s failed!\n", FAN_DEV);
	// 	exit(-1);
	// }
	// printf("open %s success! fan_fd:%d.\n", FAN_DEV, fan_fd);
	// /* 2.配置串口参数 */
	// set_com_config(fan_fd, 115200, 8, 'N', 1);
	/* 3.fan命令处理 */
	while (1)
	{
		pthread_mutex_lock(&mutex_zigbee);
		pthread_cond_wait(&cond_zigbee, &mutex_zigbee);
		fan_fd = open_port(FAN_DEV);
		set_com_config(fan_fd, 115200, 8, 'N', 1);
		printf("============fan ioctl=============\n");
        if(fan_cmd == 0x30)
        {
            if(write(fan_fd, FAN_OFF, sizeof(FAN_OFF)) < 0)
			{
				printf("[ERROR]write failed!\n");
				exit(-1);
			}
        }
        else if (fan_cmd == 0x31)
        {
            if(write(fan_fd, FAN_ON, sizeof(FAN_ON)) < 0)
			{
				printf("[ERROR]write failed!\n");
				exit(-1);
			}
        }
        else
        {
            printf("cmd_fan error.\n");
        }
		close(fan_fd);
        pthread_mutex_unlock(&mutex_zigbee);
	}
    // close(fan_fd);
    printf("[INFO ]pthread_fan will exit!\n");
    exit(0);
}