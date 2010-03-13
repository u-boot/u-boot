/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
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
#include <flash.h>
#include <linux/err.h>

#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_smi.h>

#if !defined(CONFIG_SYS_NO_FLASH)

static struct smi_regs *const smicntl =
    (struct smi_regs * const)CONFIG_SYS_SMI_BASE;
static ulong bank_base[CONFIG_SYS_MAX_FLASH_BANKS] =
    CONFIG_SYS_FLASH_ADDR_BASE;
flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

#define ST_M25Pxx_ID		0x00002020

static struct flash_dev flash_ids[] = {
	{0x10, 0x10000, 2},	/* 64K Byte */
	{0x11, 0x20000, 4},	/* 128K Byte */
	{0x12, 0x40000, 4},	/* 256K Byte */
	{0x13, 0x80000, 8},	/* 512K Byte */
	{0x14, 0x100000, 16},	/* 1M Byte */
	{0x15, 0x200000, 32},	/* 2M Byte */
	{0x16, 0x400000, 64},	/* 4M Byte */
	{0x17, 0x800000, 128},	/* 8M Byte */
	{0x18, 0x1000000, 64},	/* 16M Byte */
	{0x00,}
};

/*
 * smi_wait_xfer_finish - Wait until TFF is set in status register
 * @timeout:	 timeout in milliseconds
 *
 * Wait until TFF is set in status register
 */
static void smi_wait_xfer_finish(int timeout)
{
	while (timeout--) {
		if (readl(&smicntl->smi_sr) & TFF)
			break;
		udelay(1000);
	}
}

/*
 * smi_read_id - Read flash id
 * @info:	 flash_info structure pointer
 * @banknum:	 bank number
 *
 * Read the flash id present at bank #banknum
 */
static unsigned int smi_read_id(flash_info_t *info, int banknum)
{
	unsigned int value;

	writel(readl(&smicntl->smi_cr1) | SW_MODE, &smicntl->smi_cr1);
	writel(READ_ID, &smicntl->smi_tr);
	writel((banknum << BANKSEL_SHIFT) | SEND | TX_LEN_1 | RX_LEN_3,
	       &smicntl->smi_cr2);
	smi_wait_xfer_finish(XFER_FINISH_TOUT);

	value = (readl(&smicntl->smi_rr) & 0x00FFFFFF);

	writel(readl(&smicntl->smi_sr) & ~TFF, &smicntl->smi_sr);
	writel(readl(&smicntl->smi_cr1) & ~SW_MODE, &smicntl->smi_cr1);

	return value;
}

/*
 * flash_get_size - Detect the SMI flash by reading the ID.
 * @base:	 Base address of the flash area bank #banknum
 * @banknum:	 Bank number
 *
 * Detect the SMI flash by reading the ID. Initializes the flash_info structure
 * with size, sector count etc.
 */
static ulong flash_get_size(ulong base, int banknum)
{
	flash_info_t *info = &flash_info[banknum];
	struct flash_dev *dev;
	unsigned int value;
	unsigned int density;
	int i;

	value = smi_read_id(info, banknum);
	density = (value >> 16) & 0xff;

	for (i = 0, dev = &flash_ids[0]; dev->density != 0x0;
	     i++, dev = &flash_ids[i]) {
		if (dev->density == density) {
			info->size = dev->size;
			info->sector_count = dev->sector_count;
			break;
		}
	}

	if (dev->density == 0x0)
		return 0;

	info->flash_id = value & 0xffff;
	info->start[0] = base;

	return info->size;
}

/*
 * smi_read_sr - Read status register of SMI
 * @bank:	 bank number
 *
 * This routine will get the status register of the flash chip present at the
 * given bank
 */
static unsigned int smi_read_sr(int bank)
{
	u32 ctrlreg1;

	/* store the CTRL REG1 state */
	ctrlreg1 = readl(&smicntl->smi_cr1);

	/* Program SMI in HW Mode */
	writel(readl(&smicntl->smi_cr1) & ~(SW_MODE | WB_MODE),
	       &smicntl->smi_cr1);

	/* Performing a RSR instruction in HW mode */
	writel((bank << BANKSEL_SHIFT) | RD_STATUS_REG, &smicntl->smi_cr2);

	smi_wait_xfer_finish(XFER_FINISH_TOUT);

	/* Restore the CTRL REG1 state */
	writel(ctrlreg1, &smicntl->smi_cr1);

	return readl(&smicntl->smi_sr);
}

/*
 * smi_wait_till_ready - Wait till last operation is over.
 * @bank:	 bank number shifted.
 * @timeout:	 timeout in milliseconds.
 *
 * This routine checks for WIP(write in progress)bit in Status register(SMSR-b0)
 * The routine checks for #timeout loops, each at interval of 1 milli-second.
 * If successful the routine returns 0.
 */
