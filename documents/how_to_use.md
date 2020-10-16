# How to use

## Update zImage dtb and rbf


Get the `c5soc_opencl_lxde_fpga_reconfigurable.img` SD card Image file here [Baidu Cloud Link](https://pan.baidu.com/s/1KDyexwHD39uyvcMDm0G97A) or [Google Drive Link](https://drive.google.com/open?id=1mAYHFvOw2xtgf-e8pntFCxCGOdaYNsgG).

update the `zImage dtb rbf`  with files in MI_LCD_bins [Baidu Yun Pan Link](https://pan.baidu.com/s/1zeYMBeTREewm6KfvnTdyQQ) Code:`cde7` 


## Connector

    DE10_NANO_SoC GPIO_1  <---->  MI-LCD GPIO 
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

## DE10_NANO pins
GPIO_0 <-> MIL
```
set_location_assignment PIN_V12 -to LCD_TE
set_location_assignment PIN_E8 -to TOUCH_SCK
set_location_assignment PIN_W12 -to GPIO_0[2]
set_location_assignment PIN_D11 -to TOUCH_SDA
set_location_assignment PIN_D8 -to LCD_WR_SCLK
set_location_assignment PIN_AH13 -to LCD_SDI
set_location_assignment PIN_AF7 -to TOUCH_INT
set_location_assignment PIN_AH14 -to LCD_CS
set_location_assignment PIN_AF4 -to LCD_RS_HSD
set_location_assignment PIN_AH3 -to LCD_RD_VSD
set_location_assignment PIN_AD5 -to LCD_DATA[0]
set_location_assignment PIN_AG14 -to LCD_DATA[1]
set_location_assignment PIN_AE23 -to LCD_DATA[2]
set_location_assignment PIN_AE6 -to LCD_DATA[3]
set_location_assignment PIN_AD23 -to LCD_DATA[4]
set_location_assignment PIN_AE24 -to LCD_DATA[5]
set_location_assignment PIN_D12 -to LCD_DATA[6]
set_location_assignment PIN_AD20 -to LCD_DATA[7]
set_location_assignment PIN_C12 -to LCD_DATA[8]
set_location_assignment PIN_AD17 -to LCD_DATA[9]
set_location_assignment PIN_AC23 -to LCD_DATA[10]
set_location_assignment PIN_AC22 -to LCD_DATA[11]
set_location_assignment PIN_Y19 -to LCD_DATA[12]
set_location_assignment PIN_AB23 -to LCD_DATA[13]
set_location_assignment PIN_AA19 -to LCD_DATA[14]
set_location_assignment PIN_W11 -to LCD_DATA[15]
set_location_assignment PIN_AA18 -to LCD_DATA[16]
set_location_assignment PIN_W14 -to LCD_DATA[17]
set_location_assignment PIN_Y18 -to LCD_DATA[18]
set_location_assignment PIN_Y17 -to LCD_DATA[19]
set_location_assignment PIN_AB25 -to LCD_DATA[20]
set_location_assignment PIN_AB26 -to LCD_DATA[21]
set_location_assignment PIN_Y11 -to LCD_DATA[22]
set_location_assignment PIN_AA26 -to LCD_DATA[23]
set_location_assignment PIN_AA13 -to LCD_ENABLE
set_location_assignment PIN_AA11 -to LCD_PCLK

set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to LCD_*
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to TOUCH_*


```
GPIO_1 <-> MIL
```

set_location_assignment PIN_Y15 -to LCD_TE
set_location_assignment PIN_AC24 -to TOUCH_SCK
#set_location_assignment PIN_AA15  -to GPIO_1[2]
set_location_assignment PIN_AD26 -to TOUCH_SDA
set_location_assignment PIN_AG28 -to LCD_WR_SCLK
set_location_assignment PIN_AF28 -to LCD_SDI
set_location_assignment PIN_AE25 -to TOUCH_INT
set_location_assignment PIN_AF27 -to LCD_CS
set_location_assignment PIN_AG26 -to LCD_RS_HSD
set_location_assignment PIN_AH27 -to LCD_RD_VSD
set_location_assignment PIN_AG25 -to LCD_DATA[0]
set_location_assignment PIN_AH26 -to LCD_DATA[1]
set_location_assignment PIN_AH24 -to LCD_DATA[2]
set_location_assignment PIN_AF25 -to LCD_DATA[3]
set_location_assignment PIN_AG23 -to LCD_DATA[4]
set_location_assignment PIN_AF23 -to LCD_DATA[5]
set_location_assignment PIN_AG24 -to LCD_DATA[6]
set_location_assignment PIN_AH22 -to LCD_DATA[7]
set_location_assignment PIN_AH21 -to LCD_DATA[8]
set_location_assignment PIN_AG21 -to LCD_DATA[9]
set_location_assignment PIN_AH23 -to LCD_DATA[10]
set_location_assignment PIN_AA20 -to LCD_DATA[11]
set_location_assignment PIN_AF22 -to LCD_DATA[12]
set_location_assignment PIN_AE22 -to LCD_DATA[13]
set_location_assignment PIN_AG20 -to LCD_DATA[14]
set_location_assignment PIN_AF21 -to LCD_DATA[15]
set_location_assignment PIN_AG19 -to LCD_DATA[16]
set_location_assignment PIN_AH19 -to LCD_DATA[17]
set_location_assignment PIN_AG18 -to LCD_DATA[18]
set_location_assignment PIN_AH18 -to LCD_DATA[19]
set_location_assignment PIN_AF18 -to LCD_DATA[20]
set_location_assignment PIN_AF20 -to LCD_DATA[21]
set_location_assignment PIN_AG15 -to LCD_DATA[22]
set_location_assignment PIN_AE20 -to LCD_DATA[23]
set_location_assignment PIN_AE19 -to LCD_ENABLE
set_location_assignment PIN_AE17 -to LCD_PCLK

set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to LCD_*
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to TOUCH_*

```
