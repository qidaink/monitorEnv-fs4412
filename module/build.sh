#!/bin/sh
# =====================================================
# Copyright © hk. 2022-2025. All rights reserved.
# File name  : build.sh
# Author     : qidaink
# Date       : 2022-09-11
# Version    : 
# Description: 模块编译安装脚本
# Others     : 
# Log        : 
# ======================================================
##

echo "*****************************************************"
echo "just to compile the module int the current directory!"
echo "*****************************************************"

echo "======================================================"
echo -e "\033[32m               module complier Menu \033[0m"
echo -e "\033[32m                 by @qidaink        \033[0m"
echo "======================================================"
FileName=${0#*/}
currentPath=$(pwd)
nfs_path=/home/hk/4nfs/rootfs/01myDrivers
printf "[INFO ]当前文件[${FileName}]路径为:\033[32m${currentPath}\033[0m\n"

# 遍历当前目录下所有目录
for file in $PWD/*
do
	if [ -d "$file" ]
	then
		echo "$file is directory"
        printf "\033[34m[INFO ]$file is directory!\033[0m\n"
		cd $file
        printf "\033[34m[INFO ]当前文件[${FileName}]路径为:${currentPath}\033[0m\n"
        sleep 1
        printf "\033[34m[INFO ]清理之前的文件...\033[0m\n"
		make clean-all
        printf "\033[34m[INFO ]编译模块...\033[0m\n"
		make
        printf "\033[34m[INFO ]拷贝模块和测试app...\033[0m\n"
		cp *.ko  *_app ../
        printf "\033[34m[INFO ]清理中间文件...\033[0m\n"
		make clean-all
		cd -
        printf "\033[34m[INFO ]编译完成，回到顶层目录${currentPath}!!!\033[0m\n"
        sleep 3
	elif [ -f "$file" ]
	then
		printf "\033[33m[WARN ]$file is file!!!\033[0m\n"
	fi
done

echo "*******************************************************"
echo "***********  compile the module over! *****************"
echo "*Do you want to mv to the file system directory y/n?*"
echo "*******************************************************"

read ret

case ${ret} in
	"y"|"yes"|"Y"|"YES")
		echo "moving,wait a moment,please"
		sleep 1
		sudo mv -vf *.ko *_app ${nfs_path}
		echo "The move has been completed!"
		;;
	"n"|"no"|"N"|"NO")
		echo "Maybe you need to copy it by hand."
		;;
	*)
		echo "I get ${ret},maybe your input is wrong !"
		echo "There is no chance,you need to do it manually !"
		;;
esac
# --- end of case ---""""""""""
