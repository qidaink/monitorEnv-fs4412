##============================================================================#
# Copyright © hk. 2022-2025. All rights reserved.
# File name  : Makefile
# Author     : qidaink
# Date       : 2022-09-10
# Version    : 
# Description: 
# Others     : 
# Log        : 
##============================================================================#
##

##============================================================================#
# Copyright © hk. 2022-2025. All rights reserved.
# File name  : Makefile
# Author     : qidaink
# Date       : 2022-09-06
# Version    : 
# Description: 
# Others     : 
# Log        : 
##============================================================================#
##

# 目标文件
TARGET                    := login

# NFS相关目录
NFS_ROOTFS          := /home/hk/4nfs/rootfs
NFS_BOA_WWWDIR      := $(NFS_ROOTFS)/boa/www
NFS_BOA_CGIDIR      := $(NFS_ROOTFS)/boa/cgi-bin
# 编译器设置
CROSS_COMPILE             := arm-none-linux-gnueabi-
CC                        := $(CROSS_COMPILE)gcc
CFLAGS                    := -g -Wall

all:
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET).cgi

install:
	@sudo cp -v $(TARGET).cgi $(NFS_BOA_CGIDIR)

uninstall:
	@sudo rm -vf $(NFS_BOA_CGIDIR)/$(TARGET).cgi
