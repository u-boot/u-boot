/*
 * (C) Copyright 2000, 2001
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

/*
 * Modified 4/5/2001
 * Wait for completion of each sector erase command issued
 * 4/5/2001
 * Chris Hallinan - DS4.COM, Inc. - clh@net1plus.com
 */

/*
 * Modified 3/7/2001
 * - adapted for pip405, Denis Peter, MPL AG Switzerland
 * TODO:
 * clean-up
 */

#include <common.h>

#if !defined(CONFIG_PATI)
#include <ppc4xx.h>
#include <asm/processor.h>
#include "common_util.h"
#if defined(CONFIG_MIP405)
#include "../mip405/mip405.h"
#endif
#if defined(CONFIG_PIP405)
#include "../pip405/pip405.h"
#endif
#include <asm/4xx_pci.h>
#else /* defined(CONFIG_PATI) */
#include <mpc5xx.h>
#endif

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/
/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);

void unlock_intel_sectors(flash_info_t *info,ulong addr,ulong cnt);

#define ADDR0           0x5555
#define ADDR1           0x2aaa
#define FLASH_WORD_SIZE unsigned short

#define FALSE           0
#define TRUE            1

#if !defined(CONFIG_PATI)

/*-----------------------------------------------------------------------
 * Some CS switching routines:
 *
 * On PIP/MIP405 we have 3 (4) possible boot mode
 *
 * - Boot from Flash (Flash CS = CS0, MPS CS = CS1)
 * - Boot from MPS   (Flash CS = CS1, MPS CS = CS0)
 * - Boot from PCI with Flash map (Flash CS = CS0, MPS CS = CS1)
 * - Boot from PCI with MPS map   (Flash CS = CS1, MPS CS = CS0)
 * The flash init is the first board specific routine which is called
 * after code relocation (running from SDRAM)
 * The first thing we do is to map the Flash CS to the Flash area and
 * the MPS CS to the MPS area. Since the flash size is unknown at this
 * point, we use the max flash size and the lowest flash address as base.
 *
 * After flash detection we adjust the size of the CS area accordingly.
 * The board_init_r will fill in wrong values in the board init structure,
 * but this will be fixed in the misc_init_r routine:
 * bd->bi_flashstart=0-flash_info[0].size
 * bd->bi_flashsize=flash_info[0].size-CONFIG_SYS_MONITOR_LEN
 * bd->bi_flashoffset=0
 *
 */
int get_boot_mode(void)
{
	unsigned long pbcr;
	int res = 0;
	pbcr = mfdcr (CPC0_PSR);
	if ((pbcr & PSR_ROM_WIDTH_MASK) == 0)
		/* boot via MPS or MPS mapping */
		res = BOOT_MPS;
	if(pbcr & PSR_ROM_LOC)
		/* boot via PCI.. */
		res |= BOOT_PCI;
	 return res;
}

/* Map the flash high (in boot area)
   This code can only be executed from SDRAM (after relocation).
*/
void setup_cs_reloc(void)
{
	int mode;
	/* Since we are relocated, we can set-up the CS finaly
	 * but first of all, switch off PCI mapping (in case it was a PCI boot) */
	out32r(PMM0MA,0L);
	icache_enable (); /* we are relocated */
	/* get boot mode */
	mode=get_boot_mode();
	/* we map the flash high in every case */
	/* first findout on which cs the flash is */
	if(mode & BOOT_MPS) {
		/* map flash high on CS1 and MPS on CS0 */
		mtdcr (EBC0_CFGADDR, PB0AP);
		mtdcr (EBC0_CFGDATA, MPS_AP);
		mtdcr (EBC0_CFGADDR, PB0CR);
		mtdcr (EBC0_CFGDATA, MPS_CR);
		/* we use the default values (max values) for the flash
		 * because its real size is not yet known */
		mtdcr (EBC0_CFGADDR, PB1AP);
		mtdcr (EBC0_CFGDATA, FLASH_AP);
		mtdcr (EBC0_CFGADDR, PB1CR);
		mtdcr (EBC0_CFGDATA, FLASH_CR_B);
	}
	else {
		/* map flash high on CS0 and MPS on CS1 */
		mtdcr (EBC0_CFGADDR, PB1AP);
		mtdcr (EBC0_CFGDATA, MPS_AP);
		mtdcr (EBC0_CFGADDR, PB1CR);
		mtdcr (EBC0_CFGDATA, MPS_CR);
		/* we use the default values (max values) for the flash
		 * because its real size is not yet known */
		mtdcr (EBC0_CFGADDR, PB0AP);
		mtdcr (EBC0_CFGDATA, FLASH_AP);
		mtdcr (EBC0_CFGADDR, PB0CR);
		mtdcr (EBC0_CFGDATA, FLASH_CR_B);
	}
}

