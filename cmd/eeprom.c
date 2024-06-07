// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * Set CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS to 0
 *
 */

#include <config.h>
#include <command.h>
#include <dm.h>
#include <eeprom.h>
#include <i2c.h>
#include <i2c_eeprom.h>
#include <eeprom_layout.h>
#include <vsprintf.h>
#include <linux/delay.h>

#ifndef	I2C_RXTX_LEN
#define I2C_RXTX_LEN	128
#endif

#define	EEPROM_PAGE_SIZE	(1 << CONFIG_SYS_EEPROM_PAGE_WRITE_BITS)
#define	EEPROM_PAGE_OFFSET(x)	((x) & (EEPROM_PAGE_SIZE - 1))

#if CONFIG_IS_ENABLED(DM_I2C)
static int eeprom_i2c_bus;
#endif

__weak int eeprom_write_enable(unsigned dev_addr, int state)
{
	return 0;
}

void eeprom_init(int bus)
{
	/* I2C EEPROM */
#if CONFIG_IS_ENABLED(DM_I2C)
	eeprom_i2c_bus = bus;
#elif CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	if (bus >= 0)
		i2c_set_bus_num(bus);
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif
}

/*
 * for CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 2 (16-bit EEPROM address) offset is
 *   0x000nxxxx for EEPROM address selectors at n, offset xxxx in EEPROM.
 *
 * for CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 1 (8-bit EEPROM page address) offset is
 *   0x00000nxx for EEPROM address selectors and page number at n.
 */
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

static int eeprom_len(unsigned offset, unsigned end)
{
	unsigned len = end - offset;

	/*
	 * For a FRAM device there is no limit on the number of the
	 * bytes that can be accessed with the single read or write
	 * operation.
	 */
#if !defined(CONFIG_SYS_I2C_FRAM)
	unsigned blk_off = offset & 0xff;
	unsigned maxlen = EEPROM_PAGE_SIZE - EEPROM_PAGE_OFFSET(blk_off);

	if (maxlen > I2C_RXTX_LEN)
		maxlen = I2C_RXTX_LEN;

	if (len > maxlen)
		len = maxlen;
#endif

	return len;
}

static int eeprom_rw_block(unsigned offset, uchar *addr, unsigned alen,
			   uchar *buffer, unsigned len, bool read)
{
	int ret = 0;

#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(eeprom_i2c_bus, addr[0],
				      alen - 1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       eeprom_i2c_bus);
		return CMD_RET_FAILURE;
	}

	if (read)
		ret = dm_i2c_read(dev, offset, buffer, len);
	else
		ret = dm_i2c_write(dev, offset, buffer, len);

#else /* Non DM I2C support - will be removed */

	if (read)
		ret = i2c_read(addr[0], offset, alen - 1, buffer, len);
	else
		ret = i2c_write(addr[0], offset, alen - 1, buffer, len);
#endif /* CONFIG_DM_I2C */
	if (ret)
		ret = CMD_RET_FAILURE;

	return ret;
}

static int eeprom_rw(unsigned dev_addr, unsigned offset, uchar *buffer,
		     unsigned cnt, bool read)
{
	unsigned end = offset + cnt;
	unsigned alen, len;
	int rcode = 0;
	uchar addr[3];

#if !CONFIG_IS_ENABLED(DM_I2C) && defined(CONFIG_SYS_I2C_EEPROM_BUS)
	eeprom_init(CONFIG_SYS_I2C_EEPROM_BUS);
#endif

	while (offset < end) {
		alen = eeprom_addr(dev_addr, offset, addr);

		len = eeprom_len(offset, end);

		rcode = eeprom_rw_block(offset, addr, alen, buffer, len, read);

		buffer += len;
		offset += len;

#if CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS > 0
		if (!read)
			udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
#endif
	}

	return rcode;
}

int eeprom_read(unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt)
{
	/*
	 * Read data until done or would cross a page boundary.
	 * We must write the address again when changing pages
	 * because the next page may be in a different device.
	 */
	return eeprom_rw(dev_addr, offset, buffer, cnt, 1);
}

