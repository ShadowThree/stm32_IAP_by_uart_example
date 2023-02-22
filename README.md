## **工程简介**
1. 本项目分三个工程，分别是：
 - **QPCR_BL**: 在`MCU`端实现固件升级的`bootloader`。(`uart1`用于和`PC`端交互，接收固件；`uart3`用于打印`log`)
 - **QPCR_APP**:    实现用户功能的`userApp`。(`uart1`用于和`PC`端交互，接收升级的命令；`RTT`用于`log`)
 - **updateUI_C**:  用于下发升级命令和固件的`PC`端程序。

## **使用**
1. 通过`MDK`打开`QPCR_BL`，编译后下载到`MCU`，此时应该是红灯闪烁；(由于还没有烧录`APP`程序，现在的红灯是由`bootloader`控制的)
2. 通过`MDK`打开`QPCR_APP`，编译后下载到`MCU`，此时应该是蓝灯闪烁；(现在工作的是闪烁蓝灯的`APP`程序)
3. 修改`updateUI_C/main.c`中的串口号为实际使用的串口号，并将固件名修改为需要更新的固件名，如下：
```
#define COM_PORT        "COM6"
#define FIRMWARE_NAME   "./APP_blue_LED.bin"
```
4. 通过`"gcc .\main.c .\ymodem\common.c .\ymodem\ymodem.c -o K96.exe"`编译`updateUI_C`；
5. 然后通过`".\K96.exe"`运行并等待升级完成后，应该可以看到红灯闪烁；(现在工作的是闪烁红灯的`APP`程序)；

## **MCU上电到升级过程说明**
1. `MCU`上电后，先运行`BL`程序，对升级标志位进行判断，此时没有升级请求，所以会直接跳转到`APP`去执行；(如果跳转失败则闪烁红灯)
2. `MCU`进入`APP`后，会一直从`uart1`等待升级指令，并闪烁蓝灯；
3. `updateUI_C`工程编译后的`K96.exe`程序通过`USB`转串口向`MCU`的`uart1`发送需要升级的指令；
4. `MCU`端的`APP`程序通过`uart1`接收到升级指令后，设置升级标志位，然后重启进入`BL`程序；
5. 进入`BL`程序后，对升级标志位进行判断，此时有升级请求，所以进入升级过程；`BL`准备好接收固件后会通过`uart1`发送`'C'`；
6. 当`K96.exe`接收到`'C'`后就开始分包发送固件；
7. `BL`每接收一包数据都会回复一个`ACK`；
8. 当升级完成后，BL就会跳转到新的固件开始工作。

## **参考**
[STM32 HAL IAP example](https://www.st.com/content/my_st_com/en/products/embedded-software/mcu-mpu-embedded-software/stm32-embedded-software/stm32cube-expansion-packages/x-cube-iap-usart.license=1672305361911.product=X-CUBE-IAP-USART.version=1.0.0.html)

[IAP及Ymodem协议说明](https://blog.csdn.net/qq_29506411/article/details/127795239?spm=1001.2014.3001.5502)