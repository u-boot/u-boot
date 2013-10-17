/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Virtex2 FPGA configuration support for the GEN860T computer
 */

extern int gen860t_init_fpga(void);
extern int fpga_pgm_fn(int assert_pgm, int flush, int cookie);
extern int fpga_init_fn(int cookie);
extern int fpga_err_fn(int cookie);
extern int fpga_done_fn(int cookie);
extern int fpga_clk_fn(int assert_clk, int flush, int cookie);
extern int fpga_cs_fn(int assert_cs, int flush, int cookie);
extern int fpga_wr_fn(int assert_write, int flush, int cookie);
extern int fpga_read_data_fn(unsigned char *data, int cookie);
extern int fpga_write_data_fn(unsigned char data, int flush, int cookie);
extern int fpga_busy_fn(int cookie);
extern int fpga_abort_fn(int cookie );
extern int fpga_pre_config_fn(int cookie );
extern int fpga_post_config_fn(int cookie );