static int smi_wait_till_ready(int bank, int timeout)
{
	int count;
	unsigned int sr;

	/* One chip guarantees max 5 msec wait here after page writes,
	   but potentially three seconds (!) after page erase. */
	for (count = 0; count < timeout; count++) {

		sr = smi_read_sr(bank);
		if (sr < 0)
			break;
		else if (!(sr & WIP_BIT))
			return 0;

		/* Try again after 1m-sec */
		udelay(1000);
	}
	printf("SMI controller is still in wait, timeout=%d\n", timeout);
	return -EIO;
}

/*
 * smi_write_enable - Enable the flash to do write operation
 * @bank:	 bank number
 *
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static int smi_write_enable(int bank)
{
	u32 ctrlreg1;
	int timeout = WMODE_TOUT;

	/* Store the CTRL REG1 state */
	ctrlreg1 = readl(&smicntl->smi_cr1);

	/* Program SMI in H/W Mode */
	writel(readl(&smicntl->smi_cr1) & ~SW_MODE, &smicntl->smi_cr1);

	/* Give the Flash, Write Enable command */
	writel((bank << BANKSEL_SHIFT) | WE, &smicntl->smi_cr2);

	smi_wait_xfer_finish(XFER_FINISH_TOUT);

	/* Restore the CTRL REG1 state */
	writel(ctrlreg1, &smicntl->smi_cr1);

	while (timeout--) {
		if (smi_read_sr(bank) & (1 << (bank + WM_SHIFT)))
			break;
		udelay(1000);
	}

	if (timeout)
		return 0;

	return -1;
}

/*
 * smi_init - SMI initialization routine
 *
 * SMI initialization routine. Sets SMI control register1.
 */
static void smi_init(void)
{
	/* Setting the fast mode values. SMI working at 166/4 = 41.5 MHz */
	writel(HOLD1 | FAST_MODE | BANK_EN | DSEL_TIME | PRESCAL4,
	       &smicntl->smi_cr1);
}

/*
 * smi_sector_erase - Erase flash sector
 * @info:	 flash_info structure pointer
 * @sector:	 sector number
 *
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static int smi_sector_erase(flash_info_t *info, unsigned int sector)
{
	int bank;
	unsigned int sect_add;
	unsigned int instruction;

	switch (info->start[0]) {
	case SMIBANK0_BASE:
		bank = BANK0;
		break;
	case SMIBANK1_BASE:
		bank = BANK1;
		break;
	case SMIBANK2_BASE:
		bank = BANK2;
		break;
	case SMIBANK3_BASE:
		bank = BANK3;
		break;
	default:
		return -1;
	}

	sect_add = sector * (info->size / info->sector_count);
	instruction = ((sect_add >> 8) & 0x0000FF00) | SECTOR_ERASE;

	writel(readl(&smicntl->smi_sr) & ~(ERF1 | ERF2), &smicntl->smi_sr);

	if (info->flash_id == ST_M25Pxx_ID) {
		/* Wait until finished previous write command. */
		if (smi_wait_till_ready(bank, CONFIG_SYS_FLASH_ERASE_TOUT))
			return -EBUSY;

		/* Send write enable, before erase commands. */
		if (smi_write_enable(bank))
			return -EIO;

		/* Put SMI in SW mode */
		writel(readl(&smicntl->smi_cr1) | SW_MODE, &smicntl->smi_cr1);

		/* Send Sector Erase command in SW Mode */
		writel(instruction, &smicntl->smi_tr);
		writel((bank << BANKSEL_SHIFT) | SEND | TX_LEN_4,
		       &smicntl->smi_cr2);
		smi_wait_xfer_finish(XFER_FINISH_TOUT);

		if (smi_wait_till_ready(bank, CONFIG_SYS_FLASH_ERASE_TOUT))
			return -EBUSY;

		/* Put SMI in HW mode */
		writel(readl(&smicntl->smi_cr1) & ~SW_MODE,
		       &smicntl->smi_cr1);

		return 0;
	} else {
		/* Put SMI in HW mode */
		writel(readl(&smicntl->smi_cr1) & ~SW_MODE,
		       &smicntl->smi_cr1);
		return -EINVAL;
	}
}

/*
 * smi_write - Write to SMI flash
 * @src_addr:	 source buffer
 * @dst_addr:	 destination buffer
 * @length:	 length to write in words
 * @bank:	 bank base address
 *
 * Write to SMI flash
 */
