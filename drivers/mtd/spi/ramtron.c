/*
 * (C) Copyright 2010
 * Reinhard Meyer, EMK Elektronik, reinhard.meyer@emk-elektronik.de
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
#include "spi_flash_internal.h"

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
struct spi_flash *spi_fram_probe_ramtron(struct spi_slave *spi, u8 *idcode)
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
			if (idcode[1] == params->id1 && idcode[2] == params->id2)
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
			if (!strcmp(params->name, CONFIG_SPI_FRAM_RAMTRON_NON_JEDEC))
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
	sn->flash.spi = spi;
	sn->flash.name = params->name;

	sn->flash.write = ramtron_write;
	sn->flash.read = ramtron_read;
	sn->flash.erase = ramtron_erase;
	sn->flash.size = params->size;

	return &sn->flash;
}