int eeprom_write(unsigned dev_addr, unsigned offset,
		 uchar *buffer, unsigned cnt)
{
	int ret;

	eeprom_write_enable(dev_addr, 1);

	/*
	 * Write data until done or would cross a write page boundary.
	 * We must write the address again when changing pages
	 * because the address counter only increments within a page.
	 */
	ret = eeprom_rw(dev_addr, offset, buffer, cnt, 0);

	eeprom_write_enable(dev_addr, 0);
	return ret;
}

static long parse_numeric_param(char *str)
{
	char *endptr;
	long value = simple_strtol(str, &endptr, 16);

	return (*endptr != '\0') ? -1 : value;
}

struct eeprom_dev_spec {
#if CONFIG_IS_ENABLED(I2C_EEPROM)
	struct udevice *dev;
#endif
	int i2c_bus;
	ulong i2c_addr;
};

static void eeprom_dev_spec_init(struct eeprom_dev_spec *dev)
{
#if CONFIG_IS_ENABLED(I2C_EEPROM)
	if (!dev->dev)
#endif
		eeprom_init(dev->i2c_bus);
}

static int eeprom_dev_spec_read(struct eeprom_dev_spec *dev,
				unsigned offset, uchar *buffer, unsigned cnt)
{
#if CONFIG_IS_ENABLED(I2C_EEPROM)
	if (dev->dev)
		return i2c_eeprom_read(dev->dev, offset, buffer, cnt);
#endif
	return eeprom_read(dev->i2c_addr, offset, buffer, cnt);
}

static int eeprom_dev_spec_write(struct eeprom_dev_spec *dev,
				 unsigned offset, uchar *buffer, unsigned cnt)
{
#if CONFIG_IS_ENABLED(I2C_EEPROM)
	if (dev->dev)
		return i2c_eeprom_write(dev->dev, offset, buffer, cnt);
#endif
	return eeprom_write(dev->i2c_addr, offset, buffer, cnt);
}

/**
 * parse_eeprom_dev_spec - parse the eeprom device specifier
 *
 * @dev:	pointer to eeprom device specifier
 * @argc:	count of command line arguments that can be used to parse
 *		the device specifier
 * @argv:	command line arguments left to parse
 *
 * @returns:	number of arguments parsed or CMD_RET_USAGE if error
 */
static int parse_eeprom_dev_spec(struct eeprom_dev_spec *dev, int argc,
				 char *const argv[])
{
#if CONFIG_IS_ENABLED(I2C_EEPROM)
	if (argc == 0) {
		if (!uclass_first_device_err(UCLASS_I2C_EEPROM, &dev->dev))
			return 0;
	}

	if (argc == 1) {
		if (!uclass_get_device_by_name(UCLASS_I2C_EEPROM, argv[0],
					       &dev->dev))
			return 1;

		/*
		 * If we could not find the device by name and the parameter is
		 * not numeric (and so won't be handled later), fail.
		 */
		if (parse_numeric_param(argv[0]) == -1) {
			printf("Can't get eeprom device: %s\n", argv[0]);
			return CMD_RET_USAGE;
		}
	}
#endif

#ifdef CONFIG_SYS_I2C_EEPROM_ADDR
	if (argc == 0) {
		dev->i2c_bus = -1;
		dev->i2c_addr = CONFIG_SYS_I2C_EEPROM_ADDR;

		return 0;
	}
#endif
	if (argc == 1) {
		dev->i2c_bus = -1;
		dev->i2c_addr = parse_numeric_param(argv[0]);

		return 1;
	}

	if (argc == 2) {
		dev->i2c_bus = parse_numeric_param(argv[0]);
		dev->i2c_addr = parse_numeric_param(argv[1]);

		return 2;
	}

	return CMD_RET_USAGE;
}

#ifdef CONFIG_CMD_EEPROM_LAYOUT

#ifdef CONFIG_EEPROM_LAYOUT_VERSIONS
__weak int eeprom_parse_layout_version(char *str)
{
	return LAYOUT_VERSION_UNRECOGNIZED;
}
#endif

static unsigned char eeprom_buf[CONFIG_SYS_EEPROM_SIZE];

#endif

enum eeprom_action {
	EEPROM_LIST,
	EEPROM_READ,
	EEPROM_WRITE,
	EEPROM_PRINT,
	EEPROM_UPDATE,
	EEPROM_ACTION_INVALID,
};

