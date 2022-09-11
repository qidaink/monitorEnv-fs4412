/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : env_data.c
 * Author     : qidaink
 * Date       : 2022-09-11
 * Version    :
 * Description:
 * Others     :
 * Log        :
 * ======================================================
 */
#include "env_data.h"

char status[2][6] = {"Close", "Open"};
char fan_status[4][6] = {"Close", "One", "Two", "Three"};

int cgiMain()
{
	/* 0.相关变量定义 */
	key_t key;
	int shmid, semid;
	struct __shm_addr *shm_buf;
	/* 1.创建或打开信号量 */
	/* 1.1生成key，注意要与 fs4412/pthread_transfer.c 中使用第二个参数一致才能实现两个进程通信 */
	if ((key = ftok("/tmp", 'i')) < 0)
	{
		perror("ftok");
		exit(1);
	}
	printf("key = %x\n", key);
	/* 1.2获取信号量id */
	if ((semid = semget(key, 1, 0666)) < 0)
	{
		perror("semget");
		exit(-1);
	}
	/* 2.创建或打开共享内存 */
	/* 2.1生成key，注意要与 fs4412/pthread_transfer.c 中使用第二个参数一致才能实现两个进程通信 */
	if ((shmid = shmget(key, SHM_SIZE, 0666)) == -1)
	{
		perror("shmget");
		exit(-1);
	}
	/* 2.2获取创建的共享内存首地址 */
	if ((shm_buf = (struct __shm_addr *)shmat(shmid, NULL, 0)) == (void *)-1)
	{
		perror("shmat");
		exit(-1);
	}

	sem_p(semid, 0);/* 注意这里可能会有bug，因为没有初始化 */

	cgiHeaderContentType("text/html");
	/* C语言的 %% 可以在printf中打印出% */
	fprintf(cgiOut, "<head><meta http-equiv=\"refresh\" content=\"1\"><style><!--body{line-height:50%%}--></style> </head>");
	fprintf(cgiOut, "<HTML>\n");
	fprintf(cgiOut, "<BODY bgcolor=\"#666666\">\n");
	// fprintf(cgiOut, "<h1><font color=\"#FF0000\">HOME_ID #%d:</font></H2>\n ", shm_buf->shm_status);
	if (shm_buf->shm_status == 1)
	{
		/* 加载一个时间显示脚本， 网页加载时调用一次 以后就会自动调用了 */
		/* 
			<script>   
				function show(){   
				var date = new Date(); //日期对象   
				var now = "";   
				now = date.getFullYear()+"年";        // 读英文就行了   
				now = now + (date.getMonth()+1)+"月"; // 取月的时候取的是当前月-1如果想取当前月+1就可以了   
				now = now + date.getDate()+"日";   
				now = now + date.getHours()+"时";   
				now = now + date.getMinutes()+"分";   
				now = now + date.getSeconds()+"秒";   
				document.getElementById("nowDiv").innerHTML = now; //div的html是now这个字符串   
				setTimeout("show()",1000);          //设置过1000毫秒就是1秒，调用show方法   
				}   
			</script>   
		*/
		fprintf(cgiOut, "<script>function show(){var date =new Date(); var now = \"\"; now = date.getFullYear()+\"年\"; now = now + (date.getMonth()+1)+\"月\"; now = now + date.getDate()+\"日\"; now = now + date.getHours()+\"时\"; now = now + date.getMinutes()+\"分\";now = now + date.getSeconds()+\"秒\"; document.getElementById(\"nowDiv\").innerHTML = now; setTimeout(\"show()\",1000);} </script> \n ");
		fprintf(cgiOut, "<h2><font face=\"Broadway\"><font color=\"#00FAF0\">Home1 Real-time Environment Info:</font></font></H2>\n ");
		fprintf(cgiOut, "<h2 align=center><font color=\"#cc0033\"><body onload=\"show()\"> <div id=\"nowDiv\"></div></font></h2> \n ");
		
		fprintf(cgiOut, "<h4>ZIGBEE数据显示:</H4>\n ");
		fprintf(cgiOut, "<h4>Temperature:\t%0.2f</H4>\n ", shm_buf->shm_all_env_info.home[0].zigbee_info.temperature);
		fprintf(cgiOut, "<h4>Humidity:\t%0.2f</H4>\n ", shm_buf->shm_all_env_info.home[0].zigbee_info.humidity);

		fprintf(cgiOut, "<h4>A9数据显示:</H4>\n ");
		fprintf(cgiOut, "<h4>Adc:\t%0.2f</H4>\n ", shm_buf->shm_all_env_info.home[0].a9_info.adc);
		fprintf(cgiOut, "<h4>GYROX:\t%d</H4>\n ", shm_buf->shm_all_env_info.home[0].a9_info.gyrox);
		fprintf(cgiOut, "<h4>GYROY:\t%d</H4>\n ", shm_buf->shm_all_env_info.home[0].a9_info.gyroy);
		fprintf(cgiOut, "<h4>GYROZ:\t%d</H4>\n ", shm_buf->shm_all_env_info.home[0].a9_info.gyroz);
		fprintf(cgiOut, "<h4>AACX :\t%d</H4>\n ", shm_buf->shm_all_env_info.home[0].a9_info.aacx);
		fprintf(cgiOut, "<h4>AACY :\t%d</H4>\n ", shm_buf->shm_all_env_info.home[0].a9_info.aacy);
		fprintf(cgiOut, "<h4>AACZ :\t%d</H4>\n ", shm_buf->shm_all_env_info.home[0].a9_info.aacz);
		fprintf(cgiOut, "<h4>A9-RESERVED[0]:\t%d</H4>\n ", shm_buf->shm_all_env_info.home[0].a9_info.reserved[0]);
		fprintf(cgiOut, "<h4>A9-RESERVED[1]:\t%d</H4>\n ", shm_buf->shm_all_env_info.home[0].a9_info.reserved[1]);

		fprintf(cgiOut, "<h4>扩展平台数据显示：</H4>\n ");
		fprintf(cgiOut, "<h4>......</H4>\n ");
	}
	else
	{
		fprintf(cgiOut, "<h2><font face=\"Broadway\"><font color=\"#FFFAF0\">Close!</font></font></H2>\n ");
	}
	//	fprintf(cgiOut, "<h3>:</H3>\n ");
	fprintf(cgiOut, "</BODY></HTML>\n");
	sem_v(semid, 0);
	return 0;
}

