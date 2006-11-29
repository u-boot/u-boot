/*
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

#include <fpga.h>

#ifndef _ALTERA_H_
#define _ALTERA_H_

/*
 * See include/xilinx.h for another working example.
 */

/* Altera Model definitions
 *********************************************************************/
#define CFG_ACEX1K		CFG_FPGA_DEV( 0x1 )
#define CFG_CYCLON2		CFG_FPGA_DEV( 0x2 )

#define CFG_ALTERA_ACEX1K	(CFG_FPGA_ALTERA | CFG_ACEX1K)
#define CFG_ALTERA_CYCLON2	(CFG_FPGA_ALTERA | CFG_CYCLON2)
/* Add new models here */

/* Altera Interface definitions
 *********************************************************************/
#define CFG_ALTERA_IF_PS	CFG_FPGA_IF( 0x1 )	/* passive serial */
/* Add new interfaces here */

typedef enum {				/* typedef Altera_iface */
	min_altera_iface_type,		/* insert all new types after this */
	passive_serial,			/* serial data and external clock */
	passive_parallel_synchronous,	/* parallel data */
	passive_parallel_asynchronous,	/* parallel data */
	passive_serial_asynchronous,	/* serial data w/ internal clock (not used)	*/
	altera_jtag_mode,		/* jtag/tap serial (not used ) */
	max_altera_iface_type		/* insert all new types before this */
} Altera_iface;				/* end, typedef Altera_iface */

typedef enum {			/* typedef Altera_Family */
    min_altera_type,		/* insert all new types after this */
    Altera_ACEX1K,		/* ACEX1K Family */
    Altera_CYC2,		/* CYCLONII Family */
/* Add new models here */
    max_altera_type		/* insert all new types before this */
} Altera_Family;		/* end, typedef Altera_Family */

typedef struct {		/* typedef Altera_desc */
	Altera_Family	family;	/* part type */
	Altera_iface	iface;	/* interface type */
	size_t		size;	/* bytes of data part can accept */
	void *		iface_fns;/* interface function table */
	void *		base;	/* base interface address */
	int		cookie;	/* implementation specific cookie */
} Altera_desc;			/* end, typedef Altera_desc */

/* Generic Altera Functions
 *********************************************************************/
extern int altera_load( Altera_desc *desc, void *image, size_t size );
extern int altera_dump( Altera_desc *desc, void *buf, size_t bsize );
extern int altera_info( Altera_desc *desc );
extern int altera_reloc( Altera_desc *desc, ulong reloc_offset );

/* Board specific implementation specific function types
 *********************************************************************/
typedef int (*Altera_pre_fn)( int cookie );
typedef int (*Altera_config_fn)( int assert_config, int flush, int cookie );
typedef int (*Altera_status_fn)( int cookie );
typedef int (*Altera_done_fn)( int cookie );
typedef int (*Altera_clk_fn)( int assert_clk, int flush, int cookie );
typedef int (*Altera_data_fn)( int assert_data, int flush, int cookie );
typedef int (*Altera_write_fn)(void *buf, size_t len, int flush, int cookie);
typedef int (*Altera_abort_fn)( int cookie );
typedef int (*Altera_post_fn)( int cookie );

#endif /* _ALTERA_H_ */
