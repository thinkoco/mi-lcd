//without read function
// Revision History :
// --------------------------------------------------------------------
//   Ver  :| Author            :| Mod. Date :| Changes Made:
// --------------------------------------------------------------------

module I8080_Controller #(
	I8080_BUS_WIDTH	=	32
	)(
	// global clock & reset
	clk,
	reset_n,
	
	// mm slave
	s_chipselect_n,
//	s_read,
	s_write_n,
//	s_readdata,
	s_writedata,
	s_address,
	
	i8080_cs,
	i8080_rs,//command/data
	i8080_rd,
	i8080_wr,
	i8080_data,
);
	
// global clock & reset
input	clk;
input	reset_n;

// mm slave
input		    s_chipselect_n;
//input			s_read;
input			s_write_n;
//output	reg [31:0]	s_readdata;
input	    [31:0]	s_writedata;
input	         	s_address;

output	   	i8080_cs;
output	   	i8080_rs;//command/data
output		i8080_rd;
output		i8080_wr;
output	    [I8080_BUS_WIDTH-1:0]	i8080_data;


assign i8080_cs = s_chipselect_n;
assign i8080_rs = s_address;
assign i8080_rd = 1'b1;
assign i8080_wr = s_write_n;
assign i8080_data = s_writedata[I8080_BUS_WIDTH-1:0];


endmodule
