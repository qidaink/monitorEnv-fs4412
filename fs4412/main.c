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
extern struct __allArea_env_data g_allArea_env_info; /* 安防监控项目所有的环境信息 */
int main(int argc, const char *argv[])
{
    /* code */ 
    init_env_data(&g_allArea_env_info, 0);
    show_env_data(&g_allArea_env_info, 0);
    return 0;
}