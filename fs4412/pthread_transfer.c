/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : pthread_transfer.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    : 
 * Description: Cotex-A9和ZigBee数据采集线程
 * Others     : 
 * Log        : 
 * ======================================================
 */

#include "common.h"

/**
 * @Function   : pthread_transfer
 * @brief      : Cotex-A9和ZigBee数据采集线程
 * @param arg  : 线程创建时传递过来的参数
 * @return     : void *类型
 * @Description: 
 */
void *pthread_transfer(void *arg)
{
    printf("[INFO ]pthread_transfer is running!\n");
    while(1)
    {
        sleep(1);
    }
    printf("[INFO ]pthread_transfer will exit!\n");
    exit(0);
}