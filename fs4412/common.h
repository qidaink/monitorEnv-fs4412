/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : common.h
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    : 
 * Description: 全局共用头文件
 * Others     : 主要是一些共用变量的定义和声明
 * Log        : 
 * ======================================================
 */

/** 主要内容：
 * 全局的宏定义#define
 * 全局的线程函数声明
 * 全局的设备节点声明
 * 全局的消息队列发送函数外部extern声明
 * 全局的消息队列传递的结构体信息声明
 */

#ifndef __COMMON_H__
#define __COMMON_H__

/* 头文件 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <termios.h>
#include <syscall.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <linux/input.h>

//===========================================================================
/* 宏定义 */
#define MONITOR_NUM   1            /* 监控的房间数量 */
#define QUEUE_MSG_LEN 32           /* 消息队列中消息正文的最大长度 */

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

/* 消息队列消息传递结构体 */
struct __msg
{
    long type;                         /* 从消息队列接收消息时用于判断的消息类型，0L===home0  1L===home1 ...  */
    long devtype;                      /* 具体的消息控制设备类型，指代控制的设备，是什么类型的设备，1L表示LED，2L表示蜂鸣器 ... */
    unsigned char text[QUEUE_MSG_LEN]; /* 消息正文， CMD 控制指定的设备 */
};

//===========================================================================
/* 函数声明 */

int send_msg_queue(long type, unsigned char text);                            /* 向消息队列发送消息 */
int init_env_data(struct __allArea_env_data * allArea_env_data, int home_num);/* 安防监控项目所有的环境信息初始化（用于静态测试） */
int show_env_data(struct __allArea_env_data * allArea_env_data, int home_num);/* 安防监控项目所有的环境信息打印 */

extern void *pthread_client_request(void *arg); /* 接收CGI 等的请求 */
extern void *pthread_refresh(void *arg);        /* 刷新共享内存数据线程 */
extern void *pthread_sqlite(void *arg);         /* 数据库线程，保存数据库的数据 */
extern void *pthread_transfer(void *arg);       /* 接收ZigBee的数据并解析 */
extern void *pthread_gprs(void *arg);           /* 发送短信线程 */
extern void *pthread_buzzer(void *arg);         /* 蜂鸣器控制线程 */
extern void *pthread_led(void *arg);            /* led灯控制线程 */

#endif