static enum eeprom_action parse_action(char *cmd)
{
#if CONFIG_IS_ENABLED(I2C_EEPROM)
	if (!strncmp(cmd, "list", 4))
		return EEPROM_LIST;
#endif
	if (!strncmp(cmd, "read", 4))
		return EEPROM_READ;
	if (!strncmp(cmd, "write", 5))
		return EEPROM_WRITE;
#ifdef CONFIG_CMD_EEPROM_LAYOUT
	if (!strncmp(cmd, "print", 5))
		return EEPROM_PRINT;
	if (!strncmp(cmd, "update", 6))
		return EEPROM_UPDATE;
#endif

	return EEPROM_ACTION_INVALID;
}

#if CONFIG_IS_ENABLED(I2C_EEPROM)
static int do_eeprom_list(void)
{
	struct udevice *dev;
	struct uclass *uc;
	int err;

	err = uclass_get(UCLASS_I2C_EEPROM, &uc);
	if (err)
		return CMD_RET_FAILURE;

	uclass_foreach_dev(dev, uc)
		printf("%s (%s)\n", dev->name, dev->driver->name);

	return CMD_RET_SUCCESS;
}
#endif

static int do_eeprom_rw(struct eeprom_dev_spec *dev, bool read,
			ulong addr, ulong off, ulong cnt)
{
	const char *const fmt =
		"\nEEPROM @0x%lX %s: addr 0x%08lx  off 0x%04lx  count %ld ... ";
	uchar *memloc = (uchar *)addr;
	int ret;

	printf(fmt, dev->i2c_addr, read ? "read" : "write", addr, off, cnt);
	if (read)
		ret = eeprom_dev_spec_read(dev, off, memloc, cnt);
	else
		ret = eeprom_dev_spec_write(dev, off, memloc, cnt);
	puts("done\n");

	return ret;
}

#ifdef CONFIG_CMD_EEPROM_LAYOUT

static int do_eeprom_layout(struct eeprom_dev_spec *dev, int layout_ver,
			    struct eeprom_layout *layout)
{
	eeprom_layout_setup(layout, eeprom_buf, CONFIG_SYS_EEPROM_SIZE,
			    layout_ver);

	return eeprom_dev_spec_read(dev, 0, eeprom_buf, layout->data_size);
}

static int do_eeprom_print(struct eeprom_dev_spec *dev, int layout_ver)
{
	struct eeprom_layout layout;
	int ret;

	ret = do_eeprom_layout(dev, layout_ver, &layout);
	if (ret)
		return ret;

	layout.print(&layout);

	return 0;
}

static int do_eeprom_update(struct eeprom_dev_spec *dev, int layout_ver,
			    char *key, char *value)
{
	struct eeprom_layout layout;
	int ret;

	ret = do_eeprom_layout(dev, layout_ver, &layout);
	if (ret)
		return ret;

	ret = layout.update(&layout, key, value);
	if (ret)
		return CMD_RET_FAILURE;

	return eeprom_dev_spec_write(dev, 0, layout.data, layout.data_size);
}

#endif

static int eeprom_action_expected_argc(enum eeprom_action action)
{
	switch (action) {
	case EEPROM_LIST:
		return 0;
	case EEPROM_READ:
	case EEPROM_WRITE:
		return 3;
	case EEPROM_PRINT:
		return 0;
	case EEPROM_UPDATE:
		return 2;
	default:
		return CMD_RET_USAGE;
	}
}

