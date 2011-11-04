/*
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

#include <common.h>
#include <mpc8xx.h>

#ifndef	CONFIG_ENV_ADDR
#define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#endif

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

/*-----------------------------------------------------------------------
 * Functions
 */
static int write_word(flash_info_t *info, ulong dest, ulong data);

#ifdef CONFIG_BOOT_8B
static int my_in_8(unsigned char *addr);
static void my_out_8(unsigned char *addr, int val);
#endif
#ifdef CONFIG_BOOT_16B
static int my_in_be16(unsigned short *addr);
static void my_out_be16(unsigned short *addr, int val);
#endif
#ifdef CONFIG_BOOT_32B
static unsigned my_in_be32(unsigned *addr);
static void my_out_be32(unsigned *addr, int val);
#endif
/*-----------------------------------------------------------------------
 */

unsigned long flash_init(void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long size_b0, size_b1;
	int i;

	size_b0 = 0;
	size_b1 = 0;
	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i)
		flash_info[i].flash_id = FLASH_UNKNOWN;

#ifdef CONFIG_SYS_DOC_BASE
#ifndef CONFIG_FEL8xx_AT
	/* 32k bytes */
	memctl->memc_or5 = (0xffff8000 | CONFIG_SYS_OR_TIMING_DOC);
	memctl->memc_br5 = CONFIG_SYS_DOC_BASE | 0x401;
#else
	/* 32k bytes */
	memctl->memc_or3 = (0xffff8000 | CONFIG_SYS_OR_TIMING_DOC);
	memctl->memc_br3 = CONFIG_SYS_DOC_BASE | 0x401;
#endif
#endif
#if defined(CONFIG_BOOT_8B)
	size_b0 = 0x80000;	/* 512 K */

	flash_info[0].flash_id = FLASH_MAN_AMD | FLASH_AM040;
	flash_info[0].sector_count = 8;
	flash_info[0].size = 0x00080000;

	/* set up sector start address table */
	for (i = 0; i < flash_info[0].sector_count; i++)
		flash_info[0].start[i] = 0x40000000 + (i * 0x10000);

	/* protect all sectors */
	for (i = 0; i < flash_info[0].sector_count; i++)
		flash_info[0].protect[i] = 0x1;

#elif defined(CONFIG_BOOT_16B)
	size_b0 = 0x400000;	/* 4MB , assume AMD29LV320B */

	flash_info[0].flash_id = FLASH_MAN_AMD | FLASH_AM320B;
	flash_info[0].sector_count = 67;
	flash_info[0].size = 0x00400000;

	/* set up sector start address table */
	flash_info[0].start[0] = 0x40000000;
	flash_info[0].start[1] = 0x40000000 + 0x4000;
	flash_info[0].start[2] = 0x40000000 + 0x6000;
	flash_info[0].start[3] = 0x40000000 + 0x8000;

	for (i = 4; i < flash_info[0].sector_count; i++) {
		flash_info[0].start[i] =
			0x40000000 + 0x10000 + ((i - 4) * 0x10000);
	}

	/* protect all sectors */
	for (i = 0; i < flash_info[0].sector_count; i++)
		flash_info[0].protect[i] = 0x1;
#endif

#ifdef CONFIG_BOOT_32B

	/* Static FLASH Bank configuration here - FIXME XXX */
	size_b0 = flash_get_size((vu_long *) FLASH_BASE0_PRELIM,
			       &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size_b0, size_b0 << 20);
	}

	size_b1 = flash_get_size((vu_long *) FLASH_BASE1_PRELIM,
			       &flash_info[1]);

	if (size_b1 > size_b0) {
		printf("## ERROR: "
			"Bank 1 (0x%08lx = %ld MB) > Bank 0 (0x%08lx = %ld MB)\n",
			size_b1, size_b1 << 20, size_b0, size_b0 << 20);
		flash_info[0].flash_id = FLASH_UNKNOWN;
		flash_info[1].flash_id = FLASH_UNKNOWN;
		flash_info[0].sector_count = -1;
		flash_info[1].sector_count = -1;
		flash_info[0].size = 0;
		flash_info[1].size = 0;

		return 0;
	}

	/* Remap FLASH according to real size */
	memctl->memc_or0 = CONFIG_SYS_OR_TIMING_FLASH |
				(-size_b0 & OR_AM_MSK);
	memctl->memc_br0 = (CONFIG_SYS_FLASH_BASE & BR_BA_MSK) |
				BR_MS_GPCM | BR_V;

	/* Re-do sizing to get full correct info */
	size_b0 = flash_get_size((vu_long *) CONFIG_SYS_FLASH_BASE,
				&flash_info[0]);

	flash_get_offsets(CONFIG_SYS_FLASH_BASE, &flash_info[0]);

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		CONFIG_SYS_MONITOR_BASE,
		CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
		&flash_info[0]);
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		CONFIG_ENV_ADDR,
		CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1, &flash_info[0]);
