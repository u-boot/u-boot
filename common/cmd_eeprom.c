/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Support for read and write access to EEPROM like memory devices. This
 * includes regular EEPROM as well as  FRAM (ferroelectic nonvolaile RAM).
 * FRAM devices read and write data at bus speed. In particular, there is no
 * write delay. Also, there is no limit imposed on the number of bytes that can
 * be transferred with a single read or write.
 *
 * Use the following configuration options to ensure no unneeded performance
 * degradation (typical for EEPROM) is incured for FRAM memory:
 *
 * #define CONFIG_SYS_I2C_FRAM
 * #undef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
 *
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <i2c.h>

#ifndef	CONFIG_SYS_I2C_SPEED
#define	CONFIG_SYS_I2C_SPEED	50000
#endif

#ifndef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	0
#endif

#ifndef CONFIG_SYS_EEPROM_PAGE_WRITE_BITS
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	8
#endif

#define	EEPROM_PAGE_SIZE	(1 << CONFIG_SYS_EEPROM_PAGE_WRITE_BITS)
#define	EEPROM_PAGE_OFFSET(x)	((x) & (EEPROM_PAGE_SIZE - 1))

/*
 * for CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 2 (16-bit EEPROM address) offset is
 *   0x000nxxxx for EEPROM address selectors at n, offset xxxx in EEPROM.
 *
 * for CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 1 (8-bit EEPROM page address) offset is
 *   0x00000nxx for EEPROM address selectors and page number at n.
 */
#if !defined(CONFIG_SPI) || defined(CONFIG_ENV_EEPROM_IS_ON_I2C)
#if !defined(CONFIG_SYS_I2C_EEPROM_ADDR_LEN) || \
    (CONFIG_SYS_I2C_EEPROM_ADDR_LEN < 1) || \
    (CONFIG_SYS_I2C_EEPROM_ADDR_LEN > 2)
#error CONFIG_SYS_I2C_EEPROM_ADDR_LEN must be 1 or 2
#endif
#endif

__weak int eeprom_write_enable(unsigned dev_addr, int state)
{
	return 0;
}

void eeprom_init(void)
{
	/* SPI EEPROM */
#if defined(CONFIG_SPI) && !defined(CONFIG_ENV_EEPROM_IS_ON_I2C)
	spi_init_f();
#endif

	/* I2C EEPROM */
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SYS_I2C_SOFT)
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif
}

static int eeprom_addr(unsigned dev_addr, unsigned offset, uchar *addr)
{
	unsigned blk_off;
	int alen;

	blk_off = offset & 0xff;	/* block offset */
#if CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 1
	addr[0] = offset >> 8;		/* block number */
	addr[1] = blk_off;		/* block offset */
	alen = 2;
#else
	addr[0] = offset >> 16;		/* block number */
	addr[1] = offset >>  8;		/* upper address octet */
	addr[2] = blk_off;		/* lower address octet */
	alen = 3;
#endif	/* CONFIG_SYS_I2C_EEPROM_ADDR_LEN */

	addr[0] |= dev_addr;		/* insert device address */

	return alen;
}

static int eeprom_rw_block(unsigned offset, uchar *addr, unsigned alen,
			   uchar *buffer, unsigned len, bool read)
{
	int ret = 0;

	/* SPI */
#if defined(CONFIG_SPI) && !defined(CONFIG_ENV_EEPROM_IS_ON_I2C)
	if (read)
		spi_read(addr, alen, buffer, len);
	else
		spi_write(addr, alen, buffer, len);
#else	/* I2C */

#if defined(CONFIG_SYS_I2C_EEPROM_BUS)
	i2c_set_bus_num(CONFIG_SYS_I2C_EEPROM_BUS);
#endif

	if (read)
		ret = i2c_read(addr[0], offset, alen - 1, buffer, len);
	else
		ret = i2c_write(addr[0], offset, alen - 1, buffer, len);

	if (ret)
		ret = 1;
#endif
	return ret;
}

