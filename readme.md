# monitorEnv-fs4412


## 1.FS4412

&emsp;&emsp;FS4412程序说明部分。

### 消息类型分配

|值|说明|
|--|--|
|1L           |LED控制            |
|2L           |蜂鸣器控制          |
|3L           |四路LED灯模拟的数码管|
|4L           |风扇               |
|5L           |温湿度最值设置       |
|6L，7L，8L，9L|用于个人的扩展       |
|10L          |3G通信模块-GPRS     |

### 控制命令

```
8位
		----------------------------------------
		7	6	|  5	4	|	3	2	1	0
		平台编号  |  设备编号  |	操作设备
		----------------------------------------
```

- 平台编号

|编号|说明|
|--|--|	
|0x00|0号-ZigBee平台|
|0x40|1号-A9/A53平台|
|0x80|2号-STM32平台（可以自己扩展）|
|0xc0|3号-avr arduino....保留|

- 设备编号和操作掩码

<table>
	<tr><td align="center">设备编号</td><td align="center">设备说明</td><td align="center">操作掩码</td><td align="center">掩码说明</td></tr>
	<tr><td align="left" rowspan=7>0x00	 </td><td align="left" rowspan=7>LED</td><td align="left">0x00</td><td align="left">全部关闭</td></tr>
	<tr><td align="left">0x01</td><td align="left">全部打开</td></tr>
	<tr><td align="left">0x02</td><td align="left">打开LED2</td></tr>
	<tr><td align="left">0x03</td><td align="left">打开LED3</td></tr>
	<tr><td align="left">0X04</td><td align="left">打开LED4</td></tr>
	<tr><td align="left">0x05</td><td align="left">打开LED5</td></tr>
	<tr><td align="left">0X10</td><td align="left">打开流水灯</td></tr>
	<tr><td align="left" rowspan=4>0x10	 </td><td align="left" rowspan=4>蜂鸣器</td><td align="left">0x00</td><td align="left">关闭</td></tr>
	<tr><td align="left">0x01</td><td align="left">打开</td></tr>
	<tr><td align="left">0x02</td><td align="left">自动报警关闭</td></tr>
	<tr><td align="left">0x03</td><td align="left">自动报警打开</td></tr>
	<tr><td align="left" rowspan=2>0x20	 </td><td align="left" rowspan=2>风扇</td><td align="left">0x00</td><td align="left">关闭风扇</td></tr>
	<tr><td align="left">0x01</td><td align="left">打开风扇</td></tr>		
	<tr><td align="left" rowspan=2>0x30	 </td><td align="left" rowspan=2>数码管</td><td align="left">0x0~0xF </td><td align="left">显示0~F数字<br>四盏灯，对应<br>0000-表示0,<br>0001-表示1<br>... ...<br>1110-表示14</td></tr>
	<tr><td align="left">0x0f</td><td align="left">关闭数码管led2-3-4-5</td></tr>
</table>

## 2.html网页控制

&emsp;&emsp;html网页控制说明部分。

## 3.驱动模块

&emsp;&emsp;驱动模块说明部分。


## 4.Zigbee

&emsp;&emsp;Zigbee说明部分。