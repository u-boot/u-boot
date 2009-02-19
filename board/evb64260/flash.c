/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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

/*
 * flash.c - flash support for the 512k, 8bit boot flash on the GEVB
 *           most of this file was based on the existing U-Boot
 *           flash drivers.
 */

#include <common.h>
#include <mpc8xx.h>
#include <galileo/gt64260R.h>
#include <galileo/memory.h>
#include "intel_flash.h"

#define FLASH_ROM       0xFFFD       /* unknown flash type                   */
#define FLASH_RAM       0xFFFE       /* unknown flash type                   */
#define FLASH_MAN_UNKNOWN 0xFFFF0000

/* #define DEBUG */
/* #define FLASH_ID_OVERRIDE */	/* Hack to set type to 040B if ROM emulator is installed.
				 * Can be used to program a ROM in circuit if a programmer
				 * is not available by swapping the rom out. */

/* Intel flash commands */
int flash_erase_intel(flash_info_t *info, int s_first, int s_last);
int write_word_intel(bank_addr_t addr, bank_word_t value);

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (int portwidth, vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets (ulong base, flash_info_t *info);

/*-----------------------------------------------------------------------
 */

unsigned long
flash_init (void)
{
	unsigned int i;
	unsigned long size_b0 = 0, size_b1 = 0;
	unsigned long base, flash_size;

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* the boot flash */
	base = CONFIG_SYS_FLASH_BASE;
#ifndef CONFIG_SYS_BOOT_FLASH_WIDTH
#define CONFIG_SYS_BOOT_FLASH_WIDTH	1
#endif
	size_b0 = flash_get_size(CONFIG_SYS_BOOT_FLASH_WIDTH, (vu_long *)base,
	                         &flash_info[0]);

#ifndef CONFIG_P3G4
	printf("[");
	print_size (size_b0, "");
	printf("@%08lX] ", base);
#endif

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH at %08lx: Size = 0x%08lx = %ld MB\n",
			base, size_b0, size_b0<<20);
	}

	base = memoryGetDeviceBaseAddress(CONFIG_SYS_EXTRA_FLASH_DEVICE);
	for(i=1;i<CONFIG_SYS_MAX_FLASH_BANKS;i++) {
	    unsigned long size = flash_get_size(CONFIG_SYS_EXTRA_FLASH_WIDTH, (vu_long *)base, &flash_info[i]);

#ifndef CONFIG_P3G4
	    printf("[");
	    print_size (size, "");
	    printf("@%08lX] ", base);
#endif

	    if (flash_info[i].flash_id == FLASH_UNKNOWN) {
		if(i==1) {
		    printf ("## Unknown FLASH at %08lx: Size = 0x%08lx = %ld MB\n",
			    base, size_b1, size_b1<<20);
		}
		break;
	    }
	    size_b1+=size;
	    base+=size;
	}

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
	              CONFIG_SYS_MONITOR_BASE,
	              CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
	              flash_get_info(CONFIG_SYS_MONITOR_BASE));
#endif

#ifdef  CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
	              CONFIG_ENV_ADDR,
	              CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1,
	              flash_get_info(CONFIG_ENV_ADDR));
#endif

	flash_size = size_b0 + size_b1;
	return flash_size;
}

/*-----------------------------------------------------------------------
 */
static void
flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;
	int sector_size;

	if(!info->sector_count) return;

	/* set up sector start address table */
	switch(info->flash_id & FLASH_TYPEMASK) {
	    case FLASH_AM040:
	    case FLASH_28F128J3A:
	    case FLASH_28F640J3A:
	    case FLASH_RAM:
		/* this chip has uniformly spaced sectors */
		sector_size=info->size/info->sector_count;
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * sector_size);
		break;
	    default:
		if (info->flash_id & FLASH_BTYPE) {
		    /* set sector offsets for bottom boot block type	*/
		    info->start[0] = base + 0x00000000;
		    info->start[1] = base + 0x00008000;
		    info->start[2] = base + 0x0000C000;
		    info->start[3] = base + 0x00010000;
		    for (i = 4; i < info->sector_count; i++) {
			    info->start[i] = base + (i * 0x00020000) - 0x00060000;
		    }
		} else {
		    /* set sector offsets for top boot block type		*/
		    i = info->sector_count - 1;
		    info->start[i--] = base + info->size - 0x00008000;
		    info->start[i--] = base + info->size - 0x0000C000;
		    info->start[i--] = base + info->size - 0x00010000;
		    for (; i >= 0; i--) {
			    info->start[i] = base + i * 0x00020000;
		    }
		}
	}
}

