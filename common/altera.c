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
 * Note that this is just boilerplate - there is no Altera support yet.
 */


/*
 *  Altera FPGA support
 */
#include <common.h>
#include <fpga.h>                     /* Generic FPGA support  */
#include <altera.h>                   /* Altera specific stuff */

#if 0
#define FPGA_DEBUG
#endif

#ifdef	FPGA_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#if (CONFIG_FPGA & CFG_FPGA_ALTERA)

/* ------------------------------------------------------------------------- */
int altera_load( Altera_desc *desc, void *buf, size_t bsize )
{
	printf( "No support for Altera devices yet.\n" );
	return FPGA_FAIL;
}

int altera_dump( Altera_desc *desc, void *buf, size_t bsize )
{
	printf( "No support for Altera devices yet.\n" );
	return FPGA_FAIL;
}

int altera_info( Altera_desc *desc )
{
	printf( "No support for Altera devices yet.\n" );
	return FPGA_FAIL;
}

/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */

#endif  /* CONFIG_FPGA & CFG_FPGA_ALTERA */
