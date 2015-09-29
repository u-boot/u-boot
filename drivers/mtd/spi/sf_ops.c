/*
 * SPI flash operations
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2010 Reinhard Meyer, EMK Elektronik
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <watchdog.h>

#include "sf_internal.h"

static void spi_flash_addr(u32 addr, u8 *cmd, u8 four_byte)
{
	/* cmd[0] is actual command */
	if (four_byte) {
		cmd[1] = addr >> 24;
		cmd[2] = addr >> 16;
		cmd[3] = addr >> 8;
		cmd[4] = addr >> 0;
	} else {
		cmd[1] = addr >> 16;
		cmd[2] = addr >> 8;
		cmd[3] = addr >> 0;
	}
}

int spi_flash_cmd_read_status(struct spi_flash *flash, u8 *rs)
{
	int ret;
	u8 cmd;

	cmd = CMD_READ_STATUS;
	ret = spi_flash_read_common(flash, &cmd, 1, rs, 1);
	if (ret < 0) {
		debug("SF: fail to read status register\n");
		return ret;
	}

	return 0;
}

int spi_flash_cmd_write_status(struct spi_flash *flash, u8 ws)
{
	u8 cmd;
	int ret;

	cmd = CMD_WRITE_STATUS;
	ret = spi_flash_write_common(flash, &cmd, 1, &ws, 1);
	if (ret < 0) {
		debug("SF: fail to write status register\n");
		return ret;
	}

	return 0;
}

#if defined(CONFIG_SPI_FLASH_SST)
int spi_flash_cmd_bp_unlock(struct spi_flash *flash)
{
	u8 cmd = CMD_BLOCK_PROTECT_UNLOCK;
	int ret;

	ret = spi_flash_read_common(flash, &cmd, 1, NULL, 0);
	if (ret < 0) {
		debug("SF: fail to unlock block protect\n");
		return ret;
	}

	return 0;
}
#endif

#if defined(CONFIG_SPI_FLASH_SPANSION) || defined(CONFIG_SPI_FLASH_WINBOND)
int spi_flash_cmd_read_config(struct spi_flash *flash, u8 *rc)
{
	int ret;
	u8 cmd;

	cmd = CMD_READ_CONFIG;
	ret = spi_flash_read_common(flash, &cmd, 1, rc, 1);
	if (ret < 0) {
		debug("SF: fail to read config register\n");
		return ret;
	}

	return 0;
}

int spi_flash_cmd_write_config(struct spi_flash *flash, u8 wc)
{
	u8 data[2];
#ifdef CONFIG_SPI_GENERIC
	u8 dataup[2];
#endif
	u8 cmd;
	int ret;

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH)
		flash->spi->flags |= SPI_XFER_LOWER;
#endif
	ret = spi_flash_cmd_read_status(flash, &data[0]);
	if (ret < 0)
		return ret;

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH) {
		flash->spi->flags |= SPI_XFER_UPPER;
		ret = spi_flash_cmd_read_status(flash, &dataup[0]);
		if (ret < 0)
			return ret;
	}
#endif

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH)
		flash->spi->flags |= SPI_XFER_LOWER;
#endif

	cmd = CMD_WRITE_STATUS;
	data[1] = wc;
	ret = spi_flash_write_common(flash, &cmd, 1, &data, 2);
	if (ret) {
		debug("SF: fail to write config register\n");
		return ret;
	}

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH) {
		flash->spi->flags |= SPI_XFER_UPPER;
		dataup[1] = wc;
		ret = spi_flash_write_common(flash, &cmd, 1, &dataup, 2);
		if (ret) {
			debug("SF: fail to write config register\n");
			return ret;
		}
	}
#endif

	return 0;
}
#endif

int spi_flash_cmd_4B_addr_switch(struct spi_flash *flash,
				int enable, u8 idcode0)
{
	int ret;
	u8 cmd, bar;
	bool need_wren = false;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: unable to claim SPI bus\n");
		return ret;
	}

	switch (idcode0) {
	case SPI_FLASH_CFI_MFR_STMICRO:
		/* Some Micron need WREN command; all will accept it */
		need_wren = true;
	case SPI_FLASH_CFI_MFR_MACRONIX:
	case SPI_FLASH_CFI_MFR_WINBOND:
		if (need_wren)
			spi_flash_cmd_write_enable(flash);

		cmd = enable ? CMD_ENTER_4B_ADDR : CMD_EXIT_4B_ADDR;
		ret = spi_flash_cmd(flash->spi, cmd, NULL, 0);
		if (need_wren)
			spi_flash_cmd_write_disable(flash);

		break;
	default:
		/* Spansion style */
		bar = enable << 7;
		cmd = CMD_BANKADDR_BRWR;
		ret = spi_flash_cmd_write(flash->spi, &cmd, 1, &bar, 1);
	}

	spi_release_bus(flash->spi);

	return ret;
}

