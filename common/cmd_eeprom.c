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

#if defined(CONFIG_SYS_EEPROM_WREN)
extern int eeprom_write_enable (unsigned dev_addr, int state);
#endif

void eeprom_init(void)
{
	/* SPI EEPROM */
#if defined(CONFIG_SPI) && !defined(CONFIG_ENV_EEPROM_IS_ON_I2C)
	spi_init_f ();
#endif

	/* I2C EEPROM */
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SYS_I2C_SOFT)
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif
}

int eeprom_read (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt)
{
	unsigned end = offset + cnt;
	unsigned blk_off;
	int rcode = 0;

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

#if CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 1 && !defined(CONFIG_SPI_X)
		uchar addr[2];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 8;		/* block number */
		addr[1] = blk_off;		/* block offset */
		alen	= 2;
#else
		uchar addr[3];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 16;		/* block number */
		addr[1] = offset >>  8;		/* upper address octet */
		addr[2] = blk_off;		/* lower address octet */
		alen	= 3;
#endif	/* CONFIG_SYS_I2C_EEPROM_ADDR_LEN, CONFIG_SPI_X */

		addr[0] |= dev_addr;		/* insert device address */

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

#if defined(CONFIG_SPI) && !defined(CONFIG_ENV_EEPROM_IS_ON_I2C)
		spi_read (addr, alen, buffer, len);
#else
#if defined(CONFIG_SYS_I2C_EEPROM_BUS)
		i2c_set_bus_num(CONFIG_SYS_I2C_EEPROM_BUS);
#endif
		if (i2c_read(addr[0], offset, alen - 1, buffer, len))
			rcode = 1;
#endif
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

#if defined(CONFIG_SYS_EEPROM_WREN)
	eeprom_write_enable (dev_addr,1);
#endif
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

#if CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 1 && !defined(CONFIG_SPI_X)
		uchar addr[2];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 8;		/* block number */
		addr[1] = blk_off;		/* block offset */
		alen	= 2;
#else
		uchar addr[3];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 16;		/* block number */
		addr[1] = offset >>  8;		/* upper address octet */
		addr[2] = blk_off;		/* lower address octet */
		alen	= 3;
#endif	/* CONFIG_SYS_I2C_EEPROM_ADDR_LEN, CONFIG_SPI_X */

		addr[0] |= dev_addr;		/* insert device address */

		len = end - offset;

		/*
		 * For a FRAM device there is no limit on the number of the
		 * bytes that can be accessed with the single read or write
		 * operation.
		 */
#if !defined(CONFIG_SYS_I2C_FRAM)

#if defined(CONFIG_SYS_EEPROM_PAGE_WRITE_BITS)

#define	EEPROM_PAGE_SIZE	(1 << CONFIG_SYS_EEPROM_PAGE_WRITE_BITS)
#define	EEPROM_PAGE_OFFSET(x)	((x) & (EEPROM_PAGE_SIZE - 1))

		maxlen = EEPROM_PAGE_SIZE - EEPROM_PAGE_OFFSET(blk_off);
#else
		maxlen = 0x100 - blk_off;
#endif
		if (maxlen > I2C_RXTX_LEN)
			maxlen = I2C_RXTX_LEN;

		if (len > maxlen)
			len = maxlen;
#endif

#if defined(CONFIG_SPI) && !defined(CONFIG_ENV_EEPROM_IS_ON_I2C)
		spi_write (addr, alen, buffer, len);
#else

#if defined(CONFIG_SYS_I2C_EEPROM_BUS)
		i2c_set_bus_num(CONFIG_SYS_I2C_EEPROM_BUS);
#endif
		if (i2c_write(addr[0], offset, alen - 1, buffer, len))
			rcode = 1;

#endif
		buffer += len;
		offset += len;

#if defined(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS)
		udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
#endif
	}
#if defined(CONFIG_SYS_EEPROM_WREN)
	eeprom_write_enable (dev_addr,0);
#endif
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

# if !defined(CONFIG_SPI) || defined(CONFIG_ENV_EEPROM_IS_ON_I2C)
	eeprom_init ();
# endif /* !CONFIG_SPI */

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
