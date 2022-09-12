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

#### LED控制命令

用一个 unsigned char 类型的数据表示（8位）

```c
unsigned char led_cmd;
/** 命令说明：
 * 0x44 -- LED2关闭,   0x45 -- LED2打开
 * 0x46 -- LED3关闭,   0x47 -- LED3打开
 * 0x48 -- LED4关闭,   0x49 -- LED4打开
 * 0x4a -- LED5关闭,   0x4b -- LED5打开
 * 0x4c -- 全部LED关闭, 0x4d -- 全部LED打开
 * 0x4e -- 流水灯关闭,  0x4f -- 流水灯打开
 */
```

|位|说明|
|--|--|
| led_cmd[7:6]|平台编号，00表示Zigbee，01表示Cotex-A9|
| led_cmd[5:4]|设备编号，00--LED设备，01--BUZZER设备，10--四路模拟数码管设备，11--Zigbee风扇|
| led_cmd[3:1]|具体LED编号，010(2)--LED2，011(3)--LED3，100(4)--LED4，101(5)--LED5,110(6)--所有LED，111(7)--流水灯|
| led_cmd[0]  |LED状态，1打开，0关闭|


#### BUZZER控制命令

用一个 unsigned char 类型的数据表示（8位）

```c
unsigned char buzzer_cmd;
/** 命令说明：
 * 0x50 -- BUZZER关闭, 0x51 -- BUZZER打开
 */
```

|位|说明|
|--|--|
| buzzer_cmd[7:6]|平台编号，00表示Zigbee，01表示Cotex-A9|
| buzzer_cmd[5:4]|设备编号，00--LED设备，01--BUZZER设备，10--四路模拟数码管设备，11--Zigbee风扇|
| buzzer_cmd[3:1]|保留，默认为000|
| buzzer_cmd[0]  |BUZZER状态，1打开，0关闭|



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