#ifdef CONFIG_SPI_FLASH_BAR
static int spi_flash_cmd_bankaddr_write(struct spi_flash *flash, u8 bank_sel)
{
	u8 cmd;
	int ret;
	u8 upage_curr;

	upage_curr = flash->spi->flags & SPI_XFER_U_PAGE;

	if (flash->dual_flash != SF_DUAL_STACKED_FLASH) {
		if (flash->bank_curr == bank_sel) {
			debug("SF: not require to enable bank%d\n", bank_sel);
			return 0;
		}
	} else if (flash->upage_prev == upage_curr) {
		if (flash->bank_curr == bank_sel) {
			debug("SF: not require to enable bank%d\n", bank_sel);
			return 0;
		}
	} else {
		flash->upage_prev = upage_curr;
	}

	cmd = flash->bank_write_cmd;
	ret = spi_flash_write_common(flash, &cmd, 1, &bank_sel, 1);
	if (ret < 0) {
		debug("SF: fail to write bank register\n");
		return ret;
	}
	flash->bank_curr = bank_sel;

	return 0;
}

static int spi_flash_bank(struct spi_flash *flash, u32 offset)
{
	u8 bank_sel;
	int ret;

	bank_sel = offset / (SPI_FLASH_16MB_BOUN << flash->shift);

	ret = spi_flash_cmd_bankaddr_write(flash, bank_sel);
	if (ret) {
		debug("SF: fail to set bank%d\n", bank_sel);
		return ret;
	}

	return bank_sel;
}
#endif

#ifdef CONFIG_SF_DUAL_FLASH
static void spi_flash_dual_flash(struct spi_flash *flash, u32 *addr)
{
	switch (flash->dual_flash) {
	case SF_DUAL_STACKED_FLASH:
		if (*addr >= (flash->size >> 1)) {
			*addr -= flash->size >> 1;
			flash->spi->flags |= SPI_XFER_U_PAGE;
		} else {
			flash->spi->flags &= ~SPI_XFER_U_PAGE;
		}
		break;
	case SF_DUAL_PARALLEL_FLASH:
		*addr >>= flash->shift;
		break;
	default:
		debug("SF: Unsupported dual_flash=%d\n", flash->dual_flash);
		break;
	}
}
#endif

static int spi_flash_poll_status(struct spi_flash *flash, unsigned long timeout,
				 u8 cmd, u8 poll_bit)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timebase;
	unsigned long flags = 0;
	int ret;
	u8 status;
	u8 check_status = 0x0;
#ifdef CONFIG_SPI_GENERIC
	u8 status_up;
#endif

	if (cmd == CMD_FLAG_STATUS)
		check_status = poll_bit;

#ifdef CONFIG_SF_DUAL_FLASH
	if (spi->flags & SPI_XFER_U_PAGE)
		flags |= SPI_XFER_U_PAGE;
#endif
	ret = spi_xfer(spi, 8, &cmd, NULL, flags | SPI_XFER_BEGIN);
	if (ret) {
		debug("SF: fail to read %s status register\n",
		      cmd == CMD_READ_STATUS ? "read" : "flag");
		return ret;
	}

	timebase = get_timer(0);
	do {
		WATCHDOG_RESET();

#ifdef CONFIG_SPI_GENERIC
		if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH) {
			ret = spi_xfer(spi, 8, NULL, &status,
				       flags | SPI_XFER_LOWER);
			if (ret)
				return -1;
			ret = spi_xfer(spi, 8, NULL, &status_up,
				       flags | SPI_XFER_UPPER);
			if (ret)
				return -1;
			debug("Poll Status:0x%x, Status_up:0x%x\n", status,
			      status_up);

			if (((status & poll_bit) == check_status) &&
			    ((status_up & poll_bit) == check_status))
				break;
		} else {
#endif
			ret = spi_xfer(spi, 8, NULL, &status, flags);
			if (ret)
				return -1;

			debug("Poll Status:0x%x\n", status);
			if ((status & poll_bit) == check_status)
				break;
#ifdef CONFIG_SPI_GENERIC
		}
