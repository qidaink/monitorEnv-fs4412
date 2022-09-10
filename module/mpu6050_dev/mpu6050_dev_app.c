/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : mpu6050_dev_app.c
 * Author     : qidaink
 * Date       : 2022-09-07
 * Version    : 
 * Description:  MPU6050 驱动模块测试app
 * Others     : 
 * Log        : 
 * ======================================================
 */

#include <stdio.h>     /* printf */
#include <sys/types.h> /* open */
#include <sys/stat.h>  /* open */
#include <fcntl.h>     /* open */
#include <unistd.h>    /* close sleep */
#include <sys/ioctl.h> /* ioctl */

#include "mpu6050_dev.h"

int main(int argc, char *argv[])
{
    /* 0.相关变量定义 */
    int fd = -1;     /* 文件描述符 */
    union mpu6050_data data;
    /* 1.判断参数个数是否合法 */
    /* 1.1判断参数数量 */
    if (argc < 2)
    {
        printf("\nusage:\n");
        printf("%s /dev/dev_name\n", argv[0]);
        return -1;
    }

    /* 2.打开字符设备 */
    if ((fd = open(argv[1], O_RDWR)) < 0) /* 默认是阻塞的 */
    {
        printf("open %s failed!\n", argv[1]);
        return -1;
    }
    /* 3.读取数据 */
    while (1)
    {
        sleep(1);

        ioctl(fd, GET_ACCEL, &data);
        printf("Accel-x=0x%x\n", data.accel.x);
        printf("Accel-y=0x%x\n", data.accel.y);
        printf("Accel-z=0x%x\n", data.accel.z);

        ioctl(fd, GET_GYRO, &data);
        printf("Gyro-x=0x%x\n", data.gyro.x);
        printf("Gyro-y=0x%x\n", data.gyro.y);
        printf("Gyro-z=0x%x\n", data.gyro.z);

        ioctl(fd, GET_TEMP, &data);
        printf("Temp=0x%x\n", data.temp);

        printf("\n");
    }
    /* 4.关闭字符设备 */
    close(fd);
    fd = -1;

    return 0;
}
