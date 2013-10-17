/*
 * (C) Copyright 2010
 * Reinhard Meyer, EMK Elektronik, reinhard.meyer@emk-elektronik.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Note: RAMTRON SPI FRAMs are ferroelectric, nonvolatile RAMs
 * with an interface identical to SPI flash devices.
 * However since they behave like RAM there are no delays or
 * busy polls required. They can sustain read or write at the
 * allowed SPI bus speed, which can be 40 MHz for some devices.
 *
 * Unfortunately some RAMTRON devices do not have a means of
 * identifying them. They will leave the SO line undriven when
 * the READ-ID command is issued. It is therefore mandatory
 * that the MISO line has a proper pull-up, so that READ-ID
 * will return a row of 0xff. This 0xff pseudo-id will cause
 * probes by all vendor specific functions that are designed
 * to handle it. If the MISO line is not pulled up, READ-ID
 * could return any random noise, even mimicking another
 * device.
 *
 * We use CONFIG_SPI_FRAM_RAMTRON_NON_JEDEC
 * to define which device will be assumed after a simple status
 * register verify. This method is prone to false positive
 * detection and should therefore be the last to be tried.
 * Enter it in the last position in the table in spi_flash.c!
 *
 * The define CONFIG_SPI_FRAM_RAMTRON_NON_JEDEC both activates
 * compilation of the special handler and defines the device
 * to assume.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>
#include "sf_internal.h"

/*
 * Properties of supported FRAMs
 * Note: speed is currently not used because we have no method to deliver that
 * value to the upper layers
 */
struct ramtron_spi_fram_params {
	u32	size;		/* size in bytes */
	u8	addr_len;	/* number of address bytes */
	u8	merge_cmd;	/* some address bits are in the command byte */
	u8	id1;		/* device ID 1 (family, density) */
	u8	id2;		/* device ID 2 (sub, rev, rsvd) */
	u32	speed;		/* max. SPI clock in Hz */
	const char *name;	/* name for display and/or matching */
};

struct ramtron_spi_fram {
	struct spi_flash flash;
	const struct ramtron_spi_fram_params *params;
};

static inline struct ramtron_spi_fram *to_ramtron_spi_fram(struct spi_flash
							     *flash)
{
	return container_of(flash, struct ramtron_spi_fram, flash);
}

/*
 * table describing supported FRAM chips:
 * chips without RDID command must have the values 0xff for id1 and id2
 */
static const struct ramtron_spi_fram_params ramtron_spi_fram_table[] = {
	{
		.size = 32*1024,
		.addr_len = 2,
		.merge_cmd = 0,
		.id1 = 0x22,
		.id2 = 0x00,
		.speed = 40000000,
		.name = "FM25V02",
	},
	{
		.size = 32*1024,
		.addr_len = 2,
		.merge_cmd = 0,
		.id1 = 0x22,
		.id2 = 0x01,
		.speed = 40000000,
		.name = "FM25VN02",
	},
	{
		.size = 64*1024,
		.addr_len = 2,
		.merge_cmd = 0,
		.id1 = 0x23,
		.id2 = 0x00,
		.speed = 40000000,
		.name = "FM25V05",
	},
	{
		.size = 64*1024,
		.addr_len = 2,
		.merge_cmd = 0,
		.id1 = 0x23,
		.id2 = 0x01,
		.speed = 40000000,
		.name = "FM25VN05",
	},
	{
		.size = 128*1024,
		.addr_len = 3,
		.merge_cmd = 0,
		.id1 = 0x24,
		.id2 = 0x00,
		.speed = 40000000,
		.name = "FM25V10",
	},
	{
		.size = 128*1024,
		.addr_len = 3,
		.merge_cmd = 0,
		.id1 = 0x24,
		.id2 = 0x01,
		.speed = 40000000,
		.name = "FM25VN10",
	},
#ifdef CONFIG_SPI_FRAM_RAMTRON_NON_JEDEC
	{
		.size = 256*1024,
		.addr_len = 3,
		.merge_cmd = 0,
		.id1 = 0xff,
		.id2 = 0xff,
		.speed = 40000000,
		.name = "FM25H20",
	},
#endif
};

