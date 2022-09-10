/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : adc_dev_app.c
 * Author     : qidaink
 * Date       : 2022-08-28
 * Version    :
 * Description: adc设备驱动测试 app
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

#include "adc_dev.h"

int main(int argc, const char *argv[])
{
	/* 0.相关变量定义 */
	int fd = -1; /* 文件描述符 */
	int adc = 0;
	/* 1.判断参数个数是否合法 */
	if (argc < 2)
	{
		printf("\nusage:\n");
		printf("%s /dev/dev_name\n", argv[0]);
		return -1;
	}
	/* 2.打开字符设备 */
	if ((fd = open(argv[1], O_RDWR)) < 0) /* 默认是阻塞的 */
	{
		printf("[ERROR]open %s failed!\n", argv[1]);
		return -1;
	}
	/* 3.ADC读取测试 */
	while (1)
	{
		read(fd, &adc, 4);
		sleep(1);
		printf("adc value :%0.2fV.\n", (1.8 * adc) / 4096);
	}
	/* 4.关闭文件描述符 */
	close(fd);
	fd = -1;
	return 0;
}
