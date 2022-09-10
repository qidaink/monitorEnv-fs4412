/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : pthread_refresh.c
 * Author     : qidaink
 * Date       : 2022-09-10
 * Version    :
 * Description: 采集的数据刷新到网页处理线程
 * Others     : 我们采集到的Cotex-A9和ZigBee的数据将会从这里上传到网页
 * Log        :
 * ======================================================
 */

#include "common.h"
#include "sem.h"

#define  N 1024   /* for share memory, 共享内存大小 */

/* 共享内存区域数据结构体对象定义 */
struct __shm_addr
{
	char shm_status;                            /* shm_status可以等于home_id，用来区分共享内存数据 */
	struct __allArea_env_data shm_all_env_info; /* 共享内存区域数据 */
};

struct __shm_addr *shm_buf; /* 定义一个指针变量指向要创建的共享内存区域 */

extern int shmid;
extern int semid;

extern key_t shm_key;
extern key_t sem_key;

/**
 * @Function   : pthread_refresh
 * @brief      : 将传感器数据刷新到网页处理线程
 * @param arg  : 线程创建时传递过来的参数
 * @return     : void *类型
 * @Description:
 */
void *pthread_refresh(void *arg)
{
    printf("[INFO ]pthread_refresh is running!\n");
    /* 1.信号量初始化 */
    /* 1.1生成创建信号量对象用的key */
    if ((sem_key = ftok("/tmp", 'i')) < 0)
    {
        perror("[ERROR]ftok failed!\n");
        exit(-1);
    }
    /* 1.2获取信号量id */
    semid = semget(sem_key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1)
    {
        if (errno == EEXIST)
        {
            semid = semget(sem_key, 1, 0777);
        }
        else
        {
            perror("[ERROR]fail to semget!\n");
            exit(-1);
        }
    }
    else
    {
        init_sem(semid, 0, 1); /* 初始化信号量，信号量初始值为1 */
    }
    printf("[INFO ]semid=%d!\n", semid);
    // share memory for env_info refresh config
    /* 2.共享内存初始化 */
    /* 2.1创建共享内存key值 */
    if ((shm_key = ftok("/tmp", 'i')) < 0)
    {
        perror("[ERROR]ftok failed .\n");
        exit(-1);
    }
    /* 2.2获取共享内存id */
    shmid = shmget(shm_key, N, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1)
    {
        if (errno == EEXIST)
        {
            shmid = shmget(shm_key, N, 0777);
        }
        else
        {
            perror("[ERROR]fail to shmget");
            exit(1);
        }
    }
    // share memap
    /* 2.3映射共享内存 */
    if ((shm_buf = (struct __shm_addr *)shmat(shmid, NULL, 0)) == (void *)-1)
    {
        perror("[ERROR]fail to shmat");
        exit(1);
    }
    printf("[INFO ]shmid=%d, shm_buf=%p!\n", shmid, shm_buf);

#if 1
    bzero(shm_buf, sizeof(struct __shm_addr));
    while (1)
    {
        sem_p(semid, 0);
        shm_buf->shm_status = 1;
        // s上传数据
        sleep(1);
        sem_v(semid, 0);
    }
#endif
    printf("[INFO ]pthread_refresh will exit!\n");
    exit(0);
}
