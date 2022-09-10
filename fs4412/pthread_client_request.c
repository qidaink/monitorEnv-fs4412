/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : pthread_client_request.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    :
 * Description: 消息队列消息处理控制线程
 * Others     : 从网页下发的消息都会在这里得到处理
 * Log        :
 * ======================================================
 */

#include "common.h"

/* 消息队列IPC对象相关变量声明 */
extern int msgid;
extern key_t msg_key;

struct __msg msgbuf;/* 定义一个消息结构体变量 */

/**
 * @Function   : pthread_client_request
 * @brief      : 消息队列消息处理线程
 * @param arg  : 线程创建时传递过来的参数
 * @return     : void *类型
 * @Description:
 */
void *pthread_client_request(void *arg)
{
    printf("[INFO ]pthread_client_request is running!\n");
    /* 1.创建或者打开消息队列 */
    /* 1.1生成 msg_key */
    if ((msg_key = ftok("/tmp", 'g')) < 0)
    {
        perror("[ERROR]ftok failed!\n");
        exit(-1);
    }
    /* 1.2获取消息队列id */
    msgid = msgget(msg_key, IPC_CREAT | IPC_EXCL | 0666);
    if (msgid == -1)
    {
        if (errno == EEXIST)
        {
            msgid = msgget(msg_key, 0777);
        }
        else
        {
            perror("[ERROR]fail to msgget!\n");
            exit(-1);
        }
    }
    printf("[INFO ]msgid=%d!\n", msgid);
    /* 2.接收消息并进行处理 */
    while (1)
    {
        /* 2.1清空原来的消息 */
        bzero(&msgbuf, sizeof(msgbuf));
        printf("[INFO ]wait form client request...\n");
        /* 2.2接收消息 */
        msgrcv(msgid, &msgbuf, sizeof(msgbuf) - sizeof(long), 1L, 0); /* 接收的消息类型为0L的消息 */
        printf("Get %ldL msg\n", msgbuf.devtype);
        printf("text[0] = %#x\n", msgbuf.text[0]);
        /* 2.3处理消息 */
        switch (msgbuf.devtype) /* 具体消息处理 */
        {
        case 1L: /* LED 设备消息处理，进行LED控制 */
            printf("[INFO ]hello led!\n");
            break;
        case 2L: /* 蜂鸣器消息处理，进行蜂鸣器控制 */
            printf("[INFO ]hello beep!\n");
            break;
        case 3L: /* 模拟数码管消息处理，进行四路LED灯模拟的数码管控制 */
            printf("[INFO ]hello seg!\n");
            break;
        case 4L: /* 风扇消息处理，进行风扇控制 */
            printf("[INFO ]hello fan!\n");
            break;
        case 5L: /* 温湿度最值消息处理，进行温湿度最值设置 */
            printf("[INFO ]set env data\n");
            printf("temMAX: %d\n", *((int *)&msgbuf.text[1]));
            printf("temMIN: %d\n", *((int *)&msgbuf.text[5]));
            printf("humMAX: %d\n", *((int *)&msgbuf.text[9]));
            printf("humMAX: %d\n", *((int *)&msgbuf.text[13]));
            printf("illMAX: %d\n", *((int *)&msgbuf.text[17]));
            printf("illMAX: %d\n", *((int *)&msgbuf.text[21]));
            break;
        case 6L: /* 用于个人的扩展 */
        case 7L: /* 用于个人的扩展 */
        case 8L: /* 用于个人的扩展 */
        case 9L: /* 用于个人的扩展 */
            printf("extention!\n");
            break;
        case 10L: /* GPRS处理，3G通信模块-GPRS */
            break;
        default:
            break;
        }
    }
    printf("[INFO ]pthread_client_request will exit!\n");
    exit(0);
}