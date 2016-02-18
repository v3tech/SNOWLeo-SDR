//-----------------------------------------------------------------------------
// cpu0_stub.v
//-----------------------------------------------------------------------------

module cpu0_stub
  (
    processing_system7_0_MIO,
    processing_system7_0_PS_SRSTB_pin,
    processing_system7_0_PS_CLK_pin,
    processing_system7_0_PS_PORB_pin,
    processing_system7_0_DDR_Clk,
    processing_system7_0_DDR_Clk_n,
    processing_system7_0_DDR_CKE,
    processing_system7_0_DDR_CS_n,
    processing_system7_0_DDR_RAS_n,
    processing_system7_0_DDR_CAS_n,
    processing_system7_0_DDR_WEB_pin,
    processing_system7_0_DDR_BankAddr,
    processing_system7_0_DDR_Addr,
    processing_system7_0_DDR_ODT,
    processing_system7_0_DDR_DRSTB,
    processing_system7_0_DDR_DQ,
    processing_system7_0_DDR_DM,
    processing_system7_0_DDR_DQS,
    processing_system7_0_DDR_DQS_n,
    processing_system7_0_DDR_VRN,
    processing_system7_0_DDR_VRP,
    pldma_mrd_0_rd_clk_pin,
    pldma_mrd_0_data_rd_pin,
    pldma_mrd_0_rd_en_pin,
    pldma_mrd_0_aempty_pin,
    pldma_mrd_0_rst_pin,
    SPI0_SS_O_pin,
    SPI0_MOSI_O_pin,
    SPI0_MISO_I_pin,
    SPI0_SCLK_O_pin,
    axi_hdmi_tx_16b_0_hdmi_data_pin,
    axi_hdmi_tx_16b_0_hdmi_clk_pin,
    axi_hdmi_tx_16b_0_hdmi_vsync_pin,
    axi_hdmi_tx_16b_0_hdmi_hsync_pin,
    axi_hdmi_tx_16b_0_hdmi_data_e_pin,
    processing_system7_0_I2C1_SDA,
    processing_system7_0_I2C1_SCL,
    pldma_mwr_0_afull_pin,
    pldma_mwr_0_rst_pin,
    pldma_mwr_0_data_wr_pin,
    pldma_mwr_0_wr_en_pin,
    pldma_mwr_0_wr_clk_pin,
    pldma_mwr_0_u_ctrl0,
    pldma_mwr_0_u_ctrl1,
    pldma_mwr_0_u_ctrl2,
    pldma_mwr_0_u_ctrl3,
    pldma_mwr_0_u_ctrl4,
    pldma_mwr_0_u_ctrl5,
    fclk0
  );
  inout [53:0] processing_system7_0_MIO;
  input processing_system7_0_PS_SRSTB_pin;
  input processing_system7_0_PS_CLK_pin;
  input processing_system7_0_PS_PORB_pin;
  inout processing_system7_0_DDR_Clk;
  inout processing_system7_0_DDR_Clk_n;
  inout processing_system7_0_DDR_CKE;
  inout processing_system7_0_DDR_CS_n;
  inout processing_system7_0_DDR_RAS_n;
  inout processing_system7_0_DDR_CAS_n;
  output processing_system7_0_DDR_WEB_pin;
  inout [2:0] processing_system7_0_DDR_BankAddr;
  inout [14:0] processing_system7_0_DDR_Addr;
  inout processing_system7_0_DDR_ODT;
  inout processing_system7_0_DDR_DRSTB;
  inout [31:0] processing_system7_0_DDR_DQ;
  inout [3:0] processing_system7_0_DDR_DM;
  inout [3:0] processing_system7_0_DDR_DQS;
  inout [3:0] processing_system7_0_DDR_DQS_n;
  inout processing_system7_0_DDR_VRN;
  inout processing_system7_0_DDR_VRP;
  input pldma_mrd_0_rd_clk_pin;
  output [31:0] pldma_mrd_0_data_rd_pin;
  input pldma_mrd_0_rd_en_pin;
  output pldma_mrd_0_aempty_pin;
  output pldma_mrd_0_rst_pin;
  output SPI0_SS_O_pin;
  output SPI0_MOSI_O_pin;
  input SPI0_MISO_I_pin;
  output SPI0_SCLK_O_pin;
  output [15:0] axi_hdmi_tx_16b_0_hdmi_data_pin;
  output axi_hdmi_tx_16b_0_hdmi_clk_pin;
  output axi_hdmi_tx_16b_0_hdmi_vsync_pin;
  output axi_hdmi_tx_16b_0_hdmi_hsync_pin;
  output axi_hdmi_tx_16b_0_hdmi_data_e_pin;
  inout processing_system7_0_I2C1_SDA;
  inout processing_system7_0_I2C1_SCL;
  output pldma_mwr_0_afull_pin;
  output pldma_mwr_0_rst_pin;
  input [31:0] pldma_mwr_0_data_wr_pin;
  input pldma_mwr_0_wr_en_pin;
  input pldma_mwr_0_wr_clk_pin;
  output [31:0] pldma_mwr_0_u_ctrl0;
  output [31:0] pldma_mwr_0_u_ctrl1;
  output [31:0] pldma_mwr_0_u_ctrl2;
  output [31:0] pldma_mwr_0_u_ctrl3;
  output [31:0] pldma_mwr_0_u_ctrl4;
  input [31:0] pldma_mwr_0_u_ctrl5;
  output fclk0;

  (* BOX_TYPE = "user_black_box" *)
  cpu0
    cpu0_i (
      .processing_system7_0_MIO ( processing_system7_0_MIO ),
      .processing_system7_0_PS_SRSTB_pin ( processing_system7_0_PS_SRSTB_pin ),
      .processing_system7_0_PS_CLK_pin ( processing_system7_0_PS_CLK_pin ),
      .processing_system7_0_PS_PORB_pin ( processing_system7_0_PS_PORB_pin ),
      .processing_system7_0_DDR_Clk ( processing_system7_0_DDR_Clk ),
      .processing_system7_0_DDR_Clk_n ( processing_system7_0_DDR_Clk_n ),
      .processing_system7_0_DDR_CKE ( processing_system7_0_DDR_CKE ),
      .processing_system7_0_DDR_CS_n ( processing_system7_0_DDR_CS_n ),
      .processing_system7_0_DDR_RAS_n ( processing_system7_0_DDR_RAS_n ),
      .processing_system7_0_DDR_CAS_n ( processing_system7_0_DDR_CAS_n ),
      .processing_system7_0_DDR_WEB_pin ( processing_system7_0_DDR_WEB_pin ),
      .processing_system7_0_DDR_BankAddr ( processing_system7_0_DDR_BankAddr ),
      .processing_system7_0_DDR_Addr ( processing_system7_0_DDR_Addr ),
      .processing_system7_0_DDR_ODT ( processing_system7_0_DDR_ODT ),
      .processing_system7_0_DDR_DRSTB ( processing_system7_0_DDR_DRSTB ),
      .processing_system7_0_DDR_DQ ( processing_system7_0_DDR_DQ ),
      .processing_system7_0_DDR_DM ( processing_system7_0_DDR_DM ),
      .processing_system7_0_DDR_DQS ( processing_system7_0_DDR_DQS ),
      .processing_system7_0_DDR_DQS_n ( processing_system7_0_DDR_DQS_n ),
      .processing_system7_0_DDR_VRN ( processing_system7_0_DDR_VRN ),
      .processing_system7_0_DDR_VRP ( processing_system7_0_DDR_VRP ),
      .pldma_mrd_0_rd_clk_pin ( pldma_mrd_0_rd_clk_pin ),
      .pldma_mrd_0_data_rd_pin ( pldma_mrd_0_data_rd_pin ),
      .pldma_mrd_0_rd_en_pin ( pldma_mrd_0_rd_en_pin ),
      .pldma_mrd_0_aempty_pin ( pldma_mrd_0_aempty_pin ),
      .pldma_mrd_0_rst_pin ( pldma_mrd_0_rst_pin ),
      .SPI0_SS_O_pin ( SPI0_SS_O_pin ),
      .SPI0_MOSI_O_pin ( SPI0_MOSI_O_pin ),
      .SPI0_MISO_I_pin ( SPI0_MISO_I_pin ),
      .SPI0_SCLK_O_pin ( SPI0_SCLK_O_pin ),
      .axi_hdmi_tx_16b_0_hdmi_data_pin ( axi_hdmi_tx_16b_0_hdmi_data_pin ),
      .axi_hdmi_tx_16b_0_hdmi_clk_pin ( axi_hdmi_tx_16b_0_hdmi_clk_pin ),
      .axi_hdmi_tx_16b_0_hdmi_vsync_pin ( axi_hdmi_tx_16b_0_hdmi_vsync_pin ),
      .axi_hdmi_tx_16b_0_hdmi_hsync_pin ( axi_hdmi_tx_16b_0_hdmi_hsync_pin ),
      .axi_hdmi_tx_16b_0_hdmi_data_e_pin ( axi_hdmi_tx_16b_0_hdmi_data_e_pin ),
      .processing_system7_0_I2C1_SDA ( processing_system7_0_I2C1_SDA ),
      .processing_system7_0_I2C1_SCL ( processing_system7_0_I2C1_SCL ),
      .pldma_mwr_0_afull_pin ( pldma_mwr_0_afull_pin ),
      .pldma_mwr_0_rst_pin ( pldma_mwr_0_rst_pin ),
      .pldma_mwr_0_data_wr_pin ( pldma_mwr_0_data_wr_pin ),
      .pldma_mwr_0_wr_en_pin ( pldma_mwr_0_wr_en_pin ),
      .pldma_mwr_0_wr_clk_pin ( pldma_mwr_0_wr_clk_pin ),
      .pldma_mwr_0_u_ctrl0 ( pldma_mwr_0_u_ctrl0 ),
      .pldma_mwr_0_u_ctrl1 ( pldma_mwr_0_u_ctrl1 ),
      .pldma_mwr_0_u_ctrl2 ( pldma_mwr_0_u_ctrl2 ),
      .pldma_mwr_0_u_ctrl3 ( pldma_mwr_0_u_ctrl3 ),
      .pldma_mwr_0_u_ctrl4 ( pldma_mwr_0_u_ctrl4 ),
      .pldma_mwr_0_u_ctrl5 ( pldma_mwr_0_u_ctrl5 ),
      .fclk0 ( fclk0 )
    );

