module intr_capturer #(
  parameter NUM_INTR = 32
  // active high level interrupt is expected for the input of this capturer module
)(
  input                clk,
  input                rst_n,
  input [NUM_INTR-1:0] interrupt_in,
  //input [31:0]         wrdata,
  input                addr,
  input                read,
  output [31:0]        rddata
);

  reg  [NUM_INTR-1:0]  interrupt_reg;
  reg  [31:0]          readdata_with_waitstate;
  wire [31:0]          act_readdata;
  wire [31:0]          readdata_lower_intr;
  wire [31:0]          readdata_higher_intr;
  wire                 access_lower_32;
  wire                 access_higher_32;

  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) interrupt_reg <= 'b0;
    else        interrupt_reg <= interrupt_in;
    end

  generate
  if (NUM_INTR>32) begin : two_intr_reg_needed
    assign readdata_lower_intr  = interrupt_reg[31:0] & {(32){access_lower_32}};
    assign readdata_higher_intr = interrupt_reg[NUM_INTR-1:32] & {(NUM_INTR-32){access_higher_32}};
    end
  else begin : only_1_reg
    assign readdata_lower_intr  = interrupt_reg & {(NUM_INTR){access_lower_32}};
    assign readdata_higher_intr = {32{1'b0}};
    end
  endgenerate

  assign access_lower_32 = read & (addr == 0);
  assign access_higher_32 = read & (addr == 1);
  assign act_readdata = readdata_lower_intr | readdata_higher_intr;
  assign rddata = readdata_with_waitstate;

  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) readdata_with_waitstate <= 32'b0;
    else        readdata_with_waitstate <= act_readdata;
    end
  
endmodule
