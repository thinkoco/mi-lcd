// this ip generate a LOG which store in rom
// width : 1024 (unchangable)
// height: <=128

module log_generator(
 clock,
 reset,

 dout_ready,
 dout_valid,
 dout_data,
 dout_sop,
 dout_eop,
 dout_empty


);

input clock;
input reset;

input dout_ready;
output dout_valid;
output [23:0] dout_data;
output dout_sop;
output dout_eop;
output [1:0] dout_empty;



assign dout_empty = 0;

parameter BACK_COLOR        = 24'h000001; 
parameter FRONT_COLOR       = 24'hffffff; 
parameter MIF_FILE_LOCATION = "./log.mif";

parameter LOG_WIDTH        = 1024;
parameter  LOG_HEIGHT       = 114;

localparam CTRL_PKT_NUM     = 3;
localparam CTRL_PKT_HEADER  = 24'd15;
localparam DATA_PKT_HEADER  = 24'd0;

///////////////////////
// Catch ready signal
reg dout_ready_reg;
always @(posedge clock or posedge reset) begin
  if (reset) 
    dout_ready_reg <= 0;
  else
    dout_ready_reg <= dout_ready ;
end

///////////////////////
assign dout_valid = dout_ready_reg & !fifo_almost_empty;

//////// output data path -- fifo //////////

wire [3:0] dout_data_type_w;
wire fifo_almost_full;
wire fifo_almost_empty;
log_fifo log_fifo_inst(
	.clock(clock),
	.aclr(reset),
	.data({sop,eop,dout_data_type}),
	.wrreq(fifo_wr),
	.almost_full(fifo_almost_full),
	.almost_empty(fifo_almost_empty),
	.q({dout_sop,dout_eop,dout_data_type_w}),
	.rdreq(dout_valid)
);


