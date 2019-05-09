# How to use

## Update zImage dtb and rbf


Get the `c5soc_opencl_lxde_fpga_reconfigurable.img` SD card Image file here [Baidu Cloud Link](https://pan.baidu.com/s/1KDyexwHD39uyvcMDm0G97A) or [Google Drive Link](https://drive.google.com/open?id=1mAYHFvOw2xtgf-e8pntFCxCGOdaYNsgG).

update the `zImage dtb rbf`  with files in MI_LCD_bins [Baidu Yun Pan Link](https://pan.baidu.com/s/1zeYMBeTREewm6KfvnTdyQQ) Code:`cde7` 


## Connector

    DE10_NANO_SoC GPIO_0  <---->  MI-LCD GPIO 
    DE1_SoC GPIO_1        <---->  MI-LCD GPIO

## MI-LCD S1,S2 and Terasic Boards MSEL Settings

|  Mode    |  S1(IM2,IM1,IM0) |    S2      |
|:---------|:----------------:|:----------:| 
|RGB24+SPI | 011(on,off,off)  |--> HSD,VSD |
|i80_24b   | 010(off,on,off)  |--> RS,RD   |

|  MSEL[4:0] |  no compress rbf | compress rbf |
|:----------:|:----------------:|:------------:| 
|  VALUE     |       00000      |    00010     |



## Build linux kernel

    tar xvf linux-4.19.18.tar.xz
    cd linux-4.19.18
    export ARCH=arm
    export CROSS_COMPILE=arm-linux-gnueabihf-
    export LOADADDR=0x8000
    make c5socl_defconfig
    make zImage -j4
    make socfpga_cyclone5_de10_nano_mi_rgb.dtb
    make socfpga_cyclone5_de10_nano_mi_i80.dtb
    make modules

