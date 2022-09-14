/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : pthread_dht11.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    :
 * Description: ZigBee DHT11数据采集线程
 * Others     :
 * Log        :
 * ======================================================
 */

#include "common.h"
#include "linux_uart.h"

/* 设备节点名称 */
#define TH_DEV      "/dev/ttyUSB0" /* 温湿度获取，与风扇相同 */
/* 文件描述符定义 */
int usb_fd;

extern pthread_cond_t cond_zigbee;
extern pthread_mutex_t mutex_zigbee;
extern struct __set_env html_set_env;
extern struct __allArea_env_data g_allArea_env_info; /* 安防监控项目所有的环境信息 */

int file_env_info_zigbee(struct __allArea_env_data *rt_status, int home_num, int fd);
int printf_zigbee_sensor_info_debug(struct __allArea_env_data * allArea_env_data, int home_num);
/**
 * @Function   : pthread_dht11
 * @brief      : ZigBee数据采集线程
 * @param arg  : 线程创建时传递过来的参数
 * @return     : void *类型
 * @Description:
 */
void *pthread_dht11(void *arg)
{
    printf("[INFO ]pthread_transfer is running!\n");
    /* 0.相关变量定义 */
    int home_num = 0;
    /* 1.打开设备 */
    // usb_fd = open_port(TH_DEV);
    // if (usb_fd == -1)
    // {
    //     printf("[ERROR]open  usb device failed.\n");
    // }
    // printf("open success: usb_fd=%d\n", usb_fd);
    /* 2.采集数据 */
    while (1)
    {
        usb_fd = open_port(TH_DEV);
        set_com_config(usb_fd, 115200, 8, 'N', 1);
        printf("[INFO ]zigbee-------------------->\n");
        /* 2.3获取Zigbee中的温湿度数据 */
        file_env_info_zigbee(&g_allArea_env_info, home_num, usb_fd);
        close(usb_fd);
        sleep(5);
    } 
    printf("[INFO ]pthread_dht11 will exit!\n");
    exit(0);
}

/**
 * @Function        : file_env_info_zigbee
 * @brief           : 获取Zigbee温湿度数据
 * @param rt_status : struct __allArea_env_data *类型，表示某一房子属性结构体变量
 * @param home_num  : int类型，表示房间编号，一般是0
 * @param fd        : int类型，表示USB串口的文件描述符
 * @return          : none
 * @Description     : 
 */
int file_env_info_zigbee(struct __allArea_env_data *rt_status, int home_num, int fd)
{
    /* 0.相关变量定义 */
    char temp[32] = {0};
    bzero(temp, 0);
    /* 1.发送读取命令 */
    if(write(fd, "dht11", sizeof("dht11")) < 0)
    {
        printf("[ERROR]write failed!\n");
        exit(-1);
    }
    bzero(temp, 0);
    usleep(500);
    /* 2.读取返回值 */
    #if 1
    if(read(fd, temp, sizeof(temp)) < 0)
    {
        printf("[ERROR]read failed!\n");
        exit(-1);
    }
    // printf("temp=%s\n", temp);
    #else
    strcpy(temp, "T:13;H:44");
    printf("temp=%s\n", temp);
    #endif
    /* 3.获取数值，数据格式为：  T:xx;H:xx\0  */
    if(sscanf(temp, "T:%f;H:%f", &rt_status->home[home_num].zigbee_info.temperature, 
                                 &rt_status->home[home_num].zigbee_info.humidity) < 0)
	{
		perror("[ERROR]sscanf");
		exit(-1);
	}
    bzero(temp, 0);
    /* 1.获取Zigbee数据 */
    rt_status->home[home_num].zigbee_info.head[0] = 'q';
    rt_status->home[home_num].zigbee_info.head[1] = 's';
    rt_status->home[home_num].zigbee_info.head[2] = 'm';
    rt_status->home[home_num].zigbee_info.head[3] = '\0';

    rt_status->home[home_num].zigbee_info.tempMIN = (float)html_set_env.tempMIN;
    rt_status->home[home_num].zigbee_info.tempMAX = (float)html_set_env.tempMAX;

    rt_status->home[home_num].zigbee_info.humidityMIN = (float)html_set_env.humidityMIN;
    rt_status->home[home_num].zigbee_info.humidityMAX = (float)html_set_env.humidityMAX;
    rt_status->home[home_num].zigbee_info.reserved[0] = 0.01;
    rt_status->home[home_num].zigbee_info.reserved[1] = -0.01;


    return 0;
}


/**
 * @Function               : printf_zigbee_sensor_info_debug
 * @brief                  : zigbee环境信息
 * @param allArea_env_data : struct __allArea_env_data *类型，表示某一房子属性结构体变量
 * @param home_num         : int类型，表示房间编号，一般是0
 * @return                 : 
 * @Description            :
 */
int printf_zigbee_sensor_info_debug(struct __allArea_env_data * allArea_env_data, int home_num)
{
    /* 1.allArea_env_data->area_id打印 */
    printf("area_id = %d\n", allArea_env_data->area_id);
    /* 2.allArea_env_data->home[%d]->zigbee_info打印 */
    printf("home[%d]-->zigbee_info-->head=%s\n", home_num, allArea_env_data->home[home_num].zigbee_info.head);
    printf("home[%d]-->zigbee_info-->type=%c\n", home_num, allArea_env_data->home[home_num].zigbee_info.type);
    printf("home[%d]-->zigbee_info-->temperature=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.temperature);
    printf("home[%d]-->zigbee_info-->humidity=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.humidity);
    printf("home[%d]-->zigbee_info-->tempMIN=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.tempMIN);
    printf("home[%d]-->zigbee_info-->tempMAX=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.tempMAX);
    printf("home[%d]-->zigbee_info-->humidityMIN=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.humidityMIN);
    printf("home[%d]-->zigbee_info-->humidityMAX=%.2f\n", home_num, allArea_env_data->home[home_num].zigbee_info.humidityMAX);

    return 0;
}