#endif

	if (size_b1) {
		memctl->memc_or1 = CONFIG_SYS_OR_TIMING_FLASH |
			(-size_b1 & 0xFFFF8000);
		memctl->memc_br1 = ((CONFIG_SYS_FLASH_BASE +
			size_b0) & BR_BA_MSK) | BR_MS_GPCM | BR_V;

		/* Re-do sizing to get full correct info */
		size_b1 = flash_get_size((vu_long *)(CONFIG_SYS_FLASH_BASE +
					size_b0), &flash_info[1]);

		flash_get_offsets(CONFIG_SYS_FLASH_BASE + size_b0,
				  &flash_info[1]);

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
		/* monitor protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_SYS_MONITOR_BASE,
			      CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
			      &flash_info[1]);
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
		/* ENV protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_ENV_ADDR,
			      CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1,
			      &flash_info[1]);
#endif
	} else {
		memctl->memc_br1 = 0;	/* invalidate bank */

		flash_info[1].flash_id = FLASH_UNKNOWN;
		flash_info[1].sector_count = -1;
	}

	flash_info[0].size = size_b0;
	flash_info[1].size = size_b1;


#endif /* CONFIG_BOOT_32B */

	return size_b0 + size_b1;
}


void flash_print_info(flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:
		printf("AMD ");
		break;
	case FLASH_MAN_FUJ:
		printf("FUJITSU ");
		break;
	default:
		printf("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM400B:
		printf("AM29LV400B (4 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM400T:
		printf("AM29LV400T (4 Mbit, top boot sector)\n");
		break;
	case FLASH_AM800B:
		printf("AM29LV800B (8 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM800T:
		printf("AM29LV800T (8 Mbit, top boot sector)\n");
		break;
	case FLASH_AM160B:
		printf("AM29LV160B (16 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM160T:
		printf("AM29LV160T (16 Mbit, top boot sector)\n");
		break;
	case FLASH_AM320B:
		printf("AM29LV320B (32 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM320T:
		printf("AM29LV320T (32 Mbit, top boot sector)\n");
		break;
	default:
		printf("Unknown Chip Type\n");
		break;
	}

	printf("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	printf("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf("\n   ");
		printf(" %08lX%s",
		       info->start[i], info->protect[i] ? " (RO)" : "     ");
	}
	printf("\n");
	return;
}

/*
 * The following code cannot be run from FLASH!
 */

int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	vu_long *addr = (vu_long *) (info->start[0]);
	int flag, prot, sect, l_sect, in_mid, in_did;
	ulong start, now, last;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN)
			printf("- missing\n");
		else
			printf("- no sectors to erase\n");

		return 1;
	}

	if ((info->flash_id == FLASH_UNKNOWN) ||
	    (info->flash_id > FLASH_AMD_COMP)) {
		printf("Can't erase unknown flash type %08lx - aborted\n",
		       info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect])
			prot++;
	}

	if (prot) {
		printf("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf("\n");
	}

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

#if defined(CONFIG_BOOT_8B)
	my_out_8((unsigned char *)((ulong)addr + 0x555), 0xaa);
	my_out_8((unsigned char *)((ulong)addr + 0x2aa), 0x55);
	my_out_8((unsigned char *)((ulong)addr + 0x555), 0x90);

	in_mid = my_in_8((unsigned char *)addr);
	in_did = my_in_8((unsigned char *)((ulong)addr + 1));

	printf(" man ID=0x%x, dev ID=0x%x.\n", in_mid, in_did);

	my_out_8((unsigned char *)addr, 0xf0);
	udelay(1);

	my_out_8((unsigned char *)((ulong)addr + 0x555), 0xaa);
	my_out_8((unsigned char *)((ulong)addr + 0x2aa), 0x55);
	my_out_8((unsigned char *)((ulong)addr + 0x555), 0x80);
	my_out_8((unsigned char *)((ulong)addr + 0x555), 0xaa);
	my_out_8((unsigned char *)((ulong)addr + 0x2aa), 0x55);

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_long *) (info->start[sect]);
			/*addr[0] = 0x00300030; */
			my_out_8((unsigned char *)((ulong)addr), 0x30);
			l_sect = sect;
		}
	}
