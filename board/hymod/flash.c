/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Hacked for the Hymod board by Murray.Jensen@csiro.au, 20-Oct-00
 */

#include <common.h>
#include <mpc8260.h>
#include <board/hymod/flash.h>

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];	/* info for FLASH chips */

/*-----------------------------------------------------------------------
 * Protection Flags:
 */
#define FLAG_PROTECT_SET	0x01
#define FLAG_PROTECT_CLEAR	0x02

/*-----------------------------------------------------------------------
 */

/*
 * probe for flash bank at address "base" and store info about it
 * in the flash_info entry "fip". Fatal error if nothing there.
 */
static void
bank_probe (flash_info_t *fip, volatile bank_addr_t base)
{
	volatile bank_addr_t addr;
	bank_word_t word;
	int i;

	/* reset the flash */
	*base = BANK_CMD_RST;

	/* put flash into read id mode */
	*base = BANK_CMD_RD_ID;

	/* check the manufacturer id - must be intel */
	word = *BANK_REG_MAN_CODE (base);
	if (word != BANK_FILL_WORD (INTEL_MANUFACT&0xff))
		panic ("\nbad manufacturer's code (0x%08lx) at addr 0x%08lx",
			(unsigned long)word, (unsigned long)base);

	/* check the device id */
	word = *BANK_REG_DEV_CODE (base);
	switch (word) {

	case BANK_FILL_WORD (INTEL_ID_28F320J5&0xff):
		fip->flash_id = FLASH_MAN_INTEL | FLASH_28F320J5;
		fip->sector_count = 32;
		break;

	case BANK_FILL_WORD (INTEL_ID_28F640J5&0xff):
		fip->flash_id = FLASH_MAN_INTEL | FLASH_28F640J5;
		fip->sector_count = 64;
		break;

	case BANK_FILL_WORD (INTEL_ID_28F320J3A&0xff):
		fip->flash_id = FLASH_MAN_INTEL | FLASH_28F320J3A;
		fip->sector_count = 32;
		break;

	case BANK_FILL_WORD (INTEL_ID_28F640J3A&0xff):
		fip->flash_id = FLASH_MAN_INTEL | FLASH_28F640J3A;
		fip->sector_count = 64;
		break;

	case BANK_FILL_WORD (INTEL_ID_28F128J3A&0xff):
		fip->flash_id = FLASH_MAN_INTEL | FLASH_28F128J3A;
		fip->sector_count = 128;
		break;

	default:
		panic ("\nbad device code (0x%08lx) at addr 0x%08lx",
			(unsigned long)word, (unsigned long)base);
	}

	if (fip->sector_count >= CONFIG_SYS_MAX_FLASH_SECT)
		panic ("\ntoo many sectors (%d) in flash at address 0x%08lx",
			fip->sector_count, (unsigned long)base);

	addr = base;
	for (i = 0; i < fip->sector_count; i++) {
		fip->start[i] = (unsigned long)addr;
		fip->protect[i] = 0;
		addr = BANK_ADDR_NEXT_BLK (addr);
	}

	fip->size = (bank_size_t)addr - (bank_size_t)base;

	/* reset the flash */
	*base = BANK_CMD_RST;
}

static void
bank_reset (flash_info_t *info, int sect)
{
	volatile bank_addr_t addr = (bank_addr_t)info->start[sect];

#ifdef FLASH_DEBUG
	printf ("writing reset cmd to addr 0x%08lx\n", (unsigned long)addr);
#endif

	*addr = BANK_CMD_RST;
}

static void
bank_erase_init (flash_info_t *info, int sect)
{
	volatile bank_addr_t addr = (bank_addr_t)info->start[sect];
	int flag;

#ifdef FLASH_DEBUG
	printf ("erasing sector %d, addr = 0x%08lx\n",
		sect, (unsigned long)addr);
#endif

	/* Disable intrs which might cause a timeout here */
	flag = disable_interrupts ();

#ifdef FLASH_DEBUG
	printf ("writing erase cmd to addr 0x%08lx\n", (unsigned long)addr);
#endif
	*addr = BANK_CMD_ERASE1;
	*addr = BANK_CMD_ERASE2;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();
}

static int
bank_erase_poll (flash_info_t *info, int sect)
{
	volatile bank_addr_t addr = (bank_addr_t)info->start[sect];
	bank_word_t stat = *addr;

#ifdef FLASH_DEBUG
	printf ("checking status at addr 0x%08lx [0x%08lx]\n",
		(unsigned long)addr, (unsigned long)stat);
#endif

	if ((stat & BANK_STAT_RDY) == BANK_STAT_RDY) {
		if ((stat & BANK_STAT_ERR) != 0) {
			printf ("failed on sector %d [0x%08lx] at "
				"address 0x%08lx\n", sect,
				(unsigned long)stat, (unsigned long)addr);
			*addr = BANK_CMD_CLR_STAT;
			return (-1);
		}
		else
			return (1);
	}
	else
		return (0);
}