/*-----------------------------------------------------------------------
 */

flash_info_t *flash_get_info(ulong base)
{
	int i;
	flash_info_t * info;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i ++) {
		info = & flash_info[i];
		if (info->start[0] <= base && base <= info->start[0] + info->size - 1)
			break;
	}

	return i == CONFIG_SYS_MAX_FLASH_BANKS ? 0 : info;
}

/*-----------------------------------------------------------------------
 */
void
flash_print_info  (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	case FLASH_MAN_INTEL:	printf ("INTEL ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM040:
		printf ("AM29LV040B (4 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM400B:
		printf ("AM29LV400B (4 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM400T:
		printf ("AM29LV400T (4 Mbit, top boot sector)\n");
		break;
	case FLASH_AM800B:
		printf ("AM29LV800B (8 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM800T:
		printf ("AM29LV800T (8 Mbit, top boot sector)\n");
		break;
	case FLASH_AM160B:
		printf ("AM29LV160B (16 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM160T:
		printf ("AM29LV160T (16 Mbit, top boot sector)\n");
		break;
	case FLASH_AM320B:
		printf ("AM29LV320B (32 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM320T:
		printf ("AM29LV320T (32 Mbit, top boot sector)\n");
		break;
	case FLASH_28F640J3A:
		printf ("28F640J3A (64 Mbit)\n");
		break;
	case FLASH_28F128J3A:
		printf ("28F128J3A (128 Mbit)\n");
		break;
	case FLASH_ROM:
		printf ("ROM\n");
		break;
	case FLASH_RAM:
		printf ("RAM\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		break;
	}

	puts ("  Size: ");
	print_size (info->size, "");
	printf (" in %d Sectors\n", info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     "
		);
	}
	printf ("\n");
	return;
}

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

static inline void flash_cmd(int width, volatile unsigned char *addr, int offset, unsigned char cmd)
{
	/* supports 1x8, 1x16, and 2x16 */
	/* 2x8 and 4x8 are not supported */
	if(width==4) {
	    /* assuming chips are in 16 bit mode */
	    /* 2x16 */
	    unsigned long cmd32=(cmd<<16)|cmd;
	    *(volatile unsigned long *)(addr+offset*2)=cmd32;
	} else if (width == 2) {
	    /* 1x16 */
	    *(volatile unsigned short *)((unsigned short*)addr+offset)=cmd;
	} else {
	    /* 1x8 */
	    *(volatile unsigned char *)(addr+offset)=cmd;
	}
}

static ulong
flash_get_size (int portwidth, vu_long *addr, flash_info_t *info)
{
	short i;
	volatile unsigned char *caddr = (unsigned char *)addr;
	volatile unsigned short *saddr = (unsigned short *)addr;
	volatile unsigned long *laddr = (unsigned long *)addr;
	char old[2], save;
	ulong id, manu, base = (ulong)addr;

	info->portwidth=portwidth;

	save = *caddr;

	flash_cmd(portwidth,caddr,0,0xf0);
	flash_cmd(portwidth,caddr,0,0xf0);

	udelay(10);

	old[0] = caddr[0];
	old[1] = caddr[1];


	if(old[0]!=0xf0) {
	    flash_cmd(portwidth,caddr,0,0xf0);
	    flash_cmd(portwidth,caddr,0,0xf0);

	    udelay(10);

	    if(*caddr==0xf0) {
		/* this area is ROM */
		*caddr=save;
#ifndef FLASH_ID_OVERRIDE
		info->flash_id = FLASH_ROM + FLASH_MAN_UNKNOWN;
		info->sector_count = 8;
		info->size = 0x80000;
#else
		info->flash_id = FLASH_MAN_AMD + FLASH_AM040;
		info->sector_count = 8;
		info->size = 0x80000;
		info->chipwidth=1;
#endif
		flash_get_offsets(base, info);
		return info->size;
	    }
	} else {
	    *caddr=0;

	    udelay(10);

	    if(*caddr==0) {
		/* this area is RAM */
		*caddr=save;
		info->flash_id = FLASH_RAM + FLASH_MAN_UNKNOWN;
		info->sector_count = 8;
		info->size = 0x80000;
		flash_get_offsets(base, info);
		return info->size;
	    }
	    flash_cmd(portwidth,caddr,0,0xf0);

	    udelay(10);
	}

	/* Write auto select command: read Manufacturer ID */
	flash_cmd(portwidth,caddr,0x555,0xAA);
	flash_cmd(portwidth,caddr,0x2AA,0x55);
	flash_cmd(portwidth,caddr,0x555,0x90);

	udelay(10);

	if ((caddr[0] == old[0]) &&
	    (caddr[1] == old[1])) {

	    /* this area is ROM */
#ifndef FLASH_ID_OVERRIDE
	    info->flash_id = FLASH_ROM + FLASH_MAN_UNKNOWN;
	    info->sector_count = 8;
	    info->size = 0x80000;
#else
		info->flash_id = FLASH_MAN_AMD + FLASH_AM040;
		info->sector_count = 8;
		info->size = 0x80000;
		info->chipwidth=1;
#endif
	    flash_get_offsets(base, info);
	    return info->size;
#ifdef DEBUG
	} else {
	    printf("%px%d: %02x:%02x -> %02x:%02x\n",
		    caddr, portwidth, old[0], old[1],
		    caddr[0], caddr[1]);
#endif
	}

	switch(portwidth) {
	    case 1:
		manu = caddr[0];
		manu |= manu<<16;
		id = caddr[1];
		break;
	    case 2:
		manu = saddr[0];
		manu |= manu<<16;
		id = saddr[1];
		id |= id<<16;
		break;
	    case 4:
		manu = laddr[0];
		id = laddr[1];
		break;
	default:
		id = manu = -1;
		break;
	}

#ifdef DEBUG
	printf("\n%08lx:%08lx:%08lx\n", base, manu, id);
	printf("%08lx %08lx %08lx %08lx\n",
		laddr[0],laddr[1],laddr[2],laddr[3]);
#endif

	switch (manu) {
	    case AMD_MANUFACT:
		    info->flash_id = FLASH_MAN_AMD;
		    break;
	    case FUJ_MANUFACT:
		    info->flash_id = FLASH_MAN_FUJ;
		    break;
	    case INTEL_MANUFACT:
		    info->flash_id = FLASH_MAN_INTEL;
		    break;
	    default:
		    printf("Unknown Mfr [%08lx]:%08lx\n", manu, id);
		    info->flash_id = FLASH_UNKNOWN;
		    info->sector_count = 0;
		    info->size = 0;
		    return (0);			/* no or unknown flash	*/
	}

	switch (id) {
	    case AMD_ID_LV400T:
		    info->flash_id += FLASH_AM400T;
		    info->sector_count = 11;
		    info->size = 0x00100000;
		    info->chipwidth=1;
		    break;				/* => 1 MB	*/

	    case AMD_ID_LV400B:
		    info->flash_id += FLASH_AM400B;
		    info->sector_count = 11;
		    info->size = 0x00100000;
		    info->chipwidth=1;
		    break;				/* => 1 MB	*/

	    case AMD_ID_LV800T:
		    info->flash_id += FLASH_AM800T;
		    info->sector_count = 19;
		    info->size = 0x00200000;
		    info->chipwidth=1;
		    break;				/* => 2 MB	*/

	    case AMD_ID_LV800B:
		    info->flash_id += FLASH_AM800B;
		    info->sector_count = 19;
		    info->size = 0x00200000;
		    info->chipwidth=1;
		    break;				/* => 2 MB	*/

	    case AMD_ID_LV160T:
		    info->flash_id += FLASH_AM160T;
		    info->sector_count = 35;
		    info->size = 0x00400000;
		    info->chipwidth=1;
		    break;				/* => 4 MB	*/

	    case AMD_ID_LV160B:
		    info->flash_id += FLASH_AM160B;
		    info->sector_count = 35;
		    info->size = 0x00400000;
		    info->chipwidth=1;
		    break;				/* => 4 MB	*/
#if 0	/* enable when device IDs are available */
	    case AMD_ID_LV320T:
		    info->flash_id += FLASH_AM320T;
		    info->sector_count = 67;
		    info->size = 0x00800000;
		    break;				/* => 8 MB	*/

	    case AMD_ID_LV320B:
		    info->flash_id += FLASH_AM320B;
		    info->sector_count = 67;
		    info->size = 0x00800000;
		    break;				/* => 8 MB	*/
#endif
	    case AMD_ID_LV040B:
		    info->flash_id += FLASH_AM040;
		    info->sector_count = 8;
		    info->size = 0x80000;
		    info->chipwidth=1;
		    break;

	    case INTEL_ID_28F640J3A:
		    info->flash_id += FLASH_28F640J3A;
		    info->sector_count = 64;
		    info->size = 128*1024 * 64; /* 128kbytes x 64 blocks */
		    info->chipwidth=2;
		    if(portwidth==4) info->size*=2;	/* 2x16 */
		    break;

	    case INTEL_ID_28F128J3A:
		    info->flash_id += FLASH_28F128J3A;
		    info->sector_count = 128;
		    info->size = 128*1024 * 128; /* 128kbytes x 128 blocks */
		    info->chipwidth=2;
		    if(portwidth==4) info->size*=2;	/* 2x16 */
		    break;

	    default:
		    printf("Unknown id %lx:[%lx]\n", manu, id);
		    info->flash_id = FLASH_UNKNOWN;
		    info->chipwidth=1;
		    return (0);			/* => no or unknown flash */

	}

	flash_get_offsets(base, info);

#if 0
	/* set up sector start address table */
	if (info->flash_id & FLASH_AM040) {
		/* this chip has uniformly spaced sectors */
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x00010000);

	} else if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00008000;
		info->start[2] = base + 0x0000C000;
		info->start[3] = base + 0x00010000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00020000) - 0x00060000;
		}
	} else {
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00008000;
		info->start[i--] = base + info->size - 0x0000C000;
		info->start[i--] = base + info->size - 0x00010000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00020000;
		}
	}
#endif

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0)=0x02 */
		/* D0 = 1 if protected */
		caddr = (volatile unsigned char *)(info->start[i]);
		saddr = (volatile unsigned short *)(info->start[i]);
		laddr = (volatile unsigned long *)(info->start[i]);
		if(portwidth==1)
		    info->protect[i] = caddr[2] & 1;
		else if(portwidth==2)
		    info->protect[i] = saddr[2] & 1;
		else
		    info->protect[i] = laddr[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		caddr = (volatile unsigned char *)info->start[0];

		flash_cmd(portwidth,caddr,0,0xF0);	/* reset bank */
	}

	return (info->size);
}

