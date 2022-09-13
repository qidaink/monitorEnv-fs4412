
/** =====================================================
 * Copyright © hk. 2022-2025. All rights reserved.
 * File name  : linux_uart.c
 * Author     : qidaink
 * Date       : 2022-09-12
 * Version    :
 * Description: linux下串口控制源文件
 * Others     :
 * Log        :
 * ======================================================
 */

#include "linux_uart.h"

/**
 * @Function        : set_com_config
 * @brief           : 配置串口参数
 * @param fd        : int类型，要配置的串口的文件描述符
 * @param baud_rate : int类型，要设置的波特率
 * @param baud_rate : int类型，数据位（7或者8,默认为8）
 * @param parity    : int类型，奇偶校验位
 * @param stop_bits : int类型，停止位
 * @return          : int类型，成功返回0,失败返回-1
 * @Description     :
 */
int set_com_config(int fd, int baud_rate, int data_bits, char parity, int stop_bits)
{
	/* 0.相关变量定义 */
	struct termios new_cfg, old_cfg;
	int speed;
	/* 1.保存原有串口配置 */
	if (tcgetattr(fd, &old_cfg) != 0)
	{
		perror("[ERROR]tcgetattr");
		return -1;
	}
	new_cfg = old_cfg;

	/* 2.配置为原始模式 */
	cfmakeraw(&new_cfg);
	new_cfg.c_cflag &= ~CSIZE;

	/*3.设置波特率 */
	switch (baud_rate)
	{
		case 2400:
		{
			speed = B2400;
			break;
		}
		case 4800:
		{
			speed = B4800;
			break;
		}
		case 9600:
		{
			speed = B9600;
			break;
		}
		case 19200:
		{
			speed = B19200;
			break;
		}
		case 38400:
		{
			speed = B38400;
			break;
		}

		default:
		case 115000:
		{
			speed = B115200;
			break;
		}
	}

	cfsetispeed(&new_cfg, speed);
	cfsetospeed(&new_cfg, speed);

	/* 4.设置数据位 */
	switch (data_bits)
	{
		case 7:
		{
			new_cfg.c_cflag |= CS7;
			break;
		}
		default: /* 默认也为8 */
		case 8:
		{
			new_cfg.c_cflag |= CS8;
			break;
		}
	}

	/** 5.设置奇偶校验位
	 * PARENB ：启用奇偶校验码的生成和检测功能。
	 * PARODD ：只使用奇校验而不使用偶校验。
	 * INPCK  ：对接收到的字符执行奇偶校检。
	 * CSTOPB :表示每个字符使用两位停止位。
	 */
	switch (parity)
	{
		default:
		case 'n':
		case 'N':
		{
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_iflag &= ~INPCK;
			break;
		}
		case 'o':
		case 'O':
		{
			new_cfg.c_cflag |= (PARODD | PARENB);
			new_cfg.c_iflag |= INPCK;
			break;
		}
		case 'e':
		case 'E':
		{
			new_cfg.c_cflag |= PARENB;
			new_cfg.c_cflag &= ~PARODD;
			new_cfg.c_iflag |= INPCK;
			break;
		}
		case 's':
		case 'S':
		{
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_cflag &= ~CSTOPB;
			break;
		}
	}

	/* 6.设置停止位 */
	switch (stop_bits)
	{
		default:
		case 1:
		{
			new_cfg.c_cflag &= ~CSTOPB;
			break;
		}
		case 2:
		{
			new_cfg.c_cflag |= CSTOPB;
			break;
		}
	}

	/* 7.设置等待时间和最小接收字符 */
	new_cfg.c_cc[VTIME] = 0;
	new_cfg.c_cc[VMIN] = 1;
	tcflush(fd, TCIFLUSH);
	if ((tcsetattr(fd, TCSANOW, &new_cfg)) != 0)
	{
		perror("tcsetattr");
		return -1;
	}

	return 0;
}

/**
 * @Function       : open_port
 * @brief          : 打开串口
 * @param com_port : char *类型，表示要打开的串口节点，如/dev/ttyUSB0
 * @return         : int类型，成功返回串口的文件描述符,失败返回一个负数
 * @Description    : 
 */
int open_port(char *com_port)
{
	/* 0.相关变量定义 */
	int fd;
	/* 1.打开串口 */
	fd = open(com_port, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0)
	{
		perror("[ERROR]open serial port!");
		return -1;
	}

	/* 2.恢复串口阻塞状态 */
	if (fcntl(fd, F_SETFL, 0) < 0)
	{
		perror("[ERROR]fcntl F_SETFL!\n");
	}

	/* 3.判断是否为终端设备*/
	if (isatty(fd) == 0)
	{
		perror("[ERROR]This is not a terminal device!");
	}

	return fd;
}

/*--------------------CH340Ƥ׃---------------------------*/
/**
 * @Function        : USB_UART_Config
 * @brief           : USB串口快速初始化
 * @param path      : char *类型，USB设备的节点
 * @param baud_rate : int类型，波特率
 * @return          : none
 * @Description     : 
 */
void USB_UART_Config(char *path, int baud_rate)
{
	/* 0.相关变量定义 */
	int fd;
	/* 1.打开设备 */
	fd = open_port(path);
	if (fd < 0)
	{
		printf("[ERROR]open %s failed!\n", path);
		return;
	}
	/* 2.配置串口 */
	if (set_com_config(fd, baud_rate, 8, 'N', 1) < 0)
	{
		perror("[ERROR]set_com_config");
		return;
	}
	/* 3.关闭文件描述符 */
	close(fd);
	return;
}
