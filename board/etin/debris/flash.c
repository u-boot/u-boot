/*
 * board/eva/flash.c
 *
 * (C) Copyright 2002
 * Sangmoon Kim, Etin Systems, dogoil@etinsys.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/pci_io.h>
#include <mpc824x.h>
#include <asm/mmu.h>

int (*do_flash_erase)(flash_info_t*, uint32_t, uint32_t);
int (*write_dword)(flash_info_t*, ulong, uint64_t);

typedef uint64_t cfi_word;

#define cfi_read(flash, addr) *((volatile cfi_word*)(flash->start[0] + addr))

#define cfi_write(flash, val, addr) \
	move64((cfi_word*)&val, \
			(cfi_word*)(flash->start[0] + addr))

#define CMD(x) ((((cfi_word)x)<<48)|(((cfi_word)x)<<32)|(((cfi_word)x)<<16)|(((cfi_word)x)))

static void write32(unsigned long addr, uint32_t value)
{
	*(volatile uint32_t*)(addr) = value;
	asm volatile("sync");
}

static uint32_t read32(unsigned long addr)
{
	uint32_t value;
	value = *(volatile uint32_t*)addr;
	asm volatile("sync");
	return value;
}

static cfi_word cfi_cmd(flash_info_t *flash, uint8_t cmd, uint32_t addr)
{
	uint32_t base = flash->start[0];
	uint32_t val=(cmd << 16) | cmd;
	addr <<= 3;
	write32(base + addr, val);
	return addr;
}

static uint16_t cfi_read_query(flash_info_t *flash, uint32_t addr)
{
	uint32_t base = flash->start[0];
	addr <<= 3;
	return (uint16_t)read32(base + addr);
}

flash_info_t    flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */

static void move64(uint64_t *src, uint64_t *dest)
{
	asm volatile("lfd  0, 0(3)\n\t" /* fpr0   =  *scr       */
	 "stfd 0, 0(4)"         /* *dest  =  fpr0       */
	 : : : "fr0" );         /* Clobbers fr0         */
	return;
}

static int cfi_write_dword(flash_info_t *flash, ulong dest, cfi_word data)
{
	unsigned long start;
	cfi_word status = 0;

	status = cfi_read(flash, dest);
	data &= status;

	cfi_cmd(flash, 0x40, 0);
	cfi_write(flash, data, dest);

	udelay(10);
	start = get_timer (0);
	for(;;) {
		status = cfi_read(flash, dest);
		status &= CMD(0x80);
		if(status == CMD(0x80))
			break;
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			cfi_cmd(flash, 0xff, 0);
			return 1;
		}
		udelay(1);
	}
	cfi_cmd(flash, 0xff, 0);

	return 0;
}

static int jedec_write_dword (flash_info_t *flash, ulong dest, cfi_word data)
{
	ulong start;
	cfi_word status = 0;

	status = cfi_read(flash, dest);
	if(status != CMD(0xffff)) return 2;

	cfi_cmd(flash, 0xaa, 0x555);
	cfi_cmd(flash, 0x55, 0x2aa);
	cfi_cmd(flash, 0xa0, 0x555);

	cfi_write(flash, data, dest);

	udelay(10);
	start = get_timer (0);
	status = ~data;
	while(status != data) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT)
			return 1;
		status = cfi_read(flash, dest);
		udelay(1);
	}
	return 0;
}

static __inline__ unsigned long get_msr(void)
{
	unsigned long msr;
	__asm__ __volatile__ ("mfmsr %0" : "=r" (msr) :);
	return msr;
}

static __inline__ void set_msr(unsigned long msr)
{
	__asm__ __volatile__ ("mtmsr %0" : : "r" (msr));
}

