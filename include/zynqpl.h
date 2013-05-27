/*
 * (C) Copyright 2012-2013, Xilinx, Michal Simek
 *
 * (C) Copyright 2012
 * Joe Hershberger <joe.hershberger@ni.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _ZYNQPL_H_
#define _ZYNQPL_H_

#include <xilinx.h>

extern int zynq_load(Xilinx_desc *desc, const void *image, size_t size);
extern int zynq_dump(Xilinx_desc *desc, const void *buf, size_t bsize);
extern int zynq_info(Xilinx_desc *desc);

#define XILINX_ZYNQ_7010	0x2
#define XILINX_ZYNQ_7020	0x7
#define XILINX_ZYNQ_7030	0xc
#define XILINX_ZYNQ_7045	0x11

/* Device Image Sizes */
#define XILINX_XC7Z010_SIZE	16669920/8
#define XILINX_XC7Z020_SIZE	32364512/8
#define XILINX_XC7Z030_SIZE	47839328/8
#define XILINX_XC7Z045_SIZE	106571232/8

/* Descriptor Macros */
#define XILINX_XC7Z010_DESC(cookie) \
{ xilinx_zynq, devcfg, XILINX_XC7Z010_SIZE, NULL, cookie, "7z010" }

#define XILINX_XC7Z020_DESC(cookie) \
{ xilinx_zynq, devcfg, XILINX_XC7Z020_SIZE, NULL, cookie, "7z020" }

#define XILINX_XC7Z030_DESC(cookie) \
{ xilinx_zynq, devcfg, XILINX_XC7Z030_SIZE, NULL, cookie, "7z030" }

#define XILINX_XC7Z045_DESC(cookie) \
{ xilinx_zynq, devcfg, XILINX_XC7Z045_SIZE, NULL, cookie, "7z045" }

#endif /* _ZYNQPL_H_ */
