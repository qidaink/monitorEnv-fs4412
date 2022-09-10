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

# 目标文件名称
TARGET                     := monitorEnv
##======================================================##
## 目录路径设置							 
# 头文件路径设置(.h)
INCDIRS 		           := .
# 源文件路径设置(.c)
SRCDIRS			           := .
# 生成的中间 .o 文件的位置
OBJDIRS                    := ./obj
# 设置源文件路径
VPATH			           := $(SRCDIRS)
# NFS 共享目录
NFS_ROOTFS                 := /home/hk/4nfs/rootfs
NFS_DRIVER_DIR             := $(NFS_ROOTFS)/01myDrivers

##======================================================##
# 生成编译器的 -I dir_path 参数 
INCLUDE			           := $(patsubst %, -I %, $(INCDIRS))
# 获取所有的 .c 文件名称(包含路径)
CFILES			           := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
# 获取所有的 .c 文件名称(不包含路径)
CFILENDIR		           := $(notdir  $(CFILES))
# 指定所有的 .S 文件生成的 .o 文件路径及 生成的 .o 文件名(包含将要存放的路径)
COBJS			           := $(patsubst %, $(OBJDIRS)/%, $(CFILENDIR:.c=.o))
##======================================================##
## 编译器及编译标志
ifeq ($(ARCH), arm)
CROSS_COMPILE             := arm-none-linux-gnueabi-
else
CROSS_COMPILE             :=
endif
CC                        := $(CROSS_COMPILE)gcc
CFLAGS                    := -g -Wall -c
LDFLAGS                   := -lpthread
##======================================================##
# 链接生成最终可执行文件
$(TARGET) : $(COBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# 隐含规则推导，汇编生成所需要的所有.o中间文件
$(COBJS) : $(OBJDIRS)/%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

##======================================================##
# 安装到根文件系统
install:
	@sudo cp -v $(TARGET) $(NFS_DRIVER_DIR)

# 从根文件系统卸载
uninstall:
	@sudo rm -vf $(NFS_DRIVER_DIR)/$(TARGET)

##======================================================##
# 清理中间文件
.PHONY: clean
clean-all: clean
	@rm -rvf $(TARGET)

clean:
	@rm -rvf *.o *.out $(OBJDIRS)/*.o

print:
	@echo "INCLUDE   = $(INCLUDE)"
	@echo "CFILES    = $(CFILES)"
	@echo "CFILENDIR = $(CFILENDIR)"
	@echo "COBJS     = $(COBJS)"


