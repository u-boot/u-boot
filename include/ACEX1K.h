/*
 * (C) Copyright 2003
 * Steven Scholz, imc Measurement & Control, steven.scholz@imc-berlin.de
 *
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
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
 *
 */

#ifndef _ACEX1K_H_
#define _ACEX1K_H_

#include <altera.h>

extern int ACEX1K_load( Altera_desc *desc, void *image, size_t size );
extern int ACEX1K_dump( Altera_desc *desc, void *buf, size_t bsize );
extern int ACEX1K_info( Altera_desc *desc );

extern int CYC2_load( Altera_desc *desc, void *image, size_t size );
extern int CYC2_dump( Altera_desc *desc, void *buf, size_t bsize );
extern int CYC2_info( Altera_desc *desc );

/* Slave Serial Implementation function table */
typedef struct {
	Altera_pre_fn		pre;
	Altera_config_fn	config;
	Altera_clk_fn		clk;
	Altera_status_fn	status;
	Altera_done_fn		done;
	Altera_data_fn		data;
	Altera_abort_fn		abort;
	Altera_post_fn		post;
} Altera_ACEX1K_Passive_Serial_fns;

/* Slave Serial Implementation function table */
typedef struct {
	Altera_pre_fn		pre;
	Altera_config_fn	config;
	Altera_status_fn	status;
	Altera_done_fn		done;
	Altera_write_fn		write;
	Altera_abort_fn		abort;
	Altera_post_fn		post;
} Altera_CYC2_Passive_Serial_fns;

/* Device Image Sizes
 *********************************************************************/
/* ACEX1K */
/* FIXME: Which size do we mean?
 * Datasheet says 1337000/8=167125Bytes,
 * Filesize of an *.rbf file is 166965 Bytes
 */
#if 0
#define Altera_EP1K100_SIZE	1337000/8	/* 167125 Bytes */
#endif
#define Altera_EP1K100_SIZE	(166965*8)

#define Altera_EP2C8_SIZE	247942
#define Altera_EP2C20_SIZE	586562
#define Altera_EP2C35_SIZE	883905

/* Descriptor Macros
 *********************************************************************/
/* ACEX1K devices */
#define Altera_EP1K100_DESC(iface, fn_table, cookie) \
{ Altera_ACEX1K, iface, Altera_EP1K100_SIZE, fn_table, cookie }

#endif /* _ACEX1K_H_ */