/* TODO: 2x16 unsupported */
int
flash_erase (flash_info_t *info, int s_first, int s_last)
{
	volatile unsigned char *addr = (uchar *)(info->start[0]);
	int flag, prot, sect, l_sect;
	ulong start, now, last;

	/* TODO: 2x16 unsupported */
	if(info->portwidth==4) return 1;

	if((info->flash_id & FLASH_TYPEMASK) == FLASH_ROM) return 1;
	if((info->flash_id & FLASH_TYPEMASK) == FLASH_RAM) {
	    for (sect = s_first; sect<=s_last; sect++) {
		int sector_size=info->size/info->sector_count;
		addr = (uchar *)(info->start[sect]);
		memset((void *)addr, 0, sector_size);
	    }
	    return 0;
	}

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id&FLASH_VENDMASK) == FLASH_MAN_INTEL)  {
		return flash_erase_intel(info,
				(unsigned short)s_first,
				(unsigned short)s_last);
	}

#if 0
	if ((info->flash_id == FLASH_UNKNOWN) ||
	    (info->flash_id > FLASH_AMD_COMP)) {
		printf ("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
		return 1;
	}
#endif

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf ("\n");
	}

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	flash_cmd(info->portwidth,addr,0x555,0xAA);
	flash_cmd(info->portwidth,addr,0x2AA,0x55);
	flash_cmd(info->portwidth,addr,0x555,0x80);
	flash_cmd(info->portwidth,addr,0x555,0xAA);
	flash_cmd(info->portwidth,addr,0x2AA,0x55);

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (uchar *)(info->start[sect]);
			flash_cmd(info->portwidth,addr,0,0x30);
			l_sect = sect;
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* wait at least 80us - let's wait 1 ms */
	udelay (1000);

	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;

	start = get_timer (0);
	last  = start;
	addr = (volatile unsigned char *)(info->start[l_sect]);
	/* broken for 2x16: TODO */
	while ((addr[0] & 0x80) != 0x80) {
		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			putc ('.');
			last = now;
		}
	}

