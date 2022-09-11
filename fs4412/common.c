/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : common.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    : 
 * Description: 全局共用源文件
 * Others     : 主要是一些共用变量的定义和声明
 * Log        : 
 * ======================================================
 */

/** 主要内容：
 * 全局的互斥体声明
 * 全局的条件锁声明声明
 * 全局的id和key信息声明
 * 全局的消息队列发送函数声明
 */

#include "common.h"
/* 互斥锁变量定义 */
pthread_mutex_t mutex_client_request;
pthread_mutex_t mutex_refresh;
pthread_mutex_t mutex_sqlite;
pthread_mutex_t mutex_transfer;
pthread_mutex_t mutex_gprs;
pthread_mutex_t mutex_buzzer;
pthread_mutex_t mutex_led;
/* 条件变量定义 */
pthread_cond_t cond_client_request;
pthread_cond_t cond_refresh;
pthread_cond_t cond_sqlite;
pthread_cond_t cond_transfer;
pthread_cond_t cond_gprs;
pthread_cond_t cond_buzzer;
pthread_cond_t cond_led;

/* IPC对象标识符定义 */
int msgid; /* 消息队列id */
int shmid; /* 共享内存id */
int semid; /* System V 信号量id */

/* 保证操作同一IPC对象的key值定义 */
key_t msg_key; /* msg_key,消息队列 key 值 */
key_t shm_key; /* 共享内存 key 值 */
key_t sem_key; /* 信号量 key 值 */

/* GPRS相关变量定义 */
char recive_phone[12] = {0};
char center_phone[12] = {0};

/* 一个房子结构体全局变量定义 */
struct __allArea_env_data g_allArea_env_info; /* 安防监控项目所有的环境信息 */

/**
 * @Function   : send_msg_queue
 * @brief      : 向消息队列发送消息
 * @param type : long类型，表示控制的设备类型，1L==LED控制，2L==buzzer控制，... 
 * @param text : unsigned char类型，表示消息的内容，一般是一条命令，占1字节
 * @return     : 成功返回0
 * @Description:
 */
int send_msg_queue(long type, unsigned char text)
{
    /* 0.相关变量定义 */
    struct __msg msgbuf;
    /* 1.消息成员赋值 */
    msgbuf.type = 1L;       /* 我们现在统一监控home0,将所有的消息类型都赋值为1L,若为0的话，接收的时候无法区分了，所以最好设置为大于0的数 */
    msgbuf.devtype = type;  /* 控制的设备类型，1L==LED控制，2L==buzzer控制，...  */
    msgbuf.text[0] = text;  /* 消息正文，一般是一个1字节的命令 */
    /* 2.发送消息到消息队列 */
    if (msgsnd(msgid, &msgbuf, sizeof(msgbuf) - sizeof(long), 0) == -1) /* 注意消息大小的话不包含消息的类型 */
    {
        perror("[ERROR]fail to msgsnd type!");
        exit(-1);
    }

    return 0;
}

/**
 * @Function               : init_env_data
 * @brief                  : 安防监控项目所有的环境信息初始化（用于静态测试）
 * @param allArea_env_data : struct __allArea_env_data *类型，表示某一房子属性结构体变量
 * @param home_num         : int类型，表示房间编号，一般是0
 * @return                 : 
 * @Description            :
 */
int init_env_data(struct __allArea_env_data * allArea_env_data, int home_num)
{
    /* 1.allArea_env_data->area_id打印 */
    allArea_env_data->area_id = 1;
    /* 2.allArea_env_data->home[%d]->a9_info打印 */
    strcpy((char *)allArea_env_data->home[home_num].a9_info.head, "qsm");
    allArea_env_data->home[home_num].a9_info.type = 'a';
    allArea_env_data->home[home_num].a9_info.adc = 25.5;
    allArea_env_data->home[home_num].a9_info.gyrox = 20;
    allArea_env_data->home[home_num].a9_info.gyroy = 30;
    allArea_env_data->home[home_num].a9_info.gyroz = 40;
    allArea_env_data->home[home_num].a9_info.aacx = 50;
    allArea_env_data->home[home_num].a9_info.aacy = 60;
    allArea_env_data->home[home_num].a9_info.aacz = 70;
    /* 3.allArea_env_data->home[%d]->zigbee_info打印 */
    strcpy((char *)allArea_env_data->home[home_num].zigbee_info.head, "qsm");
    allArea_env_data->home[home_num].zigbee_info.type = 'z';
    allArea_env_data->home[home_num].zigbee_info.temperature = 26.6;
    allArea_env_data->home[home_num].zigbee_info.humidity = 56.8;
    allArea_env_data->home[home_num].zigbee_info.tempMIN = 20.2;
    allArea_env_data->home[home_num].zigbee_info.tempMAX = 35.7;
    allArea_env_data->home[home_num].zigbee_info.humidityMIN = 66.7;
    allArea_env_data->home[home_num].zigbee_info.humidityMAX = 30.2;

    return 0;
}

/**
 * @Function               : show_env_data
 * @brief                  : 安防监控项目所有的环境信息
 * @param allArea_env_data : struct __allArea_env_data *类型，表示某一房子属性结构体变量
 * @param home_num         : int类型，表示房间编号，一般是0
 * @return                 : 
 * @Description            :
 */
int show_env_data(struct __allArea_env_data * allArea_env_data, int home_num)
{
    /* 1.allArea_env_data->area_id打印 */
    printf("area_id = %d\n", allArea_env_data->area_id);
    /* 2.allArea_env_data->home[%d]->a9_info打印 */
    printf("home[%d]-->a9_info-->head=%s\n", home_num, allArea_env_data->home[home_num].a9_info.head);
    printf("home[%d]-->a9_info-->type=%c\n", home_num, allArea_env_data->home[home_num].a9_info.type);
    printf("home[%d]-->a9_info-->adc=%.2f\n", home_num, allArea_env_data->home[home_num].a9_info.adc);
    printf("home[%d]-->a9_info-->gyrox=%d\n", home_num, allArea_env_data->home[home_num].a9_info.gyrox);
    printf("home[%d]-->a9_info-->gyroy=%d\n", home_num, allArea_env_data->home[home_num].a9_info.gyroy);
    printf("home[%d]-->a9_info-->gyroz=%d\n", home_num, allArea_env_data->home[home_num].a9_info.gyroz);
    printf("home[%d]-->a9_info-->aacx=%d\n", home_num, allArea_env_data->home[home_num].a9_info.aacx);
    printf("home[%d]-->a9_info-->aacy=%d\n", home_num, allArea_env_data->home[home_num].a9_info.aacy);
    printf("home[%d]-->a9_info-->aacz=%d\n", home_num, allArea_env_data->home[home_num].a9_info.aacz);
    /* 3.allArea_env_data->home[%d]->zigbee_info打印 */
    printf("home[%d]-->zigbee_info-->head=%s\n", home_num, allArea_env_data->home[home_num].zigbee_info.head);
    printf("home[%d]-->zigbee_info-->type=%c\n", home_num, allArea_env_data->home[home_num].zigbee_info.type);
    printf("home[%d]-->zigbee_info-->temperature=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.temperature);
    printf("home[%d]-->zigbee_info-->humidity=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.humidity);
    printf("home[%d]-->zigbee_info-->tempMIN=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.tempMIN);
    printf("home[%d]-->zigbee_info-->tempMAX=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.tempMAX);
    printf("home[%d]-->zigbee_info-->humidityMIN=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.humidityMIN);
    printf("home[%d]-->zigbee_info-->humidityMAX=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.humidityMAX);

    return 0;
}

