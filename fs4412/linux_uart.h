/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : linux_uart.h
 * Author     : qidaink
 * Date       : 2022-09-12
 * Version    :
 * Description: linux下串口控制头文件
 * Others     :
 * Log        :
 * ======================================================
 */

#ifndef __LINUX_UART_H__
#define __LINUX_UART_H__

/* 头文件 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>  /* tcgetattr cfmakeraw */
#include <termios.h> /* tcgetattr cfmakeraw */
#include <string.h>

/* 函数声明 */

extern int set_com_config(int fd, int baud_rate, int data_bits, char parity, int stop_bits); /* 配置串口参数 */
extern int open_port(char *com_port);                                                        /* 打开串口 */
extern void USB_UART_Config(char *path, int baud_rate);                                      /* USB串口快速初始化 */

#endif
