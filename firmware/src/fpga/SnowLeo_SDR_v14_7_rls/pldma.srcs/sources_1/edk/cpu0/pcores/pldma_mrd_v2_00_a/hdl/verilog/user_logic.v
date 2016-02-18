//----------------------------------------------------------------------------
// Filename:          user_logic.v
// Version:           1.00.a
// Description:       User logic module.
// Date:              Tue Dec 24 17:18:55 2013 (by Create and Import Peripheral Wizard)
// Verilog Standard:  Verilog-2001
//----------------------------------------------------------------------------
// Naming Conventions:
//   active low signals:                    "*_n"
//   clock signals:                         "clk", "clk_div#", "clk_#x"
//   reset signals:                         "rst", "rst_n"
//   generics:                              "C_*"
//   user defined types:                    "*_TYPE"
//   state machine next state:              "*_ns"
//   state machine current state:           "*_cs"
//   combinatorial signals:                 "*_com"
//   pipelined or register delay signals:   "*_d#"
//   counter signals:                       "*cnt*"
//   clock enable signals:                  "*_ce"
//   internal version of output port:       "*_i"
//   device pins:                           "*_pin"
//   ports:                                 "- Names begin with Uppercase"
//   processes:                             "*_PROCESS"
//   component instantiations:              "<ENTITY_>I_<#|FUNC>"
//----------------------------------------------------------------------------

`uselib lib=unisims_ver
`uselib lib=proc_common_v3_00_a

module user_logic
(
  // -- ADD USER PORTS BELOW THIS LINE ---------------
input m_axi_aclk,
output [139:0] u_debug,
input rd_clk,
output [31:0] data_rd,
input rd_en,
output aempty,
output [14:0] rd_fifo_cnt,
output reg rst=1'b0,
output dma_done_irq,
  // -- ADD USER PORTS ABOVE THIS LINE ---------------

  // -- DO NOT EDIT BELOW THIS LINE ------------------
  // -- Bus protocol ports, do not add to or delete
input																						Bus2IP_Clk,                     // Bus to IP clock
input																						Bus2IP_Resetn,                  // Bus to IP reset
input      [C_SLV_DWIDTH-1 : 0]           			Bus2IP_Data,                    // Bus to IP data bus
input      [C_SLV_DWIDTH/8-1 : 0]         			Bus2IP_BE,                      // Bus to IP byte enables
input      [C_NUM_REG-1 : 0]              			Bus2IP_RdCE,                    // Bus to IP read chip enable
input      [C_NUM_REG-1 : 0]              			Bus2IP_WrCE,                    // Bus to IP write chip enable
output reg [C_SLV_DWIDTH-1 : 0]           			IP2Bus_Data,                    // IP to Bus data bus
output reg                                			IP2Bus_RdAck,                   // IP to Bus read transfer acknowledgement
output reg                                			IP2Bus_WrAck,                   // IP to Bus write transfer acknowledgement
output                                    			IP2Bus_Error,                   // IP to Bus error response
output reg																			ip2bus_mstrd_req,               // IP to Bus master read request
output reg																			ip2bus_mstwr_req,               // IP to Bus master write request
output     [C_MST_AWIDTH-1 : 0]           			ip2bus_mst_addr,                // IP to Bus master read/write address
output reg [(C_MST_NATIVE_DATA_WIDTH/8)-1 : 0] 	ip2bus_mst_be,                  // IP to Bus byte enable
output reg [C_LENGTH_WIDTH-1 : 0]         			ip2bus_mst_length,              // Ip to Bus master transfer length
output reg                                			ip2bus_mst_type,                // Ip to Bus burst assertion control
output reg                                			ip2bus_mst_lock,                // Ip to Bus bus lock
output reg                                			ip2bus_mst_reset,               // Ip to Bus master reset
input                                     			bus2ip_mst_cmdack,              // Bus to Ip master command ack
input                                     			bus2ip_mst_cmplt,               // Bus to Ip master trans complete
input                                     			bus2ip_mst_error,               // Bus to Ip master error
input                                     			bus2ip_mst_rearbitrate,         // Bus to Ip master re-arbitrate for bus ownership
input                                     			bus2ip_mst_cmd_timeout,         // Bus to Ip master command time out
input      [C_MST_NATIVE_DATA_WIDTH-1 : 0] 			bus2ip_mstrd_d,                 // Bus to Ip master read data
input      [(C_MST_NATIVE_DATA_WIDTH)/8-1 : 0]	bus2ip_mstrd_rem,               // Bus to Ip master read data rem
input                                     			bus2ip_mstrd_sof_n,             // Bus to Ip master read start of frame
input                                     			bus2ip_mstrd_eof_n,             // Bus to Ip master read end of frame
input                                     			bus2ip_mstrd_src_rdy_n,         // Bus to Ip master read source ready
input                                     			bus2ip_mstrd_src_dsc_n,         // Bus to Ip master read source dsc
output reg                               				ip2bus_mstrd_dst_rdy_n,         // Ip to Bus master read dest. ready
output reg                                			ip2bus_mstrd_dst_dsc_n,         // Ip to Bus master read dest. dsc
output reg [C_MST_NATIVE_DATA_WIDTH-1 : 0]			ip2bus_mstwr_d,                 // Ip to Bus master write data
output reg [(C_MST_NATIVE_DATA_WIDTH)/8-1 : 0]	ip2bus_mstwr_rem,               // Ip to Bus master write data rem
output reg                                 			ip2bus_mstwr_src_rdy_n,         // Ip to Bus master write source ready
output reg                                 			ip2bus_mstwr_src_dsc_n,         // Ip to Bus master write source dsc
output reg                                			ip2bus_mstwr_sof_n,             // Ip to Bus master write start of frame
output reg                                			ip2bus_mstwr_eof_n,             // Ip to Bus master write end of frame
input                                     			bus2ip_mstwr_dst_rdy_n,         // Bus to Ip master write dest. ready
input                                     			bus2ip_mstwr_dst_dsc_n          // Bus to Ip master write dest. ready
  // -- DO NOT EDIT ABOVE THIS LINE ------------------
); // user_logic
// -- DO NOT EDIT BELOW THIS LINE --------------------
// -- Bus protocol parameters, do not add to or delete
parameter C_MST_NATIVE_DATA_WIDTH        = 64;
parameter C_LENGTH_WIDTH                 = 12;
parameter C_MST_AWIDTH                   = 32;
parameter C_NUM_REG                      = 4;
parameter C_SLV_DWIDTH                   = 32;
// -- DO NOT EDIT ABOVE THIS LINE --------------------

