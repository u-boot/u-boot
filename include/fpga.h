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

#include <linux/types.h>               /* for ulong typedef */

#ifndef _FPGA_H_
#define _FPGA_H_

#ifndef CONFIG_MAX_FPGA_DEVICES
#define CONFIG_MAX_FPGA_DEVICES		5
#endif

/* these probably belong somewhere else */
#ifndef FALSE
#define FALSE	(0)
#endif
#ifndef TRUE
#define TRUE 	(!FALSE)
#endif

/* CONFIG_FPGA bit assignments */
#define CFG_FPGA_MAN(x)	(x)
#define CFG_FPGA_DEV(x)	((x) << 8 )
#define CFG_FPGA_IF(x)	((x) << 16 )

/* FPGA Manufacturer bits in CONFIG_FPGA */
#define CFG_FPGA_XILINX 		CFG_FPGA_MAN( 0x1 )
#define CFG_FPGA_ALTERA			CFG_FPGA_MAN( 0x2 )


/* fpga_xxxx function return value definitions */
#define FPGA_SUCCESS         0
#define FPGA_FAIL           -1

/* device numbers must be non-negative */
#define FPGA_INVALID_DEVICE -1

/* root data type defintions */
typedef enum {                 /* typedef fpga_type */
	fpga_min_type,             /* range check value */
    fpga_xilinx,               /* Xilinx Family) */
    fpga_altera,               /* unimplemented */
    fpga_undefined             /* invalid range check value */
} fpga_type;                   /* end, typedef fpga_type */

typedef struct {               /* typedef fpga_desc */
    fpga_type   devtype;       /* switch value to select sub-functions */
    void *      devdesc;       /* real device descriptor */
} fpga_desc;                   /* end, typedef fpga_desc */


/* root function definitions */
extern void fpga_init( ulong reloc_off );
extern int fpga_add( fpga_type devtype, void *desc );
extern const int fpga_count( void );
extern int fpga_load( int devnum, void *buf, size_t bsize );
extern int fpga_dump( int devnum, void *buf, size_t bsize );
extern int fpga_info( int devnum );

#endif	/* _FPGA_H_ */
