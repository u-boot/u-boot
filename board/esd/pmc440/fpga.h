/*
 * (C) Copyright 2007
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

extern int pmc440_init_fpga(void);

extern int fpga_pgm_fn(int assert_pgm, int flush, int cookie);
extern int fpga_init_fn(int cookie);
extern int fpga_err_fn(int cookie);
extern int fpga_done_fn(int cookie);
extern int fpga_clk_fn(int assert_clk, int flush, int cookie);
extern int fpga_cs_fn(int assert_cs, int flush, int cookie);
extern int fpga_wr_fn(int assert_write, int flush, int cookie);
extern int fpga_wdata_fn (uchar data, int flush, int cookie);
extern int fpga_read_data_fn(unsigned char *data, int cookie);
extern int fpga_write_data_fn(unsigned char data, int flush, int cookie);
extern int fpga_busy_fn(int cookie);
extern int fpga_abort_fn(int cookie );
extern int fpga_pre_config_fn(int cookie );
extern int fpga_post_config_fn(int cookie );

extern int ngcc_fpga_pgm_fn(int assert_pgm, int flush, int cookie);
extern int ngcc_fpga_init_fn(int cookie);
extern int ngcc_fpga_done_fn(int cookie);
extern int ngcc_fpga_clk_fn(int assert_clk, int flush, int cookie);
extern int ngcc_fpga_wr_fn(int assert_write, int flush, int cookie);
extern int ngcc_fpga_pre_config_fn(int cookie );
extern int ngcc_fpga_post_config_fn(int cookie );
