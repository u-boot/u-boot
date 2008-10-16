/*
 * (C) Copyright 2003
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

#include <common.h>
#include <asm/u-boot.h>
#include <asm/processor.h>
#include <ppc4xx.h>

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);

#ifdef MEIGSBOARD_ONBOARD_FLASH /* onboard = 2MB */
#  ifdef CONFIG_EXBITGEN
#     define FLASH_WORD_SIZE unsigned long
#  endif
#else /* Meigsboard socket flash = 512KB */
#  ifdef CONFIG_EXBITGEN
#    define FLASH_WORD_SIZE unsigned char
#  endif
#endif

#ifdef CONFIG_EXBITGEN
#define ADDR0           0x5555
#define ADDR1           0x2aaa
#define FLASH_WORD_SIZE unsigned char
#endif

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	unsigned long bank_size;
	unsigned long tot_size;
	unsigned long bank_addr;
	int i;

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		flash_info[i].size = 0;
	}

	tot_size = 0;

	/* Detect Boot Flash */
	bank_addr = CONFIG_SYS_FLASH0_BASE;
	bank_size = flash_get_size((vu_long *)bank_addr, &flash_info[0]);
	if (bank_size > 0) {
		(void)flash_protect(FLAG_PROTECT_CLEAR,
			bank_addr,
			bank_addr + bank_size - 1,
			&flash_info[0]);
	}
	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Boot Flash Bank\n");
	}
	flash_info[0].size = bank_size;
	tot_size += bank_size;

	/* Detect Application Flash */
	bank_addr = CONFIG_SYS_FLASH1_BASE;
	for (i = 1; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		bank_size = flash_get_size((vu_long *)bank_addr, &flash_info[i]);
		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
			break;
		}
		if (bank_size > 0) {
			(void)flash_protect(FLAG_PROTECT_CLEAR,
				bank_addr,
				bank_addr + bank_size - 1,
				&flash_info[i]);
		}
		flash_info[i].size = bank_size;
		tot_size += bank_size;
		bank_addr += bank_size;
	}
	if (flash_info[1].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Application Flash Bank\n");
	}

	/* Protect monitor and environment sectors */
#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH0_BASE
	flash_protect(FLAG_PROTECT_SET,
		CONFIG_SYS_MONITOR_BASE,
		CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
		&flash_info[0]);
#if 0xfffffffc >= CONFIG_SYS_FLASH0_BASE
#if 0xfffffffc <= CONFIG_SYS_FLASH0_BASE + CONFIG_SYS_FLASH0_SIZE - 1
	flash_protect(FLAG_PROTECT_SET,
		0xfffffffc, 0xffffffff,
		&flash_info[0]);
#endif
#endif
#endif

#if defined(CONFIG_ENV_IS_IN_FLASH) && defined(CONFIG_ENV_ADDR)
	flash_protect(FLAG_PROTECT_SET,
		CONFIG_ENV_ADDR,
		CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1,
		&flash_info[0]);