int write_buff (flash_info_t *flash, uchar *src, ulong addr, ulong cnt)
{
	ulong wp;
	int i, s, l, rc;
	cfi_word data;
	uint8_t *t = (uint8_t*)&data;
	unsigned long base = flash->start[0];
	uint32_t msr;

	if (flash->flash_id == FLASH_UNKNOWN)
		return 4;

	if (cnt == 0)
		return 0;

	addr -= base;

	msr = get_msr();
	set_msr(msr|MSR_FP);

	wp = (addr & ~7);   /* get lower word aligned address */

	if((addr-wp) != 0) {
		data = cfi_read(flash, wp);
		s = addr & 7;
		l = ( cnt < (8-s) ) ? cnt : (8-s);
		for(i = 0; i < l; i++)
			t[s+i] = *src++;
		if ((rc = write_dword(flash, wp, data)) != 0)
			goto DONE;
		wp += 8;
		cnt -= l;
	}

	while (cnt >= 8) {
		for (i = 0; i < 8; i++)
			t[i] = *src++;
		if ((rc = write_dword(flash, wp, data)) != 0)
			goto DONE;
		wp  += 8;
		cnt -= 8;
	}

	if (cnt == 0) {
		rc = 0;
		goto DONE;
	}

	data = cfi_read(flash, wp);
	for(i = 0; i < cnt; i++)
		t[i] = *src++;
	rc = write_dword(flash, wp, data);
DONE:
	set_msr(msr);
	return rc;
}

static int cfi_erase_oneblock(flash_info_t *flash, uint32_t sect)
{
	int sa;
	int flag;
	ulong start, last, now;
	cfi_word status;

	flag = disable_interrupts();

	sa = (flash->start[sect] - flash->start[0]);
	write32(flash->start[sect], 0x00200020);
	write32(flash->start[sect], 0x00d000d0);

	if (flag)
		enable_interrupts();

	udelay(1000);
	start = get_timer (0);
	last  = start;

	for (;;) {
		status = cfi_read(flash, sa);
		status &= CMD(0x80);
		if (status == CMD(0x80))
			break;
		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			cfi_cmd(flash, 0xff, 0);
			printf ("Timeout\n");
			return ERR_TIMOUT;
		}

		if ((now - last) > 1000) {
			serial_putc ('.');
			last = now;
		}
		udelay(10);
	}
	cfi_cmd(flash, 0xff, 0);
	return ERR_OK;
}

static int cfi_erase(flash_info_t *flash, uint32_t s_first, uint32_t s_last)
{
	int sect;
	int rc = ERR_OK;

	for (sect = s_first; sect <= s_last; sect++) {
		if (flash->protect[sect] == 0) {
			rc = cfi_erase_oneblock(flash, sect);
			if (rc != ERR_OK) break;
		}
	}
	printf (" done\n");
	return rc;
}

static int jedec_erase(flash_info_t *flash, uint32_t s_first, uint32_t s_last)
{
	int sect;
	cfi_word status;
	int sa = -1;
	int flag;
	ulong start, last, now;

	flag = disable_interrupts();

	cfi_cmd(flash, 0xaa, 0x555);
	cfi_cmd(flash, 0x55, 0x2aa);
	cfi_cmd(flash, 0x80, 0x555);
	cfi_cmd(flash, 0xaa, 0x555);
	cfi_cmd(flash, 0x55, 0x2aa);
	for ( sect = s_first; sect <= s_last; sect++) {
		if (flash->protect[sect] == 0) {
			sa = flash->start[sect] - flash->start[0];
			write32(flash->start[sect], 0x00300030);
		}
	}
	if (flag)
		enable_interrupts();

	if (sa < 0)
		goto DONE;

	udelay (1000);
	start = get_timer (0);
	last  = start;
	for(;;) {
		status = cfi_read(flash, sa);
		if (status == CMD(0xffff))
			break;

		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return ERR_TIMOUT;
		}

		if ((now - last) > 1000) {
			serial_putc ('.');
			last = now;
		}
		udelay(10);
	}
DONE:
	cfi_cmd(flash, 0xf0, 0);

	printf (" done\n");

	return ERR_OK;
}

int flash_erase (flash_info_t *flash, int s_first, int s_last)
{
	int sect;
	int prot;

	if ((s_first < 0) || (s_first > s_last)) {
		if (flash->flash_id == FLASH_UNKNOWN)
			printf ("- missing\n");
		else
			printf ("- no sectors to erase\n");
		return ERR_NOT_ERASED;
	}
	if (flash->flash_id == FLASH_UNKNOWN) {
		printf ("Can't erase unknown flash type - aborted\n");
		return ERR_NOT_ERASED;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; sect++)
		if (flash->protect[sect]) prot++;

	if (prot)
		printf ("- Warning: %d protected sectors will not be erased!\n",
					                        prot);
	else
		printf ("\n");

	return do_flash_erase(flash, s_first, s_last);
}

