/*
 * Simulate a SPI flash
 *
 * Copyright (c) 2011-2013 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <os.h>

#include <spi_flash.h>
#include "sf_internal.h"

#include <asm/getopt.h>
#include <asm/spi.h>
#include <asm/state.h>

/*
 * The different states that our SPI flash transitions between.
 * We need to keep track of this across multiple xfer calls since
 * the SPI bus could possibly call down into us multiple times.
 */
enum sandbox_sf_state {
	SF_CMD,   /* default state -- we're awaiting a command */
	SF_ID,    /* read the flash's (jedec) ID code */
	SF_ADDR,  /* processing the offset in the flash to read/etc... */
	SF_READ,  /* reading data from the flash */
	SF_WRITE, /* writing data to the flash, i.e. page programming */
	SF_ERASE, /* erase the flash */
	SF_READ_STATUS, /* read the flash's status register */
	SF_READ_STATUS1, /* read the flash's status register upper 8 bits*/
};

static const char *sandbox_sf_state_name(enum sandbox_sf_state state)
{
	static const char * const states[] = {
		"CMD", "ID", "ADDR", "READ", "WRITE", "ERASE", "READ_STATUS",
	};
	return states[state];
}

/* Bits for the status register */
#define STAT_WIP	(1 << 0)
#define STAT_WEL	(1 << 1)

/* Assume all SPI flashes have 3 byte addresses since they do atm */
#define SF_ADDR_LEN	3

#define IDCODE_LEN 3

/* Used to quickly bulk erase backing store */
static u8 sandbox_sf_0xff[0x1000];

/* Internal state data for each SPI flash */
struct sandbox_spi_flash {
	/*
	 * As we receive data over the SPI bus, our flash transitions
	 * between states.  For example, we start off in the SF_CMD
	 * state where the first byte tells us what operation to perform
	 * (such as read or write the flash).  But the operation itself
	 * can go through a few states such as first reading in the
	 * offset in the flash to perform the requested operation.
	 * Thus "state" stores the exact state that our machine is in
	 * while "cmd" stores the overall command we're processing.
	 */
	enum sandbox_sf_state state;
	uint cmd;
	/* Erase size of current erase command */
	uint erase_size;
	/* Current position in the flash; used when reading/writing/etc... */
	uint off;
	/* How many address bytes we've consumed */
	uint addr_bytes, pad_addr_bytes;
	/* The current flash status (see STAT_XXX defines above) */
	u16 status;
	/* Data describing the flash we're emulating */
	const struct spi_flash_params *data;
	/* The file on disk to serv up data from */
	int fd;
};

static int sandbox_sf_setup(void **priv, const char *spec)
{
	/* spec = idcode:file */
	struct sandbox_spi_flash *sbsf;
	const char *file;
	size_t len, idname_len;
	const struct spi_flash_params *data;

	file = strchr(spec, ':');
	if (!file) {
		printf("sandbox_sf: unable to parse file\n");
		goto error;
	}
	idname_len = file - spec;
	++file;

	for (data = spi_flash_params_table; data->name; data++) {
		len = strlen(data->name);
		if (idname_len != len)
			continue;
		if (!memcmp(spec, data->name, len))
			break;
	}
	if (!data->name) {
		printf("sandbox_sf: unknown flash '%*s'\n", (int)idname_len,
		       spec);
		goto error;
	}

	if (sandbox_sf_0xff[0] == 0x00)
		memset(sandbox_sf_0xff, 0xff, sizeof(sandbox_sf_0xff));

	sbsf = calloc(sizeof(*sbsf), 1);
	if (!sbsf) {
		printf("sandbox_sf: out of memory\n");
		goto error;
	}

	sbsf->fd = os_open(file, 02);
	if (sbsf->fd == -1) {
		free(sbsf);
		printf("sandbox_sf: unable to open file '%s'\n", file);
		goto error;
	}

	sbsf->data = data;

	*priv = sbsf;
	return 0;

 error:
	return 1;
}

static void sandbox_sf_free(void *priv)
{
	struct sandbox_spi_flash *sbsf = priv;

	os_close(sbsf->fd);
	free(sbsf);
}

static void sandbox_sf_cs_activate(void *priv)
{
	struct sandbox_spi_flash *sbsf = priv;

	debug("sandbox_sf: CS activated; state is fresh!\n");

	/* CS is asserted, so reset state */
	sbsf->off = 0;
	sbsf->addr_bytes = 0;
	sbsf->pad_addr_bytes = 0;
	sbsf->state = SF_CMD;
	sbsf->cmd = SF_CMD;
}