int eeprom_read (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt)
{
	unsigned end = offset + cnt;
	unsigned blk_off;
	int rcode = 0;
	uchar addr[3];

	/*
	 * Read data until done or would cross a page boundary.
	 * We must write the address again when changing pages
	 * because the next page may be in a different device.
	 */
	while (offset < end) {
		unsigned alen, len;
#if !defined(CONFIG_SYS_I2C_FRAM)
		unsigned maxlen;
#endif

		blk_off = offset & 0xFF;	/* block offset */
		alen = eeprom_addr(dev_addr, offset, addr);

		len = end - offset;

		/*
		 * For a FRAM device there is no limit on the number of the
		 * bytes that can be ccessed with the single read or write
		 * operation.
		 */
#if !defined(CONFIG_SYS_I2C_FRAM)
		maxlen = 0x100 - blk_off;
		if (maxlen > I2C_RXTX_LEN)
			maxlen = I2C_RXTX_LEN;
		if (len > maxlen)
			len = maxlen;
#endif

		rcode = eeprom_rw_block(offset, addr, alen, buffer, len, 1);

		buffer += len;
		offset += len;
	}

	return rcode;
}

int eeprom_write (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt)
{
	unsigned end = offset + cnt;
	unsigned blk_off;
	int rcode = 0;
	uchar addr[3];

	eeprom_write_enable(dev_addr, 1);

	/*
	 * Write data until done or would cross a write page boundary.
	 * We must write the address again when changing pages
	 * because the address counter only increments within a page.
	 */

	while (offset < end) {
		unsigned alen, len;
#if !defined(CONFIG_SYS_I2C_FRAM)
		unsigned maxlen;
#endif

		blk_off = offset & 0xFF;	/* block offset */
		alen = eeprom_addr(dev_addr, offset, addr);

		len = end - offset;

		/*
		 * For a FRAM device there is no limit on the number of the
		 * bytes that can be accessed with the single read or write
		 * operation.
		 */
#if !defined(CONFIG_SYS_I2C_FRAM)

		maxlen = EEPROM_PAGE_SIZE - EEPROM_PAGE_OFFSET(blk_off);

		if (maxlen > I2C_RXTX_LEN)
			maxlen = I2C_RXTX_LEN;

		if (len > maxlen)
			len = maxlen;
#endif

		rcode = eeprom_rw_block(offset, addr, alen, buffer, len, 0);

		buffer += len;
		offset += len;

		udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
	}

	eeprom_write_enable(dev_addr, 0);

	return rcode;
}

static int do_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *const fmt =
		"\nEEPROM @0x%lX %s: addr %08lx  off %04lx  count %ld ... ";
	char * const *args = &argv[2];
	int rcode;
	ulong dev_addr, addr, off, cnt;

	switch (argc) {
#ifdef CONFIG_SYS_DEF_EEPROM_ADDR
	case 5:
		dev_addr = CONFIG_SYS_DEF_EEPROM_ADDR;
		break;
#endif
	case 6:
		dev_addr = simple_strtoul(*args++, NULL, 16);
		break;
	default:
		return CMD_RET_USAGE;
	}

	addr = simple_strtoul(*args++, NULL, 16);
	off = simple_strtoul(*args++, NULL, 16);
	cnt = simple_strtoul(*args++, NULL, 16);

	eeprom_init();

	if (strcmp (argv[1], "read") == 0) {
		printf(fmt, dev_addr, argv[1], addr, off, cnt);

		rcode = eeprom_read(dev_addr, off, (uchar *) addr, cnt);

		puts ("done\n");
		return rcode;
	} else if (strcmp (argv[1], "write") == 0) {
		printf(fmt, dev_addr, argv[1], addr, off, cnt);

		rcode = eeprom_write(dev_addr, off, (uchar *) addr, cnt);

		puts ("done\n");
		return rcode;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	eeprom,	6,	1,	do_eeprom,
	"EEPROM sub-system",
	"read  devaddr addr off cnt\n"
	"eeprom write devaddr addr off cnt\n"
	"       - read/write `cnt' bytes from `devaddr` EEPROM at offset `off'"
)
