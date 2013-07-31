/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _VIRTEX2_H_
#define _VIRTEX2_H_

#include <xilinx.h>

extern int Virtex2_load(Xilinx_desc *desc, const void *image, size_t size);
extern int Virtex2_dump(Xilinx_desc *desc, const void *buf, size_t bsize);
extern int Virtex2_info(Xilinx_desc *desc);

/*
 * Slave SelectMap Implementation function table.
 */
typedef struct {
	Xilinx_pre_fn	pre;
	Xilinx_pgm_fn	pgm;
	Xilinx_init_fn	init;
	Xilinx_err_fn	err;
	Xilinx_done_fn	done;
	Xilinx_clk_fn	clk;
	Xilinx_cs_fn	cs;
	Xilinx_wr_fn	wr;
	Xilinx_rdata_fn	rdata;
	Xilinx_wdata_fn	wdata;
	Xilinx_busy_fn	busy;
	Xilinx_abort_fn	abort;
	Xilinx_post_fn	post;
} Xilinx_Virtex2_Slave_SelectMap_fns;

/* Slave Serial Implementation function table */
typedef struct {
	Xilinx_pgm_fn	pgm;
	Xilinx_clk_fn	clk;
	Xilinx_rdata_fn	rdata;
	Xilinx_wdata_fn	wdata;
} Xilinx_Virtex2_Slave_Serial_fns;

/* Device Image Sizes (in bytes)
 *********************************************************************/
#define XILINX_XC2V40_SIZE		(338208 / 8)
#define XILINX_XC2V80_SIZE		(597408 / 8)
#define XILINX_XC2V250_SIZE		(1591584 / 8)
#define XILINX_XC2V500_SIZE		(2557857 / 8)
#define XILINX_XC2V1000_SIZE	(3749408 / 8)
#define XILINX_XC2V1500_SIZE	(5166240 / 8)
#define XILINX_XC2V2000_SIZE	(6808352 / 8)
#define XILINX_XC2V3000_SIZE	(9589408 / 8)
#define XILINX_XC2V4000_SIZE	(14220192 / 8)
#define XILINX_XC2V6000_SIZE	(19752096 / 8)
#define XILINX_XC2V8000_SIZE	(26185120 / 8)
#define XILINX_XC2V10000_SIZE	(33519264 / 8)

/* Descriptor Macros
 *********************************************************************/
#define XILINX_XC2V40_DESC(iface, fn_table, cookie)	\
{ Xilinx_Virtex2, iface, XILINX_XC2V40_SIZE, fn_table, cookie }

#define XILINX_XC2V80_DESC(iface, fn_table, cookie) \
{ Xilinx_Virtex2, iface, XILINX_XC2V80_SIZE, fn_table, cookie }

#define XILINX_XC2V250_DESC(iface, fn_table, cookie) \
{ Xilinx_Virtex2, iface, XILINX_XC2V250_SIZE, fn_table, cookie }

#define XILINX_XC2V500_DESC(iface, fn_table, cookie) \
{ Xilinx_Virtex2, iface, XILINX_XC2V500_SIZE, fn_table, cookie }

#define XILINX_XC2V1000_DESC(iface, fn_table, cookie) \
{ Xilinx_Virtex2, iface, XILINX_XC2V1000_SIZE, fn_table, cookie }

#define XILINX_XC2V1500_DESC(iface, fn_table, cookie) \
{ Xilinx_Virtex2, iface, XILINX_XC2V1500_SIZE, fn_table, cookie }

#define XILINX_XC2V2000_DESC(iface, fn_table, cookie) \
{ Xilinx_Virtex2, iface, XILINX_XC2V2000_SIZE, fn_table, cookie }

#define XILINX_XC2V3000_DESC(iface, fn_table, cookie) \
{ Xilinx_Virtex2, iface, XILINX_XC2V3000_SIZE, fn_table, cookie }

#define XILINX_XC2V4000_DESC(iface, fn_table, cookie) \
{ Xilinx_Virtex2, iface, XILINX_XC2V4000_SIZE, fn_table, cookie }

#define XILINX_XC2V6000_DESC(iface, fn_table, cookie) \
{ Xilinx_Virtex2, iface, XILINX_XC2V6000_SIZE, fn_table, cookie }

#define XILINX_XC2V8000_DESC(iface, fn_table, cookie) \
{ Xilinx_Virtex2, iface, XILINX_XC2V8000_SIZE, fn_table, cookie }

#define XILINX_XC2V10000_DESC(iface, fn_table, cookie) \
{ Xilinx_Virtex2, iface, XILINX_XC2V10000_SIZE, fn_table, cookie }

#endif /* _VIRTEX2_H_ */
