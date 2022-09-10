/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : pthread_sqlite.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    : 
 * Description: 数据库操作线程
 * Others     : 
 * Log        : 
 * ======================================================
 */

#include "common.h"

/**
 * @Function   : pthread_sqlite
 * @brief      : 数据库操作线程
 * @param arg  : 线程创建时传递过来的参数
 * @return     : void *类型
 * @Description: 
 */
void *pthread_sqlite(void *arg)
{
    printf("[INFO ]pthread_sqlite is running!\n");
    while(1)
    {
        sleep(1);
    }
    printf("[INFO ]pthread_sqlite will exit!\n");
    exit(0);
}