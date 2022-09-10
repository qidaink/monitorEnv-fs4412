/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : adc_dev.h
 * Author     : qidaink
 * Date       : 2022-08-28
 * Version    :
 * Description: adc驱动共用头文件
 * Others     :
 * Log        :
 * ======================================================
 */

#ifndef __BUZZER_DEV_H__
#define __BUZZER_DEV_H__

/* 头文件 */
#include <asm/ioctl.h>


/* 宏定义 */
#define DEBUG_PRINTK(msg, DEBUG_FLAG)                      \
    do{                                                    \
        if (!!DEBUG_FLAG)                                  \
        {                                                  \
            printk("---->%s--->%d\n", __func__, __LINE__); \
            printk(msg);                                   \
        }                                                  \
    } while (0)

#define handle_error(msg)                                          \
    do{                                                            \
        perror(msg);                                               \
        printk("%s --> %s -->%d\n", __FILE__, __func__, __LINE__); \
        exit(EXIT_FAILURE);                                        \
    } while (0)

/* printk和printf 打印输出的颜色定义 */
/* 前景色(字体颜色) */
#define CLS           "\033[0m"    /* 清除所有颜色 */
#define BLACK         "\033[1;30m" /* 黑色加粗字体 */
#define RED           "\033[1;31m" /* 红色加粗字体 */
#define GREEN         "\033[1;32m" /* 绿色加粗字体 */
#define YELLOW        "\033[1;33m" /* 黄色加粗字体 */
#define BLUE          "\033[1;34m" /* 蓝色加粗字体 */
#define PURPLE        "\033[1;35m" /* 紫色加粗字体 */
#define CYAN          "\033[1;36m" /* 青色加粗字体 */
#define WHITE         "\033[1;37m" /* 白色加粗字体 */
#define BOLD          "\033[1m"    /* 加粗字体 */

#endif