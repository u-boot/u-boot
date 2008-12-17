/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifndef _PCMCIA_H
#define _PCMCIA_H

#include <common.h>
#include <config.h>

/*
 * Allow configuration to select PCMCIA slot,
 * or try to generate a useful default
 */
#if defined(CONFIG_CMD_PCMCIA) || \
    (defined(CONFIG_CMD_IDE) && \
	(defined(CONFIG_IDE_8xx_PCCARD) || defined(CONFIG_IDE_8xx_DIRECT) ) )

#if !defined(CONFIG_PCMCIA_SLOT_A) && !defined(CONFIG_PCMCIA_SLOT_B)

					/* The RPX series use SLOT_B	*/
#if defined(CONFIG_RPXCLASSIC) || defined(CONFIG_RPXLITE)
# define CONFIG_PCMCIA_SLOT_B
#elif defined(CONFIG_ADS)		/* The ADS  board uses SLOT_A	*/
# define CONFIG_PCMCIA_SLOT_A
#elif defined(CONFIG_FADS)		/* The FADS series are a mess	*/
# if defined(CONFIG_MPC86x) || defined(CONFIG_MPC821)
#  define CONFIG_PCMCIA_SLOT_A
# else
#  define CONFIG_PCMCIA_SLOT_B
# endif
#elif defined(CONFIG_TQM8xxL) || defined(CONFIG_SVM_SC8xx)
# define	CONFIG_PCMCIA_SLOT_B	/* The TQM8xxL use SLOT_B	*/
#elif defined(CONFIG_SPD823TS)		/* The SPD8xx  use SLOT_B	*/
# define CONFIG_PCMCIA_SLOT_B
#elif defined(CONFIG_IVMS8) || defined(CONFIG_IVML24)	/* The IVM* use SLOT_A	*/
# define CONFIG_PCMCIA_SLOT_A
#elif defined(CONFIG_LWMON)		/* The LWMON  use SLOT_B	*/
# define CONFIG_PCMCIA_SLOT_B
#elif defined(CONFIG_ICU862)		/* The ICU862 use SLOT_B	*/
# define CONFIG_PCMCIA_SLOT_B
#elif defined(CONFIG_C2MON)		/* The C2MON  use SLOT_B	*/
# define CONFIG_PCMCIA_SLOT_B
#elif defined(CONFIG_R360MPI)		/* The R360MPI use SLOT_B	*/
# define CONFIG_PCMCIA_SLOT_B
#elif defined(CONFIG_ATC)		/* The ATC use SLOT_A	*/
# define CONFIG_PCMCIA_SLOT_A
#elif defined(CONFIG_NETTA)
# define CONFIG_PCMCIA_SLOT_A
#elif defined(CONFIG_UC100)		/* The UC100 use SLOT_B	        */
# define CONFIG_PCMCIA_SLOT_B
#else
# error "PCMCIA Slot not configured"
#endif

#endif /* !defined(CONFIG_PCMCIA_SLOT_A) && !defined(CONFIG_PCMCIA_SLOT_B) */

/* Make sure exactly one slot is defined - we support only one for now */
#if !defined(CONFIG_PCMCIA_SLOT_A) && !defined(CONFIG_PCMCIA_SLOT_B)
#error Neither CONFIG_PCMCIA_SLOT_A nor CONFIG_PCMCIA_SLOT_B configured
#endif
#if defined(CONFIG_PCMCIA_SLOT_A) && defined(CONFIG_PCMCIA_SLOT_B)
#error Both CONFIG_PCMCIA_SLOT_A and CONFIG_PCMCIA_SLOT_B configured
#endif

#ifndef PCMCIA_SOCKETS_NO
#define PCMCIA_SOCKETS_NO	1
#endif
#ifndef PCMCIA_MEM_WIN_NO
#define PCMCIA_MEM_WIN_NO	4
#endif
#define PCMCIA_IO_WIN_NO	2

