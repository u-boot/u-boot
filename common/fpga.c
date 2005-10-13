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

/*
 *  Generic FPGA support
 */
#include <common.h>             /* core U-Boot definitions */
#include <xilinx.h>             /* xilinx specific definitions */
#include <altera.h>             /* altera specific definitions */

#if defined(CONFIG_FPGA)

#if 0
#define FPGA_DEBUG              /* define FPGA_DEBUG to get debug messages */
#endif

/* Local definitions */
#ifndef CONFIG_MAX_FPGA_DEVICES
#define CONFIG_MAX_FPGA_DEVICES		5
#endif

/* Enable/Disable debug console messages */
#ifdef FPGA_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define	PRINTF(fmt,args...)
#endif

/* Local static data */
static ulong relocation_offset = 0;
static int next_desc = FPGA_INVALID_DEVICE;
static fpga_desc desc_table[CONFIG_MAX_FPGA_DEVICES];

/* Local static functions */
static __attribute__((__const__)) fpga_desc * __attribute__((__const__)) fpga_get_desc( int devnum );
static __attribute__((__const__)) fpga_desc * __attribute__((__const__)) fpga_validate( int devnum, void *buf,
					 size_t bsize, char *fn );
static int fpga_dev_info( int devnum );


/* ------------------------------------------------------------------------- */

/* fpga_no_sup
 * 'no support' message function
 */
static void fpga_no_sup( char *fn, char *msg )
{
	if ( fn && msg ) {
		printf( "%s: No support for %s.  CONFIG_FPGA defined as 0x%x.\n",
				fn, msg, CONFIG_FPGA );
	} else if ( msg ) {
		printf( "No support for %s. CONFIG_FPGA defined as 0x%x.\n",
				msg, CONFIG_FPGA );
	} else {
		printf( "No FPGA suport!  CONFIG_FPGA defined as 0x%x.\n",
				CONFIG_FPGA );
	}
}


/* fpga_get_desc
 *	map a device number to a descriptor
 */
static __attribute__((__const__)) fpga_desc * __attribute__((__const__)) fpga_get_desc( int devnum )
{
	fpga_desc *desc = (fpga_desc * )NULL;

	if (( devnum >= 0 ) && (devnum < next_desc )) {
		desc = &desc_table[devnum];
		PRINTF( "%s: found fpga descriptor #%d @ 0x%p\n",
				__FUNCTION__, devnum, desc );
	}

	return desc;
}


/* fpga_validate
 *	generic parameter checking code
 */
static __attribute__((__const__)) fpga_desc * __attribute__((__const__)) fpga_validate( int devnum, void *buf,
					 size_t bsize, char *fn )
{
	fpga_desc * desc = fpga_get_desc( devnum );

	if ( !desc ) {
		printf( "%s: Invalid device number %d\n", fn, devnum );
	}

	if ( !buf ) {
		printf( "%s: Null buffer.\n", fn );
		return (fpga_desc * const)NULL;
	}
	if ( !bsize ) {
		printf( "%s: Null buffer size.\n", fn );
		return (fpga_desc * const)NULL;
	}

	return desc;
}


/* fpga_dev_info
 *	generic multiplexing code
 */
static int fpga_dev_info( int devnum )
{
	int ret_val = FPGA_FAIL;           /* assume failure */
	const fpga_desc * const desc = fpga_get_desc( devnum );

	if ( desc ) {
		PRINTF( "%s: Device Descriptor @ 0x%p\n",
				__FUNCTION__, desc->devdesc );

		switch ( desc->devtype ) {
		case fpga_xilinx:
#if CONFIG_FPGA & CFG_FPGA_XILINX
			printf( "Xilinx Device\nDescriptor @ 0x%p\n", desc );
			ret_val = xilinx_info( desc->devdesc );
#else
			fpga_no_sup( __FUNCTION__, "Xilinx devices" );
#endif
			break;
		case fpga_altera:
#if CONFIG_FPGA & CFG_FPGA_ALTERA
			printf( "Altera Device\nDescriptor @ 0x%p\n", desc );
			ret_val = altera_info( desc->devdesc );
#else
			fpga_no_sup( (char *)__FUNCTION__, "Altera devices" );
#endif
			break;
		default:
			printf( "%s: Invalid or unsupported device type %d\n",
					__FUNCTION__, desc->devtype );
		}
	} else {
		printf( "%s: Invalid device number %d\n",
			__FUNCTION__, devnum );
	}

	return ret_val;
}


/* fpga_reloc
 *	generic multiplexing code
 */
int fpga_reloc( fpga_type devtype, void *desc, ulong reloc_off )
{
	int ret_val = FPGA_FAIL;

	PRINTF( "%s: Relocating Device of type %d @ 0x%p with offset %lx\n",
				__FUNCTION__, devtype, desc, reloc_off );

	switch ( devtype ) {
	case fpga_xilinx:
#if CONFIG_FPGA & CFG_FPGA_XILINX
		ret_val = xilinx_reloc( desc, reloc_off );
#else
		fpga_no_sup( __FUNCTION__, "Xilinx devices" );
#endif
		break;
	case fpga_altera:
#if CONFIG_FPGA & CFG_FPGA_ALTERA
		ret_val = altera_reloc( desc, reloc_off );
#else
		fpga_no_sup( (char *)__FUNCTION__, "Altera devices" );
#endif
		break;
	default:
		printf( "%s: Invalid or unsupported device type %d\n",
			__FUNCTION__, devtype );
	}

	return ret_val;
}