#elif defined(CONFIG_BOOT_16B)
	my_out_be16((unsigned short *)((ulong)addr + (0xaaa)), 0xaa);
	my_out_be16((unsigned short *)((ulong)addr + (0x554)), 0x55);
	my_out_be16((unsigned short *)((ulong)addr + (0xaaa)), 0x90);
	in_mid = my_in_be16((unsigned short *)addr);
	in_did = my_in_be16((unsigned short *)((ulong)addr + 2));
	printf(" man ID=0x%x, dev ID=0x%x.\n", in_mid, in_did);
	my_out_be16((unsigned short *)addr, 0xf0);
	udelay(1);
	my_out_be16((unsigned short *)((ulong)addr + 0xaaa), 0xaa);
	my_out_be16((unsigned short *)((ulong)addr + 0x554), 0x55);
	my_out_be16((unsigned short *)((ulong)addr + 0xaaa), 0x80);
	my_out_be16((unsigned short *)((ulong)addr + 0xaaa), 0xaa);
	my_out_be16((unsigned short *)((ulong)addr + 0x554), 0x55);
	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_long *) (info->start[sect]);
			my_out_be16((unsigned short *)((ulong)addr), 0x30);
			l_sect = sect;
		}
	}

#elif defined(CONFIG_BOOT_32B)
	my_out_be32((unsigned *)((ulong)addr + 0x1554), 0xaa);
	my_out_be32((unsigned *)((ulong)addr + 0xaa8), 0x55);
	my_out_be32((unsigned *)((ulong)addr + 0x1554), 0x90);

	in_mid = my_in_be32((unsigned *)addr);
	in_did = my_in_be32((unsigned *)((ulong)addr + 4));

	printf(" man ID=0x%x, dev ID=0x%x.\n", in_mid, in_did);

	my_out_be32((unsigned *) addr, 0xf0);
	udelay(1);

	my_out_be32((unsigned *)((ulong)addr + 0x1554), 0xaa);
	my_out_be32((unsigned *)((ulong)addr + 0xaa8), 0x55);
	my_out_be32((unsigned *)((ulong)addr + 0x1554), 0x80);
	my_out_be32((unsigned *)((ulong)addr + 0x1554), 0xaa);
	my_out_be32((unsigned *)((ulong)addr + 0xaa8), 0x55);

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_long *) (info->start[sect]);
			my_out_be32((unsigned *)((ulong)addr), 0x00300030);
			l_sect = sect;
		}
	}

#else
#error CONFIG_BOOT_(size)B missing.
#endif
	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* wait at least 80us - let's wait 1 ms */
	udelay(1000);

	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;

	start = get_timer(0);
	last = start;
	addr = (vu_long *) (info->start[l_sect]);
#if defined(CONFIG_BOOT_8B)
	while ((my_in_8((unsigned char *) addr) & 0x80) != 0x80)
#elif defined(CONFIG_BOOT_16B)
	while ((my_in_be16((unsigned short *) addr) & 0x0080) != 0x0080)
#elif defined(CONFIG_BOOT_32B)
	while ((my_in_be32((unsigned *) addr) & 0x00800080) != 0x00800080)
#else
#error CONFIG_BOOT_(size)B missing.
#endif
	{
		now = get_timer(start);
		if (now > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			putc('.');
			last = now;
		}
	}
DONE:
	/* reset to read mode */
	addr = (volatile unsigned long *) info->start[0];

#if defined(CONFIG_BOOT_8B)
	my_out_8((unsigned char *) addr, 0xf0);
#elif defined(CONFIG_BOOT_16B)
	my_out_be16((unsigned short *) addr, 0x00f0);
#elif defined(CONFIG_BOOT_32B)
	my_out_be32((unsigned *) addr, 0x00F000F0);	/* reset bank */
#else
#error CONFIG_BOOT_(size)B missing.
#endif
	printf(" done\n");
	return 0;
}

/*
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	wp = (addr & ~3);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	l = addr - wp;

	if (l != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp)
			data = (data << 8) | (*(uchar *) cp);

		for (; i < 4 && cnt > 0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < 4; ++i, ++cp)
			data = (data << 8) | (*(uchar *) cp);

		rc = write_word(info, wp, data);

		if (rc != 0)
			return rc;

		wp += 4;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		data = 0;
		for (i = 0; i < 4; ++i)
			data = (data << 8) | *src++;

		rc = write_word(info, wp, data);

		if (rc != 0)
			return rc;

		wp += 4;
		cnt -= 4;
	}

	if (cnt == 0)
		return 0;

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < 4 && cnt > 0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i < 4; ++i, ++cp)
		data = (data << 8) | (*(uchar *) cp);

	return write_word(info, wp, data);
}

/*
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word(flash_info_t *info, ulong dest, ulong data)
{
	ulong addr = (ulong) (info->start[0]);
	ulong start;
	int flag;
	ulong i;
	int data_short[2];

	/* Check if Flash is (sufficiently) erased */
	if (((ulong)*(ulong *)dest & data) != data)
		return 2;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();
