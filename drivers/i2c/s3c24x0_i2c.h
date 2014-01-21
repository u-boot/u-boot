/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _S3C24X0_I2C_H
#define _S3C24X0_I2C_H

struct s3c24x0_i2c {
	u32	iiccon;
	u32	iicstat;
	u32	iicadd;
	u32	iicds;
	u32	iiclc;
};

struct exynos5_hsi2c {
	u32	usi_ctl;
	u32	usi_fifo_ctl;
	u32	usi_trailing_ctl;
	u32	usi_clk_ctl;
	u32	usi_clk_slot;
	u32	spi_ctl;
	u32	uart_ctl;
	u32	res1;
	u32	usi_int_en;
	u32	usi_int_stat;
	u32	usi_modem_stat;
	u32	usi_error_stat;
	u32	usi_fifo_stat;
	u32	usi_txdata;
	u32	usi_rxdata;
	u32	res2;
	u32	usi_conf;
	u32	usi_auto_conf;
	u32	usi_timeout;
	u32	usi_manual_cmd;
	u32	usi_trans_status;
	u32	usi_timing_hs1;
	u32	usi_timing_hs2;
	u32	usi_timing_hs3;
	u32	usi_timing_fs1;
	u32	usi_timing_fs2;
	u32	usi_timing_fs3;
	u32	usi_timing_sla;
	u32	i2c_addr;
};

struct s3c24x0_i2c_bus {
	bool active;	/* port is active and available */
	int node;	/* device tree node */
	int bus_num;	/* i2c bus number */
	struct s3c24x0_i2c *regs;
	struct exynos5_hsi2c *hsregs;
	int is_highspeed;	/* High speed type, rather than I2C */
	unsigned clock_frequency;
	int id;
	unsigned clk_cycle;
	unsigned clk_div;
};
#endif /* _S3C24X0_I2C_H */
