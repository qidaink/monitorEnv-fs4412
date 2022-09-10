/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : led_dev.h
 * Author     : qidaink
 * Date       : 2022-08-29
 * Version    : 
 * Description: MPU6050 驱动模块共用头文件
 * Others     : 
 * Log        : 
 * ======================================================
 */
#ifndef __MPU6050_DEV_H__
#define __MPU6050_DEV_H__

/* 头文件 */
#include <asm/ioctl.h>

/* 结构体定义 */
struct accel_data     /* 三轴加速度数据 */
{
	unsigned short x;
	unsigned short y;
	unsigned short z;
};

struct gyro_data     /* 三轴角速度数据 */
{
	unsigned short x;
	unsigned short y;
	unsigned short z;
};
/* 联合体，读取的数据可能是三种 */
union mpu6050_data  /* 从mpu6050读取的数据 */
{
	struct accel_data accel;
	struct gyro_data gyro;
	unsigned short temp;
};

/* 宏定义 */
#define MPU6050_MAGIC 'c'

#define GET_ACCEL _IOR(MPU6050_MAGIC, 0, union mpu6050_data)
#define GET_GYRO  _IOR(MPU6050_MAGIC, 1, union mpu6050_data)
#define GET_TEMP  _IOR(MPU6050_MAGIC, 2, union mpu6050_data)

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