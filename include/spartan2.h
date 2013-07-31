/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SPARTAN2_H_
#define _SPARTAN2_H_

#include <xilinx.h>

extern int Spartan2_load(Xilinx_desc *desc, const void *image, size_t size);
extern int Spartan2_dump(Xilinx_desc *desc, const void *buf, size_t bsize);
extern int Spartan2_info(Xilinx_desc *desc);

/* Slave Parallel Implementation function table */
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
} Xilinx_Spartan2_Slave_Parallel_fns;

/* Slave Serial Implementation function table */
typedef struct {
	Xilinx_pre_fn	pre;
	Xilinx_pgm_fn	pgm;
	Xilinx_clk_fn	clk;
	Xilinx_init_fn	init;
	Xilinx_done_fn	done;
	Xilinx_wr_fn	wr;
	Xilinx_post_fn	post;
} Xilinx_Spartan2_Slave_Serial_fns;

/* Device Image Sizes
 *********************************************************************/
/* Spartan-II (2.5V) */
#define XILINX_XC2S15_SIZE	197728/8
#define XILINX_XC2S30_SIZE	336800/8
#define XILINX_XC2S50_SIZE	559232/8
#define XILINX_XC2S100_SIZE	781248/8
#define XILINX_XC2S150_SIZE	1040128/8
#define XILINX_XC2S200_SIZE	1335872/8

/* Spartan-IIE (1.8V) */
#define XILINX_XC2S50E_SIZE     630048/8
#define XILINX_XC2S100E_SIZE    863840/8
#define XILINX_XC2S150E_SIZE    1134496/8
#define XILINX_XC2S200E_SIZE    1442016/8
#define XILINX_XC2S300E_SIZE    1875648/8

/* Descriptor Macros
 *********************************************************************/
/* Spartan-II devices */
#define XILINX_XC2S15_DESC(iface, fn_table, cookie) \
{ Xilinx_Spartan2, iface, XILINX_XC2S15_SIZE, fn_table, cookie }

#define XILINX_XC2S30_DESC(iface, fn_table, cookie) \
{ Xilinx_Spartan2, iface, XILINX_XC2S30_SIZE, fn_table, cookie }

#define XILINX_XC2S50_DESC(iface, fn_table, cookie) \
{ Xilinx_Spartan2, iface, XILINX_XC2S50_SIZE, fn_table, cookie }

#define XILINX_XC2S100_DESC(iface, fn_table, cookie) \
{ Xilinx_Spartan2, iface, XILINX_XC2S100_SIZE, fn_table, cookie }

#define XILINX_XC2S150_DESC(iface, fn_table, cookie) \
{ Xilinx_Spartan2, iface, XILINX_XC2S150_SIZE, fn_table, cookie }

#define XILINX_XC2S200_DESC(iface, fn_table, cookie) \
{ Xilinx_Spartan2, iface, XILINX_XC2S200_SIZE, fn_table, cookie }

#define XILINX_XC2S50E_DESC(iface, fn_table, cookie) \
{ Xilinx_Spartan2, iface, XILINX_XC2S50E_SIZE, fn_table, cookie }

#define XILINX_XC2S100E_DESC(iface, fn_table, cookie) \
{ Xilinx_Spartan2, iface, XILINX_XC2S100E_SIZE, fn_table, cookie }

#define XILINX_XC2S150E_DESC(iface, fn_table, cookie) \
{ Xilinx_Spartan2, iface, XILINX_XC2S150E_SIZE, fn_table, cookie }

#define XILINX_XC2S200E_DESC(iface, fn_table, cookie) \
{ Xilinx_Spartan2, iface, XILINX_XC2S200E_SIZE, fn_table, cookie }

#define XILINX_XC2S300E_DESC(iface, fn_table, cookie) \
{ Xilinx_Spartan2, iface, XILINX_XC2S300E_SIZE, fn_table, cookie }

#endif /* _SPARTAN2_H_ */
