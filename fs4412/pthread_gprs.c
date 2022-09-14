/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : pthread_gprs.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    :
 * Description: GPRS模块控制线程
 * Others     :
 * Log        :
 * ======================================================
 */

#include "common.h"
#include "linux_uart.h"

#define  GPRS_DEV "/dev/ttyUSB1"

extern pthread_mutex_t mutex_gprs;
extern pthread_cond_t cond_gprs;
extern char recive_phone[12];
extern char center_phone[12];

int gprs_fd = -1;
char * message = "Hello world!";

void call_tel(char *tel);
void send_message(char *tel, char *msg);

/**
 * @Function   : pthread_gprs
 * @brief      : GPRS模块控制线程
 * @param arg  : 线程创建时传递过来的参数
 * @return     : void *类型
 * @Description:
 */
void *pthread_gprs(void *arg)
{
    printf("[INFO ]pthread_gprs is running!\n");
    while (1)
    {
        pthread_mutex_lock(&mutex_gprs);
        pthread_cond_wait(&cond_gprs, &mutex_gprs);
        printf("recive:%s, center:%s\n", recive_phone, center_phone);
        if ((gprs_fd = open_port(GPRS_DEV)) < 0)
        {
            perror("[ERROR]open_port GPRS_DEV");
            return (void *)(-1);
        }
        printf("[INFO ]gprs_fd :%d.\n", gprs_fd);

        if (set_com_config(gprs_fd, 9600, 8, 'N', 1) < 0)
        {
            perror("[ERROR]set_com_config");
            return (void *)(-1);
        }
        sleep(1);
        call_tel(recive_phone);
        // send_message(recive_phone,message);
        close(gprs_fd);
        pthread_mutex_unlock(&mutex_gprs);
    }
    printf("[INFO ]pthread_gprs will exit!\n");
    exit(0);
}
/**
 * @Function   : call_tel
 * @brief      : 拨打电话
 * @param tel  : char *类型，表示要呼叫的号码
 * @return     : none
 * @Description:
 */
void call_tel(char *tel)
{
    printf("[INFO ]enter call_tel .....\n");
    /* 0.定义相关变量 */
    char buff[128] = {0};

    memset(buff, 0, 128); //测试串口是否正常工作，尽可能的多判断返回值
    /* 1.发送 AT 指令，确保GPRS模块在线，可以接受到数据 */
    strcpy(buff, "AT\n");
    write(gprs_fd, buff, strlen(buff));
    read(gprs_fd, buff, 128);
    if (strncmp(buff, "OK", 2)) /* 模块正常会回复一个OK */
    {
        printf("serial port connected success \n");
    }
    else
    {
        printf("serial port connected failed  \n");
    }
    sleep(1);
    /* 2.打电话 */
    printf("tel ......\n");
    memset(buff, 0, 128);
    sprintf(buff, "ATD%s;", recive_phone); /* ，向模块发送 ATD 命令，表示打电话 */
    strcat(buff, "\n");
    printf("buff :%s.\n", buff);
    write(gprs_fd, buff, strlen(buff));
    read(gprs_fd, buff, 128);
    printf("read from gprs 拨打:%s\n", buff);
    /* 3.挂断电话 */
    sleep(15); /* 15s之后挂断 */
    printf("挂断.....");
    memset(buff, 0, 128);
    strcpy(buff, "ATH;"); /* ATH命令表示挂断电话 */
    strcat(buff, "\n");
    write(gprs_fd, buff, strlen(buff));
    read(gprs_fd, buff, 128);
    printf("read from gprs 挂断:%s\n", buff);
    /* 3.重拨电话 */
    sleep(15);
    printf("重拨....\n");
    memset(buff, 0, 128); //测试串口是否正常工作，尽可能的多判断返回值
    strcpy(buff, "ATDL;");
    strcat(buff, "\n");
    write(gprs_fd, buff, strlen(buff));
    read(gprs_fd, buff, 128);
    printf("read from gprs 重拨:%s\n", buff);

    sleep(10);
    return;
}
/**
 * @Function   : send_message
 * @brief      : 发送短信
 * @param tel  : char * 类型，表示消息接收方号码
 * @param msg  : char * 类型，表示要发送的消息
 * @return     : none
 * @Description:
 */