#endif
	} while (get_timer(timebase) < timeout);

	spi_xfer(spi, 0, NULL, NULL, flags | SPI_XFER_END);

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH) {
		if (((status & poll_bit) == check_status) &&
		    ((status_up & poll_bit) == check_status))
			return 0;
	} else {
#endif
	if ((status & poll_bit) == check_status)
		return 0;
#ifdef CONFIG_SPI_GENERIC
	}
#endif
	/* Timed out */
	debug("SF: time out!\n");
	return -1;
}

int spi_flash_cmd_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
	int ret;
	u8 poll_bit = STATUS_WIP;
	u8 cmd = CMD_READ_STATUS;

	ret = spi_flash_poll_status(flash, timeout, cmd, poll_bit);
	if (ret < 0)
		return ret;

	if (flash->poll_cmd == CMD_FLAG_STATUS) {
		poll_bit = STATUS_PEC;
		cmd = CMD_FLAG_STATUS;
		ret = spi_flash_poll_status(flash, timeout, cmd, poll_bit);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int spi_flash_write_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, const void *buf, size_t buf_len)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timeout = SPI_FLASH_PROG_TIMEOUT;
	int ret;

	if (buf == NULL)
		timeout = SPI_FLASH_PAGE_ERASE_TIMEOUT;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: unable to claim SPI bus\n");
		return ret;
	}

	ret = spi_flash_cmd_write_enable(flash);
	if (ret < 0) {
		debug("SF: enabling write failed\n");
		return ret;
	}

	ret = spi_flash_cmd_write(spi, cmd, cmd_len, buf, buf_len);
	if (ret < 0) {
		debug("SF: write cmd failed\n");
		return ret;
	}

	ret = spi_flash_cmd_wait_ready(flash, timeout);
	if (ret < 0) {
		debug("SF: write %s timed out\n",
		      timeout == SPI_FLASH_PROG_TIMEOUT ?
			"program" : "page erase");
		return ret;
	}

	spi_release_bus(spi);

	return ret;
}

int spi_flash_cmd_erase_ops(struct spi_flash *flash, u32 offset, size_t len)
{
	u32 erase_size, erase_addr, bank_addr;
	u8 cmd[SPI_FLASH_CMD_LEN + 1];
	u32 cmdlen;
	int ret = -1;

	erase_size = flash->erase_size;
	if (offset % erase_size || len % erase_size) {
		debug("SF: Erase offset/length not multiple of erase size\n");
		return -1;
	}

	cmd[0] = flash->erase_cmd;
	while (len) {
		erase_addr = offset;
		bank_addr = offset;

#ifdef CONFIG_SF_DUAL_FLASH
		if (flash->dual_flash > SF_SINGLE_FLASH)
			spi_flash_dual_flash(flash, &erase_addr);
		if (flash->dual_flash == SF_DUAL_STACKED_FLASH)
			bank_addr = erase_addr;
#endif

		if (flash->spi->bytemode != SPI_4BYTE_MODE) {
#ifdef CONFIG_SPI_FLASH_BAR
			ret = spi_flash_bank(flash, bank_addr);
			if (ret < 0)
				return ret;
#endif
			spi_flash_addr(erase_addr, cmd, 0);
			cmdlen = SPI_FLASH_CMD_LEN;
		} else {
			spi_flash_addr(erase_addr, cmd, 1);
			cmdlen = SPI_FLASH_CMD_LEN + 1;
		}

		debug("SF: erase %2x %2x %2x %2x (%x)\n", cmd[0], cmd[1],
		      cmd[2], cmd[3], erase_addr);

#ifdef CONFIG_SPI_GENERIC
		if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH)
			flash->spi->flags |= SPI_XFER_STRIPE;
#endif

		ret = spi_flash_write_common(flash, cmd, cmdlen, NULL, 0);
		if (ret < 0) {
			debug("SF: erase failed\n");
			break;
		}

		offset += erase_size;
		len -= erase_size;
	}

	return ret;
}

