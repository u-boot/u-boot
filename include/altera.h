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
 * Note that this is just  Altera FPGA interface boilerplate.
 * There is no support for Altera devices yet.
 *
 * See include/xilinx.h for a working example.
 */

/* In your board's config.h file you should define CONFIG_FPGA as such:
 *	#define CONFIG_FPGA 	(CFG_ALTERA_xxx | CFG_ALTERA_IF_xxx )
 */

/* Altera Model definitions */
#define CFG_ALTERA_xxxx		( CFG_FPGA_ALTERA | CFG_FPGA_DEV( 0x1 ))
/* Add new models here */

/* Altera Interface definitions */
#define CFG_ALTERA_IF_xxx		CFG_FPGA_IF( 0x1 )
/* Add new interfaces here */

typedef enum {                     /* typedef Altera_iface */
    min_altera_iface_type,        /* insert all new types after this */
/* Add new interfaces here */
    max_altera_iface_type         /* insert all new types before this */
} Altera_iface;                   /* end, typedef Altera_iface */

typedef enum {                     /* typedef Altera_Family */
    min_altera_type,              /* insert all new types after this */
/* Add new models here */
    max_altera_type               /* insert all new types before this */
} Altera_Family;                  /* end, typedef Altera_Family */

typedef struct {                   /* typedef Altera_desc */
    Altera_Family    family;      /* part type */
    Altera_iface     iface;       /* interface type */
    size_t            size;        /* bytes of data part can accept */
    void *            base;        /* base interface address */
} Altera_desc;                    /* end, typedef Altera_desc */

extern int altera_load( Altera_desc *desc, void *image, size_t size );
extern int altera_dump( Altera_desc *desc, void *buf, size_t bsize );
extern int altera_info( Altera_desc *desc );
extern int altera_reloc( Altera_desc *desc, ulong reloc_off );

#endif  /* _ALTERA_H_ */