#endif /* #if !defined(CONFIG_PATI) */

unsigned long flash_init (void)
{
	unsigned long size_b0;
	int i;

#if !defined(CONFIG_PATI)
	unsigned long size_b1,flashcr,size_reg;
	int mode;
	extern char version_string;
	char *p = &version_string;

	/* Since we are relocated, we can set-up the CS finally */
	setup_cs_reloc();
	/* get and display boot mode */
	mode=get_boot_mode();
	if(mode & BOOT_PCI)
		printf("(PCI Boot %s Map) ",(mode & BOOT_MPS) ?
			"MPS" : "Flash");
	else
		printf("(%s Boot) ",(mode & BOOT_MPS) ?
			"MPS" : "Flash");
#endif /* #if !defined(CONFIG_PATI) */
	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	size_b0 = flash_get_size((vu_long *)CONFIG_SYS_MONITOR_BASE, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size_b0, size_b0<<20);
	}
	/* protect the bootloader */
	/* Monitor protection ON by default */
#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	flash_protect(FLAG_PROTECT_SET,
			CONFIG_SYS_MONITOR_BASE,
			CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
			&flash_info[0]);
#endif
#if !defined(CONFIG_PATI)
	/* protect reset vector */
	flash_info[0].protect[flash_info[0].sector_count-1] = 1;
	size_b1 = 0 ;
	flash_info[0].size = size_b0;
	/* set up flash cs according to the size */
	size_reg=(flash_info[0].size >>20);
	switch (size_reg) {
		case 0:
		case 1: i=0; break; /* <= 1MB */
		case 2: i=1; break; /* = 2MB */
		case 4: i=2; break; /* = 4MB */
		case 8: i=3; break; /* = 8MB */
		case 16: i=4; break; /* = 16MB */
		case 32: i=5; break; /* = 32MB */
		case 64: i=6; break; /* = 64MB */
		case 128: i=7; break; /*= 128MB */
		default:
			printf("\n #### ERROR, wrong size %ld MByte reset board #####\n",size_reg);
			while(1);
	}
	if(mode & BOOT_MPS) {
		/* flash is on CS1 */
		mtdcr(EBC0_CFGADDR, PB1CR);
		flashcr = mfdcr (EBC0_CFGDATA);
		/* we map the flash high in every case */
		flashcr&=0x0001FFFF; /* mask out address bits */
		flashcr|= ((0-flash_info[0].size) & 0xFFF00000); /* start addr */
		flashcr|= (i << 17); /* size addr */
		mtdcr(EBC0_CFGADDR, PB1CR);
		mtdcr(EBC0_CFGDATA, flashcr);
	}
	else {
		/* flash is on CS0 */
		mtdcr(EBC0_CFGADDR, PB0CR);
		flashcr = mfdcr (EBC0_CFGDATA);
		/* we map the flash high in every case */
		flashcr&=0x0001FFFF; /* mask out address bits */
		flashcr|= ((0-flash_info[0].size) & 0xFFF00000); /* start addr */
		flashcr|= (i << 17); /* size addr */
		mtdcr(EBC0_CFGADDR, PB0CR);
		mtdcr(EBC0_CFGDATA, flashcr);
	}
#if 0
	/* enable this (PIP405/MIP405 only) if you want to test if
	   the relocation has be done ok.
	   This will disable both Chipselects */
	mtdcr (EBC0_CFGADDR, PB0CR);
	mtdcr (EBC0_CFGDATA, 0L);
	mtdcr (EBC0_CFGADDR, PB1CR);
	mtdcr (EBC0_CFGDATA, 0L);
	printf("CS0 & CS1 switched off for test\n");
#endif
	/* patch version_string */
	for(i=0;i<0x100;i++) {
		if(*p=='\n') {
			*p=0;
			break;
		}
		p++;
	}
#else /* #if !defined(CONFIG_PATI) */
#ifdef	CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR+CONFIG_ENV_SECT_SIZE-1,
		      &flash_info[0]);
#endif
#endif /* #if !defined(CONFIG_PATI) */
	return (size_b0);
}


