/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

extern int mvbc_p_init_fpga(void);

extern int fpga_pgm_fn(int assert_pgm, int flush, int cookie);
extern int fpga_status_fn(int cookie);
extern int fpga_config_fn(int assert, int flush, int cookie);
extern int fpga_done_fn(int cookie);
extern int fpga_clk_fn(int assert_clk, int flush, int cookie);
extern int fpga_wr_fn(const void *buf, size_t len, int flush, int cookie);
extern int fpga_null_fn(int cookie);
