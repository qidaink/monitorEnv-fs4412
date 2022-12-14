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

/* 设备节点名称 */
#define MPU6050_DEV "/dev/mpu60500"
#define ADC_DEV     "/dev/adc_dev0"
/* 文件描述符定义 */
int adc_fd;
int mpu_fd;

extern pthread_cond_t cond_transfer;
extern pthread_mutex_t mutex_transfer;
extern struct __allArea_env_data g_allArea_env_info; /* 安防监控项目所有的环境信息 */

int file_env_info_a9(struct __allArea_env_data *rt_status, int home_num);
int printf_a9_sensor_info_debug(struct __allArea_env_data * allArea_env_data, int home_num);
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
    /* 0.相关变量定义 */
    int home_num = 0;
    /* 1.打开设备 */
    adc_fd = open(ADC_DEV, O_RDWR);
    mpu_fd = open(MPU6050_DEV, O_RDWR);
    if ((adc_fd == -1) || (mpu_fd == -1))
    {
        printf("[ERROR]open adc or mpu6050 device failed.\n");
    }
    printf("open adc and mpu6050 success: adc_fd=%d,mpu_fd=%d\n", adc_fd, mpu_fd);
    /* 2.采集数据 */
    while (1)
    {
        /* 2.1获取互斥锁 */
        pthread_mutex_lock(&mutex_transfer);
        /* 2.2等待条件变量（判断某个条件不满足时，调用此函数将线程设置为等待状态（阻塞）） */
        pthread_cond_wait(&cond_transfer, &mutex_transfer); /* pthread_refresh线程显示完毕一次，会发送一次条件变量 */
        printf("[INFO ]a9-------------------->.\n");
        /* 2.3填充一次采集到的数据 */
        file_env_info_a9(&g_allArea_env_info, home_num);
        /* 2.4释放互斥锁 */
        pthread_mutex_unlock(&mutex_transfer);
        sleep(1);
    }
    /* 3.关闭设备 */
    close(adc_fd);
    close(mpu_fd);
    printf("[INFO ]pthread_transfer will exit!\n");
    exit(0);
}
/**
 * @Function        : file_env_info_a9_zigbee
 * @brief           : FunctionRole
 * @param rt_status : struct __allArea_env_data *类型，表示某一房子属性结构体变量
 * @param home_num  : int类型，表示房间编号，一般是0
 * @return          : none
 * @Description     : 
 */
int file_env_info_a9(struct __allArea_env_data *rt_status, int home_num)
{
    /* 1.获取Cotex-A9数据 */
    /* 1.1相关变量定义 */
    int adc_sensor_data;
    union __mpu6050_data data;
    /* 1.2获取ADC数据*/
    read(adc_fd, &adc_sensor_data, 4); /* 记得加载驱动了*/
    // printf("adc value :%0.2fV.\n", (1.8 * adc_sensor_data) / 4096);
    rt_status->home[home_num].a9_info.adc = (float)((1.8 * adc_sensor_data) / 4096);

    /* 1.3获取mpu6050数据*/
    ioctl(mpu_fd, GET_GYRO, &data);
    // printf("gyro data: x = %d, y = %d, z = %d\n", data.gyro.x, data.gyro.y, data.gyro.z);
    ioctl(mpu_fd, GET_ACCEL, &data);
    // printf("accel data: x = %d, y = %d, z = %d\n", data.accel.x, data.accel.y, data.accel.z);

    rt_status->home[home_num].a9_info.head[0] = 'q';
    rt_status->home[home_num].a9_info.head[1] = 's';
    rt_status->home[home_num].a9_info.head[2] = 'm';
    rt_status->home[home_num].a9_info.head[3] = '\0';

    rt_status->home[home_num].a9_info.gyrox = (short)data.gyro.x;
    rt_status->home[home_num].a9_info.gyroy = (short)data.gyro.y;
    rt_status->home[home_num].a9_info.gyroz = (short)data.gyro.z;

    rt_status->home[home_num].a9_info.aacx = (short)data.accel.x;
    rt_status->home[home_num].a9_info.aacy = (short)data.accel.y;
    rt_status->home[home_num].a9_info.aacz = (short)data.accel.z;
    rt_status->home[home_num].a9_info.reserved[0] = 0.01;
    rt_status->home[home_num].a9_info.reserved[1] = -0.01;

    // printf_a9_sensor_info_debug(rt_status, home_num);

    return 0;
}


/**
 * @Function               : printf_sensor_info_debug
 * @brief                  : 安防监控项目所有的环境信息
 * @param allArea_env_data : struct __allArea_env_data *类型，表示某一房子属性结构体变量
 * @param home_num         : int类型，表示房间编号，一般是0
 * @return                 : 
 * @Description            :
 */
int printf_a9_sensor_info_debug(struct __allArea_env_data * allArea_env_data, int home_num)
{
    /* 1.allArea_env_data->area_id打印 */
    printf("area_id = %d\n", allArea_env_data->area_id);
    /* 2.allArea_env_data->home[%d]->a9_info打印 */
    printf("home[%d]-->a9_info-->head=%s\n", home_num, allArea_env_data->home[home_num].a9_info.head);
    printf("home[%d]-->a9_info-->type=%c\n", home_num, allArea_env_data->home[home_num].a9_info.type);
    printf("home[%d]-->a9_info-->adc=%.2f\n", home_num, allArea_env_data->home[home_num].a9_info.adc);
    printf("home[%d]-->a9_info-->gyrox=%d\n", home_num, allArea_env_data->home[home_num].a9_info.gyrox);
    printf("home[%d]-->a9_info-->gyroy=%d\n", home_num, allArea_env_data->home[home_num].a9_info.gyroy);
    printf("home[%d]-->a9_info-->gyroz=%d\n", home_num, allArea_env_data->home[home_num].a9_info.gyroz);
    printf("home[%d]-->a9_info-->aacx=%d\n", home_num, allArea_env_data->home[home_num].a9_info.aacx);
    printf("home[%d]-->a9_info-->aacy=%d\n", home_num, allArea_env_data->home[home_num].a9_info.aacy);
    printf("home[%d]-->a9_info-->aacz=%d\n", home_num, allArea_env_data->home[home_num].a9_info.aacz);

    return 0;
}