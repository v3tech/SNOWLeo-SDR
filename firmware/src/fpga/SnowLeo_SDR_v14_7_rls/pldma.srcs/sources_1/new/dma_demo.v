`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2013/12/12 10:22:44
// Design Name: 
// Module Name: dma_demo
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////
module dma_demo(
    input mclk,
 //RF SIGNAL
    input rxiqsel,
    output rxen,
    input [11:0] rxd,
    input txclk,
    output txiqsel,
    output txen, 
    output [11:0] txd,
    output rf_reset,
    inout sil5351_scl,
    inout sil5351_sda,
 // spi from PS emio 
    output spi_clk,
    output spi_mosi,
    input spi_miso,
    output spi_cs,
 //hdmi
    output hdmi_clk_pin,
    output hdmi_vsync_pin,
    output hdmi_hsync_pin,
    output [15:0] hdmi_data_pin,
    output hdmi_data_e_pin,
 //debug
    output [2:0] gpio
);
wire [139:0] u_debug_wr;
wire wr_clk;
reg [31:0] data_wr;
reg wr_en;
wire aempt_rd;

wire adc_clk;
wire dac_clk;

wire [139:0] u_debug_rd;
wire rd_clk;
wire [31:0] data_rd;
reg rd_en;
wire [14:0] rd_fifo_cnt;

wire [31:0] pldma_mwr_0_u_ctrl0;
wire [31:0] pldma_mwr_0_u_ctrl1;
wire [31:0] pldma_mwr_0_u_ctrl2;
wire [31:0] pldma_mwr_0_u_ctrl3;
wire [31:0] pldma_mwr_0_u_ctrl4;
wire [31:0] pldma_mwr_0_u_ctrl5;

wire I2C0_SCL;
wire I2C0_SDA;
wire SPI0_MISO_I_pin;
wire SPI0_MOSI_O_pin;
wire SPI0_SCLK_O_pin;
wire SPI0_SS_O_pin;

assign pldma_mwr_0_u_ctrl5 = 32'd12;

(* BOX_TYPE = "user_black_box" *)
  cpu0
    cpu0_i (
      .processing_system7_0_MIO (  ),               // inout [53:0] processing_system7_0_MIO;
      .processing_system7_0_PS_SRSTB_pin (  ),      // input processing_system7_0_PS_SRSTB_pin;
      .processing_system7_0_PS_CLK_pin (  ),        // input processing_system7_0_PS_CLK_pin;
      .processing_system7_0_PS_PORB_pin (  ),       // input processing_system7_0_PS_PORB_pin;
      .processing_system7_0_DDR_Clk (  ),           // inout processing_system7_0_DDR_Clk;
      .processing_system7_0_DDR_Clk_n (  ),         // inout processing_system7_0_DDR_Clk_n;
      .processing_system7_0_DDR_CKE (  ),           // inout processing_system7_0_DDR_CKE;
      .processing_system7_0_DDR_CS_n (  ),          // inout processing_system7_0_DDR_CS_n;
      .processing_system7_0_DDR_RAS_n (  ),         // inout processing_system7_0_DDR_RAS_n;
      .processing_system7_0_DDR_CAS_n (  ),         // inout processing_system7_0_DDR_CAS_n;
      .processing_system7_0_DDR_WEB_pin (  ),       // output processing_system7_0_DDR_WEB_pin;
      .processing_system7_0_DDR_BankAddr (  ),      // inout [2:0] processing_system7_0_DDR_BankAddr;
      .processing_system7_0_DDR_Addr (  ),          // inout [14:0] processing_system7_0_DDR_Addr;
      .processing_system7_0_DDR_ODT (  ),           // inout processing_system7_0_DDR_ODT;
      .processing_system7_0_DDR_DRSTB (  ),         // inout processing_system7_0_DDR_DRSTB;
      .processing_system7_0_DDR_DQ (  ),            // inout [31:0] processing_system7_0_DDR_DQ;
      .processing_system7_0_DDR_DM (  ),            // inout [3:0] processing_system7_0_DDR_DM;
      .processing_system7_0_DDR_DQS (  ),           // inout [3:0] processing_system7_0_DDR_DQS;
      .processing_system7_0_DDR_DQS_n (  ),         // inout [3:0] processing_system7_0_DDR_DQS_n;
      .processing_system7_0_DDR_VRN (  ),           // inout processing_system7_0_DDR_VRN;
      .processing_system7_0_DDR_VRP (  ),           // inout processing_system7_0_DDR_VRP;

      .pldma_mwr_0_afull_pin ( afull_wr ),     // output pldma_mwr_0_afull_pin;
      .pldma_mwr_0_rst_pin ( rst_wr ),     // output pldma_mwr_0_rst_pin;
      .pldma_mwr_0_data_wr_pin ( data_wr ),     // input [31:0] pldma_mwr_0_data_wr_pin;
      .pldma_mwr_0_wr_en_pin ( wr_en ),     // input pldma_mwr_0_wr_en_pin;
      .pldma_mwr_0_wr_clk_pin ( adc_clk ),     // input pldma_mwr_0_wr_clk_pin;
      .pldma_mwr_0_u_ctrl0 ( pldma_mwr_0_u_ctrl0 ),     // output [31:0] pldma_mwr_0_u_ctrl0;
      .pldma_mwr_0_u_ctrl1 ( pldma_mwr_0_u_ctrl1 ),     // output [31:0] pldma_mwr_0_u_ctrl1;
      .pldma_mwr_0_u_ctrl2 ( pldma_mwr_0_u_ctrl2 ),     // output [31:0] pldma_mwr_0_u_ctrl2;
      .pldma_mwr_0_u_ctrl3 ( pldma_mwr_0_u_ctrl3 ),     // output [31:0] pldma_mwr_0_u_ctrl3;
      .pldma_mwr_0_u_ctrl4 ( pldma_mwr_0_u_ctrl4 ),     // output [31:0] pldma_mwr_0_u_ctrl4;
      .pldma_mwr_0_u_ctrl5 ( pldma_mwr_0_u_ctrl5 ),     // input [31:0] pldma_mwr_0_u_ctrl5;

      .pldma_mrd_0_rd_clk_pin ( dac_clk ),          // input pldma_mrd_0_rd_clk_pin;
      .pldma_mrd_0_data_rd_pin ( data_rd ),         // output [31:0] pldma_mrd_0_data_rd_pin;
      .pldma_mrd_0_rd_en_pin ( rd_en ),             // input pldma_mrd_0_rd_en_pin;
      .pldma_mrd_0_aempty_pin ( aempt_rd ),         // output pldma_mrd_0_aempty_pin;
      .pldma_mrd_0_rst_pin ( rst_rd ),              // output pldma_mrd_0_rst_pin;
 
      .SPI0_SS_O_pin ( spi_cs ),     // output SPI0_SS_O_pin;
      .SPI0_MOSI_O_pin ( spi_mosi ),     // output SPI0_MOSI_O_pin;
      .SPI0_MISO_I_pin ( spi_miso ),     // input SPI0_MISO_I_pin;
      .SPI0_SCLK_O_pin ( spi_clk ),     // output SPI0_SCLK_O_pin;

      .axi_hdmi_tx_16b_0_hdmi_data_pin ( hdmi_data_pin ),       // output [15:0] axi_hdmi_tx_16b_0_hdmi_data_pin;
      .axi_hdmi_tx_16b_0_hdmi_clk_pin ( hdmi_clk_pin ),         // output axi_hdmi_tx_16b_0_hdmi_clk_pin;
      .axi_hdmi_tx_16b_0_hdmi_vsync_pin ( hdmi_vsync_pin ),     // output axi_hdmi_tx_16b_0_hdmi_vsync_pin;
      .axi_hdmi_tx_16b_0_hdmi_hsync_pin ( hdmi_hsync_pin ),     // output axi_hdmi_tx_16b_0_hdmi_hsync_pin;
      .axi_hdmi_tx_16b_0_hdmi_data_e_pin ( hdmi_data_e_pin ),   // output axi_hdmi_tx_16b_0_hdmi_data_e_pin;
      .processing_system7_0_I2C1_SDA ( sil5351_sda ),           // inout processing_system7_0_I2C1_SDA;
      .processing_system7_0_I2C1_SCL ( sil5351_scl ),            // inout processing_system7_0_I2C1_SCL;
      .fclk0 ( fclk0 )     // output fclk0 200MHz;
    );
assign gpio={pldma_mwr_0_u_ctrl0[0],pldma_mwr_0_u_ctrl0[9:8]};
assign rf_reset=1'b1;
assign txen=1'b1;
assign rxen=1'b1;

//-------------------------------------------------------------------------
    wire LOCKED1=1;
    wire LOCKED2=1;
    wire [11:0] adci,adcq;
    
	BUFG BUFG1 (.O(txclkg),.I(txclk));
	/*
	clk_wiz_v3_6_1 u_pll_tx(.CLK_IN1(txclkg),.CLK_OUT1(txiqsel),.CLK_OUT2(dac_clk),.LOCKED(LOCKED1)); // OUT
	BUFR #(
       .BUFR_DIVIDE("2")   // Values: "BYPASS, 1, 2, 3, 4, 5, 6, 7, 8" 
      // .SIM_DEVICE(""7SERIES"")  // Must be set to "7SERIES" 
    )
    BUFR_inst (
       .O(dac_clk),     // 1-bit output: Clock output port
       .CE(1'b1),   // 1-bit input: Active high, clock enable (Divided modes only)
       .CLR(1'b0), // 1-bit input: Active high, asynchronous clear (Divided modes only)
       .I(txclk)      // 1-bit input: Clock buffer input driven by an IBUF, MMCM or local interconnect
    );	

    assign  txiqsel = dac_clk;
    */
    reg clkcnt=0;
    always @(posedge txclkg) clkcnt<=~clkcnt;
    assign txiqsel = clkcnt;
    assign dac_clk = clkcnt;
    
	genvar i;
	generate
	for (i=0;i<=11;i=i+1)
	begin : loop1
		ODDR #(
			.DDR_CLK_EDGE("SAME_EDGE"), // "OPPOSITE_EDGE" or "SAME_EDGE" 
			.INIT(1'b0),    // Initial value of Q: 1'b0 or 1'b1
			.SRTYPE("ASYNC") // Set/Reset type: "SYNC" or "ASYNC" 
		) ODDR_inst (
			.Q(txd[i]),   // 1-bit DDR output
			.C(dac_clk),   // 1-bit clock input
			.CE(1), // 1-bit clock enable input
			.D1(data_rd[i+20]), // 1-bit data input (positive edge)
			.D2(data_rd[i+4]), // 1-bit data input (negative edge)
			.R(0),   // 1-bit reset
			.S(0)    // 1-bit set
		);
	end
	endgenerate


	//BUFG BUFG2 (.O(rxiqselg),.I(rxiqsel));
	BUFG BUFG2 (.O(rxiqselg),.I(rxiqsel));
	clk_wiz_v3_6_2 u_pll_rx(.CLK_IN1(rxiqselg),.CLK_OUT1(adc_clk),.LOCKED(LOCKED2));
	//BUFG BUFG2 (.O(adc_clk),.I(rxiqsel));	
	genvar j;
	generate
	for (j=0;j<=11;j=j+1)
	begin : loop2   
       IDDR #(
          .DDR_CLK_EDGE("SAME_EDGE"), // "OPPOSITE_EDGE", "SAME_EDGE" 
                                          //    or "SAME_EDGE_PIPELINED" 
          .INIT_Q1(1'b0), // Initial value of Q1: 1'b0 or 1'b1
          .INIT_Q2(1'b0), // Initial value of Q2: 1'b0 or 1'b1
          .SRTYPE("ASYNC") // Set/Reset type: "SYNC" or "ASYNC" 
       ) IDDR_inst (
          .Q1(adci[j]), // 1-bit output for positive edge of clock 
          .Q2(adcq[j]), // 1-bit output for negative edge of clock
          .C(adc_clk),   // 1-bit clock input
          .CE(1), // 1-bit clock enable input
          .D(rxd[j]),   // 1-bit DDR data input
          .R(1'b0),   // 1-bit reset
          .S(0)    // 1-bit set
       );
	end
	endgenerate
	//-------------------------------------------------------------------------
    //write ADC fifo (mwr)
    always @ (posedge adc_clk or negedge LOCKED2)
    if(!LOCKED2)
    	begin
    		data_wr<=32'h0;
    		wr_en<=1'b0;
    	end
    else if(!afull_wr)
    	begin
    		wr_en<=1'b1;
    		data_wr<={adci,4'd0,adcq,4'd0};
    	end
    else
    	begin
    		wr_en<=1'b0;
    	end
    //write DAC fifo (mrd)
    always @ (posedge dac_clk or negedge LOCKED1)
    if(!LOCKED1)
    	begin
    		rd_en<=1'b0;
    	end
    else if(!aempt_rd)
    	begin
    		rd_en<=1'b1;
    	end
    else
    	begin
    		rd_en<=1'b0;
    	end
	
//-------------------------------------------------------------------------
/*
//write ADC fifo (mwr)
reg [1:0] state_wr;
always @ (posedge adc_clk or negedge LOCKED2)
if(!LOCKED2)
    begin
        data_wr<=32'h0;
        wr_en<=1'b0;
        state_wr <= 2'b00;
    end
else begin
  case(state_wr)
    2'd0:begin
      if(!afull_wr)
        begin state_wr<=2'd1;end
      else 
        begin state_wr<=2'd0;end
      end
    2'd1:begin
      data_wr<=data_wr+1'b1;
      wr_en<=1'b1;
      end
  endcase
end    

//write DAC fifo (mrd)
always @ (posedge dac_clk or negedge LOCKED1)
if(!LOCKED1)
	begin
		rd_en<=1'b0;
	end
else if(!aempt_rd)
	begin
		rd_en<=1'b1;
	end
else
	begin
		rd_en<=1'b0;
	end
*/

//---------------------------------------------------------------
wire [35:0] CONTROL0,CONTROL1;
icon_v1_06_a_1 icon_v1_06_a_1 (
    .CONTROL0(CONTROL0),.CONTROL1(CONTROL1) // INOUT BUS [35:0]
);

	ila_v1_05_a_0 ila_v1_05_a_0 (
        .CONTROL(CONTROL0), // INOUT BUS [35:0]
        .CLK(adc_clk), // IN
        .TRIG0({
        data_wr,
        wr_en,
        afull_wr
        }) // IN BUS [39:0]
    );
     ila_v1_05_a_0 ila_v1_05_a_1 (
        .CONTROL(CONTROL1), // INOUT BUS [35:0]
        .CLK(dac_clk), // IN
        .TRIG0({
        data_rd,
        rd_en,
        aempt_rd
        }) // IN BUS [39:0]
    );   
endmodule
