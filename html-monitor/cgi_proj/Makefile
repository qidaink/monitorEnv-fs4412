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

# 目录路径设置
# 生成CGI文件的位置
CGIDIRS                    := ./out

# NFS相关目录
NFS_ROOTFS                 := /home/hk/4nfs/rootfs
NFS_BOA_WWWDIR             := $(NFS_ROOTFS)/boa/www
NFS_BOA_CGIDIR             := $(NFS_ROOTFS)/boa/cgi-bin

# 获取所有的 .c 文件名称(包含路径)
CFILES			           := $(wildcard *.c)                           # 匹配当前目录下所有.c文件
ALL_CGI_FILES			   := $(patsubst %, %, $(CFILES:.c=.cgi))       # 将.c替换为.cgi
FUNC_CGI                   := $(filter-out login.cgi cgic.cgi,$(ALL_CGI_FILES)) # 去掉两个文件，cgic.c为库文件，不需要自己生成cgi文件
LOGIN_CGI                  := login.cgi                                         # login.cgi不需要使用CGIC库
##============================================================================#
# 编译器设置
CROSS_COMPILE             := arm-none-linux-gnueabi-
CC                        := $(CROSS_COMPILE)gcc
CFLAGS                    := -g -Wall
LDFLAGS                   :=
##============================================================================#
# 生成目标文件并移动到指定目录中
all:$(FUNC_CGI) $(LOGIN_CGI)
	@mv -vf *.cgi $(CGIDIRS)

# login.c不需要cigc库
$(LOGIN_CGI) : %.cgi : %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# 编译其他需要cigc库的源文件
$(FUNC_CGI) : %.cgi : %.c
	$(CC) $(CFLAGS) $< cgic.c -o $@ $(LDFLAGS)

install:
	@sudo cp -v $(CGIDIRS)/$(LOGIN_CGI) $(NFS_BOA_CGIDIR)
	@sudo cp -v $(CGIDIRS)/$(FUNC_CGI) $(NFS_BOA_CGIDIR)

uninstall:
	@sudo rm -vf $(NFS_BOA_CGIDIR)/$(LOGIN_CGI).cgi
	@sudo rm -vf $(NFS_BOA_CGIDIR)/$(FUNC_CGI).cgi

.PHONY:clean
clean:
	rm -vf *.o *.cgi $(CGIDIRS)/*.cgi

print:
	@echo "FUNC_CGI    = $(FUNC_CGI)"
	@echo "LOGIN_CGI   = $(LOGIN_CGI)"
