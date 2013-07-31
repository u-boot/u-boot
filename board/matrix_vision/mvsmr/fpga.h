/*
 * (C) Copyright 2008
 * Andre Schwarz, Matrix Vision GmbH, andre.schwarz@matrix-vision.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

extern int mvsmr_init_fpga(void);

extern int fpga_pgm_fn(int assert_pgm, int flush, int cookie);
extern int fpga_init_fn(int cookie);
extern int fpga_clk_fn(int assert_clk, int flush, int cookie);
extern int fpga_wr_fn(int assert_write, int flush, int cookie);
extern int fpga_done_fn(int cookie);
extern int fpga_pre_config_fn(int cookie);
