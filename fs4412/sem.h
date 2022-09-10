/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : sem.h
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    :
 * Description: System V 信号量实现
 * Others     :
 * Log        :
 * ======================================================
 */

#ifndef __SEM_H__
#define __SEM_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/* semctl第四个参数用到的联合体数据类型 */
union semun
{
	int val;			   /* Value for SETVAL */
	struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
	unsigned short *array; /* Array for GETALL, SETALL */
	struct seminfo *__buf; /* Buffer for IPC_INFO(Linux-specific) */
};


/**
 * @Function   : init_sem
 * @brief      : 初始化 System V 信号量
 * @param semid: int 类型，信号量id
 * @param num  : int 类型，要操作的信号量集中的信号量编号
 * @param val  : int 类型，该参数用于设置信号集中信号量的值
 * @return     : 初始化完成返回0
 * @Description: 
 */
int init_sem(int semid, int num, int val)
{
	/* 0.相关变量定义 */
	union semun mysem;
	mysem.val = val;
	/* 1.设置信号量的值 */
	if (semctl(semid, num, SETVAL, mysem) < 0)
	{
		perror("[ERROR]semctl");
		exit(-1);
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
		exit(-1);
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
		exit(-1);
	}
	return 0;
}

#endif
