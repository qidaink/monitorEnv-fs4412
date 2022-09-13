/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : set_env_data.c
 * Author     : qidaink
 * Date       : 2022-09-13
 * Version    :
 * Description: 从网页端设置温湿度的上下限
 * Others     :
 * Log        :
 * ======================================================
 */

#include "set_env_data.h"

#define home_id 0
#define MAX_SIZE 32 /* 消息正文最大字节数 */

/* 定义消息结构体 */
struct __msg
{
	long type;					  /* 消息类型，都是1L */
	long devtype;				  /* 具体的设备消息类型 */
	char text[MAX_SIZE]; /* 消息正文，MAX_SIZE字节 */
};

int cgiMain()
{
	/* 0.相关变量定义 */
	key_t msg_key;
	char platform[2]; /* 在网页中设置为了 0 */
	char buf[4][5];
	int msgid;
	struct __msg msg_buf;

	memset(&msg_buf, 0, sizeof(msg_buf));
	cgiFormString("store", platform, 2);
	/* 1. 拷贝相关数据*/
	cgiFormString("temMAX", buf[0], 4);
	cgiFormString("temMIN", buf[1], 4);
	cgiFormString("humMAX", buf[2], 4);
	cgiFormString("humMIN", buf[3], 4);
	sprintf(&msg_buf.text[1], "&%s&%s&%s&%s", buf[0], buf[1], buf[2], buf[3]);
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
	/* 4.发送消息到消息队列 */
	msg_buf.type = 1L;
	msg_buf.devtype = 5L;
	msg_buf.text[0] = platform[0];

	msgsnd(msgid, &msg_buf, sizeof(msg_buf) - sizeof(long), 0);
	/* 6.跳转网页 */
	platform[0] -= 48;

	cgiHeaderContentType("text/html\n\n");
	fprintf(cgiOut, "<HTML><HEAD>\n");
	fprintf(cgiOut, "<TITLE>My CGI</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY>");
	fprintf(cgiOut, "<H2>send sucess</H2>");
	fprintf(cgiOut, "<H3>data:%s</H3>", &msg_buf.text[1]);
	fprintf(cgiOut, "<H3>temMAX:%s</H3>", buf[0]);
	fprintf(cgiOut, "<H3>temMIN:%s</H3>", buf[1]);
	fprintf(cgiOut, "<H3>humMAX:%s</H3>", buf[2]);
	fprintf(cgiOut, "<H3>humMIN:%s</H3>", buf[3]);
#if 0
	fprintf(cgiOut, "<a href='../home0.html'>return</a>");
#else
	fprintf(cgiOut, "<meta http-equiv=\"refresh\" content=\"1;url=../home%d.html\">", platform[0]);
#endif
	fprintf(cgiOut, "</BODY>\n");
	fprintf(cgiOut, "</HTML>\n");

	return 0;
}
