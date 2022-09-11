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

# HTML 文件
HTML_DIRS             := .
HTML_FILE_PATH        := $(foreach dir, $(HTML_DIRS), $(wildcard $(dir)/*.html))
HTML_FILE             := $(notdir  $(HTML_FILE_PATH))
NFS_BOA_WWWDIR        := ~/4nfs/rootfs/boa/www

# images 图片文件
IMAGES_DIRS           := images \
						  pic   \
# css 样式文件
CSS_DIRS              := css
# 拷贝相关文件到NFS中的根文件系统中
install:
	@sudo cp -a -v $(IMAGES_DIRS) $(NFS_BOA_WWWDIR) 
	@sudo cp -a -v $(CSS_DIRS) $(NFS_BOA_WWWDIR) 
	@sudo cp -a -v $(HTML_FILE) $(NFS_BOA_WWWDIR) 

# 删除相关文件
uninstall:
	@for var in $(HTML_FILE); do \
		sudo rm -vf $(NFS_BOA_WWWDIR)/$$var; \
	done
	@sudo rm -rvf $(NFS_BOA_WWWDIR)/$(CSS_DIRS)
	@sudo rm -rvf $(NFS_BOA_WWWDIR)/$(IMAGES_DIRS)

# 打印一些变量信息
print:
	@echo "HTML_DIRS            = $(HTML_DIRS)"
	@echo "HTML_FILE_PATH       = $(HTML_FILE_PATH)"
	@echo "HTML_FILE            = $(HTML_FILE)"