static int smi_write(unsigned int *src_addr, unsigned int *dst_addr,
		     unsigned int length, ulong bank_addr)
{
	int banknum;
	unsigned int WM;

	switch (bank_addr) {
	case SMIBANK0_BASE:
		banknum = BANK0;
		WM = WM0;
		break;
	case SMIBANK1_BASE:
		banknum = BANK1;
		WM = WM1;
		break;
	case SMIBANK2_BASE:
		banknum = BANK2;
		WM = WM2;
		break;
	case SMIBANK3_BASE:
		banknum = BANK3;
		WM = WM3;
		break;
	default:
		return -1;
	}

	if (smi_wait_till_ready(banknum, CONFIG_SYS_FLASH_WRITE_TOUT))
		return -EBUSY;

	/* Set SMI in Hardware Mode */
	writel(readl(&smicntl->smi_cr1) & ~SW_MODE, &smicntl->smi_cr1);

	if (smi_write_enable(banknum))
		return -EIO;

	/* Perform the write command */
	while (length--) {
		if (((ulong) (dst_addr) % SFLASH_PAGE_SIZE) == 0) {
			if (smi_wait_till_ready(banknum,
						CONFIG_SYS_FLASH_WRITE_TOUT))
				return -EBUSY;

			if (smi_write_enable(banknum))
				return -EIO;
		}

		*dst_addr++ = *src_addr++;

		if ((readl(&smicntl->smi_sr) & (ERF1 | ERF2)))
			return -EIO;
	}

	if (smi_wait_till_ready(banknum, CONFIG_SYS_FLASH_WRITE_TOUT))
		return -EBUSY;

	writel(readl(&smicntl->smi_sr) & ~(WCF), &smicntl->smi_sr);

	return 0;
}

/*
 * write_buff - Write to SMI flash
 * @info:	 flash info structure
 * @src:	 source buffer
 * @dest_addr:	 destination buffer
 * @length:	 length to write in words
 *
 * Write to SMI flash
 */
int write_buff(flash_info_t *info, uchar *src, ulong dest_addr, ulong length)
{
	return smi_write((unsigned int *)src, (unsigned int *)dest_addr,
		  (length + 3) / 4, info->start[0]);
}

/*
 * flash_init - SMI flash initialization
 *
 * SMI flash initialization
 */
unsigned long flash_init(void)
{
	unsigned long size = 0;
	int i, j;

	smi_init();

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		size += flash_info[i].size = flash_get_size(bank_base[i], i);
	}

	for (j = 0; j < CONFIG_SYS_MAX_FLASH_BANKS; j++) {
		for (i = 1; i < flash_info[j].sector_count; i++)
			flash_info[j].start[i] =
			    flash_info[j].start[i - 1] +
			    flash_info->size / flash_info->sector_count;

	}

	return size;
}

/*
 * flash_print_info - Print SMI flash information
 *
 * Print SMI flash information
 */
void flash_print_info(flash_info_t *info)
{
	int i;
	if (info->flash_id == FLASH_UNKNOWN) {
		puts("missing or unknown FLASH type\n");
		return;
	}
	printf("  Size: %ld MB in %d Sectors\n",
	       info->size >> 20, info->sector_count);

	puts("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
#ifdef CONFIG_SYS_FLASH_EMPTY_INFO
		int size;
		int erased;
		u32 *flash;

		/*
		 * Check if whole sector is erased
		 */
		size = (info->size) / (info->sector_count);
		flash = (u32 *) info->start[i];
		size = size / sizeof(int);

		while ((size--) && (*flash++ == ~0))
			;

		size++;
		if (size)
			erased = 0;
		else
			erased = 1;

		if ((i % 5) == 0)
			printf("\n");

		printf(" %08lX%s%s",
		       info->start[i],
		       erased ? " E" : "  ", info->protect[i] ? "RO " : "   ");
#else
		if ((i % 5) == 0)
			printf("\n   ");
		printf(" %08lX%s",
		       info->start[i], info->protect[i] ? " (RO)  " : "     ");
#endif
	}
	putc('\n');
	return;
}

/*
 * flash_erase - Erase SMI flash
 *
 * Erase SMI flash
 */
int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	int rcode = 0;
	int prot = 0;
	flash_sect_t sect;

	if (info->flash_id != ST_M25Pxx_ID) {
		puts("Can't erase unknown flash type - aborted\n");
		return 1;
	}

	if ((s_first < 0) || (s_first > s_last)) {
		puts("- no sectors to erase\n");
		return 1;
	}

	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect])
			prot++;
	}
	if (prot) {
		printf("- Warning: %d protected sectors will not be erased!\n",
		       prot);
	} else {
		putc('\n');
	}

	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {
			if (smi_sector_erase(info, sect))
				rcode = 1;
			else
				putc('.');
		}
	}
	puts(" done\n");
	return rcode;
}
#endif