assign dout_data = (dout_data_type_w[3:1] == 3'd0) ? ((dout_data_type_w[0] == 1'b1)?FRONT_COLOR:BACK_COLOR):
                   (dout_data_type_w[3:1] == 3'd1) ? CTRL_PKT_HEADER:
                   (dout_data_type_w[3:1] == 3'd2) ? { 4'b0,  w_LOG_WIDTH[ 7: 4], 4'b0,  w_LOG_WIDTH[11: 8], 4'b0,  w_LOG_WIDTH[15:12] }:
                   (dout_data_type_w[3:1] == 3'd3) ? { 4'b0,  w_LOG_HEIGHT[11: 8], 4'b0, w_LOG_HEIGHT[15:12], 4'b0,  w_LOG_WIDTH[ 3: 0] }:
                   (dout_data_type_w[3:1] == 3'd4) ? { 4'b0,            4'b0, 4'b0, w_LOG_HEIGHT[ 3: 0], 4'b0, w_LOG_HEIGHT[ 7: 4] }:
                   (dout_data_type_w[3:1] == 3'd5) ? DATA_PKT_HEADER:
						 ((dout_data_type_w[0] == 1'b1)?FRONT_COLOR:BACK_COLOR);



reg [11:0] dot_cnt;
reg [11:0] line_cnt;



///////////////////////
// State Machine
//reg     out;
reg  [1:0]   pkt_state;
        
localparam STATE_CTRL_PKT_SOP = 0;
localparam STATE_CTRL_PKT_DAT = 1;
localparam STATE_DATA_PKT_SOP = 2;
localparam STATE_DATA_PKT_DAT = 3;



wire ctrl_pkt_sop = (pkt_state == STATE_CTRL_PKT_SOP ) ? 1 : 0 ;
wire ctrl_pkt_eop = ((pkt_state == STATE_CTRL_PKT_DAT) & (dot_cnt==(CTRL_PKT_NUM-1))  ) ? 1 : 0 ; 


wire data_pkt_sop = (pkt_state == STATE_DATA_PKT_SOP ) ? 1 : 0 ;
wire data_pkt_eop = ((pkt_state == STATE_DATA_PKT_DAT) & (dot_cnt==(LOG_WIDTH-1)) & (line_cnt==(LOG_HEIGHT-1))  ) ? 1 : 0 ;


always @ (posedge clock or posedge reset) begin
  if (reset) pkt_state <= STATE_CTRL_PKT_SOP; 
  else 
    case (pkt_state) // state transitions
        STATE_CTRL_PKT_SOP: if (!fifo_almost_full) pkt_state <= STATE_CTRL_PKT_DAT;
        STATE_CTRL_PKT_DAT: if (!fifo_almost_full & ctrl_pkt_eop) pkt_state <= STATE_DATA_PKT_SOP;
        STATE_DATA_PKT_SOP: if (!fifo_almost_full) pkt_state <= STATE_DATA_PKT_DAT;
        STATE_DATA_PKT_DAT: if (!fifo_almost_full & data_pkt_eop) pkt_state <= STATE_CTRL_PKT_SOP;
        default : pkt_state = STATE_CTRL_PKT_DAT;
    endcase
end

///////////////////////


/////////////////////////
// sop and eop signals
wire sop = (ctrl_pkt_sop | data_pkt_sop) ;
wire eop = (ctrl_pkt_eop | data_pkt_eop) ;

reg fifo_wr;
always @(posedge clock or posedge reset) begin
  if (reset) 
    fifo_wr <= 0;
  else
    fifo_wr <= ~fifo_almost_full ;
end

/////////////////////////
// dot and line counter
always @(posedge clock or posedge reset) begin
  if (reset) begin
    dot_cnt <= 0;
  end
  else begin
    if (!fifo_almost_full)
      if ((pkt_state == STATE_DATA_PKT_DAT) ) begin
        if ( dot_cnt < (LOG_WIDTH-1) )
          dot_cnt <= dot_cnt + 11'd1;
        else
         dot_cnt <= 0;
      end
      else if ((pkt_state == STATE_CTRL_PKT_DAT) )begin // control packet
        if ( dot_cnt < (CTRL_PKT_NUM-1) )
          dot_cnt <= dot_cnt + 11'd1;
        else
          dot_cnt <= 0;
      end
  end
end

always @(posedge clock or posedge reset) begin
  if (reset) begin
    line_cnt <= 0;
  end
  else begin
    if (!fifo_almost_full ) begin
      if (pkt_state == STATE_DATA_PKT_DAT) begin
        if ( dot_cnt == (LOG_WIDTH-1) )  begin
          if ( line_cnt < (LOG_HEIGHT-1) )
            line_cnt <= line_cnt + 11'd1;
          else
            line_cnt <= 0;
        end
      end
      else
          line_cnt <= 0;
    end
  end
end

////////////////////////////
//reading from rom
//wire [23:0] image_data= (rom_data) ? FRONT_COLOR : BACK_COLOR;

wire rom_data;
 log_rom log_rom_inst(
//	.address({line_cnt[5:0],dot_cnt[8:0]}),  //512 x 64
	.address({line_cnt[6:0],dot_cnt[9:0]}),  //1024 x 128

	.clock(clock),
	.q(rom_data));
	
defparam log_rom_inst.altsyncram_component.init_file = MIF_FILE_LOCATION;



///////////////////////
// Making Final Output Data
reg [3:0] dout_data_type;
always @(pkt_state or ctrl_data_type or rom_data ) begin
  case (pkt_state) 
     STATE_CTRL_PKT_SOP: dout_data_type = {3'd1,1'b0};//CTRL_PKT_HEADER;
     STATE_CTRL_PKT_DAT: dout_data_type = {ctrl_data_type,1'b0};//ctrl_data_type;
     STATE_DATA_PKT_SOP: dout_data_type = {3'd5,1'b0};//DATA_PKT_HEADER;
     default:            dout_data_type = {3'd0,rom_data};//image_data; 
  endcase
end
reg  [2:0] ctrl_data_type;
wire [15:0] w_LOG_WIDTH = LOG_WIDTH;
wire [15:0] w_LOG_HEIGHT = LOG_HEIGHT;
always @(dot_cnt[3:0] or w_LOG_WIDTH or w_LOG_HEIGHT) begin
  case (dot_cnt[3:0])
    0 : ctrl_data_type = 3'd2;//{ 4'b0,  w_LOG_WIDTH[ 7: 4], 4'b0,  w_LOG_WIDTH[11: 8], 4'b0,  w_LOG_WIDTH[15:12] };
    1 : ctrl_data_type = 3'd3;//{ 4'b0, w_LOG_HEIGHT[11: 8], 4'b0, w_LOG_HEIGHT[15:12], 4'b0,  w_LOG_WIDTH[ 3: 0] };
    2 : ctrl_data_type = 3'd4;//{ 4'b0,            4'b0, 4'b0, w_LOG_HEIGHT[ 3: 0], 4'b0, w_LOG_HEIGHT[ 7: 4] };
    default : ctrl_data_type = 3'd0;//24'bx;
  endcase
end


endmodule