static void sandbox_sf_cs_deactivate(void *priv)
{
	debug("sandbox_sf: CS deactivated; cmd done processing!\n");
}

/* Figure out what command this stream is telling us to do */
static int sandbox_sf_process_cmd(struct sandbox_spi_flash *sbsf, const u8 *rx,
				  u8 *tx)
{
	enum sandbox_sf_state oldstate = sbsf->state;

	/* We need to output a byte for the cmd byte we just ate */
	sandbox_spi_tristate(tx, 1);

	sbsf->cmd = rx[0];
	switch (sbsf->cmd) {
	case CMD_READ_ID:
		sbsf->state = SF_ID;
		sbsf->cmd = SF_ID;
		break;
	case CMD_READ_ARRAY_FAST:
		sbsf->pad_addr_bytes = 1;
	case CMD_READ_ARRAY_SLOW:
	case CMD_PAGE_PROGRAM:
		sbsf->state = SF_ADDR;
		break;
	case CMD_WRITE_DISABLE:
		debug(" write disabled\n");
		sbsf->status &= ~STAT_WEL;
		break;
	case CMD_READ_STATUS:
		sbsf->state = SF_READ_STATUS;
		break;
	case CMD_READ_STATUS1:
		sbsf->state = SF_READ_STATUS1;
		break;
	case CMD_WRITE_ENABLE:
		debug(" write enabled\n");
		sbsf->status |= STAT_WEL;
		break;
	default: {
		int flags = sbsf->data->flags;

		/* we only support erase here */
		if (sbsf->cmd == CMD_ERASE_CHIP) {
			sbsf->erase_size = sbsf->data->sector_size *
				sbsf->data->nr_sectors;
		} else if (sbsf->cmd == CMD_ERASE_4K && (flags & SECT_4K)) {
			sbsf->erase_size = 4 << 10;
		} else if (sbsf->cmd == CMD_ERASE_32K && (flags & SECT_32K)) {
			sbsf->erase_size = 32 << 10;
		} else if (sbsf->cmd == CMD_ERASE_64K &&
			   !(flags & (SECT_4K | SECT_32K))) {
			sbsf->erase_size = 64 << 10;
		} else {
			debug(" cmd unknown: %#x\n", sbsf->cmd);
			return 1;
		}
		sbsf->state = SF_ADDR;
		break;
	}
	}

	if (oldstate != sbsf->state)
		debug(" cmd: transition to %s state\n",
		      sandbox_sf_state_name(sbsf->state));

	return 0;
}

int sandbox_erase_part(struct sandbox_spi_flash *sbsf, int size)
{
	int todo;
	int ret;

	while (size > 0) {
		todo = min(size, sizeof(sandbox_sf_0xff));
		ret = os_write(sbsf->fd, sandbox_sf_0xff, todo);
		if (ret != todo)
			return ret;
		size -= todo;
	}

	return 0;
}