static int ramtron_common(struct spi_flash *flash,
		u32 offset, size_t len, void *buf, u8 command)
{
	struct ramtron_spi_fram *sn = to_ramtron_spi_fram(flash);
	u8 cmd[4];
	int cmd_len;
	int ret;

	if (sn->params->addr_len == 3 && sn->params->merge_cmd == 0) {
		cmd[0] = command;
		cmd[1] = offset >> 16;
		cmd[2] = offset >> 8;
		cmd[3] = offset;
		cmd_len = 4;
	} else if (sn->params->addr_len == 2 && sn->params->merge_cmd == 0) {
		cmd[0] = command;
		cmd[1] = offset >> 8;
		cmd[2] = offset;
		cmd_len = 3;
	} else {
		printf("SF: unsupported addr_len or merge_cmd\n");
		return -1;
	}

	/* claim the bus */
	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	if (command == CMD_PAGE_PROGRAM) {
		/* send WREN */
		ret = spi_flash_cmd_write_enable(flash);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			goto releasebus;
		}
	}

	/* do the transaction */
	if (command == CMD_PAGE_PROGRAM)
		ret = spi_flash_cmd_write(flash->spi, cmd, cmd_len, buf, len);
	else
		ret = spi_flash_cmd_read(flash->spi, cmd, cmd_len, buf, len);
	if (ret < 0)
		debug("SF: Transaction failed\n");

releasebus:
	/* release the bus */
	spi_release_bus(flash->spi);
	return ret;
}

static int ramtron_read(struct spi_flash *flash,
		u32 offset, size_t len, void *buf)
{
	return ramtron_common(flash, offset, len, buf,
		CMD_READ_ARRAY_SLOW);
}

static int ramtron_write(struct spi_flash *flash,
		u32 offset, size_t len, const void *buf)
{
	return ramtron_common(flash, offset, len, (void *)buf,
		CMD_PAGE_PROGRAM);
}

static int ramtron_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	debug("SF: Erase of RAMTRON FRAMs is pointless\n");
	return -1;
}

/*
 * nore: we are called here with idcode pointing to the first non-0x7f byte
 * already!
 */
static struct spi_flash *spi_fram_probe_ramtron(struct spi_slave *spi,
		u8 *idcode)
{
	const struct ramtron_spi_fram_params *params;
	struct ramtron_spi_fram *sn;
	unsigned int i;
#ifdef CONFIG_SPI_FRAM_RAMTRON_NON_JEDEC
	int ret;
	u8 sr;
#endif

	/* NOTE: the bus has been claimed before this function is called! */
	switch (idcode[0]) {
	case 0xc2:
		/* JEDEC conformant RAMTRON id */
		for (i = 0; i < ARRAY_SIZE(ramtron_spi_fram_table); i++) {
			params = &ramtron_spi_fram_table[i];
			if (idcode[1] == params->id1 &&
			    idcode[2] == params->id2)
				goto found;
		}
		break;
#ifdef CONFIG_SPI_FRAM_RAMTRON_NON_JEDEC
	case 0xff:
		/*
		 * probably open MISO line, pulled up.
		 * We COULD have a non JEDEC conformant FRAM here,
		 * read the status register to verify
		 */
		ret = spi_flash_cmd(spi, CMD_READ_STATUS, &sr, 1);
		if (ret)
			return NULL;

		/* Bits 5,4,0 are fixed 0 for all devices */
		if ((sr & 0x31) != 0x00)
			return NULL;
		/* now find the device */
		for (i = 0; i < ARRAY_SIZE(ramtron_spi_fram_table); i++) {
			params = &ramtron_spi_fram_table[i];
			if (!strcmp(params->name,
				    CONFIG_SPI_FRAM_RAMTRON_NON_JEDEC))
				goto found;
		}
		debug("SF: Unsupported non-JEDEC RAMTRON device "
			CONFIG_SPI_FRAM_RAMTRON_NON_JEDEC "\n");
		break;
#endif
	default:
		break;
	}

	/* arriving here means no method has found a device we can handle */
	debug("SF/ramtron: unsupported device id0=%02x id1=%02x id2=%02x\n",
	      idcode[0], idcode[1], idcode[2]);
	return NULL;

found:
	sn = malloc(sizeof(*sn));
	if (!sn) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	sn->params = params;

	sn->flash.write = ramtron_write;
	sn->flash.read = ramtron_read;
	sn->flash.erase = ramtron_erase;
	sn->flash.size = params->size;

	return &sn->flash;
}