#define NEXT_PARAM(argc, index)	{ (argc)--; (index)++; }
int do_eeprom(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	enum eeprom_action action = EEPROM_ACTION_INVALID;
	struct eeprom_dev_spec dev;
	ulong addr = 0, cnt = 0, off = 0;
	int ret, index = 0;
#ifdef CONFIG_CMD_EEPROM_LAYOUT
	char *field_name = "";
	char *field_value = "";
	int layout_ver = LAYOUT_VERSION_AUTODETECT;
#endif

	if (argc <= 1)
		return CMD_RET_USAGE;

	NEXT_PARAM(argc, index); /* Skip program name */

	action = parse_action(argv[index]);
	NEXT_PARAM(argc, index);

	if (action == EEPROM_ACTION_INVALID)
		return CMD_RET_USAGE;

#if CONFIG_IS_ENABLED(I2C_EEPROM)
	if (action == EEPROM_LIST)
		return do_eeprom_list();
#endif

#ifdef CONFIG_EEPROM_LAYOUT_VERSIONS
	if (action == EEPROM_PRINT || action == EEPROM_UPDATE) {
		if (!strcmp(argv[index], "-l")) {
			NEXT_PARAM(argc, index);
			layout_ver = eeprom_parse_layout_version(argv[index]);
			NEXT_PARAM(argc, index);
		}
	}
#endif

	ret = parse_eeprom_dev_spec(&dev,
				    argc - eeprom_action_expected_argc(action),
				    argv + index);
	if (ret == CMD_RET_USAGE)
		return ret;

	while (ret--)
		NEXT_PARAM(argc, index);

	if (action == EEPROM_READ || action == EEPROM_WRITE) {
		addr = parse_numeric_param(argv[index]);
		NEXT_PARAM(argc, index);
		off = parse_numeric_param(argv[index]);
		NEXT_PARAM(argc, index);
		cnt = parse_numeric_param(argv[index]);
	}

#ifdef CONFIG_CMD_EEPROM_LAYOUT
	if (action == EEPROM_UPDATE) {
		field_name = argv[index];
		NEXT_PARAM(argc, index);
		field_value = argv[index];
		NEXT_PARAM(argc, index);
	}
#endif

	eeprom_dev_spec_init(&dev);

	switch (action) {
	case EEPROM_READ:
	case EEPROM_WRITE:
		return do_eeprom_rw(&dev, action == EEPROM_READ,
				    addr, off, cnt);
#ifdef CONFIG_CMD_EEPROM_LAYOUT
	case EEPROM_PRINT:
		return do_eeprom_print(&dev, layout_ver);
	case EEPROM_UPDATE:
		return do_eeprom_update(&dev, layout_ver,
					field_name, field_value);
#endif
	default:
		return CMD_RET_USAGE;
	}
}

#ifdef CONFIG_EEPROM_LAYOUT_VERSIONS
#define EEPROM_LAYOUT_SPEC	"[-l <layout_version>] "
#else
#define EEPROM_LAYOUT_SPEC	""
#endif

#if CONFIG_IS_ENABLED(I2C_EEPROM)
# define EEPROM_DEV_SPEC	"[device_specifier]"
#else
# define EEPROM_DEV_SPEC	"[[bus] devaddr]"
#endif

U_BOOT_CMD(
	eeprom,	8,	1,	do_eeprom,
	"EEPROM sub-system",
#if CONFIG_IS_ENABLED(I2C_EEPROM)
	"list\n"
	"eeprom "
#endif
	"read  " EEPROM_DEV_SPEC " addr off cnt\n"
	"eeprom write " EEPROM_DEV_SPEC " addr off cnt\n"
	"       - read/write `cnt' bytes from `devaddr` EEPROM at offset `off'"
#ifdef CONFIG_CMD_EEPROM_LAYOUT
	"\n"
	"eeprom print " EEPROM_LAYOUT_SPEC EEPROM_DEV_SPEC "\n"
	"       - Print layout fields and their data in human readable format\n"
	"eeprom update " EEPROM_LAYOUT_SPEC EEPROM_DEV_SPEC " field_name field_value\n"
	"       - Update a specific eeprom field with new data.\n"
	"         The new data must be written in the same human readable format as shown by the print command."
#endif
#if CONFIG_IS_ENABLED(I2C_EEPROM)
	"\n\n"
	"DEVICE SPECIFIER - the eeprom device can be specified\n"
	"  [dev_name] - by device name (devices can listed with the eeprom list command)\n"
	"  [[bus] devaddr] - or by I2C bus and I2C device address\n"
	"If no device specifier is given, the first driver-model found device is used."
#endif
#ifdef CONFIG_EEPROM_LAYOUT_VERSIONS
	"\n\n"
	"LAYOUT VERSIONS\n"
	"The -l option can be used to force the command to interpret the EEPROM data using the chosen layout.\n"
	"If the -l option is omitted, the command will auto detect the layout based on the data in the EEPROM.\n"
	"The values which can be provided with the -l option are:\n"
	CONFIG_EEPROM_LAYOUT_HELP_STRING"\n"
#endif
);