static int sandbox_sf_xfer(void *priv, const u8 *rx, u8 *tx,
		uint bytes)
{
	struct sandbox_spi_flash *sbsf = priv;
	uint cnt, pos = 0;
	int ret;

	debug("sandbox_sf: state:%x(%s) bytes:%u\n", sbsf->state,
	      sandbox_sf_state_name(sbsf->state), bytes);

	if (sbsf->state == SF_CMD) {
		/* Figure out the initial state */
		if (sandbox_sf_process_cmd(sbsf, rx, tx))
			return 1;
		++pos;
	}

	/* Process the remaining data */
	while (pos < bytes) {
		switch (sbsf->state) {
		case SF_ID: {
			u8 id;

			debug(" id: off:%u tx:", sbsf->off);
			if (sbsf->off < IDCODE_LEN) {
				/* Extract correct byte from ID 0x00aabbcc */
				id = sbsf->data->jedec >>
					(8 * (IDCODE_LEN - 1 - sbsf->off));
			} else {
				id = 0;
			}
			debug("%d %02x\n", sbsf->off, id);
			tx[pos++] = id;
			++sbsf->off;
			break;
		}
		case SF_ADDR:
			debug(" addr: bytes:%u rx:%02x ", sbsf->addr_bytes,
			      rx[pos]);

			if (sbsf->addr_bytes++ < SF_ADDR_LEN)
				sbsf->off = (sbsf->off << 8) | rx[pos];
			debug("addr:%06x\n", sbsf->off);

			sandbox_spi_tristate(&tx[pos++], 1);

			/* See if we're done processing */
			if (sbsf->addr_bytes <
					SF_ADDR_LEN + sbsf->pad_addr_bytes)
				break;

			/* Next state! */
			if (os_lseek(sbsf->fd, sbsf->off, OS_SEEK_SET) < 0) {
				puts("sandbox_sf: os_lseek() failed");
				return 1;
			}
			switch (sbsf->cmd) {
			case CMD_READ_ARRAY_FAST:
			case CMD_READ_ARRAY_SLOW:
				sbsf->state = SF_READ;
				break;
			case CMD_PAGE_PROGRAM:
				sbsf->state = SF_WRITE;
				break;
			default:
				/* assume erase state ... */
				sbsf->state = SF_ERASE;
				goto case_sf_erase;
			}
			debug(" cmd: transition to %s state\n",
			      sandbox_sf_state_name(sbsf->state));
			break;
		case SF_READ:
			/*
			 * XXX: need to handle exotic behavior:
			 *      - reading past end of device
			 */

			cnt = bytes - pos;
			debug(" tx: read(%u)\n", cnt);
			ret = os_read(sbsf->fd, tx + pos, cnt);
			if (ret < 0) {
				puts("sandbox_spi: os_read() failed\n");
				return 1;
			}
			pos += ret;
			break;
		case SF_READ_STATUS:
			debug(" read status: %#x\n", sbsf->status);
			cnt = bytes - pos;
			memset(tx + pos, sbsf->status, cnt);
			pos += cnt;
			break;
		case SF_READ_STATUS1:
			debug(" read status: %#x\n", sbsf->status);
			cnt = bytes - pos;
			memset(tx + pos, sbsf->status >> 8, cnt);
			pos += cnt;
			break;
		case SF_WRITE:
			/*
			 * XXX: need to handle exotic behavior:
			 *      - unaligned addresses
			 *      - more than a page (256) worth of data
			 *      - reading past end of device
			 */
			if (!(sbsf->status & STAT_WEL)) {
				puts("sandbox_sf: write enable not set before write\n");
				goto done;
			}

			cnt = bytes - pos;
			debug(" rx: write(%u)\n", cnt);
			sandbox_spi_tristate(&tx[pos], cnt);
			ret = os_write(sbsf->fd, rx + pos, cnt);
			if (ret < 0) {
				puts("sandbox_spi: os_write() failed\n");
				return 1;
			}
			pos += ret;
			sbsf->status &= ~STAT_WEL;
			break;
		case SF_ERASE:
 case_sf_erase: {
			if (!(sbsf->status & STAT_WEL)) {
				puts("sandbox_sf: write enable not set before erase\n");
				goto done;
			}

			/* verify address is aligned */
			if (sbsf->off & (sbsf->erase_size - 1)) {
				debug(" sector erase: cmd:%#x needs align:%#x, but we got %#x\n",
				      sbsf->cmd, sbsf->erase_size,
				      sbsf->off);
				sbsf->status &= ~STAT_WEL;
				goto done;
			}

			debug(" sector erase addr: %u, size: %u\n", sbsf->off,
			      sbsf->erase_size);

			cnt = bytes - pos;
			sandbox_spi_tristate(&tx[pos], cnt);
			pos += cnt;

			/*
			 * TODO(vapier@gentoo.org): latch WIP in status, and
			 * delay before clearing it ?
			 */
			ret = sandbox_erase_part(sbsf, sbsf->erase_size);
			sbsf->status &= ~STAT_WEL;
			if (ret) {
				debug("sandbox_sf: Erase failed\n");
				goto done;
			}
			goto done;
		}
		default:
			debug(" ??? no idea what to do ???\n");
			goto done;
		}
	}

 done:
	return pos == bytes ? 0 : 1;
}

static const struct sandbox_spi_emu_ops sandbox_sf_ops = {
	.setup         = sandbox_sf_setup,
	.free          = sandbox_sf_free,
	.cs_activate   = sandbox_sf_cs_activate,
	.cs_deactivate = sandbox_sf_cs_deactivate,
	.xfer          = sandbox_sf_xfer,
};

static int sandbox_cmdline_cb_spi_sf(struct sandbox_state *state,
				     const char *arg)
{
	unsigned long bus, cs;
	const char *spec = sandbox_spi_parse_spec(arg, &bus, &cs);

	if (!spec)
		return 1;

	/*
	 * It is safe to not make a copy of 'spec' because it comes from the
	 * command line.
	 *
	 * TODO(sjg@chromium.org): It would be nice if we could parse the
	 * spec here, but the problem is that no U-Boot init has been done
	 * yet. Perhaps we can figure something out.
	 */
	state->spi[bus][cs].ops = &sandbox_sf_ops;
	state->spi[bus][cs].spec = spec;
	return 0;
}
SANDBOX_CMDLINE_OPT(spi_sf, 1, "connect a SPI flash: <bus>:<cs>:<id>:<file>");