//----------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------
assign dma_done_irq = dma_done;
// ------------------------------------------------------
// Example code to read/write user logic slave model s/w accessible registers
// Bus2IP_WrCE/Bus2IP_RdCE   Memory Mapped Register
//"1000"   C_BASEADDR + 0x0
//"0100"   C_BASEADDR + 0x4
//"0010"   C_BASEADDR + 0x8
//"0001"   C_BASEADDR + 0xC
//
// ------------------------------------------------------
assign IP2Bus_Error = 1'b0;
reg [C_SLV_DWIDTH-1 : 0] slv_reg0;
reg [C_SLV_DWIDTH-1 : 0] slv_reg1;
reg [C_SLV_DWIDTH-1 : 0] slv_reg2;
reg [C_SLV_DWIDTH-1 : 0] slv_reg3;
always @(posedge Bus2IP_Clk or negedge Bus2IP_Resetn)
if(!Bus2IP_Resetn)
	begin
		slv_reg0 <= 0;slv_reg1 <= 0;slv_reg2 <= 0;slv_reg3 <= 0;
	end
else
	begin
		case (Bus2IP_WrCE[3:0])
			4'b1000 :begin slv_reg0 <= Bus2IP_Data;IP2Bus_WrAck <= 1'b1;end
			4'b0100 :begin slv_reg1 <= Bus2IP_Data;IP2Bus_WrAck <= 1'b1;end
			4'b0010 :begin slv_reg2 <= Bus2IP_Data;IP2Bus_WrAck <= 1'b1;end
			4'b0001 :begin slv_reg3 <= Bus2IP_Data;IP2Bus_WrAck <= 1'b1;end
			default :begin IP2Bus_WrAck <= 1'b0;end
		endcase
	end