/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
	int i;
	int k;
	int size;
	int erased;
	volatile unsigned long *flash;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	case FLASH_MAN_SST:	printf ("SST ");		break;
	case FLASH_MAN_INTEL:	printf ("Intel ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM040:	printf ("AM29F040 (512 Kbit, uniform sector size)\n");
				break;
	case FLASH_AM400B:	printf ("AM29LV400B (4 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM400T:	printf ("AM29LV400T (4 Mbit, top boot sector)\n");
				break;
	case FLASH_AM800B:	printf ("AM29LV800B (8 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM800T:	printf ("AM29LV800T (8 Mbit, top boot sector)\n");
				break;
	case FLASH_AM160B:	printf ("AM29LV160B (16 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM160T:	printf ("AM29LV160T (16 Mbit, top boot sector)\n");
				break;
	case FLASH_AM320B:	printf ("AM29LV320B (32 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM320T:	printf ("AM29LV320T (32 Mbit, top boot sector)\n");
				break;
	case FLASH_SST800A:	printf ("SST39LF/VF800 (8 Mbit, uniform sector size)\n");
				break;
	case FLASH_SST160A:	printf ("SST39LF/VF160 (16 Mbit, uniform sector size)\n");
				break;
	case FLASH_INTEL320T:	printf ("TE28F320C3 (32 Mbit, top sector size)\n");
				break;
	case FLASH_AM640U:	printf ("AM29LV640U (64 Mbit, uniform sector size)\n");
				break;
	default:		printf ("Unknown Chip Type\n");
				break;
	}

	printf ("  Size: %ld KB in %d Sectors\n",
		info->size >> 10, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		/*
		 * Check if whole sector is erased
		*/
		if (i != (info->sector_count-1))
			size = info->start[i+1] - info->start[i];
		else
			size = info->start[0] + info->size - info->start[i];
		erased = 1;
		flash = (volatile unsigned long *)info->start[i];
		size = size >> 2;        /* divide by 4 for longword access */
		for (k=0; k<size; k++) {
			if (*flash++ != 0xffffffff) {
				erased = 0;
				break;
			}
		}
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s%s",
			info->start[i],
			erased ? " E" : "  ",
			info->protect[i] ? "RO " : "   ");
	}
	printf ("\n");
}

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------

*/

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	short i;
	FLASH_WORD_SIZE value;
	ulong base;
	volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *)addr;

	/* Write auto select command: read Manufacturer ID */
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
	addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00900090;

	value = addr2[0];
	/*	printf("flash_get_size value: %x\n",value); */
	switch (value) {
	case (FLASH_WORD_SIZE)AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;
	case (FLASH_WORD_SIZE)FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case (FLASH_WORD_SIZE)INTEL_MANUFACT:
		info->flash_id = FLASH_MAN_INTEL;
		break;
	case (FLASH_WORD_SIZE)SST_MANUFACT:
		info->flash_id = FLASH_MAN_SST;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}
	value = addr2[1];			/* device ID		*/
	/*	printf("Device value %x\n",value);		    */
	switch (value) {
	case (FLASH_WORD_SIZE)AMD_ID_F040B:
		info->flash_id += FLASH_AM040;
		info->sector_count = 8;
		info->size = 0x0080000; /* => 512 ko */
		break;
	case (FLASH_WORD_SIZE)AMD_ID_LV400T:
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00080000;
		break;				/* => 0.5 MB		*/

	case (FLASH_WORD_SIZE)AMD_ID_LV400B:
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00080000;
		break;				/* => 0.5 MB		*/

	case (FLASH_WORD_SIZE)AMD_ID_LV800T:
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/

	case (FLASH_WORD_SIZE)AMD_ID_LV800B:
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/

	case (FLASH_WORD_SIZE)AMD_ID_LV160T:
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/

	case (FLASH_WORD_SIZE)AMD_ID_LV160B:
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/
	case (FLASH_WORD_SIZE)AMD_ID_LV320T:
		info->flash_id += FLASH_AM320T;
		info->sector_count = 67;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/
	case (FLASH_WORD_SIZE)AMD_ID_LV640U:
		info->flash_id += FLASH_AM640U;
		info->sector_count = 128;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/
#if 0	/* enable when device IDs are available */

	case (FLASH_WORD_SIZE)AMD_ID_LV320B:
		info->flash_id += FLASH_AM320B;
		info->sector_count = 67;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/
#endif
	case (FLASH_WORD_SIZE)SST_ID_xF800A:
		info->flash_id += FLASH_SST800A;
		info->sector_count = 16;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/
	case (FLASH_WORD_SIZE)INTEL_ID_28F320C3T:
		info->flash_id += FLASH_INTEL320T;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/


	case (FLASH_WORD_SIZE)SST_ID_xF160A:
		info->flash_id += FLASH_SST160A;
		info->sector_count = 32;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}
	/* base address calculation */
	base=0-info->size;
	/* set up sector start address table */
	if (((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_SST) ||
	     (info->flash_id  == FLASH_AM040) ||
	     (info->flash_id  == FLASH_AM640U)){
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x00010000);
	}
	else {
		if (info->flash_id & FLASH_BTYPE) {
			/* set sector offsets for bottom boot block type	*/
			info->start[0] = base + 0x00000000;
			info->start[1] = base + 0x00004000;
			info->start[2] = base + 0x00006000;
			info->start[3] = base + 0x00008000;
			for (i = 4; i < info->sector_count; i++)
				info->start[i] = base + (i * 0x00010000) - 0x00030000;
		}
		else {
			/* set sector offsets for top boot block type		*/
			i = info->sector_count - 1;
			if(info->sector_count==71) {

				info->start[i--] = base + info->size - 0x00002000;
				info->start[i--] = base + info->size - 0x00004000;
				info->start[i--] = base + info->size - 0x00006000;
				info->start[i--] = base + info->size - 0x00008000;
				info->start[i--] = base + info->size - 0x0000A000;
				info->start[i--] = base + info->size - 0x0000C000;
				info->start[i--] = base + info->size - 0x0000E000;
				for (; i >= 0; i--)
					info->start[i] = base + i * 0x000010000;
			}
			else {
				info->start[i--] = base + info->size - 0x00004000;
				info->start[i--] = base + info->size - 0x00006000;
				info->start[i--] = base + info->size - 0x00008000;
				for (; i >= 0; i--)
					info->start[i] = base + i * 0x00010000;
			}
		}
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr2 = (volatile FLASH_WORD_SIZE *)(info->start[i]);
		if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
			info->protect[i] = 0;
		else
			info->protect[i] = addr2[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr2 = (FLASH_WORD_SIZE *)info->start[0];
		*addr2 = (FLASH_WORD_SIZE)0x00F000F0;	/* reset bank */
	}
	return (info->size);
}


int wait_for_DQ7(flash_info_t *info, int sect)
{
	ulong start, now, last;
	volatile FLASH_WORD_SIZE *addr = (FLASH_WORD_SIZE *)(info->start[sect]);

	start = get_timer (0);
	last  = start;
	while ((addr[0] & (FLASH_WORD_SIZE)0x00800080) != (FLASH_WORD_SIZE)0x00800080) {
		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return ERR_TIMOUT;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {  /* every second */
			putc ('.');
			last = now;
		}
	}
	return ERR_OK;
}

int intel_wait_for_DQ7(flash_info_t *info, int sect)
{
	ulong start, now, last, status;
	volatile FLASH_WORD_SIZE *addr = (FLASH_WORD_SIZE *)(info->start[sect]);

	start = get_timer (0);
	last  = start;
	while ((addr[0] & (FLASH_WORD_SIZE)0x00800080) != (FLASH_WORD_SIZE)0x00800080) {
		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return ERR_TIMOUT;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {  /* every second */
			putc ('.');
			last = now;
		}
	}
	status = addr[0] & (FLASH_WORD_SIZE)0x00280028;
	/* clear status register */
	addr[0] = (FLASH_WORD_SIZE)0x00500050;
	/* check status for block erase fail and VPP low */
	return (status == 0 ? ERR_OK : ERR_NOT_ERASED);
}

/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	volatile FLASH_WORD_SIZE *addr = (FLASH_WORD_SIZE *)(info->start[0]);
	volatile FLASH_WORD_SIZE *addr2;
	int flag, prot, sect, l_sect;
	int i, rcode = 0;


	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("Can't erase unknown flash type - aborted\n");
		return 1;
	}

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

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr2 = (FLASH_WORD_SIZE *)(info->start[sect]);
			/*  printf("Erasing sector %p\n", addr2); */ /* CLH */
			if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_SST) {
				addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
				addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
				addr[ADDR0] = (FLASH_WORD_SIZE)0x00800080;
				addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
				addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
				addr2[0] = (FLASH_WORD_SIZE)0x00500050;  /* block erase */
				for (i=0; i<50; i++)
					udelay(1000);  /* wait 1 ms */
				rcode |= wait_for_DQ7(info, sect);
			}
			else {
				if((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL){
					addr2[0] = (FLASH_WORD_SIZE)0x00600060;  /* unlock sector */
					addr2[0] = (FLASH_WORD_SIZE)0x00D000D0;  /* sector erase */
					intel_wait_for_DQ7(info, sect);
					addr2[0] = (FLASH_WORD_SIZE)0x00200020;  /* sector erase */
					addr2[0] = (FLASH_WORD_SIZE)0x00D000D0;  /* sector erase */
					rcode |= intel_wait_for_DQ7(info, sect);
				}
				else {
					addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
					addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
					addr[ADDR0] = (FLASH_WORD_SIZE)0x00800080;
					addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
					addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
					addr2[0] = (FLASH_WORD_SIZE)0x00300030;  /* sector erase */
					rcode |= wait_for_DQ7(info, sect);
				}
			}
			l_sect = sect;
			/*
			 * Wait for each sector to complete, it's more
			 * reliable.  According to AMD Spec, you must
			 * issue all erase commands within a specified
			 * timeout.  This has been seen to fail, especially
			 * if printf()s are included (for debug)!!
			 */
			/*   wait_for_DQ7(info, sect); */
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* wait at least 80us - let's wait 1 ms */
	udelay (1000);

#if 0
	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;
	wait_for_DQ7(info, l_sect);

DONE:
#endif
	/* reset to read mode */
	addr = (FLASH_WORD_SIZE *)info->start[0];
	addr[0] = (FLASH_WORD_SIZE)0x00F000F0;	/* reset bank */

	if (!rcode)
	    printf (" done\n");

	return rcode;
}


