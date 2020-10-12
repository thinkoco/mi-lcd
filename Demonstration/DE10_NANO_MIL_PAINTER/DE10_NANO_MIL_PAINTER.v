//`define ENABLE_HPS
module DE10_NANO_MIL_PAINTER(

      ///////// ADC /////////
      output             ADC_CONVST           ,
      output             ADC_SCK              ,
      output             ADC_SDI              ,
      input              ADC_SDO              ,

      ///////// ARDUINO /////////
      inout       [15:0] ARDUINO_IO           ,
      inout              ARDUINO_RESET_N      ,

      ///////// FPGA /////////
      input              FPGA_CLK1_50         ,
      input              FPGA_CLK2_50         ,
      input              FPGA_CLK3_50         ,

      ///////// SDRAM <-> GPIO_0 /////////
      output             SDRAM_CLK            ,
      //output             SDRAM_CKE          ,
      output      [12:0] SDRAM_A              ,
      output       [1:0] SDRAM_BA             ,
      inout       [15:0] SDRAM_DQ             ,
      //output             SDRAM_DQML         ,
      //output             SDRAM_DQMH         ,
      output             SDRAM_nCS            ,
      output             SDRAM_nCAS           ,
      output             SDRAM_nRAS           ,
      output             SDRAM_nWE            ,
      ///////// LCD <-> GPIO_1 /////////
      input              LCD_TE               ,
      inout              LCD_SDI              ,
      output             LCD_WR_SCLK          ,
      output             LCD_CS               ,
      output             LCD_RS_HSD           ,
      output             LCD_RD_VSD           ,
      inout     [23:0]   LCD_DATA             ,
      output             LCD_ENABLE           ,
      output             LCD_PCLK             ,
      output             TOUCH_SCK            ,
      inout              TOUCH_SDA            ,
      input              TOUCH_INT            ,

      ///////// HDMI /////////
      inout              HDMI_I2C_SCL         ,
      inout              HDMI_I2C_SDA         ,
      inout              HDMI_I2S             ,
      inout              HDMI_LRCLK           ,
      inout              HDMI_MCLK            ,
      inout              HDMI_SCLK            ,
      output             HDMI_TX_CLK          ,
      output      [23:0] HDMI_TX_D            ,
      output             HDMI_TX_DE           ,
      output             HDMI_TX_HS           ,
      input              HDMI_TX_INT          ,
      output             HDMI_TX_VS           ,

`ifdef ENABLE_HPS
      ///////// HPS /////////
      inout              HPS_CONV_USB_N       ,
      output      [14:0] HPS_DDR3_ADDR        ,
      output      [2:0]  HPS_DDR3_BA          ,
      output             HPS_DDR3_CAS_N       ,
      output             HPS_DDR3_CKE         ,
      output             HPS_DDR3_CK_N        ,
      output             HPS_DDR3_CK_P        ,
      output             HPS_DDR3_CS_N        ,
      output      [3:0]  HPS_DDR3_DM          ,
      inout       [31:0] HPS_DDR3_DQ          ,
      inout       [3:0]  HPS_DDR3_DQS_N       ,
      inout       [3:0]  HPS_DDR3_DQS_P       ,
      output             HPS_DDR3_ODT         ,
      output             HPS_DDR3_RAS_N       ,
      output             HPS_DDR3_RESET_N     ,
      input              HPS_DDR3_RZQ         ,
      output             HPS_DDR3_WE_N        ,
      output             HPS_ENET_GTX_CLK     ,
      inout              HPS_ENET_INT_N       ,
      output             HPS_ENET_MDC         ,
      inout              HPS_ENET_MDIO        ,
      input              HPS_ENET_RX_CLK      ,
      input       [3:0]  HPS_ENET_RX_DATA     ,
      input              HPS_ENET_RX_DV       ,
      output      [3:0]  HPS_ENET_TX_DATA     ,
      output             HPS_ENET_TX_EN       ,
      inout              HPS_GSENSOR_INT      ,
      inout              HPS_I2C0_SCLK        ,
      inout              HPS_I2C0_SDAT        ,
      inout              HPS_I2C1_SCLK        ,
      inout              HPS_I2C1_SDAT        ,
      inout              HPS_KEY              ,
      inout              HPS_LED              ,
      inout              HPS_LTC_GPIO         ,
      output             HPS_SD_CLK           ,
      inout              HPS_SD_CMD           ,
      inout       [3:0]  HPS_SD_DATA          ,
      output             HPS_SPIM_CLK         ,
      input              HPS_SPIM_MISO        ,
      output             HPS_SPIM_MOSI        ,
      inout              HPS_SPIM_SS          ,
      input              HPS_UART_RX          ,
      output             HPS_UART_TX          ,
      input              HPS_USB_CLKOUT       ,
      inout       [7:0]  HPS_USB_DATA         ,
      input              HPS_USB_DIR          ,
      input              HPS_USB_NXT          ,
      output             HPS_USB_STP          ,
`endif /*ENABLE_HPS*/

      ///////// KEY /////////
      input       [1:0]  KEY                  ,

      ///////// LED /////////
      output      [7:0]  LED                  ,

      ///////// SW /////////
      input       [3:0]  SW
                                                                                        ) ;