/* define _slot_ to be able to optimize macros */
#ifdef CONFIG_PCMCIA_SLOT_A
# define _slot_			0
# define PCMCIA_SLOT_MSG	"slot A"
# define PCMCIA_SLOT_x		PCMCIA_PSLOT_A
#else
# define _slot_			1
# define PCMCIA_SLOT_MSG	"slot B"
# define PCMCIA_SLOT_x		PCMCIA_PSLOT_B
#endif

/*
 * The TQM850L hardware has two pins swapped! Grrrrgh!
 */
#ifdef	CONFIG_TQM850L
#define __MY_PCMCIA_GCRX_CXRESET	PCMCIA_GCRX_CXOE
#define __MY_PCMCIA_GCRX_CXOE		PCMCIA_GCRX_CXRESET
#else
#define __MY_PCMCIA_GCRX_CXRESET	PCMCIA_GCRX_CXRESET
#define __MY_PCMCIA_GCRX_CXOE		PCMCIA_GCRX_CXOE
#endif

/*
 * This structure is used to address each window in the PCMCIA controller.
 *
 * Keep in mind that we assume that pcmcia_win_t[n+1] is mapped directly
 * after pcmcia_win_t[n]...
 */

typedef struct {
	ulong	br;
	ulong	or;
} pcmcia_win_t;

/*
 * Definitions for PCMCIA control registers to operate in IDE mode
 *
 * All timing related setup (PCMCIA_SHT, PCMCIA_SST, PCMCIA_SL)
 * to be done later (depending on CPU clock)
 */

/* Window 0:
 *	Base: 0xFE100000	CS1
 *	Port Size:     2 Bytes
 *	Port Size:    16 Bit
 *	Common Memory Space
 */

#define CONFIG_SYS_PCMCIA_PBR0		0xFE100000
#define CONFIG_SYS_PCMCIA_POR0	    (	PCMCIA_BSIZE_2	\
			    |	PCMCIA_PPS_16	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 1:
 *	Base: 0xFE100080	CS1
 *	Port Size:     8 Bytes
 *	Port Size:     8 Bit
 *	Common Memory Space
 */

#define CONFIG_SYS_PCMCIA_PBR1		0xFE100080
#define CONFIG_SYS_PCMCIA_POR1	    (	PCMCIA_BSIZE_8	\
			    |	PCMCIA_PPS_8	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 2:
 *	Base: 0xFE100100	CS2
 *	Port Size:     8 Bytes
 *	Port Size:     8 Bit
 *	Common Memory Space
 */

#define CONFIG_SYS_PCMCIA_PBR2		0xFE100100
#define CONFIG_SYS_PCMCIA_POR2	    (	PCMCIA_BSIZE_8	\
			    |	PCMCIA_PPS_8	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 3:
 *	not used
 */
#define CONFIG_SYS_PCMCIA_PBR3		0
#define CONFIG_SYS_PCMCIA_POR3		0

/* Window 4:
 *	Base: 0xFE100C00	CS1
 *	Port Size:     2 Bytes
 *	Port Size:    16 Bit
 *	Common Memory Space
 */

#define CONFIG_SYS_PCMCIA_PBR4		0xFE100C00
#define CONFIG_SYS_PCMCIA_POR4	    (	PCMCIA_BSIZE_2	\
			    |	PCMCIA_PPS_16	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 5:
 *	Base: 0xFE100C80	CS1
 *	Port Size:     8 Bytes
 *	Port Size:     8 Bit
 *	Common Memory Space
 */

#define CONFIG_SYS_PCMCIA_PBR5		0xFE100C80
#define CONFIG_SYS_PCMCIA_POR5	    (	PCMCIA_BSIZE_8	\
			    |	PCMCIA_PPS_8	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 6:
 *	Base: 0xFE100D00	CS2
 *	Port Size:     8 Bytes
 *	Port Size:     8 Bit
 *	Common Memory Space
 */