DONE:
	/* reset to read mode */
	addr = (volatile unsigned char *)info->start[0];
	flash_cmd(info->portwidth,addr,0,0xf0);
	flash_cmd(info->portwidth,addr,0,0xf0);

	printf (" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

/* broken for 2x16: TODO */
int
write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	if(info->portwidth==4) return 1;

	if((info->flash_id & FLASH_TYPEMASK) == FLASH_ROM) return 0;
	if((info->flash_id & FLASH_TYPEMASK) == FLASH_RAM) {
	    memcpy((void *)addr, src, cnt);
	    return 0;
	}

	wp = (addr & ~3);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i=0, cp=wp; i<l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}
		for (; i<4 && cnt>0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt==0 && i<4; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}

		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp += 4;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		data = 0;
		for (i=0; i<4; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += 4;
		cnt -= 4;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i=0, cp=wp; i<4 && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<4; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
	}

	return (write_word(info, wp, data));
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
/* broken for 2x16: TODO */
static int
write_word (flash_info_t *info, ulong dest, ulong data)
{
	volatile unsigned char *addr = (uchar *)(info->start[0]);
	ulong start;
	int flag, i;

	if(info->portwidth==4) return 1;

	if((info->flash_id & FLASH_TYPEMASK) == FLASH_ROM) return 1;
	if((info->flash_id & FLASH_TYPEMASK) == FLASH_RAM) {
	    *(unsigned long *)dest=data;
	    return 0;
	}
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)  {
		unsigned short low = data & 0xffff;
		unsigned short hi  = (data >> 16) & 0xffff;
		int ret = write_word_intel((bank_addr_t)dest, hi);

		if (!ret) ret = write_word_intel((bank_addr_t)(dest+2), low);

		return ret;
	}

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_long *)dest) & data) != data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* first, perform an unlock bypass command to speed up flash writes */
	addr[0x555] = 0xAA;
	addr[0x2AA] = 0x55;
	addr[0x555] = 0x20;

	/* write each byte out */
	for (i = 0; i < 4; i++) {
		char *data_ch = (char *)&data;
		addr[0] = 0xA0;
		*(((char *)dest)+i) = data_ch[i];
		udelay(10); /* XXX */
	}

	/* we're done, now do an unlock bypass reset */
	addr[0] = 0x90;
	addr[0] = 0x00;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);
	while ((*((vu_long *)dest) & 0x00800080) != (data & 0x00800080)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			return (1);
		}
	}
	return (0);
}