/**
 * @Function   : init_sem
 * @brief      : 初始化System V信号量
 * @param semid: int 类型，信号量id
 * @param num  : int 类型，要操作的信号量集中的信号量编号
 * @param val  : int 类型，该参数用于设置信号集中信号量的值
 * @return     : 初始化完成返回0
 * @Description:
 */
int init_sem(int semid, int num, int val)
{
	/* 0.相关变量定义 */
	union semun myun;
	myun.val = val;
	/* 1.设置信号量的值 */
	if (semctl(semid, num, SETVAL, myun) < 0)
	{
		perror("[ERROR]semctl");
		exit(1);
	}
	return 0;
}
/**
 * @Function   : sem_p
 * @brief      : System V信号量 P 操作
 * @param semid: int 类型，信号量id
 * @param num  : int 类型，要操作的信号量集中的信号量编号
 * @return     : 操作完成返回0
 * @Description:
 */
int sem_p(int semid, int num)
{
	struct sembuf mybuf;
	mybuf.sem_num = num;
	mybuf.sem_op = -1;
	mybuf.sem_flg = SEM_UNDO;
	if (semop(semid, &mybuf, 1) < 0)
	{
		perror("[ERROR]semop");
		exit(1);
	}

	return 0;
}
/**
 * @Function   : sem_v
 * @brief      : System V信号量 V 操作
 * @param semid: int 类型，信号量id
 * @param num  : int 类型，要操作的信号量集中的信号量编号
 * @return     : 操作完成返回0
 * @Description:
 */
int sem_v(int semid, int num)
{
	struct sembuf mybuf;
	mybuf.sem_num = num;
	mybuf.sem_op = 1;
	mybuf.sem_flg = SEM_UNDO;
	if (semop(semid, &mybuf, 1) < 0)
	{
		perror("[ERROR]semop");
		exit(1);
	}

	return 0;
}
