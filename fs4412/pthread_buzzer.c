/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : pthread_buzzer.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    : 
 * Description: buzzer 设备控制线程
 * Others     : 
 * Log        : 
 * ======================================================
 */

#include "common.h"
/* 宏定义 */
#define BUZZER_DEV       "/dev/buzzer_dev0" /* BUZZER驱动加载后的设备节点名称 */

/* 变量声明 */
/* 声明条件变量和互斥锁 */
pthread_mutex_t mutex_buzzer;
pthread_cond_t cond_buzzer;

/* 变量与结构体定义 */
typedef struct buzzer_desc /* BUZZER属性结构体定义 */
{
    int buzzer;
    int status;      /* 状态 */
    int tcnt;        /* 占空比 */
    int tcmp;        /* 调节占空比 */
} buzzer_desc_t;

unsigned char buzzer_cmd;         /* BUZZER命令 */

/* 第2N个元素表示声调   第2N+1个元素表示该声调的时间 */
unsigned char MUSIC[500] = { /* 祝你平安 */
	0x26,0x20,0x20,0x20,0x20,0x20,0x26,0x10,0x20,0x10,0x20,0x80,0x26,0x20,0x30,0x20,
	0x30,0x20,0x39,0x10,0x30,0x10,0x30,0x80,0x26,0x20,0x20,0x20,0x20,0x20,0x1c,0x20,
	0x20,0x80,0x2b,0x20,0x26,0x20,0x20,0x20,0x2b,0x10,0x26,0x10,0x2b,0x80,0x26,0x20,
	0x30,0x20,0x30,0x20,0x39,0x10,0x26,0x10,0x26,0x60,0x40,0x10,0x39,0x10,0x26,0x20,
	0x30,0x20,0x30,0x20,0x39,0x10,0x26,0x10,0x26,0x80,0x26,0x20,0x2b,0x10,0x2b,0x10,
	0x2b,0x20,0x30,0x10,0x39,0x10,0x26,0x10,0x2b,0x10,0x2b,0x20,0x2b,0x40,0x40,0x20,
	0x20,0x10,0x20,0x10,0x2b,0x10,0x26,0x30,0x30,0x80,0x18,0x20,0x18,0x20,0x26,0x20,
	0x20,0x20,0x20,0x40,0x26,0x20,0x2b,0x20,0x30,0x20,0x30,0x20,0x1c,0x20,0x20,0x20,
	0x20,0x80,0x1c,0x20,0x1c,0x20,0x1c,0x20,0x30,0x20,0x30,0x60,0x39,0x10,0x30,0x10,
	0x20,0x20,0x2b,0x10,0x26,0x10,0x2b,0x10,0x26,0x10,0x26,0x10,0x2b,0x10,0x2b,0x80,
	0x18,0x20,0x18,0x20,0x26,0x20,0x20,0x20,0x20,0x60,0x26,0x10,0x2b,0x20,0x30,0x20,
	0x30,0x20,0x1c,0x20,0x20,0x20,0x20,0x80,0x26,0x20,0x30,0x10,0x30,0x10,0x30,0x20,
	0x39,0x20,0x26,0x10,0x2b,0x10,0x2b,0x20,0x2b,0x40,0x40,0x10,0x40,0x10,0x20,0x10,
	0x20,0x10,0x2b,0x10,0x26,0x30,0x30,0x80,0x00,
};

/* BUZZER控制宏 */
#define BUZZER_DEV_MAGIC 'b'
#define BUZZER_ON        _IOW(BUZZER_DEV_MAGIC, 0, buzzer_desc_t)
#define BUZZER_OFF       _IOW(BUZZER_DEV_MAGIC, 1, buzzer_desc_t)
#define BUZZER_FREQ      _IOW(BUZZER_DEV_MAGIC, 2, buzzer_desc_t)

/**
 * @Function   : pthread_buzzer
 * @brief      : buzzer设备控制线程
 * @param arg  : 线程创建时传递过来的参数
 * @return     : void *类型
 * @Description: 
 */
void *pthread_buzzer(void *arg)
{
    printf("[INFO ]pthread_buzzer is running!\n");
	/* 0.相关变量定义 */
	int i = 0;
    buzzer_desc_t buzzer;
	int buzzer_fd;
	/* 1.打开BUZZER设备 */
	buzzer_fd = open(BUZZER_DEV, O_RDWR);
	if (buzzer_fd < 0)
	{
		printf("[ERROR]open %s failed!\n", BUZZER_DEV);
		exit(-1);
	}
	printf("open %s success! buzzer_fd:%d.\n", BUZZER_DEV, buzzer_fd);
	/* 2.LED命令处理 */
	while (1)
	{
		pthread_mutex_lock(&mutex_buzzer);
		pthread_cond_wait(&cond_buzzer, &mutex_buzzer);
		printf("============buzzer ioctl=============\n");
        /* 2.1获取LED控制状态 */
        buzzer.status = (buzzer_cmd >> 0) & 0x01;
        printf("\nbuzzer_status=%d\n", buzzer.status);
        if(buzzer_cmd == 0x51)
        {
            ioctl(buzzer_fd, BUZZER_ON);/* 打开蜂鸣器 */
            buzzer.tcnt = MUSIC[i];
            buzzer.tcmp = MUSIC[i] / 2;
            ioctl(buzzer_fd, BUZZER_FREQ, &buzzer);
        }
        else if (buzzer_cmd == 0x50)
        {
            ioctl(buzzer_fd, BUZZER_OFF);
        }
        else
        {
            printf("cmd_buzzer error.\n");
        }
        pthread_mutex_unlock(&mutex_buzzer);
	}
    close(buzzer_fd);
    printf("[INFO ]pthread_buzzer will exit!\n");
    exit(0);
}