struct jedec_flash_info {
	const uint16_t mfr_id;
	const uint16_t dev_id;
	const char *name;
	const int DevSize;
	const int InterfaceDesc;
	const int NumEraseRegions;
	const ulong regions[4];
};

#define ERASEINFO(size,blocks) (size<<8)|(blocks-1)

#define SIZE_1MiB 20
#define SIZE_2MiB 21
#define SIZE_4MiB 22

static const struct jedec_flash_info jedec_table[] = {
	{
		mfr_id: (uint16_t)AMD_MANUFACT,
		dev_id: (uint16_t)AMD_ID_LV800T,
		name: "AMD AM29LV800T",
		DevSize: SIZE_1MiB,
		NumEraseRegions: 4,
		regions: {ERASEINFO(0x10000,15),
			  ERASEINFO(0x08000,1),
			  ERASEINFO(0x02000,2),
			  ERASEINFO(0x04000,1)
		}
	}, {
		mfr_id: (uint16_t)AMD_MANUFACT,
		dev_id: (uint16_t)AMD_ID_LV800B,
		name: "AMD AM29LV800B",
		DevSize: SIZE_1MiB,
		NumEraseRegions: 4,
		regions: {ERASEINFO(0x10000,15),
		          ERASEINFO(0x08000,1),
			  ERASEINFO(0x02000,2),
			  ERASEINFO(0x04000,1)
		}
	}, {
		mfr_id: (uint16_t)AMD_MANUFACT,
		dev_id: (uint16_t)AMD_ID_LV160T,
		name: "AMD AM29LV160T",
		DevSize: SIZE_2MiB,
		NumEraseRegions: 4,
		regions: {ERASEINFO(0x10000,31),
		          ERASEINFO(0x08000,1),
			  ERASEINFO(0x02000,2),
			  ERASEINFO(0x04000,1)
		}
	}, {
		mfr_id: (uint16_t)AMD_MANUFACT,
		dev_id: (uint16_t)AMD_ID_LV160B,
		name: "AMD AM29LV160B",
		DevSize: SIZE_2MiB,
		NumEraseRegions: 4,
		regions: {ERASEINFO(0x04000,1),
		          ERASEINFO(0x02000,2),
			  ERASEINFO(0x08000,1),
			  ERASEINFO(0x10000,31)
		}
	}, {
		mfr_id: (uint16_t)AMD_MANUFACT,
		dev_id: (uint16_t)AMD_ID_LV320T,
		name: "AMD AM29LV320T",
		DevSize: SIZE_4MiB,
		NumEraseRegions: 2,
		regions: {ERASEINFO(0x10000,63),
		          ERASEINFO(0x02000,8)
		}

	}, {
		mfr_id: (uint16_t)AMD_MANUFACT,
		dev_id: (uint16_t)AMD_ID_LV320B,
		name: "AMD AM29LV320B",
		DevSize: SIZE_4MiB,
		NumEraseRegions: 2,
		regions: {ERASEINFO(0x02000,8),
		          ERASEINFO(0x10000,63)
		}
	}
};

