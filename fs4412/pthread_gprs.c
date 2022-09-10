/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : pthread_gprs.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    : 
 * Description: GPRS模块控制线程
 * Others     : 
 * Log        : 
 * ======================================================
 */

#include "common.h"

/**
 * @Function   : pthread_gprs
 * @brief      : GPRS模块控制线程
 * @param arg  : 线程创建时传递过来的参数
 * @return     : void *类型
 * @Description: 
 */
void *pthread_gprs(void *arg)
{
    printf("[INFO ]pthread_gprs is running!\n");
    while(1)
    {
        sleep(1);
    }
    printf("[INFO ]pthread_gprs will exit!\n");
    exit(0);
}