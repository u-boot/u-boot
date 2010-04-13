/*
 * iopin settings are controlled by four different sets of registers
 *	iopad mux control
 *	individual iopad setup (voltage select, pull/keep, drive strength ...)
 *	group iopad setup (same as above but for groups of signals)
 *	input select when multiple inputs are possible
 */

/*
 * software pad mux control
 */
/* SW Input On (Loopback) */
#define MX25_PIN_MUX_SION		(1 << 4)
/* MUX Mode (0-7) */
#define MX25_PIN_MUX_MODE(mode)		((mode & 0x7) << 0)
struct iomuxc_mux_ctl {
	u32 gpr1;
	u32 observe_int_mux;
	u32 pad_a10;
	u32 pad_a13;
	u32 pad_a14;
	u32 pad_a15;
	u32 pad_a16;
	u32 pad_a17;
	u32 pad_a18;
	u32 pad_a19;
	u32 pad_a20;
	u32 pad_a21;
	u32 pad_a22;
	u32 pad_a23;
	u32 pad_a24;
	u32 pad_a25;
	u32 pad_eb0;
	u32 pad_eb1;
	u32 pad_oe;
	u32 pad_cs0;
	u32 pad_cs1;
	u32 pad_cs4;
	u32 pad_cs5;
	u32 pad_nf_ce0;
	u32 pad_ecb;
	u32 pad_lba;
	u32 pad_bclk;
	u32 pad_rw;
	u32 pad_nfwe_b;
	u32 pad_nfre_b;
	u32 pad_nfale;
	u32 pad_nfcle;
	u32 pad_nfwp_b;
	u32 pad_nfrb;
	u32 pad_d15;
	u32 pad_d14;
	u32 pad_d13;
	u32 pad_d12;
	u32 pad_d11;
	u32 pad_d10;
	u32 pad_d9;
	u32 pad_d8;
	u32 pad_d7;
	u32 pad_d6;
	u32 pad_d5;
	u32 pad_d4;
	u32 pad_d3;
	u32 pad_d2;
	u32 pad_d1;
	u32 pad_d0;
	u32 pad_ld0;
	u32 pad_ld1;
	u32 pad_ld2;
	u32 pad_ld3;
	u32 pad_ld4;
	u32 pad_ld5;
	u32 pad_ld6;
	u32 pad_ld7;
	u32 pad_ld8;
	u32 pad_ld9;
	u32 pad_ld10;
	u32 pad_ld11;
	u32 pad_ld12;
	u32 pad_ld13;
	u32 pad_ld14;
	u32 pad_ld15;
	u32 pad_hsync;
	u32 pad_vsync;
	u32 pad_lsclk;
	u32 pad_oe_acd;
	u32 pad_contrast;
	u32 pad_pwm;
	u32 pad_csi_d2;
	u32 pad_csi_d3;
	u32 pad_csi_d4;
	u32 pad_csi_d5;
	u32 pad_csi_d6;
	u32 pad_csi_d7;
	u32 pad_csi_d8;
	u32 pad_csi_d9;
	u32 pad_csi_mclk;
	u32 pad_csi_vsync;
	u32 pad_csi_hsync;
	u32 pad_csi_pixclk;
	u32 pad_i2c1_clk;
	u32 pad_i2c1_dat;
	u32 pad_cspi1_mosi;
	u32 pad_cspi1_miso;
	u32 pad_cspi1_ss0;
	u32 pad_cspi1_ss1;
	u32 pad_cspi1_sclk;
	u32 pad_cspi1_rdy;
	u32 pad_uart1_rxd;
	u32 pad_uart1_txd;
	u32 pad_uart1_rts;
	u32 pad_uart1_cts;
	u32 pad_uart2_rxd;
	u32 pad_uart2_txd;
	u32 pad_uart2_rts;
	u32 pad_uart2_cts;
	u32 pad_sd1_cmd;
	u32 pad_sd1_clk;
	u32 pad_sd1_data0;
	u32 pad_sd1_data1;
	u32 pad_sd1_data2;
	u32 pad_sd1_data3;
	u32 pad_kpp_row0;
	u32 pad_kpp_row1;
	u32 pad_kpp_row2;
	u32 pad_kpp_row3;
	u32 pad_kpp_col0;
	u32 pad_kpp_col1;
	u32 pad_kpp_col2;
	u32 pad_kpp_col3;
	u32 pad_fec_mdc;
	u32 pad_fec_mdio;
	u32 pad_fec_tdata0;
	u32 pad_fec_tdata1;
	u32 pad_fec_tx_en;
	u32 pad_fec_rdata0;
	u32 pad_fec_rdata1;
	u32 pad_fec_rx_dv;
	u32 pad_fec_tx_clk;
	u32 pad_rtck;
	u32 pad_de_b;
	u32 pad_gpio_a;
	u32 pad_gpio_b;
	u32 pad_gpio_c;
	u32 pad_gpio_d;
	u32 pad_gpio_e;
	u32 pad_gpio_f;
	u32 pad_ext_armclk;
	u32 pad_upll_bypclk;
	u32 pad_vstby_req;
	u32 pad_vstby_ack;
	u32 pad_power_fail;
	u32 pad_clko;
	u32 pad_boot_mode0;
	u32 pad_boot_mode1;
};