#endif

	return tot_size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	case FLASH_MAN_SST:	printf ("SST ");		break;
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
	case FLASH_AMDLV033C:	printf ("AM29LV033C (32 Mbit, uniform sector size)\n");
				break;
	case FLASH_AMDLV065D:	printf ("AM29LV065D (64 Mbit, uniform sector size)\n");
				break;
	case FLASH_SST800A:	printf ("SST39LF/VF800 (8 Mbit, uniform sector size)\n");
				break;
	case FLASH_SST160A:	printf ("SST39LF/VF160 (16 Mbit, uniform sector size)\n");
				break;
	case FLASH_SST040:	printf ("SST39LF/VF040 (4 Mbit, uniform sector size)\n");
				break;
	default:		printf ("Unknown Chip Type\n");
				break;
	}

	printf ("  Size: %ld KB in %d Sectors\n",
		info->size >> 10, info->sector_count);

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
	ulong base = (ulong)addr;
	volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *)addr;

	/* Write auto select command: read Manufacturer ID */
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
	addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00900090;

	value = addr2[0];

	switch (value) {
	case (FLASH_WORD_SIZE)AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;
	case (FLASH_WORD_SIZE)FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ;
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

	case (FLASH_WORD_SIZE)AMD_ID_LV033C:
		info->flash_id += FLASH_AMDLV033C;
		info->sector_count = 64;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/

	case (FLASH_WORD_SIZE)AMD_ID_LV065D:
		info->flash_id += FLASH_AMDLV065D;
		info->sector_count = 128;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/

	case (FLASH_WORD_SIZE)AMD_ID_LV320T:
		info->flash_id += FLASH_AM320T;
		info->sector_count = 67;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/

	case (FLASH_WORD_SIZE)AMD_ID_LV320B:
		info->flash_id += FLASH_AM320B;
		info->sector_count = 67;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/

	case (FLASH_WORD_SIZE)SST_ID_xF800A:
		info->flash_id += FLASH_SST800A;
		info->sector_count = 16;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/

	case (FLASH_WORD_SIZE)SST_ID_xF160A:
		info->flash_id += FLASH_SST160A;
		info->sector_count = 32;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/
	case (FLASH_WORD_SIZE)SST_ID_xF040:
		info->flash_id += FLASH_SST040;
		info->sector_count = 128;
		info->size = 0x00080000;
		break;				/* => 512KB		*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}

	/* set up sector start address table */
	if (((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_SST) ||
		(info->flash_id  == FLASH_AM040) ||
		(info->flash_id == FLASH_AMDLV033C) ||
		(info->flash_id == FLASH_AMDLV065D)) {
		ulong sectsize = info->size / info->sector_count;
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * sectsize);
	} else {
	    if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00004000;
		info->start[2] = base + 0x00006000;
		info->start[3] = base + 0x00008000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00010000) - 0x00030000;
		}
	    } else {
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00004000;
		info->start[i--] = base + info->size - 0x00006000;
		info->start[i--] = base + info->size - 0x00008000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00010000;
		}
	    }
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */

		addr2 = (volatile FLASH_WORD_SIZE *)(info->start[i]);
		if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_SST)
			info->protect[i] = 0;
		else
			info->protect[i] = addr2[2] & 1;
	}

	/* switch to the read mode */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr2 = (FLASH_WORD_SIZE *)info->start[0];
		*addr2 = (FLASH_WORD_SIZE)0x00F000F0;	/* reset bank */
	}

	return (info->size);
}

/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t *info, int s_first, int s_last)
{
	volatile FLASH_WORD_SIZE *addr = (FLASH_WORD_SIZE *)(info->start[0]);
	volatile FLASH_WORD_SIZE *addr2;
	int flag, prot, sect;
	ulong start, now, last;

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

	start = get_timer (0);
	last  = start;
	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr2 = (FLASH_WORD_SIZE *)(info->start[sect]);

			/* Disable interrupts which might cause a timeout here */
			flag = disable_interrupts();

			addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
			addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
			addr[ADDR0] = (FLASH_WORD_SIZE)0x00800080;
			addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
			addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
			addr2[0] = (FLASH_WORD_SIZE)0x00300030;

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);

			while ((addr2[0] & 0x00800080) !=
				(FLASH_WORD_SIZE) 0x00800080) {
				if ((now=get_timer(start)) >
					   CONFIG_SYS_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					addr[0] = (FLASH_WORD_SIZE)0x00F000F0;
					return 1;
				}

				/* show that we're waiting */
				if ((now - last) > 1000) {  /* every second  */
					putc ('.');
					last = now;
				}
			}

			addr[0] = (FLASH_WORD_SIZE)0x00F000F0;
		}
	}

	printf (" done\n");

	return 0;
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
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *)(info->start[0]);
	volatile FLASH_WORD_SIZE *dest2 = (FLASH_WORD_SIZE *)dest;
	volatile FLASH_WORD_SIZE *data2 = (FLASH_WORD_SIZE *)&data;
	ulong start;
	int flag;
	int i;

	/* Check if Flash is (sufficiently) erased */
	if ((*((volatile ulong *)dest) & data) != data) {
		printf("dest = %08lx, *dest = %08lx, data = %08lx\n",
			dest, *(volatile ulong *)dest, data);
		return 2;
	}

	for (i=0; i < 4/sizeof(FLASH_WORD_SIZE); i++) {
		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

		addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
		addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
		addr2[ADDR0] = (FLASH_WORD_SIZE)0x00A000A0;
		dest2[i] = data2[i];

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		/* data polling for D7 */
		start = get_timer (0);
		while ((dest2[i] & 0x00800080) != (data2[i] & 0x00800080)) {
			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
				addr2[0] = (FLASH_WORD_SIZE)0x00F000F0;
				return (1);
			}
		}
	}

	addr2[0] = (FLASH_WORD_SIZE)0x00F000F0;

	return (0);
}

/*-----------------------------------------------------------------------
 */
