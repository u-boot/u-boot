/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <board/cogent/flash.h>
#include <linux/compiler.h>

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

#if defined(CONFIG_ENV_IS_IN_FLASH)
# ifndef  CONFIG_ENV_ADDR
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# endif
# ifndef  CONFIG_ENV_SIZE
#  define CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
# endif
# ifndef  CONFIG_ENV_SECT_SIZE
#  define CONFIG_ENV_SECT_SIZE  CONFIG_ENV_SIZE
# endif
#endif

/*-----------------------------------------------------------------------
 * Functions
 */
static int write_word (flash_info_t *info, ulong dest, ulong data);

/*-----------------------------------------------------------------------
 */


#if defined(CONFIG_CMA302)

/*
 * probe for the existence of flash at address "addr"
 * 0 = yes, 1 = bad Manufacturer's Id, 2 = bad Device Id
 */
static int
c302f_probe_word(c302f_addr_t addr)
{
	/* reset the flash */
	*addr = C302F_BNK_CMD_RST;

	/* check the manufacturer id */
	*addr = C302F_BNK_CMD_RD_ID;
	if (*C302F_BNK_ADDR_MAN(addr) != C302F_BNK_RD_ID_MAN)
		return 1;

	/* check the device id */
	*addr = C302F_BNK_CMD_RD_ID;
	if (*C302F_BNK_ADDR_DEV(addr) != C302F_BNK_RD_ID_DEV)
		return 2;

#ifdef FLASH_DEBUG
	{
		int i;

		printf("\nMaster Lock Config = 0x%08lx\n",
			*C302F_BNK_ADDR_CFGM(addr));
		for (i = 0; i < C302F_BNK_NBLOCKS; i++)
			printf("Block %2d Lock Config = 0x%08lx\n",
				i, *C302F_BNK_ADDR_CFG(i, addr));
	}
#endif

	/* reset the flash again */
	*addr = C302F_BNK_CMD_RST;

	return 0;
}

/*
 * probe for Cogent CMA302 flash module at address "base" and store
 * info for any found into flash_info entry "fip". Must find at least
 * one bank.
 */
static void
c302f_probe(flash_info_t *fip, c302f_addr_t base)
{
	c302f_addr_t addr, eaddr;
	int nbanks;

	fip->size = 0L;
	fip->sector_count = 0;

	addr = base;
	eaddr = C302F_BNK_ADDR_BASE(addr, C302F_MAX_BANKS);
	nbanks = 0;

	while (addr < eaddr) {
		c302f_addr_t addrw, eaddrw, addrb;
		int i, osc, nsc;

		addrw = addr;
		eaddrw = C302F_BNK_ADDR_NEXT_WORD(addrw);

		while (addrw < eaddrw)
			if (c302f_probe_word(addrw++) != 0)
				goto out;

		/* bank exists - append info for this bank to *fip */
		fip->flash_id = FLASH_MAN_INTEL|FLASH_28F008S5;
		fip->size += C302F_BNK_SIZE;
		osc = fip->sector_count;
		fip->sector_count += C302F_BNK_NBLOCKS;
		if ((nsc = fip->sector_count) >= CONFIG_SYS_MAX_FLASH_SECT)
			panic("Too many sectors in flash at address 0x%08lx\n",
				(unsigned long)base);

		addrb = addr;
		for (i = osc; i < nsc; i++) {
			fip->start[i] = (ulong)addrb;
			fip->protect[i] = 0;
			addrb = C302F_BNK_ADDR_NEXT_BLK(addrb);
		}

		addr = C302F_BNK_ADDR_NEXT_BNK(addr);
		nbanks++;
	}

out:
	if (nbanks == 0)
		panic("ERROR: no flash found at address 0x%08lx\n",
			(unsigned long)base);
}

static void
c302f_reset(flash_info_t *info, int sect)
{
	c302f_addr_t addrw, eaddrw;

	addrw = (c302f_addr_t)info->start[sect];
	eaddrw = C302F_BNK_ADDR_NEXT_WORD(addrw);

	while (addrw < eaddrw) {
#ifdef FLASH_DEBUG
		printf("  writing reset cmd to addr 0x%08lx\n",
			(unsigned long)addrw);
#endif
		*addrw = C302F_BNK_CMD_RST;
		addrw++;
	}
}