static ulong cfi_init(uint32_t base,  flash_info_t *flash)
{
	int sector;
	int block;
	int block_count;
	int offset = 0;
	int reverse = 0;
	int primary;
	int mfr_id;
	int dev_id;

	flash->start[0] = base;
	cfi_cmd(flash, 0xF0, 0);
	cfi_cmd(flash, 0x98, 0);
	if ( !( cfi_read_query(flash, 0x10) == 'Q' &&
		cfi_read_query(flash, 0x11) == 'R' &&
		cfi_read_query(flash, 0x12) == 'Y' )) {
		cfi_cmd(flash, 0xff, 0);
		return 0;
	}

	flash->size = 1 << cfi_read_query(flash, 0x27);
	flash->size *= 4;
	block_count = cfi_read_query(flash, 0x2c);
	primary = cfi_read_query(flash, 0x15);
	if ( cfi_read_query(flash, primary + 4) == 0x30)
		reverse = (cfi_read_query(flash, 0x1) & 0x01);
	else
		reverse = (cfi_read_query(flash, primary+15) == 3);

	flash->sector_count = 0;

	for ( block = reverse ? block_count - 1	: 0;
		      reverse ? block >= 0	: block < block_count;
		      reverse ? block--		: block ++) {
		int sector_size =
			(cfi_read_query(flash, 0x2d + block*4+2) |
			(cfi_read_query(flash, 0x2d + block*4+3) << 8)) << 8;
		int sector_count =
			(cfi_read_query(flash, 0x2d + block*4+0) |
			(cfi_read_query(flash, 0x2d + block*4+1) << 8)) + 1;
		for(sector = 0; sector < sector_count; sector++) {
			flash->start[flash->sector_count++] = base + offset;
			offset += sector_size * 4;
		}
	}
	mfr_id = cfi_read_query(flash, 0x00);
	dev_id = cfi_read_query(flash, 0x01);

	cfi_cmd(flash, 0xff, 0);

	flash->flash_id = (mfr_id << 16) | dev_id;

	for (sector = 0; sector < flash->sector_count; sector++) {
		write32(flash->start[sector], 0x00600060);
		write32(flash->start[sector], 0x00d000d0);
	}
	cfi_cmd(flash, 0xff, 0);

	for (sector = 0; sector < flash->sector_count; sector++)
		flash->protect[sector] = 0;

	do_flash_erase = cfi_erase;
	write_dword = cfi_write_dword;

	return flash->size;
}

static ulong jedec_init(unsigned long base, flash_info_t *flash)
{
	int i;
	int block, block_count;
	int sector, offset;
	int mfr_id, dev_id;
	flash->start[0] = base;
	cfi_cmd(flash, 0xF0, 0x000);
	cfi_cmd(flash, 0xAA, 0x555);
	cfi_cmd(flash, 0x55, 0x2AA);
	cfi_cmd(flash, 0x90, 0x555);
	mfr_id = cfi_read_query(flash, 0x000);
	dev_id = cfi_read_query(flash, 0x0001);
	cfi_cmd(flash, 0xf0, 0x000);

	for(i=0; i<sizeof(jedec_table)/sizeof(struct jedec_flash_info); i++) {
		if((jedec_table[i].mfr_id == mfr_id) &&
			(jedec_table[i].dev_id == dev_id)) {

			flash->flash_id = (mfr_id << 16) | dev_id;
			flash->size = 1 << jedec_table[0].DevSize;
			flash->size *= 4;
			block_count = jedec_table[i].NumEraseRegions;
			offset = 0;
			flash->sector_count = 0;
			for (block = 0; block < block_count; block++) {
				int sector_size = jedec_table[i].regions[block];
				int sector_count = (sector_size & 0xff) + 1;
				sector_size >>= 8;
				for (sector=0; sector<sector_count; sector++) {
					flash->start[flash->sector_count++] =
						base + offset;
					offset += sector_size * 4;
				}
			}
			break;
		}
	}

	for (sector = 0; sector < flash->sector_count; sector++)
		flash->protect[sector] = 0;

	do_flash_erase = jedec_erase;
	write_dword = jedec_write_dword;

	return flash->size;
}

inline void mtibat1u(unsigned int x)
{
	__asm__ __volatile__ ("mtspr   530, %0" :: "r" (x));
}

inline void mtibat1l(unsigned int x)
{
	__asm__ __volatile__ ("mtspr   531, %0" :: "r" (x));
}

inline void mtdbat1u(unsigned int x)
{
	__asm__ __volatile__ ("mtspr   538, %0" :: "r" (x));
}

inline void mtdbat1l(unsigned int x)
{
	__asm__ __volatile__ ("mtspr   539, %0" :: "r" (x));
}