int spi_flash_cmd_write_ops(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
	unsigned long byte_addr, page_size;
	u32 write_addr, bank_addr;
	size_t chunk_len, actual;
	u8 cmd[SPI_FLASH_CMD_LEN + 1];
	int ret = -1;
	u32 cmdlen;

	page_size = flash->page_size;

	cmd[0] = flash->write_cmd;
	for (actual = 0; actual < len; actual += chunk_len) {
		write_addr = offset;
		bank_addr = offset;

#ifdef CONFIG_SF_DUAL_FLASH
		if (flash->dual_flash > SF_SINGLE_FLASH)
			spi_flash_dual_flash(flash, &write_addr);
		if (flash->dual_flash == SF_DUAL_STACKED_FLASH)
			bank_addr = write_addr;
#endif

		if (flash->spi->bytemode != SPI_4BYTE_MODE) {
#ifdef CONFIG_SPI_FLASH_BAR
			ret = spi_flash_bank(flash, bank_addr);
			if (ret < 0)
				return ret;
#endif
		}

		byte_addr = offset % page_size;
		chunk_len = min(len - actual, (size_t)(page_size - byte_addr));

		if (flash->spi->max_write_size)
			chunk_len = min(chunk_len,
					(size_t)flash->spi->max_write_size);

		if (flash->spi->bytemode == SPI_4BYTE_MODE) {
			spi_flash_addr(write_addr, cmd, 1);
			cmdlen = SPI_FLASH_CMD_LEN + 1;
		} else {
			spi_flash_addr(write_addr, cmd, 0);
			cmdlen = SPI_FLASH_CMD_LEN;
		}

		debug("SF: 0x%p => cmd = { 0x%02x 0x%02x%02x%02x } chunk_len = %zu\n",
		      buf + actual, cmd[0], cmd[1], cmd[2], cmd[3], chunk_len);

#ifdef CONFIG_SPI_GENERIC
		if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH)
			flash->spi->flags |= SPI_XFER_STRIPE;
#endif
		ret = spi_flash_write_common(flash, cmd, cmdlen,
					buf + actual, chunk_len);
		if (ret < 0) {
			debug("SF: write failed\n");
			break;
		}

		offset += chunk_len;
	}

	return ret;
}

int spi_flash_read_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	struct spi_slave *spi = flash->spi;
	int ret;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: unable to claim SPI bus\n");
		return ret;
	}

	ret = spi_flash_cmd_read(spi, cmd, cmd_len, data, data_len);
	if (ret < 0) {
		debug("SF: read cmd failed\n");
		return ret;
	}

	spi_release_bus(spi);

	return ret;
}

int spi_flash_cmd_read_ops(struct spi_flash *flash, u32 offset,
		size_t len, void *data)
{
	u8 *cmd, cmdsz;
	u32 remain_len, read_len, read_addr, bank_addr;
	int bank_sel = 0;
	int ret = -1;

	/* Handle memory-mapped SPI */
	if (flash->memory_map) {
		ret = spi_claim_bus(flash->spi);
		if (ret) {
			debug("SF: unable to claim SPI bus\n");
			return ret;
		}
		spi_xfer(flash->spi, 0, NULL, NULL, SPI_XFER_MMAP);
		memcpy(data, flash->memory_map + offset, len);
		spi_xfer(flash->spi, 0, NULL, NULL, SPI_XFER_MMAP_END);
		spi_release_bus(flash->spi);
		return 0;
	}

	cmdsz = SPI_FLASH_CMD_LEN + flash->dummy_byte;

	if (flash->spi->bytemode == SPI_4BYTE_MODE)
		cmdsz += 1;

	cmd = calloc(1, cmdsz);
	if (!cmd) {
		debug("SF: Failed to allocate cmd\n");
		return -ENOMEM;
	}

	cmd[0] = flash->read_cmd;
	while (len) {
		read_addr = offset;
		bank_addr = offset;

#ifdef CONFIG_SF_DUAL_FLASH
		if (flash->dual_flash > SF_SINGLE_FLASH)
			spi_flash_dual_flash(flash, &read_addr);
		if (flash->dual_flash == SF_DUAL_STACKED_FLASH)
			bank_addr = read_addr;
#endif
		if (flash->spi->bytemode != SPI_4BYTE_MODE) {
#ifdef CONFIG_SPI_FLASH_BAR
			bank_sel = spi_flash_bank(flash, bank_addr);
			if (bank_sel < 0)
				return ret;
			if ((flash->dual_flash == SF_DUAL_STACKED_FLASH) &&
			    (flash->spi->flags & SPI_XFER_U_PAGE))
				bank_sel += (flash->size >> 1)/
					     SPI_FLASH_16MB_BOUN;
#endif
			remain_len = ((SPI_FLASH_16MB_BOUN << flash->shift) *
					(bank_sel + 1)) - offset;
			if (len < remain_len)
				read_len = len;
			else
				read_len = remain_len;
		} else {
			read_len = len;
		}

		if (flash->spi->bytemode == SPI_4BYTE_MODE)
			spi_flash_addr(read_addr, cmd, 1);
		else
			spi_flash_addr(read_addr, cmd, 0);

		debug("%s: Byte Mode:0x%x\n",__func__,  flash->spi->bytemode);
#ifdef CONFIG_SPI_GENERIC
		if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH)
			flash->spi->flags |= SPI_XFER_STRIPE;
#endif

		ret = spi_flash_read_common(flash, cmd, cmdsz, data, read_len);
		if (ret < 0) {
			debug("SF: read failed\n");
			break;
		}

		offset += read_len;
		len -= read_len;
		data += read_len;
	}

	free(cmd);
	return ret;
}

