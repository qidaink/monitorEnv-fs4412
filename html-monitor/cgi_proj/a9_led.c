/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : a9_led.c
 * Author     : qidaink
 * Date       : 2022-09-06
 * Version    :
 * Description: 下发LED控制命令到消息队列，进而控制Cotex-A9上的LED灯
 * Others     :
 * Log        :
 * ======================================================
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "cgic.h"

#define MAX_SIZE 8  /* 消息正文最大字节数 */
#define DEBUG    0  /* 调试开关 */
/* 定义消息结构体 */
struct __msg
{
	long type;			   /* 消息类型，都是1L */
	long devtype;		   /* 具体的设备消息类型 */
	unsigned char text[MAX_SIZE]; /* 消息正文，MAX_SIZE字节 */
};

int cgiMain()
{
	/* 0.相关变量定义 */
	key_t msg_key;                        /* 创建消息队列时要用的key */
	char led_number[MAX_SIZE];            /* html表单提交过来的数据 */
	char led_status[MAX_SIZE];            /* html表单提交过来的数据 */
	char platform[2];                     /* 存放平台编号，代表平台，html中全部设置为了 1 */
	int led_status_num;                   /* 转化为数字后的led状态 */
	int msgid;                            /* 消息队列id */
	struct __msg msg_buf;                 /* 存储消息 */
	memset(&msg_buf, 0, sizeof(msg_buf)); /* 将消息先清空 */
	/* 1. 拷贝相关数据*/
	cgiFormString("led_name", led_number, MAX_SIZE);    /* 获取表单中name的值，表示led的编号*/
	cgiFormString("led_status", led_status, MAX_SIZE);  /* 获取表单中value的值，表示led的开关状态*/
	cgiFormString("store", platform, 2);/* 代表平台，html中全部设置为了 1 */
	if(sscanf(led_status, "%d", &led_status_num) < 0)
	{
		perror("[ERROR]sscanf");
		exit(-1);
	}
	/* 2.创建消息队列 */
	/* 2.1生成消息队列 msg_key，注意与 fs4412-monitoring/pthread_client_request.c 中的key使用同一个字符生成 */
	if ((msg_key = ftok("/tmp", 'g')) < 0)
	{
		perror("[ERROR]ftok");
		exit(-1);
	}
	/* 2.2获取消息队列id */
	if ((msgid = msgget(msg_key, 0666)) < 0)
	{
		perror("[ERROR]msgget");
		exit(-1);
	}
	/* 3.清空消息正文缓冲区 */
	bzero(msg_buf.text, sizeof(msg_buf.text));
	/* 4.组合命令 */
	/** LED命令解读
	 * command[7:6]:平台编号，00表示Zigbee，01表示Cotex-A9
	 * command[5:4]:设备编号，00--LED设备，01--BUZZER设备，10--四路模拟数码管设备，11--Zigbee风扇
	 * command[3:1]:具体LED编号，010(2)--LED2，011(3)--LED3，100(4)--LED4，101(5)--LED5,110(6)--所有LED，111(7)--流水灯
	 * command[0]  ：状态，1打开，0关闭
	 */
	switch(led_number[0])
	{
		case '2': /* LED2控制命令 */
			/* (1 << 6) | (0 << 4) | (2 << 1) | (0 << 0) = 01_00__010_0 = 0x44 */
			/* (1 << 6) | (0 << 4) | (2 << 1) | (1 << 0) = 01_00__010_1 = 0x45 */
			msg_buf.text[0] = ((platform[0] - 48)) << 6 | (0 << 4) | (2 << 1) | (led_status_num << 0);
			break;
		case '3': /* LED3控制命令 */
			/* (1 << 6) | (0 << 4) | (3 << 1) | (0 << 0) = 01_00__011_0 = 0x46 */
			/* (1 << 6) | (0 << 4) | (3 << 1) | (1 << 0) = 01_00__011_1 = 0x47 */
			msg_buf.text[0] = ((platform[0] - 48)) << 6 | (0 << 4) | (3 << 1) | (led_status_num << 0);
			break;
		case '4': /* LED4控制命令 */
			/* (1 << 6) | (0 << 4) | (4 << 1) | (0 << 0) = 01_00__100_0 = 0x48 */
			/* (1 << 6) | (0 << 4) | (4 << 1) | (1 << 0) = 01_00__100_1 = 0x49 */
			msg_buf.text[0] = ((platform[0] - 48)) << 6 | (0 << 4) | (4 << 1) | (led_status_num << 0);
			break;
		case '5': /* LED5控制命令 */
			/* (1 << 6) | (0 << 4) | (5 << 1) | (0 << 0) = 01_00__101_0 = 0x4a */
			/* (1 << 6) | (0 << 4) | (5 << 1) | (1 << 0) = 01_00__101_1 = 0x4b */
			msg_buf.text[0] = ((platform[0] - 48)) << 6 | (0 << 4) | (5 << 1) | (led_status_num << 0);
			break;
		case '6': /* 所有LED控制命令 */
			/* (1 << 6) | (0 << 4) | (6 << 1) | (0 << 0) = 01_00__110_0 = 0x4c */
			/* (1 << 6) | (0 << 4) | (6 << 1) | (1 << 0) = 01_00__110_1 = 0x4d */
			msg_buf.text[0] = ((platform[0] - 48)) << 6 | (0 << 4) | (6 << 1) | (led_status_num << 0);
			break;
		case '7': /* 流水灯控制命令 */
			/* (1 << 6) | (0 << 4) | (7 << 1) | (0 << 0) = 01_00__111_0 = 0x4e */
			/* (1 << 6) | (0 << 4) | (7 << 1) | (1 << 0) = 01_00__111_1 = 0x4f */
			msg_buf.text[0] = ((platform[0] - 48)) << 6 | (0 << 4) | (7 << 1) | (led_status_num << 0);
			break;
		default:
			break;
	}
	/* 5.发送LED命令到消息队列 */
	msg_buf.type = 1L;    /* home1的消息队列消息 */
	msg_buf.devtype = 1L; /* 表示是LED控制消息 */
	msgsnd(msgid, &msg_buf, sizeof(msg_buf) - sizeof(long), 0);
	/* 6.跳转网页 */
	cgiHeaderContentType("text/html\n\n");
	fprintf(cgiOut, "<HTML><HEAD>\n");
	fprintf(cgiOut, "<TITLE>My CGI</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY>");

	fprintf(cgiOut, "<H1>send sucess</H1>");
	fprintf(cgiOut, "<H2>led_number = 2 : LED2<br> \
						 led_number = 3 : LED3<br> \
						 led_number = 4 : LED4<br> \
						 led_number = 5 : LED5<br> \
						 led_number = 6 : all led(LED2-5)<br> \
						 led_number = 7 : Running water light<br> \
						 platform   = 1 : Cotex-A9<br> \
						 platform   = 0 : ZigBee \
						 </H2>");
	fprintf(cgiOut, "<H2>data:</H2>");
	fprintf(cgiOut, "<H3>led_number:%s</H3>", led_number);
	fprintf(cgiOut, "<H3>led_status:%s</H3>", led_status);
	fprintf(cgiOut, "<H3>platform:%s</H3>", platform);
	fprintf(cgiOut, "<H2>msg:</H2>");
	fprintf(cgiOut, "<H3>msg_devtype:%ld</H3>", msg_buf.devtype);
	fprintf(cgiOut, "<H3>msg_text[0]:%#x</H3>", msg_buf.text[0]);
#if DEBUG == 1
	fprintf(cgiOut, "<a href='../a9_zigbee1.html'>return</a>");
#else
	fprintf(cgiOut, "<meta http-equiv=\"refresh\" content=\"1;url=../a9_zigbee1.html\">");
#endif
	fprintf(cgiOut, "</BODY>\n");
	fprintf(cgiOut, "</HTML>\n");

	return 0;
}