unsigned long flash_init (void)
{
	unsigned long size = 0;
	int i;
	unsigned int msr;

	/* BAT1 */
	CONFIG_WRITE_WORD(ERCR3, 0x0C00000C);
	CONFIG_WRITE_WORD(ERCR4, 0x0800000C);
	msr = get_msr();
	set_msr(msr & ~(MSR_IR | MSR_DR));
	mtibat1l(0x70000000 | BATL_PP_10 | BATL_CACHEINHIBIT);
	mtibat1u(0x70000000 | BATU_BL_256M | BATU_VS | BATU_VP);
	mtdbat1l(0x70000000 | BATL_PP_10 | BATL_CACHEINHIBIT);
	mtdbat1u(0x70000000 | BATU_BL_256M | BATU_VS | BATU_VP);
	set_msr(msr);

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++)
		flash_info[i].flash_id = FLASH_UNKNOWN;
	size = cfi_init(FLASH_BASE0_PRELIM, &flash_info[0]);
	if (!size)
		size = jedec_init(FLASH_BASE0_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN)
		printf ("# Unknown FLASH on Bank 1 - Size = 0x%08lx = %ld MB\n",
			size, size<<20);

	return size;
}

void flash_print_info  (flash_info_t *flash)
{
	int i;
	int k;
	int size;
	int erased;
	volatile unsigned long *p;

	if (flash->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		flash_init();
	}

	if (flash->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (((flash->flash_id) >> 16) & 0xff) {
	case 0x01:
		printf ("AMD ");
		break;
	case 0x04:
		printf("FUJITSU ");
		break;
	case 0x20:
		printf("STM ");
		break;
	case 0xBF:
		printf("SST ");
		break;
	case 0x89:
	case 0xB0:
		printf("INTEL ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch ((flash->flash_id) & 0xffff) {
	case (uint16_t)AMD_ID_LV800T:
		printf ("AM29LV800T\n");
		break;
	case (uint16_t)AMD_ID_LV800B:
		printf ("AM29LV800B\n");
		break;
	case (uint16_t)AMD_ID_LV160T:
		printf ("AM29LV160T\n");
		break;
	case (uint16_t)AMD_ID_LV160B:
		printf ("AM29LV160B\n");
		break;
	case (uint16_t)AMD_ID_LV320T:
		printf ("AM29LV320T\n");
		break;
	case (uint16_t)AMD_ID_LV320B:
		printf ("AM29LV320B\n");
		break;
	case (uint16_t)INTEL_ID_28F800C3T:
		printf ("28F800C3T\n");
		break;
	case (uint16_t)INTEL_ID_28F800C3B:
		printf ("28F800C3B\n");
		break;
	case (uint16_t)INTEL_ID_28F160C3T:
		printf ("28F160C3T\n");
		break;
	case (uint16_t)INTEL_ID_28F160C3B:
		printf ("28F160C3B\n");
		break;
	case (uint16_t)INTEL_ID_28F320C3T:
		printf ("28F320C3T\n");
		break;
	case (uint16_t)INTEL_ID_28F320C3B:
		printf ("28F320C3B\n");
		break;
	case (uint16_t)INTEL_ID_28F640C3T:
		printf ("28F640C3T\n");
		break;
	case (uint16_t)INTEL_ID_28F640C3B:
		printf ("28F640C3B\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		break;
	}

	if (flash->size >= (1 << 20)) {
		printf ("  Size: %ld MB in %d Sectors\n",
				flash->size >> 20, flash->sector_count);
	} else {
		printf ("  Size: %ld kB in %d Sectors\n",
				flash->size >> 10, flash->sector_count);
	}

	printf ("  Sector Start Addresses:");
	for (i = 0; i < flash->sector_count; ++i) {
		/* Check if whole sector is erased*/
		if (i != (flash->sector_count-1))
			size = flash->start[i+1] - flash->start[i];
		else
			size = flash->start[0] + flash->size - flash->start[i];

		erased = 1;
		p = (volatile unsigned long *)flash->start[i];
		size = size >> 2;        /* divide by 4 for longword access */
		for (k=0; k<size; k++) {
			if (*p++ != 0xffffffff) {
				erased = 0;
				break;
			}
		}

		if ((i % 5) == 0)
			printf ("\n   ");

		printf (" %08lX%s%s",
			flash->start[i],
			erased ? " E" : "  ",
			flash->protect[i] ? "RO " : "   ");
	}
	printf ("\n");
}
