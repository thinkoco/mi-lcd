#************************************************************
# THIS IS A WIZARD-GENERATED FILE.                           
#
# Version 13.0.0 Build 156 04/24/2013 Service Pack 0.dp1 SJ Full Version
#
#************************************************************

# Copyright (C) 1991-2013 Altera Corporation
# Your use of Altera Corporation's design tools, logic functions 
# and other software and tools, and its AMPP partner logic 
# functions, and any output files from any of the foregoing 
# (including device programming or simulation files), and any 
# associated documentation or information are expressly subject 
# to the terms and conditions of the Altera Program License 
# Subscription Agreement, Altera MegaCore Function License 
# Agreement, or other applicable license agreement, including, 
# without limitation, that your use is for the sole purpose of 
# programming logic devices manufactured by Altera and sold by 
# Altera or its authorized distributors.  Please refer to the 
# applicable agreement for further details.



# Clock constraints

create_clock -name "clock_50_0" -period 20.000ns [get_ports {CLOCK_50}]
create_clock -name "clock_50_1" -period 20.000ns [get_ports {CLOCK2_50}]
create_clock -name "clock_50_2" -period 20.000ns [get_ports {CLOCK3_50}]
create_clock -name "clock_50_3" -period 20.000ns [get_ports {CLOCK4_50}]


# Automatically constrain PLL and other generated clocks
derive_pll_clocks -create_base_clocks

# Automatically calculate clock uncertainty to jitter and other effects.
derive_clock_uncertainty


create_clock -name "VGA_CLK" -period "65.0 MHz" [get_ports {VGA_CLK}]

set_output_delay -max -clock clk_vga 0.220 [get_ports VGA_R*]
set_output_delay -min -clock clk_vga -1.506 [get_ports VGA_R*]
set_output_delay -max -clock clk_vga 0.212 [get_ports VGA_G*]
set_output_delay -min -clock clk_vga -1.519 [get_ports VGA_G*]
set_output_delay -max -clock clk_vga 0.264 [get_ports VGA_B*]
set_output_delay -min -clock clk_vga -1.519 [get_ports VGA_B*]
set_output_delay -max -clock clk_vga 0.215 [get_ports VGA_BLANK]
set_output_delay -min -clock clk_vga -1.485 [get_ports VGA_BLANK]


create_clock -period "27.0 MHZ" -name "LCD_CLK" [get_ports  LCD_PCLK]

# set_output_delay -clock  "LCD_CLK" 1.0  [get_ports {LCD_DATA[*]}] 
# set_output_delay -clock  "LCD_CLK" 1.0  [get_ports LCD_RS_HSD] 
# set_output_delay -clock  "LCD_CLK" 1.0  [get_ports LCD_RD_VSD] 
# set_output_delay -clock  "LCD_CLK" 1.0  [get_ports LCD_ENABLE] 

# tsu/th constraints

# tco constraints

# tpd constraints