static void
c302f_erase_init(flash_info_t *info, int sect)
{
	c302f_addr_t addrw, saddrw, eaddrw;
	int flag;

#ifdef FLASH_DEBUG
	printf("0x%08lx C302F_BNK_CMD_PROG\n", C302F_BNK_CMD_PROG);
	printf("0x%08lx C302F_BNK_CMD_ERASE1\n", C302F_BNK_CMD_ERASE1);
	printf("0x%08lx C302F_BNK_CMD_ERASE2\n", C302F_BNK_CMD_ERASE2);
	printf("0x%08lx C302F_BNK_CMD_CLR_STAT\n", C302F_BNK_CMD_CLR_STAT);
	printf("0x%08lx C302F_BNK_CMD_RST\n", C302F_BNK_CMD_RST);
	printf("0x%08lx C302F_BNK_STAT_RDY\n", C302F_BNK_STAT_RDY);
	printf("0x%08lx C302F_BNK_STAT_ERR\n", C302F_BNK_STAT_ERR);
#endif

	saddrw = (c302f_addr_t)info->start[sect];
	eaddrw = C302F_BNK_ADDR_NEXT_WORD(saddrw);

#ifdef FLASH_DEBUG
	printf("erasing sector %d, start addr = 0x%08lx "
		"(bank next word addr = 0x%08lx)\n", sect,
		(unsigned long)saddrw, (unsigned long)eaddrw);
#endif

	/* Disable intrs which might cause a timeout here */
	flag = disable_interrupts();

	for (addrw = saddrw; addrw < eaddrw; addrw++) {
#ifdef FLASH_DEBUG
		printf("  writing erase cmd to addr 0x%08lx\n",
			(unsigned long)addrw);
#endif
		*addrw = C302F_BNK_CMD_ERASE1;
		*addrw = C302F_BNK_CMD_ERASE2;
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();
}

static int
c302f_erase_poll(flash_info_t *info, int sect)
{
	c302f_addr_t addrw, saddrw, eaddrw;
	int sectdone, haderr;

	saddrw = (c302f_addr_t)info->start[sect];
	eaddrw = C302F_BNK_ADDR_NEXT_WORD(saddrw);

	sectdone = 1;
	haderr = 0;

	for (addrw = saddrw; addrw < eaddrw; addrw++) {
		c302f_word_t stat = *addrw;

#ifdef FLASH_DEBUG
		printf("  checking status at addr "
			"0x%08lx [0x%08lx]\n",
			(unsigned long)addrw, stat);
#endif
		if ((stat & C302F_BNK_STAT_RDY) != C302F_BNK_STAT_RDY)
			sectdone = 0;
		else if ((stat & C302F_BNK_STAT_ERR) != 0) {
			printf(" failed on sector %d "
				"(stat = 0x%08lx) at "
				"address 0x%08lx\n",
				sect, stat,
				(unsigned long)addrw);
			*addrw = C302F_BNK_CMD_CLR_STAT;
			haderr = 1;
		}
	}

	if (haderr)
		return (-1);
	else
		return (sectdone);
}

static int
c302f_write_word(c302f_addr_t addr, c302f_word_t value)
{
	c302f_word_t stat;
	ulong start;
	int flag, retval;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	*addr = C302F_BNK_CMD_PROG;

	*addr = value;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	retval = 0;

	/* data polling for D7 */
	start = get_timer (0);
	do {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			retval = 1;
			goto done;
		}
		stat = *addr;
	} while ((stat & C302F_BNK_STAT_RDY) != C302F_BNK_STAT_RDY);

	if ((stat & C302F_BNK_STAT_ERR) != 0) {
		printf("flash program failed (stat = 0x%08lx) "
			"at address 0x%08lx\n", (ulong)stat, (ulong)addr);
		*addr = C302F_BNK_CMD_CLR_STAT;
		retval = 3;
	}

done:
	/* reset to read mode */
	*addr = C302F_BNK_CMD_RST;

	return (retval);
}

#endif	/* CONFIG_CMA302 */

unsigned long
flash_init(void)
{
	unsigned long total;
	int i;
	__maybe_unused flash_info_t *fip;

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	fip = &flash_info[0];
	total = 0L;

#if defined(CONFIG_CMA302)
	c302f_probe(fip, (c302f_addr_t)CONFIG_SYS_FLASH_BASE);
	total += fip->size;
	fip++;
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_FLASH)
	/* not yet ...
	cmbf_probe(fip, (cmbf_addr_t)CMA_MB_FLASH_BASE);
	total += fip->size;
	fip++;
	*/
