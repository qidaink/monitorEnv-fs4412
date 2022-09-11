/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : a9_beep.c
 * Author     : qidaink
 * Date       : 2022-09-06
 * Version    :
 * Description: 下发buzzer控制命令到消息队列，进而控制Cotex-A9上的蜂鸣器
 * Others     :
 * Log        :
 * ======================================================
 */

#include <stdio.h>
#include "cgic.h"
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

#define N 8 /* 消息正文最大字节数 */

/* 定义消息结构体 */
struct __msg
{
	long type;			   /* 消息类型，都是1L */
	long devtype;		   /* 具体的设备消息类型 */
	unsigned char text[N]; /* 消息正文，N字节 */
};
/* 主函数 */
int cgiMain()
{
	/* 0.相关变量定义 */
	key_t msg_key;                            /* 创建消息队列时要用的key */
	char buf[N];                          /* html表单提交过来的数据 */
	char sto_no[2];                       /* 代表平台，html中全部设置为了 1 */
	int msgid;                            /* 消息队列id */
	struct __msg msg_buf;                   /* 存储消息 */
	memset(&msg_buf, 0, sizeof(msg_buf)); /* 将消息先清空 */
	/* 1. 拷贝相关数据*/
	cgiFormString("beep", buf, N);    /* 将网页中的beep表单数据拷贝到buf(匹配input标签中name=beep的value数据) */
	cgiFormString("store", sto_no, 2);/* 代表平台，html中全部设置为了 1 */
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
	/* 3.组合命令 */
	switch (buf[0])
	{
		case '0':
		{
			/* (1 << 6) | (1 << 4) | (0 << 0) = 0100 0000 | 0001 0000 | 0000 0000 = 0101 0000 = 0x50 */
			msg_buf.text[0] = ((sto_no[0] - 48)) << 6 | (1 << 4) | (0 << 0);/* 组合出来为： */
			break;
		}
		case '1':
		{
			/* (1 << 6) | (1 << 4) | (1 << 0) = 0100 0000 | 0001 0000 | 0000 0001 = 0101 0001 = 0x51 */
			msg_buf.text[0] = ((sto_no[0] - 48)) << 6 | (1 << 4) | (1 << 0);
			break;
		}
		case '2':
		{
			/* (1 << 6) | (1 << 4) | (2 << 0) = 0100 0000 | 0001 0000 | 0000 0010 = 0101 0010 = 0x52 */
			msg_buf.text[0] = ((sto_no[0] - 48)) << 6 | (1 << 4) | (2 << 0);
			break;
		}
		case '3':
		{
			/* (1 << 6) | (1 << 4) | (3 << 0) = 0100 0000 | 0001 0000 | 0000 0011 = 0101 0011 = 0x53 */
			msg_buf.text[0] = ((sto_no[0] - 48)) << 6 | (1 << 4) | (3 << 0);
			break;
		}
	}
	/* 4.发送蜂鸣器命令到消息队列 */
	msg_buf.type = 1L;    /* home1的消息队列消息 */
	msg_buf.devtype = 2L; /* 表示是蜂鸣器控制消息 */
	msgsnd(msgid, &msg_buf, sizeof(msg_buf) - sizeof(long), 0); /* 向消息队列发送消息 */

	/* 5.跳转网页 */
	sto_no[0] -= 48;
	cgiHeaderContentType("text/html\n\n");
	fprintf(cgiOut, "<HTML><HEAD>\n");
	fprintf(cgiOut, "<TITLE>My CGI</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY>");

	fprintf(cgiOut, "<H2>send sucess!</H2>");

	// fprintf(cgiOut, "<a href='.html'>返回</a>");
	fprintf(cgiOut, "<meta http-equiv=\"refresh\" content=\"1;url=../a9_zigbee%d.html\">", sto_no[0]);/* 跳转回原来的网页 */
	fprintf(cgiOut, "</BODY>\n");
	fprintf(cgiOut, "</HTML>\n");

	return 0;
}
