/*
 * (C) Copyright 2004
 * Xiaogeng (Shawn) Jin, Agilent Technologies, xiaogeng_jin@agilent.com
 *
 * (C) Copyright 2001
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2001-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
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

#include <common.h>
#include <linux/byteorder/swab.h>

#define DEBUG

#define PHYS_FLASH_SECT_SIZE	0x00040000	/* 256 KB sectors (x2) */
flash_info_t flash_info[CFG_MAX_FLASH_BANKS];	/* info for FLASH chips */

/* Board support for 1 or 2 flash devices */
#define FLASH_PORT_WIDTH32
#undef FLASH_PORT_WIDTH16

#ifdef FLASH_PORT_WIDTH16
#define FLASH_PORT_WIDTH	ushort
#define FLASH_PORT_WIDTHV	vu_short
#define SWAP(x)			__swab16(x)
#else
#define FLASH_PORT_WIDTH	ulong
#define FLASH_PORT_WIDTHV	vu_long
#define SWAP(x)			__swab32(x)
#endif

#define FPW	FLASH_PORT_WIDTH
#define FPWV	FLASH_PORT_WIDTHV

#define mb() __asm__ __volatile__ ("" : : : "memory")


/* Flash Organization Structure */
typedef struct OrgDef {
	unsigned int sector_number;
	unsigned int sector_size;
} OrgDef;


/* Flash Organizations */
OrgDef OrgIntel_28F256L18T[] = {
	{4, 32 * 1024},				/* 4 * 32kBytes sectors */
	{255, 128 * 1024},			/* 255 * 128kBytes sectors */
};

/* CP control register base address */
#define CPCR_BASE      		0xCB000000
#define CPCR_EXTRABANK		0x8
#define CPCR_FLASHSIZE		0x4
#define CPCR_FLWREN		0x2
#define CPCR_FLVPPEN		0x1

/*-----------------------------------------------------------------------
 * Functions
 */
unsigned long flash_init (void);
static ulong flash_get_size (FPW * addr, flash_info_t * info);
static int write_data (flash_info_t * info, ulong dest, FPW data);
static void flash_get_offsets (ulong base, flash_info_t * info);
void inline spin_wheel (void);
void flash_print_info (flash_info_t * info);
void flash_unprotect_sectors (FPWV * addr);
int flash_erase (flash_info_t * info, int s_first, int s_last);
int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt);

/*-----------------------------------------------------------------------
 */
unsigned long flash_init (void)
{
	int i, nbanks;
	ulong size = 0;
	vu_long *cpcr = (vu_long *)CPCR_BASE;

	/* Check if there is an extra bank of flash */
	if (cpcr[1] & CPCR_EXTRABANK)
		nbanks = 2;
	else
		nbanks = 1;

	if (nbanks > CFG_MAX_FLASH_BANKS)
		nbanks = CFG_MAX_FLASH_BANKS;

	/* Enable flash write */
	cpcr[1] |= 3;

	for (i = 0; i < nbanks; i++) {
		flash_get_size ((FPW *)(CFG_FLASH_BASE + size), &flash_info[i]);
		flash_get_offsets (CFG_FLASH_BASE + size, &flash_info[i]);
		size += flash_info[i].size;
	}

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	/* monitor protection */
	flash_protect (FLAG_PROTECT_SET,
		       CFG_MONITOR_BASE,
		       CFG_MONITOR_BASE + monitor_flash_len - 1, &flash_info[0]);
#endif

#ifdef CFG_ENV_IS_IN_FLASH
	/* ENV protection ON */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_ENV_ADDR,
		      CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1,
		      &flash_info[0]);
