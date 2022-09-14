/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : set_sms.c
 * Author     : qidaink
 * Date       : 2022-09-14
 * Version    :
 * Description: 从网页端设置中心号码和接收号码
 * Others     :
 * Log        :
 * ======================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include "cgic.h"

#define MAX_SIZE 32

struct __msg
{
	long type;
	long devtype;
	char text[MAX_SIZE];
};

struct From_to_send
{
	char receive_number[12];
	char center_number[12];
} phone_NUM;

int cgiMain()
{
	/* 0.相关变量定义 */
	key_t mag_key;
	int msgid;
	char platform[2];
	struct __msg msg_buf;
	memset(&msg_buf, 0, sizeof(msg_buf));
	/* 1.拷贝相关数据 */
	cgiFormStringNoNewlines("receive", phone_NUM.receive_number, 12);
	cgiFormStringNoNewlines("center", phone_NUM.center_number, 12);
	cgiFormString("store", platform, 2);

	/* 2.创建消息队列 */
	/* 2.1生成消息队列 msg_key，注意与 fs4412-monitoring/pthread_client_request.c 中的key使用同一个字符生成 */
	if ((mag_key = ftok("/tmp", 'g')) < 0)
	{
		perror("[ERROR]ftok");
		exit(-1);
	}
	/* 2.2获取消息队列id */
	if ((msgid = msgget(mag_key, 0666)) < 0)
	{
		perror("[ERROR]msgget");
		exit(-1);
	}
	sprintf(msg_buf.text, "%s&%s&", phone_NUM.receive_number,phone_NUM.center_number);
	/* 4.发送消息到消息队列 */
	msg_buf.type = 1L;
	msg_buf.devtype = 10L;
	msgsnd(msgid, &msg_buf, sizeof(msg_buf) - sizeof(long), 0);
	/* 6.跳转网页 */
	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<HTML><HEAD>\n");
	fprintf(cgiOut, "<TITLE>My CGI</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY bgcolor=\"#fffffffff\">\n");
	fprintf(cgiOut, "<H2>setting send tel  message successfully!</H2>");
	fprintf(cgiOut, "<H3>data:%s</H3>", msg_buf.text);
	#if 0
	fprintf(cgiOut, "<a href='../home0.html'>return</a>");
	#else
	fprintf(cgiOut, "<meta http-equiv=\"refresh\" content=\"1;url=../home0.html\">");
	#endif
	fprintf(cgiOut, "</BODY></HTML>\n");
	return 0;
}