#ifdef CONFIG_SPI_FLASH_SST
static int sst_byte_write(struct spi_flash *flash, u32 offset, const void *buf)
{
	int ret;
	u8 cmd[4] = {
		CMD_SST_BP,
		offset >> 16,
		offset >> 8,
		offset,
	};

	debug("BP[%02x]: 0x%p => cmd = { 0x%02x 0x%06x }\n",
	      spi_w8r8(flash->spi, CMD_READ_STATUS), buf, cmd[0], offset);

	ret = spi_flash_cmd_write_enable(flash);
	if (ret)
		return ret;

	ret = spi_flash_cmd_write(flash->spi, cmd, sizeof(cmd), buf, 1);
	if (ret)
		return ret;

	return spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
}

int sst_write_wp(struct spi_flash *flash, u32 offset, size_t len,
		const void *buf)
{
	size_t actual, cmd_len;
	int ret;
	u8 cmd[4];

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	/* If the data is not word aligned, write out leading single byte */
	actual = offset % 2;
	if (actual) {
		ret = sst_byte_write(flash, offset, buf);
		if (ret)
			goto done;
	}
	offset += actual;

	ret = spi_flash_cmd_write_enable(flash);
	if (ret)
		goto done;

	cmd_len = 4;
	cmd[0] = CMD_SST_AAI_WP;
	cmd[1] = offset >> 16;
	cmd[2] = offset >> 8;
	cmd[3] = offset;

	for (; actual < len - 1; actual += 2) {
		debug("WP[%02x]: 0x%p => cmd = { 0x%02x 0x%06x }\n",
		      spi_w8r8(flash->spi, CMD_READ_STATUS), buf + actual,
		      cmd[0], offset);

		ret = spi_flash_cmd_write(flash->spi, cmd, cmd_len,
					buf + actual, 2);
		if (ret) {
			debug("SF: sst word program failed\n");
			break;
		}

		ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret)
			break;

		cmd_len = 1;
		offset += 2;
	}

	if (!ret)
		ret = spi_flash_cmd_write_disable(flash);

	/* If there is a single trailing byte, write it out */
	if (!ret && actual != len)
		ret = sst_byte_write(flash, offset, buf + actual);

 done:
	debug("SF: sst: program %s %zu bytes @ 0x%zx\n",
	      ret ? "failure" : "success", len, offset - actual);

	spi_release_bus(flash->spi);
	return ret;
}

int sst_write_bp(struct spi_flash *flash, u32 offset, size_t len,
		const void *buf)
{
	size_t actual;
	int ret;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	for (actual = 0; actual < len; actual++) {
		ret = sst_byte_write(flash, offset, buf + actual);
		if (ret) {
			debug("SF: sst byte program failed\n");
			break;
		}
		offset++;
	}

	if (!ret)
		ret = spi_flash_cmd_write_disable(flash);

	debug("SF: sst: program %s %zu bytes @ 0x%zx\n",
	      ret ? "failure" : "success", len, offset - actual);

	spi_release_bus(flash->spi);
	return ret;
}
#endif
