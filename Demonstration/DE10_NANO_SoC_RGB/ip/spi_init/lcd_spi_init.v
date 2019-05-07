`timescale 1ps / 1ps
//-------------------------------------------------------------------
//  Author  :   Welon Tank
//  Email   :   thinkoco@foxmail.com
//  Time    :   Wed 02 Jan 2019 10:00:14 AM CST
//------------------------------------------------------------------
module lcd_spi_init#(
    parameter CLK_DIV   = 32    ,   // clock frequency division
    parameter SPI_BITS  = 9     ,   // the bits number per a spi data 
    parameter WRITE_NUM = 149   ,   // number of spi writing data
    parameter DELAY_NUM = 5000      // for delay cnt, spi_sclk
    )
    (
    input   clk     ,   //  system clock
    input   rst_n   ,   //  system reset
    output  lcd_cs  ,   //  spi chip select
    output  lcd_sclk,   //  spi clock
    output  lcd_mosi    //  spi data , master out slave in
     ); 

parameter  CMD  = 1'b0;
parameter  DATA = 1'b1;
parameter  DELAY_POINTER_0 = 147;
parameter  DELAY_POINTER_1 = 148;


parameter  IDLE     = 2'b10;
parameter  SEND     = 2'b11;
parameter  WAITE    = 2'b01;
parameter  DELAY    = 2'b00;

reg                                 spi_cs      ;
reg                                 spi_mosi    ;
reg     [1                  : 0]    steps       ;
reg     [$clog2(WRITE_NUM)-1: 0]    lut_addr    ;
reg     [SPI_BITS-1         : 0]    lut_data    ;
reg     [SPI_BITS-1         : 0]    spi_shift   ;
reg     [$clog2(SPI_BITS)-1 : 0]    spi_cnt     ;
reg     [$clog2(DELAY_NUM)-1: 0]    delay_cnt   ;
reg     [$clog2(CLK_DIV)-1  : 0]    clk_cnt     ;

wire   spi_sck;

assign lcd_sclk =   clk_cnt[$clog2(CLK_DIV)  - 1];
assign lcd_cs   =   spi_cs;
assign lcd_mosi =   spi_mosi;

assign spi_sck  =   clk_cnt[$clog2(CLK_DIV)  - 1];


always@ (posedge clk or negedge rst_n )
if(!rst_n)
    clk_cnt<= {($clog2(CLK_DIV)){1'b0}};
else
    clk_cnt<=clk_cnt+1'b1;  


always@(negedge spi_sck or negedge rst_n)
begin
    if(!rst_n) begin
        lut_addr    <=  {($clog2(WRITE_NUM)){1'b0}} ;
        steps       <=  DELAY                       ;
        spi_cnt     <=  SPI_BITS                    ;
        spi_cs      <=  1'b1                        ;
        spi_mosi    <=  1'b0                        ;
        spi_shift   <=  {SPI_BITS{1'b0}}            ;
        delay_cnt   <=  {($clog2(DELAY_NUM)){1'b0}} ;
    end else
        begin
            case(steps)
            IDLE    : begin
                        spi_cs      <= 1'b1 ;
                        if(lut_addr < WRITE_NUM) begin
                            spi_cnt     <= SPI_BITS                     ;
                            spi_shift   <= lut_data                     ;
                            steps       <= SEND                         ;
                            delay_cnt   <= {($clog2(DELAY_NUM)){1'b0}}  ;
                        end else begin
                            steps       <= IDLE ;
                       end
                    end
            SEND    : begin
                        if(spi_cnt > 0) begin
                            spi_cs                      <= 1'b0                     ;
                            spi_mosi                    <= spi_shift[SPI_BITS - 1]  ;
                            spi_shift[SPI_BITS-1 : 1]   <= spi_shift[SPI_BITS-2:0]  ;
                            spi_cnt                     <= spi_cnt - 1'b1           ;
                            steps                       <= SEND                     ;
                        end else begin
                            steps       <=  WAITE   ;
                            spi_cs      <=  1'b1    ;
                        end
                    end
            WAITE   : begin
                        lut_addr    <=  lut_addr + 1'b1 ;
                        spi_cs      <=  1'b1            ;
                        if((lut_addr == DELAY_POINTER_0 - 1) || (lut_addr == DELAY_POINTER_1 - 1)) begin
                            steps       <= DELAY;  
                        end else begin
                            steps       <= IDLE ;
                        end 
                    end
            DELAY   : begin
                        spi_cs  <= 1'b1;
                        if(delay_cnt < DELAY_NUM) begin
                            delay_cnt   <= delay_cnt + 1'b1;
                            steps       <= DELAY;
                        end else begin
                            steps       <= IDLE ;
                        end
                    end               
            default :  begin
                        lut_addr    <=  {($clog2(WRITE_NUM)){1'b0}} ;
                        steps       <=  DELAY                       ;
                        spi_cnt     <=  SPI_BITS                    ;
                        spi_cs      <=  1'b1                        ;
                        spi_mosi    <=  1'b0                        ;
                        spi_shift   <=  {SPI_BITS{1'b0}}            ;
                       end
            endcase
        end
end

always @(*)
begin
    case(lut_addr)
    0   :   lut_data    <=  {CMD    , 8'hFF};
    1   :   lut_data    <=  {DATA   , 8'hFF};
    2   :   lut_data    <=  {DATA   , 8'h98};
    3   :   lut_data    <=  {DATA   , 8'h06};
    4   :   lut_data    <=  {CMD    , 8'hBA};
    5   :   lut_data    <=  {DATA   , 8'hE0};
    6   :   lut_data    <=  {CMD    , 8'hBC};
    7   :   lut_data    <=  {DATA   , 8'h03};
    8   :   lut_data    <=  {DATA   , 8'h0F};
    9   :   lut_data    <=  {DATA   , 8'h63};
    10  :   lut_data    <=  {DATA   , 8'h69};
    11  :   lut_data    <=  {DATA   , 8'h01};
    12  :   lut_data    <=  {DATA   , 8'h01};
    13  :   lut_data    <=  {DATA   , 8'h1B};
    14  :   lut_data    <=  {DATA   , 8'h11};
    15  :   lut_data    <=  {DATA   , 8'h70};
    16  :   lut_data    <=  {DATA   , 8'h73};
    17  :   lut_data    <=  {DATA   , 8'hFF};
    18  :   lut_data    <=  {DATA   , 8'hFF};
    19  :   lut_data    <=  {DATA   , 8'h08};
    20  :   lut_data    <=  {DATA   , 8'h09};
    21  :   lut_data    <=  {DATA   , 8'h05};
    22  :   lut_data    <=  {DATA   , 8'h00};
    23  :   lut_data    <=  {DATA   , 8'hEE};
    24  :   lut_data    <=  {DATA   , 8'hE2};
    25  :   lut_data    <=  {DATA   , 8'h01};
    26  :   lut_data    <=  {DATA   , 8'h00};
    27  :   lut_data    <=  {DATA   , 8'hC1};
    28  :   lut_data    <=  {CMD    , 8'hBD};
    29  :   lut_data    <=  {DATA   , 8'h01};
    30  :   lut_data    <=  {DATA   , 8'h23};
    31  :   lut_data    <=  {DATA   , 8'h45};
    32  :   lut_data    <=  {DATA   , 8'h67};
    33  :   lut_data    <=  {DATA   , 8'h01};
    34  :   lut_data    <=  {DATA   , 8'h23};
    35  :   lut_data    <=  {DATA   , 8'h45};
    36  :   lut_data    <=  {DATA   , 8'h67};
    37  :   lut_data    <=  {CMD    , 8'hBE};
    38  :   lut_data    <=  {DATA   , 8'h00};
    39  :   lut_data    <=  {DATA   , 8'h22};
    40  :   lut_data    <=  {DATA   , 8'h27};
    41  :   lut_data    <=  {DATA   , 8'h6A};
    42  :   lut_data    <=  {DATA   , 8'hBC};
    43  :   lut_data    <=  {DATA   , 8'hD8};
    44  :   lut_data    <=  {DATA   , 8'h92};
    45  :   lut_data    <=  {DATA   , 8'h22};
    46  :   lut_data    <=  {DATA   , 8'h22};
    47  :   lut_data    <=  {CMD    , 8'hC7};
    48  :   lut_data    <=  {DATA   , 8'h1E};
    49  :   lut_data    <=  {CMD    , 8'hED};
    50  :   lut_data    <=  {DATA   , 8'h7F};
    51  :   lut_data    <=  {DATA   , 8'h0F};
    52  :   lut_data    <=  {DATA   , 8'h00};
    53  :   lut_data    <=  {CMD    , 8'hC0};
    54  :   lut_data    <=  {DATA   , 8'hE3};
    55  :   lut_data    <=  {DATA   , 8'h0B};
    56  :   lut_data    <=  {DATA   , 8'h00};
    57  :   lut_data    <=  {CMD    , 8'hFC};
    58  :   lut_data    <=  {DATA   , 8'h08};
    59  :   lut_data    <=  {CMD    , 8'hDF};
    60  :   lut_data    <=  {DATA   , 8'h00};
    61  :   lut_data    <=  {DATA   , 8'h00};
    62  :   lut_data    <=  {DATA   , 8'h00};
    63  :   lut_data    <=  {DATA   , 8'h00};
    64  :   lut_data    <=  {DATA   , 8'h00};
    65  :   lut_data    <=  {DATA   , 8'h02};
    66  :   lut_data    <=  {CMD    , 8'hF3};
    67  :   lut_data    <=  {DATA   , 8'h74};
    68  :   lut_data    <=  {CMD    , 8'hB4};
    69  :   lut_data    <=  {DATA   , 8'h00};
    70  :   lut_data    <=  {DATA   , 8'h00};
    71  :   lut_data    <=  {DATA   , 8'h00};
    72  :   lut_data    <=  {CMD    , 8'hF7};
    73  :   lut_data    <=  {DATA   , 8'h81};
    74  :   lut_data    <=  {CMD    , 8'hB1};
    75  :   lut_data    <=  {DATA   , 8'h00};
    76  :   lut_data    <=  {DATA   , 8'h10};
    77  :   lut_data    <=  {DATA   , 8'h14};
    78  :   lut_data    <=  {CMD    , 8'hF1};
    79  :   lut_data    <=  {DATA   , 8'h29};
    80  :   lut_data    <=  {DATA   , 8'h8A};
    81  :   lut_data    <=  {DATA   , 8'h07};
    82  :   lut_data    <=  {CMD    , 8'hF2};
    83  :   lut_data    <=  {DATA   , 8'h40};
    84  :   lut_data    <=  {DATA   , 8'hD2};
    85  :   lut_data    <=  {DATA   , 8'h50};
    86  :   lut_data    <=  {DATA   , 8'h28};
    87  :   lut_data    <=  {CMD    , 8'hC1};
    88  :   lut_data    <=  {DATA   , 8'h17};
    89  :   lut_data    <=  {DATA   , 8'h85};
    90  :   lut_data    <=  {DATA   , 8'h85};
    91  :   lut_data    <=  {DATA   , 8'h20};
    92  :   lut_data    <=  {CMD    , 8'hE0};
    93  :   lut_data    <=  {DATA   , 8'h00};
    94  :   lut_data    <=  {DATA   , 8'h0C};
    95  :   lut_data    <=  {DATA   , 8'h15};
    96  :   lut_data    <=  {DATA   , 8'h0D};
    97  :   lut_data    <=  {DATA   , 8'h0F};
    98  :   lut_data    <=  {DATA   , 8'h0C};
    99  :   lut_data    <=  {DATA   , 8'h07};
    100 :   lut_data    <=  {DATA   , 8'h05};
    101 :   lut_data    <=  {DATA   , 8'h07};
    102 :   lut_data    <=  {DATA   , 8'h0B};
    103 :   lut_data    <=  {DATA   , 8'h10};
    104 :   lut_data    <=  {DATA   , 8'h10};
    105 :   lut_data    <=  {DATA   , 8'h0D};
    106 :   lut_data    <=  {DATA   , 8'h17};
    107 :   lut_data    <=  {DATA   , 8'h0F};
    108 :   lut_data    <=  {DATA   , 8'h00};
    109 :   lut_data    <=  {CMD    , 8'hE1};
    110 :   lut_data    <=  {DATA   , 8'h00};
    111 :   lut_data    <=  {DATA   , 8'h0D};
    112 :   lut_data    <=  {DATA   , 8'h15};
    113 :   lut_data    <=  {DATA   , 8'h0E};
    114 :   lut_data    <=  {DATA   , 8'h10};
    115 :   lut_data    <=  {DATA   , 8'h0D};
    116 :   lut_data    <=  {DATA   , 8'h08};
    117 :   lut_data    <=  {DATA   , 8'h06};
    118 :   lut_data    <=  {DATA   , 8'h07};
    119 :   lut_data    <=  {DATA   , 8'h0C};
    120 :   lut_data    <=  {DATA   , 8'h11};
    121 :   lut_data    <=  {DATA   , 8'h11};
    122 :   lut_data    <=  {DATA   , 8'h0E};
    123 :   lut_data    <=  {DATA   , 8'h17};
    124 :   lut_data    <=  {DATA   , 8'h0F};
    125 :   lut_data    <=  {DATA   , 8'h00};
    126 :   lut_data    <=  {CMD    , 8'h35};
    127 :   lut_data    <=  {DATA   , 8'h00};
    128 :   lut_data    <=  {CMD    , 8'h36};
    129 :   lut_data    <=  {DATA   , 8'h22};
    130 :   lut_data    <=  {CMD    , 8'h2A};
    131 :   lut_data    <=  {DATA   , 8'h00};
    132 :   lut_data    <=  {DATA   , 8'h00};
    133 :   lut_data    <=  {DATA   , 8'h03};
    134 :   lut_data    <=  {DATA   , 8'h55};
    135 :   lut_data    <=  {CMD    , 8'h37};
    136 :   lut_data    <=  {DATA   , 8'h00};
    137 :   lut_data    <=  {DATA   , 8'h10};
    138 :   lut_data    <=  {CMD    , 8'h2B};
    139 :   lut_data    <=  {DATA   , 8'h00};
    140 :   lut_data    <=  {DATA   , 8'h00};
    141 :   lut_data    <=  {DATA   , 8'h01};
    142 :   lut_data    <=  {DATA   , 8'hDF};
    143 :   lut_data    <=  {CMD    , 8'h3A};
    144 :   lut_data    <=  {DATA   , 8'h77};
    145 :   lut_data    <=  {CMD    , 8'hB6};
    146 :   lut_data    <=  {DATA   , 8'hA2};
    147 :   lut_data    <=  {CMD    , 8'h11};
    148 :   lut_data    <=  {CMD    , 8'h29};
/*
    149 :   lut_data    <=  {DATA   , 8'h};
    150 :   lut_data    <=  {DATA   , 8'h};
    151 :   lut_data    <=  {DATA   , 8'h};
    152 :   lut_data    <=  {CMD    , 8'h};
    153 :   lut_data    <=  {DATA   , 8'h};
    154 :   lut_data    <=  {DATA   , 8'h};
    155 :   lut_data    <=  {DATA   , 8'h};
    156 :   lut_data    <=  {CMD    , 8'h};
    157 :   lut_data    <=  {DATA   , 8'h};
    158 :   lut_data    <=  {CMD    , 8'h};
    159 :   lut_data    <=  {DATA   , 8'h};
*/
    default : lut_data  <={SPI_BITS{1'b0}};  
    endcase
end
endmodule
