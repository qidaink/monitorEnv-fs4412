/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : led_dev_app.c
 * Author     : qidaink
 * Date       : 2022-08-28
 * Version    :
 * Description: LED设备驱动测试 app
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

#include "led_dev.h"

int main(int argc, char *argv[])
{
    /* 0.相关变量定义 */
    int fd = -1;     /* 文件描述符 */
    led_desc_t led;  /* LED状态结构体变量 */
    /* 1.判断参数个数是否合法 */
    /* 1.1判断参数数量 */
    if (argc < 4)
    {
        printf("\nusage:\n");
        printf("%s /dev/dev_name led_status led_name.\n", argv[0]);
        printf("\tled_status:0,OFF; 1,ON\n");
        printf("\tled_name  :2,3,4,5\n\n");
        return -1;
    }
    /* 1.2获取整数 */
    sscanf(argv[2], "%d", &led.status);
    sscanf(argv[3], "%d", &led.num);
    /* 1.3判断参数范围 */
    if ((led.num < 2) || (led.num > 5))
    {
        printf("[ERROR]led_name is invalid!\n");
        return -1;
    }
    /* 2.打开字符设备 */
    if ((fd = open(argv[1], O_RDWR)) < 0) /* 默认是阻塞的 */
    {
        printf("[ERROR]open %s failed!\n", argv[1]);
        return -1;
    }
    /* 3.控制LED */
    if(led.status) /* led.status == 1 开灯 */
        ioctl(fd, LED_DEV_ON, led.num);
    else
        ioctl(fd, LED_DEV_OFF, led.num);
    /* 4.关闭字符设备 */
    close(fd);
    fd = -1;

    return 0;
}
