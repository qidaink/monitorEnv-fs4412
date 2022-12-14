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

/* LED控制相关变量声明 */
extern pthread_mutex_t mutex_led; /* LED互斥锁 */
extern pthread_cond_t cond_led;   /* LED条件变量 */
extern unsigned char led_cmd;     /* LED命令 */
extern unsigned char seg_cmd;     /* SEG模拟数码管命令 */
extern char function_flag;              /* 区分LED命令和seg命令 */
/* BUZZER控制相关变量声明 */
extern pthread_mutex_t mutex_buzzer;
extern pthread_cond_t cond_buzzer;
extern unsigned char buzzer_cmd; /* BUZZER命令 */

/* FAN控制相关变量声明 */
extern pthread_mutex_t mutex_zigbee;
extern pthread_cond_t cond_zigbee;
extern unsigned char fan_cmd; /* FAN命令 */

/* GPRS */
extern pthread_mutex_t mutex_gprs;
extern pthread_cond_t cond_gprs;
extern char recive_phone[12];
extern char center_phone[12];

struct __set_env html_set_env;

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
        msgrcv(msgid, &msgbuf, sizeof(msgbuf) - sizeof(long), 1L, 0); /* 阻塞式接收消息类型为1L的消息，没有该类型的消息msgrcv函数一直阻塞等待 */
        printf("Get %ldL msg\n", msgbuf.devtype);
        printf("text[0] = %#x\n", msgbuf.text[0]);
        /* 2.3处理消息 */
        switch (msgbuf.devtype) /* 具体消息处理 */
        {
            case 1L: /* LED 设备消息处理，进行LED控制 */
                printf("[INFO ]hello led!\n");
                pthread_mutex_lock(&mutex_led);
				led_cmd = msgbuf.text[0];  /* 获取LED控制命令 */
                function_flag = 1;
				pthread_mutex_unlock(&mutex_led);
				pthread_cond_signal(&cond_led);
				break;
            case 2L: /* 蜂鸣器消息处理，进行蜂鸣器控制 */
                printf("[INFO ]hello buzzer!\n");
                pthread_mutex_lock(&mutex_buzzer);
				buzzer_cmd = msgbuf.text[0];  /* 获取BUZZER控制命令 */
				pthread_mutex_unlock(&mutex_buzzer);
				pthread_cond_signal(&cond_buzzer);
                break;
            case 3L: /* 模拟数码管消息处理，进行四路LED灯模拟的数码管控制 */
                pthread_mutex_lock(&mutex_led);
				seg_cmd = msgbuf.text[0];  /* 获取seg控制命令 */
                function_flag = 0;
				pthread_mutex_unlock(&mutex_led);
				pthread_cond_signal(&cond_led);
                break;
            case 4L: /* 风扇消息处理，进行风扇控制 */
                printf("[INFO ]hello fan!\n");
                pthread_mutex_lock(&mutex_zigbee);
				fan_cmd = msgbuf.text[0];  /* 获取fan控制命令 */
				pthread_mutex_unlock(&mutex_zigbee);
				pthread_cond_signal(&cond_zigbee);
                break;
            case 5L: /* 温湿度最值消息处理，进行温湿度最值设置 */
                printf("[INFO ]set env data\n");
                if(sscanf((char *)&msgbuf.text[1], "&%d&%d&%d&%d", &html_set_env.tempMAX, &html_set_env.tempMIN, 
                                                           &html_set_env.humidityMAX, &html_set_env.humidityMIN) != 4)
                {
                    perror("[ERROR]sscanf");
                    exit(-1);
                }
                printf("temMAX: %d\n", html_set_env.tempMAX);
                printf("temMIN: %d\n", html_set_env.tempMIN);
                printf("humMAX: %d\n", html_set_env.humidityMAX);
                printf("humMAX: %d\n", html_set_env.humidityMIN);
                break;
            case 6L: /* 用于个人的扩展 */
            case 7L: /* 用于个人的扩展 */
            case 8L: /* 用于个人的扩展 */
            case 9L: /* 用于个人的扩展 */
                printf("extention!\n");
                break;
            case 10L: /* GPRS处理，3G通信模块-GPRS */
                printf("[INFO ]set phone data!\n");
                {
                    pthread_mutex_lock(&mutex_gprs);
                    int i = 0, j = 0;
                    /* 13800100500&15137735092& */
                    while(msgbuf.text[i] != '&')
                    {
                        recive_phone[i] = msgbuf.text[i];
                        i++;
                    }
                    recive_phone[i++] = '\0'; /* i = 11, msgbuf.text[11] = &, 执行完毕后，i=12  */
                    printf("recive:%s\n", recive_phone);
                    for (j = 0; msgbuf.text[i] != '\0' && msgbuf.text[i] != '&'; i++, j++)
                    {
                        center_phone[j] = msgbuf.text[i];
                    }
                    center_phone[j] = '\0';
                    printf("center:%s\n", center_phone);
                    pthread_cond_signal(&cond_gprs);
                    pthread_mutex_unlock(&mutex_gprs);
                }
                break;
            default:
                break;
        }
    }
    printf("[INFO ]pthread_client_request will exit!\n");
    exit(0);
}