//define slave reg
wire dma_start=slv_reg0[0];
wire dma_cyc_start=slv_reg0[1];
wire fifo_rst=slv_reg0[4];
wire [31:0] ps_memaddr_base=slv_reg1;
wire [11:0] dma_length=slv_reg2[11:0];//length with byte.must be x8byte,min=16 max=4088
wire [31:0] dma_cyc=slv_reg3;//realy dma cycle=dma_cyc+1
//read reg
reg dma_done;
reg [31:0] dma_cyc_cnt;
always @(posedge Bus2IP_Clk or negedge Bus2IP_Resetn)
if(!Bus2IP_Resetn)
	begin
		IP2Bus_Data<=0;
	end
else
	begin
		case (Bus2IP_RdCE[3:0])
			4'b1000 :begin IP2Bus_Data <= {31'd0,dma_done};IP2Bus_RdAck<=1'b1;end
			4'b0100 :begin IP2Bus_Data <= slv_reg1;IP2Bus_RdAck<=1'b1;end
			4'b0010 :begin IP2Bus_Data <= slv_reg2;IP2Bus_RdAck<=1'b1;end
			4'b0001 :begin IP2Bus_Data <= dma_cyc_cnt;IP2Bus_RdAck<=1'b1;end
			default :begin IP2Bus_Data <= 32'd0;IP2Bus_RdAck<=1'b0;end
		endcase
	end
//pldma ip
always @ (posedge	m_axi_aclk) rst<=dma_start;
//cmd ctrl
wire [13:0] wr_fifo_cnt;
reg	[31:0] app_rd_addr;
assign ip2bus_mst_addr=app_rd_addr;

reg	[4:0]	state;
reg	read_data_end,read_data_start;
reg [13:0] wr_fifo_left;
always @ (posedge	m_axi_aclk	or negedge rst)
if(!rst)
	begin
		ip2bus_mstrd_req<=1'b0;
		ip2bus_mstwr_req<=1'b0;
		ip2bus_mst_length<='d0;
		ip2bus_mst_type<=1'b0;
		ip2bus_mstrd_dst_rdy_n<=1'b1;

		ip2bus_mst_reset<=1'b0;
		ip2bus_mst_lock<=1'b0;//must be decide
		ip2bus_mst_be<='hffff;
		ip2bus_mstrd_dst_dsc_n<=1'b1;//must be always 1
		ip2bus_mstwr_src_dsc_n<=1'b1;//must be always 1

		app_rd_addr<=32'd0;
		state<=5'd0;
		dma_cyc_cnt<=32'd0;
		dma_done<=1'b0;
		read_data_start<=1'b0;
		wr_fifo_left<=14'd0;
	end
else
	begin
		case(state)
			5'd0:
				begin
					app_rd_addr<=ps_memaddr_base;
					state<=5'd1;
				end
			5'd1:
				begin
					wr_fifo_left<=14'd8192-wr_fifo_cnt;
					if(wr_fifo_left>=dma_length[11:3])
						begin
							state<=5'd2;
						end
					else
						begin
							state<=5'd1;
						end
				end
			5'd2://read
				begin
					ip2bus_mstrd_req<=1'b1;ip2bus_mst_type<=1'b1;ip2bus_mst_length<=dma_length;ip2bus_mstrd_dst_rdy_n<=1'b0;
					state<=5'd3;read_data_start<=1'b1;
				end
			5'd3:
				begin
					if(bus2ip_mst_cmdack)
						begin
							ip2bus_mstrd_req<=1'b0;ip2bus_mst_type<=1'b0;ip2bus_mst_length<='d0;ip2bus_mstrd_dst_rdy_n<=1'b0;
							state<=5'd4;read_data_start<=1'b1;
						end
					else
						begin
							state<=5'd3;
						end
				end
			5'd4://wait	read	data end
				begin
					if(read_data_end)
						begin
							ip2bus_mstrd_dst_rdy_n<=1'b1;state<=5'd5;read_data_start<=1'b0;
						end
					else
						begin
							state<=5'd4;
						end
				end
			5'd5:
				begin
					if(bus2ip_mst_cmplt)
						begin state<=5'd6;end
					else
						begin state<=5'd5;end
				end
			5'd6:
				begin
					wr_fifo_left<=14'd8192-wr_fifo_cnt;
					if(dma_cyc_cnt!=dma_cyc)
						begin
							dma_cyc_cnt<=dma_cyc_cnt+1'b1;
							app_rd_addr<=app_rd_addr+dma_length;
							state<=5'd1;
						end
					else
						begin
							state<=5'd7;
						end
				end
			5'd7:
				begin
					if(dma_cyc_start)
						begin
							dma_cyc_cnt<=32'd0;
							state<=5'd0;
						end
					else
						begin
							dma_done<=1'b1;
							state<=5'd8;
						end
				end
			5'd8:
				begin
				end
		endcase
	end