void unlock_intel_sectors(flash_info_t *info,ulong addr,ulong cnt)
{
	int i;
	volatile FLASH_WORD_SIZE *addr2;
	long c;
	c= (long)cnt;
	for(i=info->sector_count-1;i>0;i--)
	{
		if(addr>=info->start[i])
			break;
	}
	do {
		addr2 = (FLASH_WORD_SIZE *)(info->start[i]);
		addr2[0] = (FLASH_WORD_SIZE)0x00600060;  /* unlock sector setup */
		addr2[0] = (FLASH_WORD_SIZE)0x00D000D0;  /* unlock sector */
		intel_wait_for_DQ7(info, i);
		i++;
		c-=(info->start[i]-info->start[i-1]);
	}while(c>0);
}


/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	if((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL){
		unlock_intel_sectors(info,addr,cnt);
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
		if((wp % 0x10000)==0)
			printf("."); /* show Progress */
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
	rc=write_word(info, wp, data);
	return rc;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static FLASH_WORD_SIZE *read_val = (FLASH_WORD_SIZE *)0x200000;

static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	volatile FLASH_WORD_SIZE *addr2 = (volatile FLASH_WORD_SIZE *)(info->start[0]);
	volatile FLASH_WORD_SIZE *dest2 = (volatile FLASH_WORD_SIZE *)dest;
	volatile FLASH_WORD_SIZE *data2;
	ulong start;
	ulong *data_p;
	int flag;
	int i;

	data_p = &data;
	data2 = (volatile FLASH_WORD_SIZE *)data_p;

	/* Check if Flash is (sufficiently) erased */
	if ((*((volatile FLASH_WORD_SIZE *)dest) &
		(FLASH_WORD_SIZE)data) != (FLASH_WORD_SIZE)data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();
	for (i=0; i<4/sizeof(FLASH_WORD_SIZE); i++)
	{
		if((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL){
			/* intel style writting */
			dest2[i] = (FLASH_WORD_SIZE)0x00500050;
			dest2[i] = (FLASH_WORD_SIZE)0x00400040;
			*read_val++ = data2[i];
			dest2[i] = data2[i];
			if (flag)
				enable_interrupts();
			/* data polling for D7 */
			start = get_timer (0);
			udelay(10);
			while ((dest2[i] & (FLASH_WORD_SIZE)0x00800080) != (FLASH_WORD_SIZE)0x00800080)
			{
				if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT)
					return (1);
			}
			dest2[i] = (FLASH_WORD_SIZE)0x00FF00FF; /* return to read mode */
			udelay(10);
			dest2[i] = (FLASH_WORD_SIZE)0x00FF00FF; /* return to read mode */
			if(dest2[i]!=data2[i])
				printf("Error at %p 0x%04X != 0x%04X\n",&dest2[i],dest2[i],data2[i]);
		}
		else {
			addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
			addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
			addr2[ADDR0] = (FLASH_WORD_SIZE)0x00A000A0;
			dest2[i] = data2[i];
			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();
			/* data polling for D7 */
			start = get_timer (0);
			while ((dest2[i] & (FLASH_WORD_SIZE)0x00800080) !=
				(data2[i] & (FLASH_WORD_SIZE)0x00800080)) {
				if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
					return (1);
				}
			}
		}
	}
	return (0);
}

/*-----------------------------------------------------------------------
 */
