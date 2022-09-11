#!/bin/sh
# =====================================================
# Copyright © hk. 2022-2025. All rights reserved.
# File name  : start.sh
# Author     : qidaink
# Date       : 2022-09-11
# Version    : 
# Description: 在开发板启动相关服务的脚本
# Others     : 暂时写一个简易的脚本，后续完成后再优化
# Log        : 
# ======================================================
##

echo -e "\033[32m======================================================\033[0m"
echo -e "\033[32m*               start project Menu                   *\033[0m"
echo -e "\033[32m*                 by @qidaink                        *\033[0m"
echo -e "\033[32m======================================================\033[0m"
FileName=${0#*/}
current_path=$(pwd)
boa_path=/boa
driver_path=/01myDrivers
video_path=/mjpg-streamer

printf "[INFO ]当前文件\033[1;34m[${FileName}]\033[0m路径为:\033[1;34m${current_path}\033[0m\n"
echo ""
printf "\033[1;34m *[0] \033[0m 插入所有相关内核模块\n"
printf "\033[1;34m *[1] \033[0m 启动boa服务器\n"
printf "\033[1;34m *[2] \033[0m 启动摄像头图像采集\n"
printf "\033[1;34m *[3] \033[0m 卸载所有相关内核模块\n"
echo " "

read -p "请选择需要的功能:" answer
# [0] 安装html文件
if [ "$answer" = "0" ];  then
	cd ${driver_path}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]插入内核模块(pwd=${current_path})...\033[0m\n"
    sleep 1
    for file in $(ls)
    do
        if [ -f "${file}" ] ; then
            filename=${file%.*}
            extension=${file##*.}
            if [ "$extension" = "ko" ];  then
                insmod ${file}
            fi
        elif [ -d "$file" ] ; then
            printf "\033[33m[WARN ]$file is dictionary!!!\033[0m\n"
        fi
        sleep 1
    done
    exit 0
# [1]启动boa服务器
elif [ "$answer" = "1" ]; then
	cd ${boa_path}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]启动boa服务器(pwd=${current_path})...\033[0m\n"
    sleep 1
    ./boa &
	exit 0
# [2] 启动摄像头图像采集
elif [ "$answer" = "2" ]; then
	cd ${video_path}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]启动摄像头采集(pwd=${current_path})...\033[0m\n"
    sleep 1
    ./start.sh &
	exit 0
# [3] 卸载所有内核模块
elif [ "$answer" = "3" ]; then
	cd ${driver_path}
    current_path=$(pwd)
    printf "\033[1;34m[INFO ]插入内核模块(pwd=${current_path})...\033[0m\n"
    sleep 1
    for file in $(ls)
    do
        if [ -f "${file}" ] ; then
            filename=${file%.*}
            extension=${file##*.}
            if [ "$extension" = "ko" ];  then
                rmmod ${filename}
            fi
        elif [ -d "$file" ] ; then
            printf "\033[33m[WARN ]$file is dictionary!!!\033[0m\n"
        fi
        sleep 1
    done
fi





# cd /01myDrivers
# # 插入内核模块
# insmod adc_dev.ko
# sleep 1
# insmod buzzer_dev.ko
# sleep 1
# insmod key_input.ko
# sleep 1
# insmod led_dev.ko
# sleep 1
# insmod mpu6050_dev.ko
# sleep 1
# ./monitorEnv &
# # 启动boa
# cd /boa
# ./boa &
# sleep 1
# # 启动摄像头视频采集
# cd /mjpg-streamer/
# ./start.sh &