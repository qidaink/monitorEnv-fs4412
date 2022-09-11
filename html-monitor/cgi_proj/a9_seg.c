/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : a9_seg.c
 * Author     : qidaink
 * Date       : 2022-09-06
 * Version    :
 * Description: 下发四位模拟数码管控制命令到消息队列，进而控制Cotex-A9上的LED灯
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

int cgiMain()
{
	/* 0.相关变量定义 */
	key_t msg_key;
	char seg[N]; /* html表单提交过来的数据 */
	char segvalue[N];
	char sto_no[2];						  /* 代表平台，html中全部设置为了 1 */
	int msgid;							  /* 消息队列id */
	struct __msg msg_buf;					  /* 存储消息 */
	memset(&msg_buf, 0, sizeof(msg_buf)); /* 将消息先清空 */
	/* 1. 拷贝相关数据*/
	cgiFormString("seg", seg, N);
	cgiFormString("segvalue", segvalue, N);
	cgiFormString("store", sto_no, 2);

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
	if (seg[0] == '1')
	{
		/* (1 << 6) | (3 << 4) | (1 << 0) = 0100 0000 | 0011 0000 | 0000 xxxx = 0111 xxxx = 0x7x */
		msg_buf.text[0] = ((sto_no[0] - 48)) << 6 | (3 << 4) | ((segvalue[0] - 48) << 0);
	}
	else if (seg[0] == '0')
	{
		/* (1 << 6) | (2 << 4) | (1 << 0) = 0100 0000 | 0011 0000 | 0000 1111 = 0111 1111 = 0x7f */
		msg_buf.text[0] = ((sto_no[0] - 48)) << 6 | (3 << 4) | (15 << 0);
	}
	/* 5.发送数码管控制命令到消息队列 */
	msg_buf.type = 1L;
	msg_buf.devtype = 3L;
	msgsnd(msgid, &msg_buf, sizeof(msg_buf) - sizeof(long), 0);
	/* 6.跳转网页 */
	sto_no[0] -= 48;

	cgiHeaderContentType("text/html\n\n");
	fprintf(cgiOut, "<HTML><HEAD>\n");
	fprintf(cgiOut, "<TITLE>My CGI</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY>");

	fprintf(cgiOut, "<H2>send sucess</H2>");

	// fprintf(cgiOut, "<a href='.html'>返回</a>");
	fprintf(cgiOut, "<meta http-equiv=\"refresh\" content=\"1;url=../a9_zigbee%d.html\">", sto_no[0]);
	fprintf(cgiOut, "</BODY>\n");
	fprintf(cgiOut, "</HTML>\n");

	return 0;
}