#define CONFIG_SYS_PCMCIA_PBR6		0xFE100D00
#define CONFIG_SYS_PCMCIA_POR6	    (	PCMCIA_BSIZE_8	\
			    |	PCMCIA_PPS_8	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 7:
 *	not used
 */
#define CONFIG_SYS_PCMCIA_PBR7		0
#define CONFIG_SYS_PCMCIA_POR7		0

/**********************************************************************/

/*
 * CIS Tupel codes
 */
#define CISTPL_NULL		0x00
#define CISTPL_DEVICE		0x01
#define CISTPL_LONGLINK_CB	0x02
#define CISTPL_INDIRECT		0x03
#define CISTPL_CONFIG_CB	0x04
#define CISTPL_CFTABLE_ENTRY_CB 0x05
#define CISTPL_LONGLINK_MFC	0x06
#define CISTPL_BAR		0x07
#define CISTPL_PWR_MGMNT	0x08
#define CISTPL_EXTDEVICE	0x09
#define CISTPL_CHECKSUM		0x10
#define CISTPL_LONGLINK_A	0x11
#define CISTPL_LONGLINK_C	0x12
#define CISTPL_LINKTARGET	0x13
#define CISTPL_NO_LINK		0x14
#define CISTPL_VERS_1		0x15
#define CISTPL_ALTSTR		0x16
#define CISTPL_DEVICE_A		0x17
#define CISTPL_JEDEC_C		0x18
#define CISTPL_JEDEC_A		0x19
#define CISTPL_CONFIG		0x1a
#define CISTPL_CFTABLE_ENTRY	0x1b
#define CISTPL_DEVICE_OC	0x1c
#define CISTPL_DEVICE_OA	0x1d
#define CISTPL_DEVICE_GEO	0x1e
#define CISTPL_DEVICE_GEO_A	0x1f
#define CISTPL_MANFID		0x20
#define CISTPL_FUNCID		0x21
#define CISTPL_FUNCE		0x22
#define CISTPL_SWIL		0x23
#define CISTPL_END		0xff

/*
 * CIS Function ID codes
 */
#define CISTPL_FUNCID_MULTI	0x00
#define CISTPL_FUNCID_MEMORY	0x01
#define CISTPL_FUNCID_SERIAL	0x02
#define CISTPL_FUNCID_PARALLEL	0x03
#define CISTPL_FUNCID_FIXED	0x04
#define CISTPL_FUNCID_VIDEO	0x05
#define CISTPL_FUNCID_NETWORK	0x06
#define CISTPL_FUNCID_AIMS	0x07
#define CISTPL_FUNCID_SCSI	0x08

/*
 * Fixed Disk FUNCE codes
 */
#define CISTPL_IDE_INTERFACE	0x01

#define CISTPL_FUNCE_IDE_IFACE	0x01
#define CISTPL_FUNCE_IDE_MASTER	0x02
#define CISTPL_FUNCE_IDE_SLAVE	0x03

/* First feature byte */
#define CISTPL_IDE_SILICON	0x04
#define CISTPL_IDE_UNIQUE	0x08
#define CISTPL_IDE_DUAL		0x10

/* Second feature byte */
#define CISTPL_IDE_HAS_SLEEP	0x01
#define CISTPL_IDE_HAS_STANDBY	0x02
#define CISTPL_IDE_HAS_IDLE	0x04
#define CISTPL_IDE_LOW_POWER	0x08
#define CISTPL_IDE_REG_INHIBIT	0x10
#define CISTPL_IDE_HAS_INDEX	0x20
#define CISTPL_IDE_IOIS16	0x40

#endif

#ifdef	CONFIG_8xx
extern u_int *pcmcia_pgcrx[];
#define	PCMCIA_PGCRX(slot)	(*pcmcia_pgcrx[slot])
#endif

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_8xx_PCCARD) \
	|| defined(CONFIG_PXA_PCMCIA)
extern int check_ide_device(int slot);
#endif

#endif /* _PCMCIA_H */