//=======================================================
//  REG/WIRE declarations
//=======================================================

    wire        clk_30m     ;

    lcd_pll  lcd_pll_inst(
               .refclk    ( FPGA_CLK2_50 ) , // refclk.clk
               .rst       ( 1'b0         ) , // reset.reset
               .outclk_0  ( clk_30m      ) , // outclk0.clk
               .outclk_1  ( LCD_PCLK     ) , // outclk1.clk
               .locked    (              )   // locked.export
              ) ;
    
    MIL_PAINTER u0(
            .reset_n                                      ( KEY[0]       ) , // clk_50_clk_in_reset.reset_n
            .clk_50                                       ( FPGA_CLK1_50 ) , // clk_50_clk_in.clk
            .pll_sdram_clk                                ( SDRAM_CLK    ) , // pll_sdram.clk
            .mister_sdram32m_conduit_end_sdram_addr       ( SDRAM_A      ) , // mister_sdram32m_conduit_end.sdram_addr
            .mister_sdram32m_conduit_end_sdram_ba         ( SDRAM_BA     ) , // .sdram_ba
            .mister_sdram32m_conduit_end_sdram_cas_n      ( SDRAM_nCAS   ) , // .sdram_cas_n
            .mister_sdram32m_conduit_end_sdram_cke        (              ) , // .sdram_cke
            .mister_sdram32m_conduit_end_sdram_cs_n       ( SDRAM_nCS    ) , // .sdram_cs_n
            .mister_sdram32m_conduit_end_sdram_dq         ( SDRAM_DQ     ) , // .sdram_dq
            .mister_sdram32m_conduit_end_sdram_dqm        (              ) , // .sdram_dqm
            .mister_sdram32m_conduit_end_sdram_ras_n      ( SDRAM_nRAS   ) , // .sdram_ras_n
            .mister_sdram32m_conduit_end_sdram_we_n       ( SDRAM_nWE    ) , // .sdram_we_n
            .vid_clk_to_the_alt_vip_itc_0                 ( clk_30m      ) , // alt_vip_itc_0_clocked_video.vid_clk
            .vid_data_from_the_alt_vip_itc_0              ( LCD_DATA     ) , // .vid_data
            .underflow_from_the_alt_vip_itc_0             (              ) , // .underflow
            .vid_datavalid_from_the_alt_vip_itc_0         ( LCD_ENABLE   ) , // .vid_datavalid
            .vid_v_sync_from_the_alt_vip_itc_0            ( ~LCD_RD_VSD  ) , // .vid_v_sync
            .vid_h_sync_from_the_alt_vip_itc_0            ( ~LCD_RS_HSD  ) , // .vid_h_sync
            .vid_f_from_the_alt_vip_itc_0                 (              ) , // .vid_f
            .vid_h_from_the_alt_vip_itc_0                 (              ) , // .vid_h
            .vid_v_from_the_alt_vip_itc_0                 (              ) , // .vid_v

            .out_port_from_the_led                        ( LEDR         ) , // led_external_connection.export
            .in_port_to_the_sw                            ( SW           ) , // sw_external_connection.export
            .in_port_to_the_key                           ( KEY[1]       ) , // key_external_connection.export

            .lcd_touch_int_external_connection_export     ( TOUCH_INT    ) , // lcd_touch_int_external_connection.export
            .i2c_opencores_0_export_scl_pad_io            ( TOUCH_SCK    ) , // i2c_opencores_0_export.scl_pad_io
            .i2c_opencores_0_export_sda_pad_io            ( TOUCH_SDA    ) , // .sda_pad_io

            .lcd_spi_external_MOSI                        ( LCD_SDI      ) , // .MOSI
            .lcd_spi_external_SCLK                        ( LCD_WR_SCLK  ) , // .SCLK
            .lcd_spi_external_SS_n                        ( LCD_CS       )   //

        );

endmodule