#if defined(CONFIG_BOOT_8B)
#ifdef DEBUG
	{
		int in_mid, in_did;

		my_out_8((unsigned char *) (addr + 0x555), 0xaa);
		my_out_8((unsigned char *) (addr + 0x2aa), 0x55);
		my_out_8((unsigned char *) (addr + 0x555), 0x90);

		in_mid = my_in_8((unsigned char *) addr);
		in_did = my_in_8((unsigned char *) (addr + 1));

		printf(" man ID=0x%x, dev ID=0x%x.\n", in_mid, in_did);

		my_out_8((unsigned char *) addr, 0xf0);
		udelay(1);
	}
#endif
	{
		int data_ch[4];

		data_ch[0] = (int) ((data >> 24) & 0xff);
		data_ch[1] = (int) ((data >> 16) & 0xff);
		data_ch[2] = (int) ((data >> 8) & 0xff);
		data_ch[3] = (int) (data & 0xff);

		for (i = 0; i < 4; i++) {
			my_out_8((unsigned char *) (addr + 0x555), 0xaa);
			my_out_8((unsigned char *) (addr + 0x2aa), 0x55);
			my_out_8((unsigned char *) (addr + 0x555), 0xa0);
			my_out_8((unsigned char *) (dest + i), data_ch[i]);

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();

			start = get_timer(0);
			while ((my_in_8((unsigned char *)(dest + i))) !=
			       (data_ch[i])) {
				if (get_timer(start) >
				    CONFIG_SYS_FLASH_WRITE_TOUT) {
					return 1;
				}
			}
		}		/* for */
	}
#elif defined(CONFIG_BOOT_16B)
	data_short[0] = (int) (data >> 16) & 0xffff;
	data_short[1] = (int) data & 0xffff;
	for (i = 0; i < 2; i++) {
		my_out_be16((unsigned short *)((ulong)addr + 0xaaa), 0xaa);
		my_out_be16((unsigned short *)((ulong)addr + 0x554), 0x55);
		my_out_be16((unsigned short *)((ulong)addr + 0xaaa), 0xa0);
		my_out_be16((unsigned short *)(dest + (i * 2)),
			    data_short[i]);

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		start = get_timer(0);
		while ((my_in_be16((unsigned short *)(dest + (i * 2)))) !=
							(data_short[i])) {
			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT)
				return 1;
		}
	}
#elif defined(CONFIG_BOOT_32B)
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00A000A0;

	*((vu_long *)dest) = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer(0);
	while ((*((vu_long *)dest) & 0x00800080) != (data & 0x00800080)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT)
			return 1;
	}
#endif
	return 0;
}

#ifdef CONFIG_BOOT_8B
static int my_in_8(unsigned char *addr)
{
	int ret;
	__asm__ __volatile__("lbz%U1%X1 %0,%1; eieio":"=r"(ret):"m"(*addr));

	return ret;
}

static void my_out_8(unsigned char *addr, int val)
{
	__asm__ __volatile__("stb%U0%X0 %1,%0; eieio":"=m"(*addr):"r"(val));
}
#endif
#ifdef CONFIG_BOOT_16B
static int my_in_be16(unsigned short *addr)
{
	int ret;
	__asm__ __volatile__("lhz%U1%X1 %0,%1; eieio":"=r"(ret):"m"(*addr));

	return ret;
}

static void my_out_be16(unsigned short *addr, int val)
{
	__asm__ __volatile__("sth%U0%X0 %1,%0; eieio":"=m"(*addr):"r"(val));
}
#endif
#ifdef CONFIG_BOOT_32B
static unsigned my_in_be32(unsigned *addr)
{
	unsigned ret;
	__asm__ __volatile__("lwz%U1%X1 %0,%1; eieio":"=r"(ret):"m"(*addr));

	return ret;
}

static void my_out_be32(unsigned *addr, int val)
{
	__asm__ __volatile__("stw%U0%X0 %1,%0; eieio":"=m"(*addr):"r"(val));
}
#endif
