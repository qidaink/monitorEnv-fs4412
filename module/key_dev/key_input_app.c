/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : key_input_app.c
 * Author     : qidaink
 * Date       : 2022-08-28
 * Version    :
 * Description:  按键驱动模块测试app
 * Others     :
 * Log        :
 * ======================================================
 */
#include <stdio.h>     /* printf */
#include <sys/types.h> /* open */
#include <sys/stat.h>  /* open */
#include <fcntl.h>     /* open */
#include <unistd.h>    /* close sleep */
#include <linux/input.h>

int main(int argc, char *argv[])
{
    /* 0.相关变量定义 */
    int fd = -1; /* 文件描述符 */
    struct input_event evt;
    /* 1.判断参数个数是否合法 */
    if (argc < 2)
    {
        printf("\nusage:\n");
        printf("%s /dev/input/eventx\n", argv[0]);
        return -1;
    }
    /* 2.打开字符设备 */
    if ((fd = open(argv[1], O_RDWR)) < 0) /* 默认是阻塞的 */
    {
        printf("open %s failed!\n", argv[1]);
        return -1;
    }
    /* 3.获取KEY3状态 */
    while (1)
    {
        if(read(fd, &evt, sizeof(evt)) < 0)
        {
            perror("read");
			return -1;
        }
        if (evt.type == EV_KEY)
        {
            switch(evt.code)
            {
                case 2: /* key2 */
                    (evt.value)?printf("KEY2 DOWN\n"):printf("KEY2 UP\n");
                    break;
                case 3: /* key3 */
                    (evt.value)?printf("KEY3 DOWN\n"):printf("KEY3 UP\n");
                    break;
                default:
                    break;
            }
            
        }
    }
    /* 4.关闭字符设备 */
    close(fd);
    fd = -1;

    return 0;
}