//data rd
wire [63:0] data_wr={bus2ip_mstrd_d[31:0],bus2ip_mstrd_d[63:32]};
wire wr_en=~bus2ip_mstrd_src_rdy_n;
reg	[11:0] read_cnt;
reg [11:0] read_len1;
reg	[4:0]	state_rd;
always @ (posedge	m_axi_aclk	or negedge rst)
if(!rst)
	begin
		read_cnt<=12'd0;
		read_len1<=12'd0;
		state_rd<=5'd0;
		read_data_end<=1'b0;
	end
else
	begin
		case(state_rd)
			5'd0:
				begin
					if(read_data_start)
						begin state_rd<=5'd1;read_len1<=dma_length[11:3]-2'd1;end
					else
						begin	state_rd<=5'd0;end
				end
			5'd1:
				begin
					if(!bus2ip_mstrd_src_rdy_n)
						begin
							if(read_cnt!=read_len1)
								begin
									read_cnt<=read_cnt+1'b1;state_rd<=5'd1;
								end
							else
								begin
									read_cnt<=12'd0;state_rd<=5'd2;read_data_end<=1'b1;
								end
						end
					else
						begin end
				end
			5'd2:
				begin
					if(!read_data_start)
						begin read_data_end<=1'b0;state_rd<=5'd0;end
					else state_rd<=5'd2;
				end
		endcase
	end
//fifo
fifo_8192x64_16384x32 fifo_8192x64_16384x32 (
.rst						(~fifo_rst	), // input rst
.wr_clk					(m_axi_aclk	), // input wr_clk
.rd_clk					(rd_clk			), // input rd_clk
.din						(data_wr		), // input [63 : 0] din
.wr_en					(wr_en			), // input wr_en
.rd_en					(rd_en			), // input rd_en
.dout						(data_rd		), // output [31 : 0] dout
.full						(full				), // output full
.empty					(empty			), // output empty
.almost_empty		(aempty			), // output almost_empty
.rd_data_count	(rd_fifo_cnt), // output [14 : 0] rd_data_count
.wr_data_count	(wr_fifo_cnt) // output [13 : 0] wr_data_count
);
// for chipscope
assign u_debug = {
dma_cyc_start							,//139
empty											,//138
full											,//137
bus2ip_mst_error					,//136
bus2ip_mst_cmd_timeout		,//135
bus2ip_mstwr_dst_dsc_n		,//134

dma_start									,//133
dma_done									,//132
dma_cyc[9:0]							,//131:122

app_rd_addr[15:0]					,//121:106
wr_fifo_cnt								,//105:92
read_data_start						,//91
ip2bus_mstrd_req					,//90
bus2ip_mst_cmdack					,//89
dma_cyc_cnt[9:0]					,//88:79
bus2ip_mst_cmplt					,//78

read_len1									,//77:66
bus2ip_mstrd_src_rdy_n		,//65
wr_en											,//64
data_wr[47:0]							,//63:16
read_cnt									,//15:4
ip2bus_mstrd_dst_rdy_n		,//3
bus2ip_mstrd_sof_n				,//2
bus2ip_mstrd_eof_n				,//1
read_data_end							 //0
};
endmodule