/* ------------------------------------------------------------------------- */
/* fgpa_init is usually called from misc_init_r() and MUST be called
 * before any of the other fpga functions are used.
 */
void fpga_init( ulong reloc_off )
{
	relocation_offset = reloc_off;
	next_desc = 0;
	memset( desc_table, 0, sizeof(desc_table));

	PRINTF( "%s: CONFIG_FPGA = 0x%x\n", __FUNCTION__, CONFIG_FPGA );
#if 0
	PRINTF( "%s: CFG_FPGA_XILINX = 0x%x\n", __FUNCTION__, CFG_FPGA_XILINX );
	PRINTF( "%s: CFG_FPGA_ALTERA = 0x%x\n", __FUNCTION__, CFG_FPGA_ALTERA );
#endif
}

/* fpga_count
 * Basic interface function to get the current number of devices available.
 */
int fpga_count( void )
{
	return next_desc;
}

/* fpga_add
 *	Attempts to relocate the device/board specific interface code
 *	to the proper RAM locations and adds the device descriptor to
 *	the device table.
 */
int fpga_add( fpga_type devtype, void *desc )
{
	int devnum = FPGA_INVALID_DEVICE;

	if ( next_desc  < 0 ) {
		printf( "%s: FPGA support not initialized!\n", __FUNCTION__ );
	} else if (( devtype > fpga_min_type ) && ( devtype < fpga_undefined )) {
		if ( desc ) {
			if ( next_desc < CONFIG_MAX_FPGA_DEVICES ) {
				if ( fpga_reloc( devtype, desc, relocation_offset )
				  == FPGA_SUCCESS ) {
					devnum = next_desc;
					desc_table[next_desc].devtype = devtype;
					desc_table[next_desc++].devdesc = desc;
				} else {
					printf( "%s: Unable to relocate device interface table!\n",
						__FUNCTION__ );
				}
			} else {
				printf( "%s: Exceeded Max FPGA device count\n", __FUNCTION__ );
			}
		} else {
			printf( "%s: NULL device descriptor\n", __FUNCTION__ );
		}
	} else {
		printf( "%s: Unsupported FPGA type %d\n", __FUNCTION__, devtype );
	}

	return devnum;
}

/*
 *	Generic multiplexing code
 */
int fpga_load( int devnum, void *buf, size_t bsize )
{
	int ret_val = FPGA_FAIL;           /* assume failure */
	fpga_desc * desc = fpga_validate( devnum, buf, bsize, (char *)__FUNCTION__ );

	if ( desc ) {
		switch ( desc->devtype ) {
		case fpga_xilinx:
#if CONFIG_FPGA & CFG_FPGA_XILINX
			ret_val = xilinx_load( desc->devdesc, buf, bsize );
#else
			fpga_no_sup( __FUNCTION__, "Xilinx devices" );
#endif
			break;
		case fpga_altera:
#if CONFIG_FPGA & CFG_FPGA_ALTERA
			ret_val = altera_load( desc->devdesc, buf, bsize );
#else
			fpga_no_sup( (char *)__FUNCTION__, "Altera devices" );
#endif
			break;
		default:
			printf( "%s: Invalid or unsupported device type %d\n",
				__FUNCTION__, desc->devtype );
		}
	}

	return ret_val;
}

/* fpga_dump
 *	generic multiplexing code
 */
int fpga_dump( int devnum, void *buf, size_t bsize )
{
	int ret_val = FPGA_FAIL;           /* assume failure */
	fpga_desc * desc = fpga_validate( devnum, buf, bsize, (char *)__FUNCTION__ );

	if ( desc ) {
		switch ( desc->devtype ) {
		case fpga_xilinx:
#if CONFIG_FPGA & CFG_FPGA_XILINX
			ret_val = xilinx_dump( desc->devdesc, buf, bsize );
#else
			fpga_no_sup( __FUNCTION__, "Xilinx devices" );
#endif
			break;
		case fpga_altera:
#if CONFIG_FPGA & CFG_FPGA_ALTERA
			ret_val = altera_dump( desc->devdesc, buf, bsize );
#else
			fpga_no_sup( (char *)__FUNCTION__, "Altera devices" );
#endif
			break;
		default:
			printf( "%s: Invalid or unsupported device type %d\n",
				__FUNCTION__, desc->devtype );
		}
	}

	return ret_val;
}


/* fpga_info
 *	front end to fpga_dev_info.  If devnum is invalid, report on all
 *	available devices.
 */
int fpga_info( int devnum )
{
	if ( devnum == FPGA_INVALID_DEVICE ) {
		if ( next_desc > 0 ) {
			int dev;

			for ( dev = 0; dev < next_desc; dev++ ) {
				fpga_dev_info( dev );
			}
			return FPGA_SUCCESS;
		} else {
			printf( "%s: No FPGA devices available.\n", __FUNCTION__ );
			return FPGA_FAIL;
		}
	}
	else return fpga_dev_info( devnum );
}

/* ------------------------------------------------------------------------- */

#endif  /* CONFIG_FPGA */