endmodule

module cpu0
  (
    processing_system7_0_MIO,
    processing_system7_0_PS_SRSTB_pin,
    processing_system7_0_PS_CLK_pin,
    processing_system7_0_PS_PORB_pin,
    processing_system7_0_DDR_Clk,
    processing_system7_0_DDR_Clk_n,
    processing_system7_0_DDR_CKE,
    processing_system7_0_DDR_CS_n,
    processing_system7_0_DDR_RAS_n,
    processing_system7_0_DDR_CAS_n,
    processing_system7_0_DDR_WEB_pin,
    processing_system7_0_DDR_BankAddr,
    processing_system7_0_DDR_Addr,
    processing_system7_0_DDR_ODT,
    processing_system7_0_DDR_DRSTB,
    processing_system7_0_DDR_DQ,
    processing_system7_0_DDR_DM,
    processing_system7_0_DDR_DQS,
    processing_system7_0_DDR_DQS_n,
    processing_system7_0_DDR_VRN,
    processing_system7_0_DDR_VRP,
    pldma_mrd_0_rd_clk_pin,
    pldma_mrd_0_data_rd_pin,
    pldma_mrd_0_rd_en_pin,
    pldma_mrd_0_aempty_pin,
    pldma_mrd_0_rst_pin,
    SPI0_SS_O_pin,
    SPI0_MOSI_O_pin,
    SPI0_MISO_I_pin,
    SPI0_SCLK_O_pin,
    axi_hdmi_tx_16b_0_hdmi_data_pin,
    axi_hdmi_tx_16b_0_hdmi_clk_pin,
    axi_hdmi_tx_16b_0_hdmi_vsync_pin,
    axi_hdmi_tx_16b_0_hdmi_hsync_pin,
    axi_hdmi_tx_16b_0_hdmi_data_e_pin,
    processing_system7_0_I2C1_SDA,
    processing_system7_0_I2C1_SCL,
    pldma_mwr_0_afull_pin,
    pldma_mwr_0_rst_pin,
    pldma_mwr_0_data_wr_pin,
    pldma_mwr_0_wr_en_pin,
    pldma_mwr_0_wr_clk_pin,
    pldma_mwr_0_u_ctrl0,
    pldma_mwr_0_u_ctrl1,
    pldma_mwr_0_u_ctrl2,
    pldma_mwr_0_u_ctrl3,
    pldma_mwr_0_u_ctrl4,
    pldma_mwr_0_u_ctrl5,
    fclk0
  );
  inout [53:0] processing_system7_0_MIO;
  input processing_system7_0_PS_SRSTB_pin;
  input processing_system7_0_PS_CLK_pin;
  input processing_system7_0_PS_PORB_pin;
  inout processing_system7_0_DDR_Clk;
  inout processing_system7_0_DDR_Clk_n;
  inout processing_system7_0_DDR_CKE;
  inout processing_system7_0_DDR_CS_n;
  inout processing_system7_0_DDR_RAS_n;
  inout processing_system7_0_DDR_CAS_n;
  output processing_system7_0_DDR_WEB_pin;
  inout [2:0] processing_system7_0_DDR_BankAddr;
  inout [14:0] processing_system7_0_DDR_Addr;
  inout processing_system7_0_DDR_ODT;
  inout processing_system7_0_DDR_DRSTB;
  inout [31:0] processing_system7_0_DDR_DQ;
  inout [3:0] processing_system7_0_DDR_DM;
  inout [3:0] processing_system7_0_DDR_DQS;
  inout [3:0] processing_system7_0_DDR_DQS_n;
  inout processing_system7_0_DDR_VRN;
  inout processing_system7_0_DDR_VRP;
  input pldma_mrd_0_rd_clk_pin;
  output [31:0] pldma_mrd_0_data_rd_pin;
  input pldma_mrd_0_rd_en_pin;
  output pldma_mrd_0_aempty_pin;
  output pldma_mrd_0_rst_pin;
  output SPI0_SS_O_pin;
  output SPI0_MOSI_O_pin;
  input SPI0_MISO_I_pin;
  output SPI0_SCLK_O_pin;
  output [15:0] axi_hdmi_tx_16b_0_hdmi_data_pin;
  output axi_hdmi_tx_16b_0_hdmi_clk_pin;
  output axi_hdmi_tx_16b_0_hdmi_vsync_pin;
  output axi_hdmi_tx_16b_0_hdmi_hsync_pin;
  output axi_hdmi_tx_16b_0_hdmi_data_e_pin;
  inout processing_system7_0_I2C1_SDA;
  inout processing_system7_0_I2C1_SCL;
  output pldma_mwr_0_afull_pin;
  output pldma_mwr_0_rst_pin;
  input [31:0] pldma_mwr_0_data_wr_pin;
  input pldma_mwr_0_wr_en_pin;
  input pldma_mwr_0_wr_clk_pin;
  output [31:0] pldma_mwr_0_u_ctrl0;
  output [31:0] pldma_mwr_0_u_ctrl1;
  output [31:0] pldma_mwr_0_u_ctrl2;
  output [31:0] pldma_mwr_0_u_ctrl3;
  output [31:0] pldma_mwr_0_u_ctrl4;
  input [31:0] pldma_mwr_0_u_ctrl5;
  output fclk0;
endmodule

