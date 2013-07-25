/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

extern int mvblx_init_fpga(void);

extern int fpga_status_fn(int cookie);
extern int fpga_config_fn(int assert, int flush, int cookie);
extern int fpga_done_fn(int cookie);
extern int fpga_wr_fn(const void *buf, size_t len, int flush, int cookie);
extern int fpga_null_fn(int cookie);