/*
 * software pad control
 */
/* Select 3.3 or 1.8 volts */
#define MX25_PIN_PAD_CTL_DVS_33			(0 << 13)
#define MX25_PIN_PAD_CTL_DVS_18			(1 << 13)
/* Enable hysteresis */
#define MX25_PIN_PAD_CTL_HYS			(1 << 8)
/* Enable pull/keeper */
#define MX25_PIN_PAD_CTL_PKE			(1 << 7)
/* 0 - keeper / 1 - pull */
#define MX25_PIN_PAD_CTL_PUE			(1 << 6)
/* pull up/down strength */
#define MX25_PIN_PAD_CTL_100K_PD		(0 << 4)
#define MX25_PIN_PAD_CTL_47K_PU			(1 << 4)
#define MX25_PIN_PAD_CTL_100K_PU		(2 << 4)
#define MX25_PIN_PAD_CTL_22K_PU			(3 << 4)
/* open drain control */
#define MX25_PIN_PAD_CTL_OD			(1 << 3)
/* drive strength */
#define MX25_PIN_PAD_CTL_DS_NOM			(0 << 1)
#define MX25_PIN_PAD_CTL_DS_HIGH		(1 << 1)
#define MX25_PIN_PAD_CTL_DS_MAX			(2 << 1)
#define MX25_PIN_PAD_CTL_DS_MAX11		(3 << 1)
/* slew rate */
#define MX25_PIN_PAD_CTL_SRE_SLOW		(0 << 0)
#define MX25_PIN_PAD_CTL_SRE_FAST		(1 << 0)
struct  iomuxc_pad_ctl {
	u32 pad_a13;
	u32 pad_a14;
	u32 pad_a15;
	u32 pad_a17;
	u32 pad_a18;
	u32 pad_a19;
	u32 pad_a20;
	u32 pad_a21;
	u32 pad_a23;
	u32 pad_a24;
	u32 pad_a25;
	u32 pad_eb0;
	u32 pad_eb1;
	u32 pad_oe;
	u32 pad_cs4;
	u32 pad_cs5;
	u32 pad_nf_ce0;
	u32 pad_ecb;
	u32 pad_lba;
	u32 pad_rw;
	u32 pad_nfrb;
	u32 pad_d15;
	u32 pad_d14;
	u32 pad_d13;
	u32 pad_d12;
	u32 pad_d11;
	u32 pad_d10;
	u32 pad_d9;
	u32 pad_d8;
	u32 pad_d7;
	u32 pad_d6;
	u32 pad_d5;
	u32 pad_d4;
	u32 pad_d3;
	u32 pad_d2;
	u32 pad_d1;
	u32 pad_d0;
	u32 pad_ld0;
	u32 pad_ld1;
	u32 pad_ld2;
	u32 pad_ld3;
	u32 pad_ld4;
	u32 pad_ld5;
	u32 pad_ld6;
	u32 pad_ld7;
	u32 pad_ld8;
	u32 pad_ld9;
	u32 pad_ld10;
	u32 pad_ld11;
	u32 pad_ld12;
	u32 pad_ld13;
	u32 pad_ld14;
	u32 pad_ld15;
	u32 pad_hsync;
	u32 pad_vsync;
	u32 pad_lsclk;
	u32 pad_oe_acd;
	u32 pad_contrast;
	u32 pad_pwm;
	u32 pad_csi_d2;
	u32 pad_csi_d3;
	u32 pad_csi_d4;
	u32 pad_csi_d5;
	u32 pad_csi_d6;
	u32 pad_csi_d7;
	u32 pad_csi_d8;
	u32 pad_csi_d9;
	u32 pad_csi_mclk;
	u32 pad_csi_vsync;
	u32 pad_csi_hsync;
	u32 pad_csi_pixclk;
	u32 pad_i2c1_clk;
	u32 pad_i2c1_dat;
	u32 pad_cspi1_mosi;
	u32 pad_cspi1_miso;
	u32 pad_cspi1_ss0;
	u32 pad_cspi1_ss1;
	u32 pad_cspi1_sclk;
	u32 pad_cspi1_rdy;
	u32 pad_uart1_rxd;
	u32 pad_uart1_txd;
	u32 pad_uart1_rts;
	u32 pad_uart1_cts;
	u32 pad_uart2_rxd;
	u32 pad_uart2_txd;
	u32 pad_uart2_rts;
	u32 pad_uart2_cts;
	u32 pad_sd1_cmd;
	u32 pad_sd1_clk;
	u32 pad_sd1_data0;
	u32 pad_sd1_data1;
	u32 pad_sd1_data2;
	u32 pad_sd1_data3;
	u32 pad_kpp_row0;
	u32 pad_kpp_row1;
	u32 pad_kpp_row2;
	u32 pad_kpp_row3;
	u32 pad_kpp_col0;
	u32 pad_kpp_col1;
	u32 pad_kpp_col2;
	u32 pad_kpp_col3;
	u32 pad_fec_mdc;
	u32 pad_fec_mdio;
	u32 pad_fec_tdata0;
	u32 pad_fec_tdata1;
	u32 pad_fec_tx_en;
	u32 pad_fec_rdata0;
	u32 pad_fec_rdata1;
	u32 pad_fec_rx_dv;
	u32 pad_fec_tx_clk;
	u32 pad_rtck;
	u32 pad_tdo;
	u32 pad_de_b;
	u32 pad_gpio_a;
	u32 pad_gpio_b;
	u32 pad_gpio_c;
	u32 pad_gpio_d;
	u32 pad_gpio_e;
	u32 pad_gpio_f;
	u32 pad_vstby_req;
	u32 pad_vstby_ack;
	u32 pad_power_fail;
	u32 pad_clko;
};


