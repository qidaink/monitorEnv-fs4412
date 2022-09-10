##============================================================================#
# Copyright © hk. 2022-2025. All rights reserved.
# File name  : Makefile
# Author     : qidaink
# Date       : 2022-08-28
# Version    : 
# Description: 驱动模块编译Makefile
# Others     : 
# Log        : 
##============================================================================#
##

# 模块名和模块测试APP名称
MODULE_NAME               :=key_input
APP_NAME                  := $(MODULE_NAME)_app
##============================================================================#
# 当前路径
CURRENT_PATH              := $(shell pwd)

# Linux源码目录
KERNELDIR                 := /home/hk/5linux/linux-3.14

# NFS 共享目录
NFS_ROOTFS                := /home/hk/4nfs/rootfs
NFS_DRIVER_DIR            := $(NFS_ROOTFS)/01myDrivers

# TFTP服务器目录
TFTP_SERVER_DIR           := /home/hk/3tftp/fs4412

# 设备树文件源码目录及设备树名称
DT_NAME                   := exynos4412-fs4412
DT_IN_LINUX_DIR           := $(KERNELDIR)/arch/arm/boot/dts
DTS_IN_LINUX_FILE         := $(DT_IN_LINUX_DIR)/$(DT_NAME).dts
DTB_IN_LINUX_FILE         := $(DT_IN_LINUX_DIR)/$(DT_NAME).dtb
DTS_IN_MODULE_FILE        := $(CURRENT_PATH)/$(DT_NAME).dts
DTB_IN_MODULE_FILE        := $(CURRENT_PATH)/$(DT_NAME).dtb
##============================================================================#
# 当前makefile中 KERNELRELEASE是没有定义的，为空，直接执行下边的内容，但是当进入linux源码后，该变量就有定义了，就会执行else后边的部分
ifeq ($(KERNELRELEASE),)

# 编译器
CFLAGS                    += -g -Wall
CROSS_COMPILE             := arm-none-linux-gnueabi-
CC                        := $(CROSS_COMPILE)gcc

##============================================================================#
# 编译模块和测试程序
modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules -j4
	@cp -v $(DTS_IN_MODULE_FILE) $(DT_IN_LINUX_DIR)
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) dtbs -j4
	@cp -v $(DTB_IN_LINUX_FILE) $(CURRENT_PATH)
	$(CC) $(CFLAGS) $(APP_NAME).c -o $(APP_NAME)

# 拷贝相关文件到nfs共享目录
install:
	@sudo cp -v $(MODULE_NAME).ko $(NFS_DRIVER_DIR)
	@sudo cp -v $(APP_NAME) $(NFS_DRIVER_DIR)
	@sudo cp -v $(DTB_IN_MODULE_FILE) $(TFTP_SERVER_DIR)

# 删除相关文件
uninstall:
	@sudo rm -vf $(NFS_DRIVER_DIR)/$(MODULE_NAME).ko 
	@sudo rm -vf $(NFS_DRIVER_DIR)/$(APP_NAME)
#	@rm -vf $(TFTP_SERVER_DIR)/$(DTB_IN_MODULE_FILE)

# 清理中间文件和生成的文件
.PHONY: clean-all clean
clean-all: clean
	@rm -vf *.ko $(APP_NAME) $(DTB_IN_MODULE_FILE) *.cgi

clean: 
#	$(MAKE) V=1 -C $(KERNELDIR) M=$(CURRENT_PATH) clean
	@rm -rvf *.o .*.cmd  *.mod.*  modules.order  Module.symvers .tmp_versions .cache.mk 

##============================================================================#
# 打印帮助命令
help:
	@echo "\033[1;32m================================ Help ================================\033[0m"
	@echo "Ubuntu may need to add sudo:"
	@echo "insmod <module_name>.ko   # Load module"
	@echo "rmmod <module_name>       # Uninstall the module"
	@echo "dmesg -C                  # Clear the kernel print information"
	@echo "lsmod                     # Check the kernel modules that have been inserted"
	@echo "dmesg                     # View information printed by the kernel"
	@echo "file <module_name>.ko     # View \".Ko\" file information"
	@echo "make ARCH=arm             # arm platform"
	@echo "\033[1;32m======================================================================\033[0m"

# 打印变量的值，调试makefile的时候用
print:
	@echo "NFS_ROOTFS                := $(NFS_ROOTFS)"
	@echo "NFS_DRIVER_DIR            := $(NFS_DRIVER_DIR)"
	@echo "TFTP_SERVER_DIR           := $(TFTP_SERVER_DIR)"
	@echo "DT_NAME                   := $(DT_NAME)"
	@echo "DT_IN_LINUX_DIR           := $(DT_IN_LINUX_DIR)"
	@echo "DTS_IN_LINUX_FILE         := $(DTS_IN_LINUX_FILE)"
	@echo "DTB_IN_LINUX_FILE         := $(DTB_IN_LINUX_FILE)"
	@echo "DTS_IN_MODULE_FILE        := $(DTS_IN_MODULE_FILE)"
	@echo "DTB_IN_MODULE_FILE        := $(DTB_IN_MODULE_FILE)"

else
CONFIG_MODULE_SIG=n
obj-m            += $(MODULE_NAME).o
endif

