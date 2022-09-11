/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : env_data.h
 * Author     : qidaink
 * Date       : 2022-09-11
 * Version    : 
 * Description: 
 * Others     : 
 * Log        : 
 * ======================================================
 */

#ifndef __ENV_DATA_H__
#define __ENV_DATA_H__

/* 头文件 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include "cgic.h"
#include "env_data.h"

//===========================================================================
/* 宏定义 */
#define MONITOR_NUM   1            /* 监控的房间数量 */
#define SHM_SIZE      64           /* 共享内存大小 */

/* 前景色(字体颜色)，printk和printf 打印输出的颜色定义，主要是用于实现带颜色的输出 */
#define CLS           "\033[0m"    /* 清除所有颜色 */
#define BLACK         "\033[1;30m" /* 黑色加粗字体 */
#define RED           "\033[1;31m" /* 红色加粗字体 */
#define GREEN         "\033[1;32m" /* 绿色加粗字体 */
#define YELLOW        "\033[1;33m" /* 黄色加粗字体 */
#define BLUE          "\033[1;34m" /* 蓝色加粗字体 */
#define PURPLE        "\033[1;35m" /* 紫色加粗字体 */
#define CYAN          "\033[1;36m" /* 青色加粗字体 */
#define WHITE         "\033[1;37m" /* 白色加粗字体 */
#define BOLD          "\033[1m"    /* 加粗字体 */

//===========================================================================
/*数据类型重命名 */
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

//===========================================================================
/* 通信数据结构体定义 */
struct __zigbee_info        /* zigbee数据结构体 */
{
    uint8_t head[4];      /* 标识位: 'q' 's' 'm'  qidaink-security-monitor */
    uint8_t type;         /* 数据类型  'z'--- zigbee  'a'--- a9 */
    float temperature;    /* 温度值 */
    float humidity;       /* 湿度值 */
    float tempMIN;        /* 温度下限 */
    float tempMAX;        /* 温度上限 */
    float humidityMIN;    /* 湿度下限 */
    float humidityMAX;    /* 湿度上限 */
    uint32_t reserved[2]; /* 保留扩展位，默认填充0 */
};

struct __cotex_a9_info    /* Cotex-A9平台（FS4412开发板数据）数据结构体 */
{
    uint8_t head[4];      /* 标识位: 'q' 's' 'm'  qidaink-security-monitor */
    uint8_t type;         /* 数据类型  'z'--- zigbee  'a'--- a9 */
    float adc;            /* 板载 ADC 数据 */
    short gyrox;          /* x轴陀螺仪数据 */
    short gyroy;          /* y轴陀螺仪数据 */
    short gyroz;          /* z轴陀螺仪数据 */
    short aacx;           /* x轴加速计数据 */
    short aacy;           /* y轴加速计数据 */
    short aacz;           /* z轴加速计数据 */
    uint32_t reserved[2]; /* 保留扩展位，默认填充0 */
};

struct __env_data        /* 一个监控区域的所有环境数据结构体 */
{
    struct __cotex_a9_info a9_info;   /* Cotex-A9 数据，里边包括了ADC，三轴陀螺仪数据和三轴加速度计数据 */
    struct __zigbee_info zigbee_info; /* Zigbee 数据，里边包含了温湿度范围设定值以及温湿度数据 */
};

struct __allArea_env_data   /* 多个监控区域的属性结构体 */
{
    int area_id;            /* 房子编号 */
	struct __env_data  home[MONITOR_NUM];	/* 这是一个结构体数组，每一个成员都表示一个房间环境数据 */
}__attribute__((aligned (1))); 

/* 共享内存区域数据类型结构体 */
struct __shm_addr
{
    char shm_status;
    struct __allArea_env_data  shm_all_env_info;
};

/* semctl第四个参数用到的联合体数据类型 */
union semun
{
	int val;			   /* Value for SETVAL */
	struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
	unsigned short *array; /* Array for GETALL, SETALL */
	struct seminfo *__buf; /* Buffer for IPC_INFO
							  (Linux-specific) */
};

/* 函数声明 */

int init_sem(int semid, int num, int val);
int sem_p(int semid, int num);
int sem_v(int semid, int num);

#endif