/*
 * Pad group drive strength and voltage select
 * Same fields as iomuxc_pad_ctl plus ddr type
 */
/* Select DDR type */
#define MX25_PIN_PAD_CTL_DDR_18			(0 << 11)
#define MX25_PIN_PAD_CTL_DDR_33			(1 << 11)
#define MX25_PIN_PAD_CTL_DDR_MAX		(2 << 11)
struct iomuxc_pad_grp_ctl {
	u32 grp_dvs_misc;
	u32 grp_dse_fec;
	u32 grp_dvs_jtag;
	u32 grp_dse_nfc;
	u32 grp_dse_csi;
	u32 grp_dse_weim;
	u32 grp_dse_ddr;
	u32 grp_dvs_crm;
	u32 grp_dse_kpp;
	u32 grp_dse_sdhc1;
	u32 grp_dse_lcd;
	u32 grp_dse_uart;
	u32 grp_dvs_nfc;
	u32 grp_dvs_csi;
	u32 grp_dse_cspi1;
	u32 grp_ddrtype;
	u32 grp_dvs_sdhc1;
	u32 grp_dvs_lcd;
};

/*
 * Pad input select control
 * Select which pad to connect to an input port
 * where multiple pads can function as given input
 */
#define MX25_PAD_INPUT_SELECT_DAISY(in)		((in & 0x7) << 0)
struct iomuxc_pad_input_select {
	u32 audmux_p4_input_da_amx;
	u32 audmux_p4_input_db_amx;
	u32 audmux_p4_input_rxclk_amx;
	u32 audmux_p4_input_rxfs_amx;
	u32 audmux_p4_input_txclk_amx;
	u32 audmux_p4_input_txfs_amx;
	u32 audmux_p7_input_da_amx;
	u32 audmux_p7_input_txfs_amx;
	u32 can1_ipp_ind_canrx;
	u32 can2_ipp_ind_canrx;
	u32 csi_ipp_csi_d_0;
	u32 csi_ipp_csi_d_1;
	u32 cspi1_ipp_ind_ss3_b;
	u32 cspi2_ipp_cspi_clk_in;
	u32 cspi2_ipp_ind_dataready_b;
	u32 cspi2_ipp_ind_miso;
	u32 cspi2_ipp_ind_mosi;
	u32 cspi2_ipp_ind_ss0_b;
	u32 cspi2_ipp_ind_ss1_b;
	u32 cspi3_ipp_cspi_clk_in;
	u32 cspi3_ipp_ind_dataready_b;
	u32 cspi3_ipp_ind_miso;
	u32 cspi3_ipp_ind_mosi;
	u32 cspi3_ipp_ind_ss0_b;
	u32 cspi3_ipp_ind_ss1_b;
	u32 cspi3_ipp_ind_ss2_b;
	u32 cspi3_ipp_ind_ss3_b;
	u32 esdhc1_ipp_dat4_in;
	u32 esdhc1_ipp_dat5_in;
	u32 esdhc1_ipp_dat6_in;
	u32 esdhc1_ipp_dat7_in;
	u32 esdhc2_ipp_card_clk_in;
	u32 esdhc2_ipp_cmd_in;
	u32 esdhc2_ipp_dat0_in;
	u32 esdhc2_ipp_dat1_in;
	u32 esdhc2_ipp_dat2_in;
	u32 esdhc2_ipp_dat3_in;
	u32 esdhc2_ipp_dat4_in;
	u32 esdhc2_ipp_dat5_in;
	u32 esdhc2_ipp_dat6_in;
	u32 esdhc2_ipp_dat7_in;
	u32 fec_fec_col;
	u32 fec_fec_crs;
	u32 fec_fec_rdata_2;
	u32 fec_fec_rdata_3;
	u32 fec_fec_rx_clk;
	u32 fec_fec_rx_er;
	u32 i2c2_ipp_scl_in;
	u32 i2c2_ipp_sda_in;
	u32 i2c3_ipp_scl_in;
	u32 i2c3_ipp_sda_in;
	u32 kpp_ipp_ind_col_4;
	u32 kpp_ipp_ind_col_5;
	u32 kpp_ipp_ind_col_6;
	u32 kpp_ipp_ind_col_7;
	u32 kpp_ipp_ind_row_4;
	u32 kpp_ipp_ind_row_5;
	u32 kpp_ipp_ind_row_6;
	u32 kpp_ipp_ind_row_7;
	u32 sim1_pin_sim_rcvd1_in;
	u32 sim1_pin_sim_simpd1;
	u32 sim1_sim_rcvd1_io;
	u32 sim2_pin_sim_rcvd1_in;
	u32 sim2_pin_sim_simpd1;
	u32 sim2_sim_rcvd1_io;
	u32 uart3_ipp_uart_rts_b;
	u32 uart3_ipp_uart_rxd_mux;
	u32 uart4_ipp_uart_rts_b;
	u32 uart4_ipp_uart_rxd_mux;
	u32 uart5_ipp_uart_rts_b;
	u32 uart5_ipp_uart_rxd_mux;
	u32 usb_top_ipp_ind_otg_usb_oc;
	u32 usb_top_ipp_ind_uh2_usb_oc;
};