#endif

	/* Protect SIB (0x24800000) and bootMonitor (0x24c00000) */
	flash_protect (FLAG_PROTECT_SET,
		       flash_info[0].start[62],
		       flash_info[0].start[63] + PHYS_FLASH_SECT_SIZE - 1,
		       &flash_info[0]);

	return size;
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t * info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		return;
	}

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL) {
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base +	(i * PHYS_FLASH_SECT_SIZE);
			info->protect[i] = 0;
		}
	}
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:
		printf ("INTEL ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	/* Integrator CP board uses 28F640J3C or 28F128J3C parts,
	 * which have the same device id numbers as 28F640J3A or
	 * 28F128J3A
	 */
	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F256L18T:
		printf ("FLASH 28F256L18T\n");
		break;
	case FLASH_28F640J3A:
		printf ("FLASH 28F640J3C\n");
		break;
	case FLASH_28F128J3A:
		printf ("FLASH 28F128J3C\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i], info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
	return;
}

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (FPW * addr, flash_info_t * info)
{
	volatile FPW value;
	vu_long *cpcr = (vu_long *)CPCR_BASE;
	int nsects;

	/* Check the flash size */
	if (cpcr[1] & CPCR_FLASHSIZE)
		nsects = 128;
	else
		nsects = 64;

	if (nsects > CFG_MAX_FLASH_SECT)
		nsects = CFG_MAX_FLASH_SECT;

	/* Write auto select command: read Manufacturer ID */
	addr[0x5555] = (FPW) 0x00AA00AA;
	addr[0x2AAA] = (FPW) 0x00550055;
	addr[0x5555] = (FPW) 0x00900090;

	mb ();
	value = addr[0];

	switch (value) {

	case (FPW) INTEL_MANUFACT:
		info->flash_id = FLASH_MAN_INTEL;
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		addr[0] = (FPW) 0x00FF00FF; /* restore read mode */
		return (0); /* no or unknown flash */
	}

	mb ();
	value = addr[1]; /* device ID */
	switch (value) {

	case (FPW) (INTEL_ID_28F256L18T):
		info->flash_id += FLASH_28F256L18T;
		info->sector_count = 259;
		info->size = 0x02000000;
		break;			/* => 32 MB */

	case (FPW) (INTEL_ID_28F640J3A):
		info->flash_id += FLASH_28F640J3A;
		info->sector_count = nsects;
		info->size = nsects * PHYS_FLASH_SECT_SIZE;
		break;

	case (FPW) (INTEL_ID_28F128J3A):
		info->flash_id += FLASH_28F128J3A;
		info->sector_count = nsects;
		info->size = nsects * PHYS_FLASH_SECT_SIZE;
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		break;
	}

	if (info->sector_count > CFG_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
				info->sector_count, CFG_MAX_FLASH_SECT);
		info->sector_count = CFG_MAX_FLASH_SECT;
	}

	addr[0] = (FPW) 0x00FF00FF;	/* restore read mode */

	return (info->size);
}


/* unprotects a sector for write and erase
 * on some intel parts, this unprotects the entire chip, but it
 * wont hurt to call this additional times per sector...
 */
void flash_unprotect_sectors (FPWV * addr)
{
	FPW status;

	*addr = (FPW) 0x00500050;	/* clear status register */

	/* this sends the clear lock bit command */
	*addr = (FPW) 0x00600060;
	*addr = (FPW) 0x00D000D0;

	reset_timer_masked();
	while (((status = *addr) & (FPW)0x00800080) != 0x00800080) {
		if (get_timer_masked() > CFG_FLASH_ERASE_TOUT) {
			printf("Timeout");
			break;
		}
	}

	*addr = (FPW) 0x00FF00FF;
}