#endif

	/*
	 * protect monitor and environment sectors
	 */

#if CONFIG_SYS_MONITOR_BASE == CONFIG_SYS_FLASH_BASE
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR+CONFIG_ENV_SECT_SIZE-1,
		      &flash_info[0]);
#endif
	return total;
}

/*-----------------------------------------------------------------------
 */
void
flash_print_info(flash_info_t *info)
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
	case FLASH_28F008S5:	printf ("28F008S5\n");
				break;
	default:		printf ("Unknown Chip Type\n");
				break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 4) == 0)
			printf ("\n   ");
		printf (" %2d - %08lX%s", i,
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

int
flash_erase(flash_info_t *info, int s_first, int s_last)
{
	int prot, sect, haderr;
	ulong start, now, last;
	void (*erase_init)(flash_info_t *, int);
	int (*erase_poll)(flash_info_t *, int);
	void (*reset)(flash_info_t *, int);
	int rcode = 0;

#ifdef FLASH_DEBUG
	printf("\nflash_erase: erase %d sectors (%d to %d incl.) from\n"
		"  Bank # %d: ", s_last - s_first + 1, s_first, s_last,
		(info - flash_info) + 1);
	flash_print_info(info);
#endif

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	switch (info->flash_id) {

#if defined(CONFIG_CMA302)
	case FLASH_MAN_INTEL|FLASH_28F008S5:
		erase_init = c302f_erase_init;
		erase_poll = c302f_erase_poll;
		reset = c302f_reset;
		break;
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_FLASH)
	case FLASH_MAN_INTEL|FLASH_28F800_B:
	case FLASH_MAN_AMD|FLASH_AM29F800B:
		/* not yet ...
		erase_init = cmbf_erase_init;
		erase_poll = cmbf_erase_poll;
		reset = cmbf_reset;
		break;
		*/
#endif

	default:
		printf ("Flash type %08lx not supported - aborted\n",
			info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf("- Warning: %d protected sector%s will not be erased!\n",
			prot, (prot > 1 ? "s" : ""));
	}

	start = get_timer (0);
	last = 0;
	haderr = 0;

	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			ulong estart;
			int sectdone;

			(*erase_init)(info, sect);

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);

			estart = get_timer(start);

			do {
				now = get_timer(start);

				if (now - estart > CONFIG_SYS_FLASH_ERASE_TOUT) {
					printf ("Timeout (sect %d)\n", sect);
					haderr = 1;
					break;
				}

#ifndef FLASH_DEBUG
				/* show that we're waiting */
				if ((now - last) > 1000) { /* every second */
					putc ('.');
					last = now;
				}
#endif

				sectdone = (*erase_poll)(info, sect);

				if (sectdone < 0) {
					haderr = 1;
					break;
				}

			} while (!sectdone);

			if (haderr)
				break;
		}
	}

	if (haderr > 0) {
		printf (" failed\n");
		rcode = 1;
	}
	else
		printf (" done\n");

	/* reset to read mode */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			(*reset)(info, sect);
		}
	}
	return rcode;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 3 - write error
 */

int
write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;
	ulong start, now, last;

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
	start = get_timer (0);
	last = 0;
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

		/* show that we're waiting */
		now = get_timer(start);
		if ((now - last) > 1000) {	/* every second */
			putc ('.');
			last = now;
		}
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
 * 3 - write error
 */
static int
write_word(flash_info_t *info, ulong dest, ulong data)
{
	int retval;

	/* Check if Flash is (sufficiently) erased */
	if ((*(ulong *)dest & data) != data) {
		return (2);
	}

	switch (info->flash_id) {

#if defined(CONFIG_CMA302)
	case FLASH_MAN_INTEL|FLASH_28F008S5:
		retval = c302f_write_word((c302f_addr_t)dest, (c302f_word_t)data);
		break;
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_FLASH)
	case FLASH_MAN_INTEL|FLASH_28F800_B:
	case FLASH_MAN_AMD|FLASH_AM29F800B:
		/* not yet ...
		retval = cmbf_write_word((cmbf_addr_t)dest, (cmbf_word_t)data);
		*/
		retval = 3;
		break;
#endif

	default:
		printf ("Flash type %08lx not supported - aborted\n",
			info->flash_id);
		retval = 3;
		break;
	}

	return (retval);
}

/*-----------------------------------------------------------------------
 */