static int
bank_write_word (volatile bank_addr_t addr, bank_word_t value)
{
	bank_word_t stat;
	ulong start;
	int flag, retval;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	*addr = BANK_CMD_PROG;

	*addr = value;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	retval = 0;

	/* data polling for D7 */
	start = get_timer (0);
	do {
		if (get_timer (start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			retval = 1;
			goto done;
		}
		stat = *addr;
	} while ((stat & BANK_STAT_RDY) != BANK_STAT_RDY);

	if ((stat & BANK_STAT_ERR) != 0) {
		printf ("flash program failed [0x%08lx] at address 0x%08lx\n",
			(unsigned long)stat, (unsigned long)addr);
		*addr = BANK_CMD_CLR_STAT;
		retval = 3;
	}

done:
	/* reset to read mode */
	*addr = BANK_CMD_RST;

	return (retval);
}

/*-----------------------------------------------------------------------
 */

unsigned long
flash_init (void)
{
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	bank_probe (&flash_info[0], (bank_addr_t)CONFIG_SYS_FLASH_BASE);

	/*
	 * protect monitor and environment sectors
	 */

#if CONFIG_SYS_MONITOR_BASE == CONFIG_SYS_FLASH_BASE
	(void)flash_protect (FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
#endif

#if defined(CONFIG_SYS_FLASH_ENV_ADDR)
	(void)flash_protect (FLAG_PROTECT_SET,
		      CONFIG_SYS_FLASH_ENV_ADDR,
#if defined(CONFIG_SYS_FLASH_ENV_BUF)
		      CONFIG_SYS_FLASH_ENV_ADDR + CONFIG_SYS_FLASH_ENV_BUF - 1,
#else
		      CONFIG_SYS_FLASH_ENV_ADDR + CONFIG_SYS_FLASH_ENV_SIZE - 1,
#endif
		      &flash_info[0]);
#endif

	return flash_info[0].size;
}

/*-----------------------------------------------------------------------
 */
void
flash_print_info (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:	printf ("INTEL ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F320J5:	printf ("28F320J5 (32 Mbit, 2 x 16bit)\n");
				break;
	case FLASH_28F640J5:	printf ("28F640J5 (64 Mbit, 2 x 16bit)\n");
				break;
	case FLASH_28F320J3A:	printf ("28F320J3A (32 Mbit, 2 x 16bit)\n");
				break;
	case FLASH_28F640J3A:	printf ("28F640J3A (64 Mbit, 2 x 16bit)\n");
				break;
	case FLASH_28F128J3A:	printf ("28F320J3A (128 Mbit, 2 x 16bit)\n");
				break;
	default:		printf ("Unknown Chip Type\n");
				break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

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

/*
 * The following code cannot be run from FLASH!
 */

/*-----------------------------------------------------------------------
 */

int
flash_erase (flash_info_t *info, int s_first, int s_last)
{
	int prot, sect, haderr;
	ulong start, now, last;
	int rcode = 0;

#ifdef FLASH_DEBUG
	printf ("\nflash_erase: erase %d sectors (%d to %d incl.) from\n"
		"  Bank # %d: ", s_last - s_first + 1, s_first, s_last,
		(info - flash_info) + 1);
	flash_print_info (info);
#endif

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sector%s will not be erased\n",
			prot, (prot > 1 ? "s" : ""));
	}

	start = get_timer (0);
	last = 0;
	haderr = 0;

	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			ulong estart;
			int sectdone;

			bank_erase_init (info, sect);

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);

			estart = get_timer (start);

			do {
				now = get_timer (start);

				if (now - estart > CONFIG_SYS_FLASH_ERASE_TOUT) {
					printf ("Timeout (sect %d)\n", sect);
					haderr = 1;
					rcode = 1;
					break;
				}

#ifndef FLASH_DEBUG
				/* show that we're waiting */
				if ((now - last) > 1000) { /* every second */
					putc ('.');
					last = now;
				}
#endif

				sectdone = bank_erase_poll (info, sect);

				if (sectdone < 0) {
					haderr = 1;
					rcode = 1;
					break;
				}

			} while (!sectdone);

			if (haderr)
				break;
		}
	}

	if (haderr > 0)
		printf (" failed\n");
	else
		printf (" done\n");

	/* reset to read mode */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			bank_reset (info, sect);
		}
	}
	return rcode;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 3 - Program failed
 */
static int
write_word (flash_info_t *info, ulong dest, ulong data)
{
	/* Check if Flash is (sufficiently) erased */
	if ((*(ulong *)dest & data) != data)
		return (2);

	return (bank_write_word ((bank_addr_t)dest, (bank_word_t)data));
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 3 - Program failed
 */

int
write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
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

		if ((rc = write_word (info, wp, data)) != 0) {
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
		if ((rc = write_word (info, wp, data)) != 0) {
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

	return (write_word (info, wp, data));
}

/*-----------------------------------------------------------------------
 */