/*-----------------------------------------------------------------------
 */
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int flag, prot, sect;
	ulong type;
	int rcode = 0;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	type = (info->flash_id & FLASH_VENDMASK);
	if ((type != FLASH_MAN_INTEL)) {
		printf ("Can't erase unknown flash type %08lx - aborted\n",
				info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
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

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			FPWV *addr = (FPWV *) (info->start[sect]);
			FPW status;

			printf ("Erasing sector %2d ... ", sect);

			/* Disable interrupts which might cause a timeout here */
			flag = disable_interrupts ();

			/* flash_unprotect_sectors (addr); */

			/* arm simple, non interrupt dependent timer */
			reset_timer_masked ();

			*addr = (FPW) 0x00500050; /* clear status register */
			*addr = (FPW) 0x00200020; /* erase setup */
			*addr = (FPW) 0x00D000D0; /* erase confirm */
			mb();

			udelay(1000); /* Let's wait 1 ms */

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();

			while (((status = *addr) & (FPW) 0x00800080) != (FPW) 0x00800080) {
				if (get_timer_masked () > CFG_FLASH_ERASE_TOUT) {
					*addr = (FPW)0x00700070;
					status = *addr;
					if ((status & (FPW) 0x00400040) == (FPW) 0x00400040) {
						/* erase suspended? Resume it */
						reset_timer_masked();
						*addr = (FPW) 0x00D000D0;
					} else {
#ifdef DEBUG
						printf ("Timeout,0x%08x\n", status);
#else
						printf("Timeout\n");
#endif

						*addr = (FPW) 0x00500050;
						*addr = (FPW) 0x00FF00FF; /* reset to read mode */
						rcode = 1;
						break;
					}
				}
			}

			*addr = (FPW) 0x00FF00FF; /* resest to read mode */
			printf (" done\n");
		}
	}

	return rcode;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - Flash not identified
 */
int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong cp, wp;
	FPW data;
	int count, i, l, rc, port_width;

	if (info->flash_id == FLASH_UNKNOWN) {
		return 4;
	}
/* get lower word aligned address */
#ifdef FLASH_PORT_WIDTH16
	wp = (addr & ~1);
	port_width = 2;
#else
	wp = (addr & ~3);
	port_width = 4;
#endif

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *) cp);
		}
		for (; i < port_width && cnt > 0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < port_width; ++i, ++cp) {
			data = (data << 8) | (*(uchar *) cp);
		}

		if ((rc = write_data (info, wp, SWAP (data))) != 0) {
			return (rc);
		}
		wp += port_width;
	}

	/*
	 * handle word aligned part
	 */
	count = 0;
	while (cnt >= port_width) {
		data = 0;
		for (i = 0; i < port_width; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_data (info, wp, SWAP (data))) != 0) {
			return (rc);
		}
		wp += port_width;
		cnt -= port_width;
		if (count++ > 0x800) {
			spin_wheel ();
			count = 0;
		}
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < port_width && cnt > 0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i < port_width; ++i, ++cp) {
		data = (data << 8) | (*(uchar *) cp);
	}

	return (write_data (info, wp, SWAP (data)));
}

/*-----------------------------------------------------------------------
 * Write a word or halfword to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t * info, ulong dest, FPW data)
{
	FPWV *addr = (FPWV *) dest;
	ulong status;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & data) != data) {
		printf ("not erased at %08lx (%x)\n", (ulong) addr, *addr);
		return (2);
	}

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	/* flash_unprotect_sectors (addr); */

	*addr = (FPW) 0x00400040;	/* write setup */
	*addr = data;

	mb();

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked ();

	/* wait while polling the status register */
	while (((status = *addr) & (FPW) 0x00800080) != (FPW) 0x00800080) {
		if (get_timer_masked () > CFG_FLASH_WRITE_TOUT) {
#ifdef DEBUG
			*addr = (FPW) 0x00700070;
			status = *addr;
			printf("## status=0x%08x, addr=0x%08x\n", status, addr);
#endif
			*addr = (FPW) 0x00500050; /* clear status register cmd */
			*addr = (FPW) 0x00FF00FF; /* restore read mode */
			return (1);
		}
	}

	*addr = (FPW) 0x00FF00FF; /* restore read mode */
	return (0);
}

void inline spin_wheel (void)
{
	static int p = 0;
	static char w[] = "\\/-";

	printf ("\010%c", w[p]);
	(++p == 3) ? (p = 0) : 0;
}