void send_message(char *tel, char *msg)
{
    printf("enter send_message .....\n");
    /* 0.相关变量定义 */
    int len = 0;
    char end = 26;
    char buff[128] = {0};

    memset(buff, 0, 128);
    /* 1.测试串口是否正常工作，发送AT指令，判断是否返回OK */
    strcpy(buff, "AT\n");
    write(gprs_fd, buff, strlen(buff));
    read(gprs_fd, buff, 128);
    if (strncmp(buff, "OK", 2))
    {
        printf("[INFO ]serial port connected success!\n");
    }
    else
    {
        printf("[ERROR]serial port connected failed!\n");
    }
    usleep(10000);
    /* 2.检测SIM卡 */
    memset(buff, 0, 128);
    strcpy(buff, "AT+CPIN?\n"); //查询是否检测到SIM卡
    write(gprs_fd, buff, strlen(buff));
    read(gprs_fd, buff, 128);
    printf("[INFO ]set coding type success,SIM卡成功检测到.\n");
    usleep(10000);
    /* 3.设置短信发送方式 */
    memset(buff, 0, 128);
    strcpy(buff, "AT+CMGF=1\n");        /* 设置短信发送模式 */
    write(gprs_fd, buff, strlen(buff)); /* CMGF :0(默认)：PDU模式 1：文本模式 */
    read(gprs_fd, buff, 128);
    if (strncmp(buff, "OK", 2))
    {
        printf("[INFO ]set message type success!\n");
    }
    else
    {
        printf("[ERROR]set message type failed!\n");
    }
    usleep(100000);
    /* 4.查询短信编码方式 */
    memset(buff, 0, 128);
    strcpy(buff, "AT+CSCS?\n");         /* 查询短信编码方式 , CSCS编码设置 短信相关常用主要是GSM、UCS2编码格式 */
    write(gprs_fd, buff, strlen(buff)); /* 模块会根据SIM卡自动设置短消息中心号码，只做查询即可 */
    read(gprs_fd, buff, 128);
    printf("the coding type is \n");
    usleep(100000);
    /* 5.设置短信显示方式 */
    memset(buff, 0, 128);
    strcpy(buff, "AT+CSMP=17,167,0,240\n"); //参数4：241 短信存在sim卡中 240直接在终端显示
    write(gprs_fd, buff, strlen(buff));
    read(gprs_fd, buff, 128);
    usleep(10000);
    printf("[INFO ]message save at sim card!\n");
    usleep(100000);

    memset(buff, 0, 128);
    strcpy(buff, "AT+CMGDA=6\n"); //删除之前发送的信息与指令
    write(gprs_fd, buff, strlen(buff));
    read(gprs_fd, buff, 128);
    if (strncmp(buff, "OK", 2))
    {
        printf("[INFO ]delect message success!\n");
    }
    else
    {
        printf("[ERROR]delect failed!\n");
    }
    /* 发送短信 */
    memset(buff, 0, 128);
    printf("[INFO ]recive_phone :%s strlen %d sizeof %d.\n", recive_phone, strlen(recive_phone), sizeof(recive_phone) / sizeof(recive_phone[0]));
    sprintf(buff, "AT+CMGS=\"%s\"", recive_phone); //发送短信
    strcat(buff, "\n");
    write(gprs_fd, buff, strlen(buff));
    read(gprs_fd, buff, 128);
    if (strncmp(buff, ">", 1))
    {
        if (message == '\0')
        {
            printf("[INFO ]please input the message information :");
            scanf("%s", buff);
            // strcpy(buff,"hello boy,come on!.\n");
            write(gprs_fd, buff, strlen(buff));
            read(gprs_fd, buff, 128);
            write(gprs_fd, &end, 1);
            read(gprs_fd, buff, 128);
        }
        else
        {
            strcpy(buff, message);
            write(gprs_fd, buff, strlen(buff));
            read(gprs_fd, buff, 128);
            write(gprs_fd, &end, 1);
            read(gprs_fd, buff, 128);
        }

        len = strlen(buff);
        if (strncmp(&buff[len - 1], "O", 1) && strncmp(&buff[len], "K", 1))
        {
            printf("[INFO ]send message success :%s\n", msg);
        }
        else
        {
            printf("[INFO ]send message failed!\n");
        }
        sleep(1);
    }
    else
    {
        printf("[ERROR]send message failed!\n");
    }
    sleep(2);
    return;
}
