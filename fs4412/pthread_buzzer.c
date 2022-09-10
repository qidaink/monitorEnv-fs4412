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
    while(1)
    {
        sleep(1);
    }
    printf("[INFO ]pthread_buzzer will exit!\n");
    exit(0);
}