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

echo "======================================================"
echo -e "\033[32m               html install Menu \033[0m"
echo -e "\033[32m                 by @qidaink        \033[0m"
echo "======================================================"
FileName=${0#*/}
current_path=$(pwd)
boa_www_path=/home/hk/4nfs/rootfs/boa/www
boa_cig_path=/home/hk/4nfs/rootfs/boa/cig-bin
html_file_dir=html
cgi_file_dir=cgi_proj

printf "[INFO ]当前文件:\033[1;34m[${FileName}]\033[0m路径为:\033[1;34m${current_path}\033[0m\n"
echo ""
printf "\033[34m *[0] \033[0m 安装 html 相关文件到开发板 /boa/www 目录\n"
printf "\033[34m *[1] \033[0m 卸载开发板 /boa/www 目录的 html 相关文件\n"
printf "\033[34m *[2] \033[0m 安装 cgi 相关文件到开发板 /boa/cgi-bin\n"
printf "\033[34m *[3] \033[0m 卸载开发板 /boa/cgi-bin 目录的相关 cgi 文件\n"
printf "\033[34m *[4] \033[0m 安装 html 和 cgi 相关文件到开发板\n"
printf "\033[34m *[5] \033[0m 卸载开发板中 html 和 cgi 相关文件\n"
echo " "

read -p "请选择需要的功能,默认选择[4],同时按装html和cgi相关文件:" answer

# [0] 安装html文件
if [ "$answer" = "0" ];  then
	cd ${html_file_dir}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]拷贝html文件到开发板(pwd=${current_path})...\033[0m\n"
    sleep 1
    make install
    exit 0
# [1]卸载html文件
elif [ "$answer" = "1" ]; then
	cd ${html_file_dir}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]从开发板卸载html文件(pwd=${current_path})...\033[0m\n"
    sleep 1
    make uninstall
	exit 0
# [2] 安装cgi文件
elif [ "$answer" = "2" ]; then
	cd ${cgi_file_dir}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]拷贝cgi文件到开发板(pwd=${current_path})...\033[0m\n"
    sleep 1
    make clean
    make ARCH=arm
    make install
    make clean
	exit 0
# [3] 卸载cgi文件
elif [ "$answer" = "3" ]; then
	cd ${cgi_file_dir}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]从开发板卸载cgi文件(pwd=${current_path})...\033[0m\n"
    sleep 1
    make uninstall
    make clean
	exit 0
# [4] 安装html和cgi
elif [ "$answer" = "4" ] || [ "$answer" = "" ]; then
	cd ${html_file_dir}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]拷贝html文件到开发板(pwd=${current_path})...\033[0m\n"
    sleep 1
    make install
    cd -
    cd ${cgi_file_dir}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]拷贝cgi文件到开发板(pwd=${current_path})...\033[0m\n"
    sleep 1
    make clean
    make ARCH=arm
    make install
    make clean
	exit 0
# [5] 卸载html和cgi
elif [ "$answer" = "5" ]; then
	cd ${html_file_dir}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]从开发板卸载html文件(pwd=${current_path})...\033[0m\n"
    sleep 1
    make uninstall
    cd -
    cd ${cgi_file_dir}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]从开发板卸载cgi文件(pwd=${current_path})...\033[0m\n"
    sleep 1
    make uninstall
    make clean
	exit 0
fi


