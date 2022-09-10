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
    while(1)
    {
        sleep(1);
    }
    printf("[INFO ]pthread_led will exit!\n");
    exit(0);
}