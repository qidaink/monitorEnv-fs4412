/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : main.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    :
 * Description: 主函数
 * Others     :
 * Log        :
 * ======================================================
 */

#include <stdio.h>
#include "common.h"

/* 互斥锁 */
extern pthread_mutex_t mutex_client_request;
extern pthread_mutex_t mutex_refresh;
extern pthread_mutex_t mutex_sqlite;
extern pthread_mutex_t mutex_transfer;
extern pthread_mutex_t mutex_gprs;
extern pthread_mutex_t mutex_buzzer;
extern pthread_mutex_t mutex_led;
/* 条件变量 */
extern pthread_cond_t cond_client_request;
extern pthread_cond_t cond_refresh;
extern pthread_cond_t cond_sqlite;
extern pthread_cond_t cond_transfer;
extern pthread_cond_t cond_gprs;
extern pthread_cond_t cond_buzzer;
extern pthread_cond_t cond_led;
/* IPC对象ID */
extern int msgid;
extern int shmid;
extern int semid;

/* 线程ID */
pthread_t tid_client_request;
pthread_t tid_refresh;
pthread_t tid_sqlite;
pthread_t tid_transfer;
pthread_t tid_gprs;
pthread_t tid_buzzer;
pthread_t tid_led;

void release_pthread_resource(int signo);/* 信号处理函数，用于释放资源 */

int main(int argc, const char *argv[])
{
    /* 1.初始化所有的互斥锁 */
    pthread_mutex_init(&mutex_client_request, NULL);
    pthread_mutex_init(&mutex_refresh, NULL);
    pthread_mutex_init(&mutex_sqlite, NULL);
    pthread_mutex_init(&mutex_transfer, NULL);
    pthread_mutex_init(&mutex_gprs, NULL);
    pthread_mutex_init(&mutex_buzzer, NULL);
    pthread_mutex_init(&mutex_led, NULL);
    /* 2.等待接受信号（Ctrl+C触发），信号处理函数 */
    signal(SIGINT, release_pthread_resource);

    /* 3.初始化各种条件变量 */
    pthread_cond_init(&cond_client_request, NULL);
    pthread_cond_init(&cond_refresh, NULL);
    pthread_cond_init(&cond_sqlite, NULL);
    pthread_cond_init(&cond_transfer, NULL);
    pthread_cond_init(&cond_gprs, NULL);
    pthread_cond_init(&cond_buzzer, NULL);
    pthread_cond_init(&cond_led, NULL);

    /* 4.创建所需线程 */
    pthread_create(&tid_client_request, NULL, pthread_client_request, NULL);
    pthread_create(&tid_refresh, NULL, pthread_refresh, NULL);
    pthread_create(&tid_sqlite, NULL, pthread_sqlite, NULL);
    pthread_create(&tid_transfer, NULL, pthread_transfer, NULL);
    pthread_create(&tid_gprs, NULL, pthread_gprs, NULL);
    pthread_create(&tid_buzzer, NULL, pthread_buzzer, NULL);
    pthread_create(&tid_led, NULL, pthread_led, NULL);
#if 0
    /* 用于测试全局环境参数结构体变量赋值与显示 */
    init_env_data(&sm_allArea_env_info, 0);
    show_env_data(&sm_allArea_env_info, 0);
#endif
    /* 5.等待线程退出 */
    pthread_join(tid_client_request, NULL);
    printf("[INFO ]pthread1!\n");
    pthread_join(tid_refresh, NULL);
    printf("[INFO ]pthread2!\n");
    pthread_join(tid_sqlite, NULL);
    printf("[INFO ]pthread3!\n");
    pthread_join(tid_transfer, NULL);
    printf("[INFO ]pthread4!\n");
    pthread_join(tid_gprs, NULL);
    printf("[INFO ]pthread5!\n");
    pthread_join(tid_buzzer, NULL);
    printf("[INFO ]pthread6!\n");
    pthread_join(tid_led, NULL);
    printf("[INFO ]pthread7!\n");
    return 0;
}

/**
 * @Function   : release_pthread_resource
 * @brief      : 信号处理函数，用于释放资源
 * @param signo: 信号编号
 * @return     : none
 * @Description: 
 */
void release_pthread_resource(int signo)
{
    /* 1.销毁互斥锁 */
    pthread_mutex_destroy(&mutex_client_request);
    pthread_mutex_destroy(&mutex_refresh);
    pthread_mutex_destroy(&mutex_sqlite);
    pthread_mutex_destroy(&mutex_transfer);
    pthread_mutex_destroy(&mutex_gprs);
    pthread_mutex_destroy(&mutex_buzzer);
    pthread_mutex_destroy(&mutex_led);
    /* 2.销毁条件变量 */
    pthread_cond_destroy(&cond_client_request);
    pthread_cond_destroy(&cond_refresh);
    pthread_cond_destroy(&cond_sqlite);
    pthread_cond_destroy(&cond_transfer);
    pthread_cond_destroy(&cond_gprs);
    pthread_cond_destroy(&cond_buzzer);
    pthread_cond_destroy(&cond_led);
    /* 3.分离线程，线程结束后不会产生僵尸线程 */
    pthread_detach(tid_client_request);
    pthread_detach(tid_refresh);
    pthread_detach(tid_sqlite);
    pthread_detach(tid_transfer);
    pthread_detach(tid_gprs);
    pthread_detach(tid_buzzer);
    pthread_detach(tid_led);

    printf("\n[INFO ]all pthread is detached!\n");
    /* 4.销毁IPC对象 */
    msgctl(msgid, IPC_RMID, NULL);    /* 删除消息队列 */
    shmctl(shmid, IPC_RMID, NULL);    /* 删除共享内存 */
    semctl(semid, 1, IPC_RMID, NULL); /* 删除 System V 信号量*/
    /* 5.退出 */
    exit(0);
}