/*
 * The following table holds all device probe functions
 * (All flashes are removed and implemented a common probe at
 *  spi_flash_probe.c)
 *
 * shift:  number of continuation bytes before the ID
 * idcode: the expected IDCODE or 0xff for non JEDEC devices
 * probe:  the function to call
 *
 * Non JEDEC devices should be ordered in the table such that
 * the probe functions with best detection algorithms come first.
 *
 * Several matching entries are permitted, they will be tried
 * in sequence until a probe function returns non NULL.
 *
 * IDCODE_CONT_LEN may be redefined if a device needs to declare a
 * larger "shift" value.  IDCODE_PART_LEN generally shouldn't be
 * changed.  This is the max number of bytes probe functions may
 * examine when looking up part-specific identification info.
 *
 * Probe functions will be given the idcode buffer starting at their
 * manu id byte (the "idcode" in the table below).  In other words,
 * all of the continuation bytes will be skipped (the "shift" below).
 */
#define IDCODE_CONT_LEN 0
#define IDCODE_PART_LEN 5
static const struct {
	const u8 shift;
	const u8 idcode;
	struct spi_flash *(*probe) (struct spi_slave *spi, u8 *idcode);
} flashes[] = {
	/* Keep it sorted by define name */
#ifdef CONFIG_SPI_FRAM_RAMTRON
	{ 6, 0xc2, spi_fram_probe_ramtron, },
# undef IDCODE_CONT_LEN
# define IDCODE_CONT_LEN 6
#endif
#ifdef CONFIG_SPI_FRAM_RAMTRON_NON_JEDEC
	{ 0, 0xff, spi_fram_probe_ramtron, },
#endif
};
#define IDCODE_LEN (IDCODE_CONT_LEN + IDCODE_PART_LEN)

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_slave *spi;
	struct spi_flash *flash = NULL;
	int ret, i, shift;
	u8 idcode[IDCODE_LEN], *idp;

	spi = spi_setup_slave(bus, cs, max_hz, spi_mode);
	if (!spi) {
		printf("SF: Failed to set up slave\n");
		return NULL;
	}

	ret = spi_claim_bus(spi);
	if (ret) {
		debug("SF: Failed to claim SPI bus: %d\n", ret);
		goto err_claim_bus;
	}

	/* Read the ID codes */
	ret = spi_flash_cmd(spi, CMD_READ_ID, idcode, sizeof(idcode));
	if (ret)
		goto err_read_id;

#ifdef DEBUG
	printf("SF: Got idcodes\n");
	print_buffer(0, idcode, 1, sizeof(idcode), 0);
#endif

	/* count the number of continuation bytes */
	for (shift = 0, idp = idcode;
	     shift < IDCODE_CONT_LEN && *idp == 0x7f;
	     ++shift, ++idp)
		continue;

	/* search the table for matches in shift and id */
	for (i = 0; i < ARRAY_SIZE(flashes); ++i)
		if (flashes[i].shift == shift && flashes[i].idcode == *idp) {
			/* we have a match, call probe */
			flash = flashes[i].probe(spi, idp);
			if (flash)
				break;
		}

	if (!flash) {
		printf("SF: Unsupported manufacturer %02x\n", *idp);
		goto err_manufacturer_probe;
	}

	printf("SF: Detected %s with total size ", flash->name);
	print_size(flash->size, "");
	puts("\n");

	spi_release_bus(spi);

	return flash;

err_manufacturer_probe:
err_read_id:
	spi_release_bus(spi);
err_claim_bus:
	spi_free_slave(spi);
	return NULL;
}

void spi_flash_free(struct spi_flash *flash)
{
	spi_free_slave(flash->spi);
	free(flash);
}
