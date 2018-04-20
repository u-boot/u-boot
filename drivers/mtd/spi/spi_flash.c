/*
 * SPI Flash Core
 *
 * Copyright (C) 2015 Jagan Teki <jteki@openedev.com>
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2010 Reinhard Meyer, EMK Elektronik
 * Copyright (C) 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <mapmem.h>
#include <spi.h>
#include <spi_flash.h>
#include <linux/log2.h>

#include "sf_internal.h"

DECLARE_GLOBAL_DATA_PTR;

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

static int read_sr(struct spi_flash *flash, u8 *rs)
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

static int read_fsr(struct spi_flash *flash, u8 *fsr)
{
	int ret;
	const u8 cmd = CMD_FLAG_STATUS;

	ret = spi_flash_read_common(flash, &cmd, 1, fsr, 1);
	if (ret < 0) {
		debug("SF: fail to read flag status register\n");
		return ret;
	}

	return 0;
}

static int write_sr(struct spi_flash *flash, u8 ws)
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

#if defined(CONFIG_SPI_FLASH_SPANSION) || defined(CONFIG_SPI_FLASH_WINBOND)
static int read_cr(struct spi_flash *flash, u8 *rc)
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

static int write_cr(struct spi_flash *flash, u8 wc)
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
	ret = read_sr(flash, &data[0]);
	if (ret < 0)
		return ret;

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH) {
		flash->spi->flags |= SPI_XFER_UPPER;
		ret = read_sr(flash, &dataup[0]);
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

#ifdef CONFIG_SPI_FLASH_BAR
/*
 * This "clean_bar" is necessary in a situation when one was accessing
 * spi flash memory > 16 MiB by using Bank Address Register's BA24 bit.
 *
 * After it the BA24 bit shall be cleared to allow access to correct
 * memory region after SW reset (by calling "reset" command).
 *
 * Otherwise, the BA24 bit may be left set and then after reset, the
 * ROM would read/write/erase SPL from 16 MiB * bank_sel address.
 */
static int clean_bar(struct spi_flash *flash)
{
	u8 cmd, bank_sel = 0;
	int ret;

	if (flash->bank_curr == 0)
		return 0;
	cmd = flash->bank_write_cmd;

	ret = spi_flash_write_common(flash, &cmd, 1, &bank_sel, 1);
	if (ret) {
		debug("SF: fail to write bank register\n");
		return ret;
	}

	flash->bank_curr = bank_sel;

	return ret;
}

static int write_bar(struct spi_flash *flash, u32 offset)
{
	u8 cmd, bank_sel, upage_curr;
	int ret;

	bank_sel = offset / (SPI_FLASH_16MB_BOUN << flash->shift);

	upage_curr = flash->spi->flags & SPI_XFER_U_PAGE;

	if (flash->dual_flash != SF_DUAL_STACKED_FLASH) {
		if (flash->bank_curr == bank_sel) {
			debug("SF: not require to enable bank%d\n", bank_sel);
			goto bar_end;
		}
	} else if (flash->upage_prev == upage_curr) {
		if (flash->bank_curr == bank_sel) {
			debug("SF: not require to enable bank%d\n", bank_sel);
			goto bar_end;
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

bar_end:
	flash->bank_curr = bank_sel;
	return flash->bank_curr;
}

static int read_bar(struct spi_flash *flash, const struct spi_flash_info *info)
{
	u8 curr_bank = 0;
#ifdef CONFIG_SPI_GENERIC
	u8 curr_bank_up = 0;
#endif
	int ret;

	if (flash->size <= SPI_FLASH_16MB_BOUN)
		goto bar_end;

	switch (JEDEC_MFR(info)) {
	case SPI_FLASH_CFI_MFR_SPANSION:
		flash->bank_read_cmd = CMD_BANKADDR_BRRD;
		flash->bank_write_cmd = CMD_BANKADDR_BRWR;
		break;
	default:
		flash->bank_read_cmd = CMD_EXTNADDR_RDEAR;
		flash->bank_write_cmd = CMD_EXTNADDR_WREAR;
	}

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH) {
		flash->spi->flags |= SPI_XFER_LOWER;
		ret = spi_flash_read_common(flash, &flash->bank_read_cmd, 1,
					    &curr_bank, 1);
		if (ret)
			return ret;

		flash->spi->flags |= SPI_XFER_UPPER;
		ret = spi_flash_read_common(flash, &flash->bank_read_cmd, 1,
					    &curr_bank_up, 1);
		if (ret)
			return ret;
		if (curr_bank != curr_bank_up) {
			printf("Incorrect Bank selections Dual parallel\n");
			return -EINVAL;
		}
	} else {
#endif
		ret = spi_flash_read_common(flash, &flash->bank_read_cmd, 1,
					    &curr_bank, 1);
		if (ret) {
			debug("SF: fail to read bank addr register\n");
			return ret;
		}
#ifdef CONFIG_SPI_GENERIC
	}
#endif

bar_end:
	flash->bank_curr = curr_bank;
	return 0;
}
#endif

#ifdef CONFIG_SF_DUAL_FLASH
static void spi_flash_dual(struct spi_flash *flash, u32 *addr)
{
	switch (flash->dual_flash) {
	case SF_DUAL_STACKED_FLASH:
		if (*addr >= (flash->size >> 1)) {
			*addr -= flash->size >> 1;
			flash->flags |= SNOR_F_USE_UPAGE;
			flash->spi->flags |= SPI_XFER_U_PAGE;
		} else {
			flash->flags &= ~SNOR_F_USE_UPAGE;
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

static int spi_flash_sr_ready(struct spi_flash *flash)
{
	u8 sr;
	int ret;
#ifdef CONFIG_SPI_GENERIC
	u8 sr_up;
#endif

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH) {
		flash->spi->flags |= SPI_XFER_LOWER;
		ret = read_sr(flash, &sr);
		if (ret < 0)
			return ret;

		flash->spi->flags |= SPI_XFER_UPPER;
		ret = read_sr(flash, &sr_up);
		if (ret < 0)
			return ret;

		sr = (!(sr & STATUS_WIP) && !(sr_up & STATUS_WIP));
		return sr;
	} else {
#endif
		ret = read_sr(flash, &sr);
		if (ret < 0)
			return ret;
		return !(sr & STATUS_WIP);
#ifdef CONFIG_SPI_GENERIC
	}
#endif
}

static int spi_flash_fsr_ready(struct spi_flash *flash)
{
	u8 fsr;
	int ret;
#ifdef CONFIG_SPI_GENERIC
	u8 fsr_up;
#endif

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH) {
		flash->spi->flags |= SPI_XFER_LOWER;
		ret = read_fsr(flash, &fsr);
		if (ret < 0)
			return ret;

		flash->spi->flags |= SPI_XFER_UPPER;
		ret = read_fsr(flash, &fsr_up);
		if (ret < 0)
			return ret;

		fsr = ((fsr & STATUS_PEC) && (fsr_up & STATUS_PEC));
		return fsr;
	} else {
#endif
	ret = read_fsr(flash, &fsr);
	if (ret < 0)
		return ret;
	return fsr & STATUS_PEC;
#ifdef CONFIG_SPI_GENERIC
	}
#endif
}

static int spi_flash_ready(struct spi_flash *flash)
{
	int sr, fsr;

	sr = spi_flash_sr_ready(flash);
	if (sr < 0)
		return sr;

	fsr = 1;
	if (flash->flags & SNOR_F_USE_FSR) {
		fsr = spi_flash_fsr_ready(flash);
		if (fsr < 0)
			return fsr;
	}

	return sr && fsr;
}

static int spi_flash_wait_till_ready(struct spi_flash *flash,
				     unsigned long timeout)
{
	unsigned long timebase;
	int ret;

	timebase = get_timer(0);

	while (get_timer(timebase) < timeout) {
		ret = spi_flash_ready(flash);
		if (ret < 0)
			return ret;
		if (ret)
			return 0;
	}

	printf("SF: Timeout!\n");

	return -ETIMEDOUT;
}

int spi_flash_write_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, const void *buf, size_t buf_len)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timeout = SPI_FLASH_PROG_TIMEOUT;
	int ret;
#ifdef CONFIG_SPI_GENERIC
	u32 flags = 0;
#endif

	if (buf == NULL)
		timeout = SPI_FLASH_PAGE_ERASE_TIMEOUT;

	ret = spi_claim_bus(spi);
	if (ret) {
		debug("SF: unable to claim SPI bus\n");
		return ret;
	}

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH)
		flags = flash->spi->flags;
#endif
	ret = spi_flash_cmd_write_enable(flash);
	if (ret < 0) {
		debug("SF: enabling write failed\n");
		return ret;
	}

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH)
		flash->spi->flags = flags;
#endif
	ret = spi_flash_cmd_write(spi, cmd, cmd_len, buf, buf_len);
	if (ret < 0) {
		debug("SF: write cmd failed\n");
		return ret;
	}

	ret = spi_flash_wait_till_ready(flash, timeout);
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
	u32 erase_size, erase_addr;
	u8 cmd[SPI_FLASH_CMD_LEN + 1];
	int ret = -1;
	u32 cmdlen;
#if defined(CONFIG_SF_DUAL_FLASH) || defined(CONFIG_SPI_FLASH_BAR)
	u32 bank_addr;
#endif

	erase_size = flash->erase_size;
	if (offset % erase_size || len % erase_size) {
		debug("SF: Erase offset/length not multiple of erase size\n");
		return -1;
	}

	if (flash->flash_is_locked) {
		if (flash->flash_is_locked(flash, offset, len) > 0) {
			printf("offset 0x%x is protected and cannot be erased\n",
			       offset);
			return -EINVAL;
		}
	}

	cmd[0] = flash->erase_cmd;
	while (len) {
		erase_addr = offset;
#if defined(CONFIG_SF_DUAL_FLASH) || defined(CONFIG_SPI_FLASH_BAR)
		bank_addr = offset;
#endif

#ifdef CONFIG_SF_DUAL_FLASH
		if (flash->dual_flash > SF_SINGLE_FLASH)
			spi_flash_dual(flash, &erase_addr);
		if (flash->dual_flash == SF_DUAL_STACKED_FLASH)
			bank_addr = erase_addr;
#endif

		if (flash->spi->bytemode != SPI_4BYTE_MODE) {
#ifdef CONFIG_SPI_FLASH_BAR
			ret = write_bar(flash, bank_addr);
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

	if (flash->spi->bytemode != SPI_4BYTE_MODE) {
#ifdef CONFIG_SPI_FLASH_BAR
		ret = clean_bar(flash);
#endif
	}

	return ret;
}

int spi_flash_cmd_write_ops(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
	unsigned long byte_addr, page_size;
	u32 write_addr;
	size_t chunk_len, actual;
	u8 cmd[SPI_FLASH_CMD_LEN + 1];
	u32 cmdlen;
	int ret = -1;
#if defined(CONFIG_SF_DUAL_FLASH) || defined(CONFIG_SPI_FLASH_BAR)
	u32 bank_addr;
#endif

	page_size = flash->page_size;

	if (flash->flash_is_locked) {
		if (flash->flash_is_locked(flash, offset, len) > 0) {
			printf("offset 0x%x is protected and cannot be written\n",
			       offset);
			return -EINVAL;
		}
	}

	cmd[0] = flash->write_cmd;
	for (actual = 0; actual < len; actual += chunk_len) {
		write_addr = offset;
#if defined(CONFIG_SF_DUAL_FLASH) || defined(CONFIG_SPI_FLASH_BAR)
		bank_addr = offset;
#endif

#ifdef CONFIG_SF_DUAL_FLASH
		if (flash->dual_flash > SF_SINGLE_FLASH)
			spi_flash_dual(flash, &write_addr);
		if (flash->dual_flash == SF_DUAL_STACKED_FLASH)
			bank_addr = write_addr;
#endif

		if (flash->spi->bytemode != SPI_4BYTE_MODE) {
#ifdef CONFIG_SPI_FLASH_BAR
			ret = write_bar(flash, bank_addr);
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

	if (flash->spi->bytemode != SPI_4BYTE_MODE) {
#ifdef CONFIG_SPI_FLASH_BAR
		ret = clean_bar(flash);
#endif
	}

	return ret;
}

int spi_flash_read_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	struct spi_slave *spi = flash->spi;
	int ret;

	ret = spi_claim_bus(spi);
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

void __weak spi_flash_copy_mmap(void *data, void *offset, size_t len)
{
	memcpy(data, offset, len);
}

int spi_flash_cmd_read_ops(struct spi_flash *flash, u32 offset,
		size_t len, void *data)
{
	struct spi_slave *spi = flash->spi;
	u8 *cmd, cmdsz;
	u32 remain_len, read_len, read_addr;
	int bank_sel = 0;
	int ret = -1;
#if defined(CONFIG_SF_DUAL_FLASH) || defined(CONFIG_SPI_FLASH_BAR)
	u32 bank_addr;
#endif

#ifdef CONFIG_SF_DUAL_FLASH
	u8 moveoffs = 0;
	void *tempbuf = NULL;
	size_t length = len;
#endif

#ifdef CONFIG_SF_DUAL_FLASH
	/*
	 * Incase of dual parallel, if odd offset is received
	 * decrease it by 1 and read extra byte, otherwise
	 * any read with odd offset fails
	 */
	if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH) {
		if (offset & 1) {
			offset -= 1;
			len += 1;
			moveoffs = 1;
			tempbuf = data;
		}
	}
#endif

	/* Handle memory-mapped SPI */
	if (flash->memory_map) {
		ret = spi_claim_bus(spi);
		if (ret) {
			debug("SF: unable to claim SPI bus\n");
			return ret;
		}
		spi_xfer(spi, 0, NULL, NULL, SPI_XFER_MMAP);
		spi_flash_copy_mmap(data, flash->memory_map + offset, len);
		spi_xfer(spi, 0, NULL, NULL, SPI_XFER_MMAP_END);
		spi_release_bus(spi);
		return 0;
	}

	cmdsz = SPI_FLASH_CMD_LEN + flash->dummy_byte;

	spi->dummy_bytes = flash->dummy_byte;

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
#if defined(CONFIG_SF_DUAL_FLASH) || defined(CONFIG_SPI_FLASH_BAR)
		bank_addr = offset;
#endif

#ifdef CONFIG_SF_DUAL_FLASH
		if (flash->dual_flash > SF_SINGLE_FLASH)
			spi_flash_dual(flash, &read_addr);
		if (flash->dual_flash == SF_DUAL_STACKED_FLASH)
			bank_addr = read_addr;
#endif

		if (flash->spi->bytemode != SPI_4BYTE_MODE) {
#ifdef CONFIG_SPI_FLASH_BAR
			bank_sel = write_bar(flash, bank_addr);
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
			if (len > (SPI_FLASH_16MB_BOUN << flash->shift))
				read_len = SPI_FLASH_16MB_BOUN << flash->shift;
			else
				read_len = len;
		}

		if (flash->spi->bytemode == SPI_4BYTE_MODE)
			spi_flash_addr(read_addr, cmd, 1);
		else
			spi_flash_addr(read_addr, cmd, 0);

		debug("%s: Byte Mode:0x%x\n", __func__, flash->spi->bytemode);
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

#ifdef CONFIG_SF_DUAL_FLASH
	if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH) {
		if (moveoffs) {
			data = tempbuf + 1;
			memcpy(tempbuf, data, length);
		}
	}
#endif

	spi->dummy_bytes = 0;

	if (flash->spi->bytemode != SPI_4BYTE_MODE) {
#ifdef CONFIG_SPI_FLASH_BAR
		ret = clean_bar(flash);
#endif
	}

	free(cmd);
	return ret;
}

#ifdef CONFIG_SPI_FLASH_SST
static int sst_byte_write(struct spi_flash *flash, u32 offset, const void *buf)
{
	struct spi_slave *spi = flash->spi;
	int ret;
	u8 cmd[4] = {
		CMD_SST_BP,
		offset >> 16,
		offset >> 8,
		offset,
	};

	debug("BP[%02x]: 0x%p => cmd = { 0x%02x 0x%06x }\n",
	      spi_w8r8(spi, CMD_READ_STATUS), buf, cmd[0], offset);

	ret = spi_flash_cmd_write_enable(flash);
	if (ret)
		return ret;

	ret = spi_flash_cmd_write(spi, cmd, sizeof(cmd), buf, 1);
	if (ret)
		return ret;

	return spi_flash_wait_till_ready(flash, SPI_FLASH_PROG_TIMEOUT);
}

int sst_write_wp(struct spi_flash *flash, u32 offset, size_t len,
		const void *buf)
{
	struct spi_slave *spi = flash->spi;
	size_t actual, cmd_len;
	int ret;
	u8 cmd[4];

	ret = spi_claim_bus(spi);
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
		      spi_w8r8(spi, CMD_READ_STATUS), buf + actual,
		      cmd[0], offset);

		ret = spi_flash_cmd_write(spi, cmd, cmd_len,
					buf + actual, 2);
		if (ret) {
			debug("SF: sst word program failed\n");
			break;
		}

		ret = spi_flash_wait_till_ready(flash, SPI_FLASH_PROG_TIMEOUT);
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

	spi_release_bus(spi);
	return ret;
}

int sst_write_bp(struct spi_flash *flash, u32 offset, size_t len,
		const void *buf)
{
	struct spi_slave *spi = flash->spi;
	size_t actual;
	int ret;

	ret = spi_claim_bus(spi);
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

	spi_release_bus(spi);
	return ret;
}
#endif

#if defined(CONFIG_SPI_FLASH_STMICRO) || defined(CONFIG_SPI_FLASH_SST)
static void stm_get_locked_range(struct spi_flash *flash, u8 sr, loff_t *ofs,
				 u64 *len)
{
	u8 mask = SR_BP2 | SR_BP1 | SR_BP0;
	int shift = ffs(mask) - 1;
	int pow;

	if (!(sr & mask)) {
		/* No protection */
		*ofs = 0;
		*len = 0;
	} else {
		pow = ((sr & mask) ^ mask) >> shift;
		*len = flash->size >> pow;
		*ofs = flash->size - *len;
	}
}

/*
 * Return 1 if the entire region is locked, 0 otherwise
 */
static int stm_is_locked_sr(struct spi_flash *flash, loff_t ofs, u64 len,
			    u8 sr)
{
	loff_t lock_offs;
	u64 lock_len;

	stm_get_locked_range(flash, sr, &lock_offs, &lock_len);

	return (ofs + len <= lock_offs + lock_len) && (ofs >= lock_offs);
}

/*
 * Check if a region of the flash is (completely) locked. See stm_lock() for
 * more info.
 *
 * Returns 1 if entire region is locked, 0 if any portion is unlocked, and
 * negative on errors.
 */
int stm_is_locked(struct spi_flash *flash, u32 ofs, size_t len)
{
	int status;
	u8 sr;
#ifdef CONFIG_SPI_GENERIC
	int status_up;
	u8 sr_up;
#endif

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash == SF_DUAL_PARALLEL_FLASH) {
		flash->spi->flags |= SPI_XFER_LOWER;
		status = read_sr(flash, &sr);
		if (status < 0)
			return 0;
		status = stm_is_locked_sr(flash, ofs, len, sr);

		flash->spi->flags |= SPI_XFER_UPPER;
		status_up = read_sr(flash, &sr_up);
		if (status_up < 0)
			return status_up;
		status_up = stm_is_locked_sr(flash, ofs, len, sr_up);
		status = status && status_up;

		return status;
	} else {
#endif
	status = read_sr(flash, &sr);
	if (status < 0)
		return status;

	return stm_is_locked_sr(flash, ofs, len, sr);
#ifdef CONFIG_SPI_GENERIC
	}
#endif
}

/*
 * Lock a region of the flash. Compatible with ST Micro and similar flash.
 * Supports only the block protection bits BP{0,1,2} in the status register
 * (SR). Does not support these features found in newer SR bitfields:
 *   - TB: top/bottom protect - only handle TB=0 (top protect)
 *   - SEC: sector/block protect - only handle SEC=0 (block protect)
 *   - CMP: complement protect - only support CMP=0 (range is not complemented)
 *
 * Sample table portion for 8MB flash (Winbond w25q64fw):
 *
 *   SEC  |  TB   |  BP2  |  BP1  |  BP0  |  Prot Length  | Protected Portion
 *  --------------------------------------------------------------------------
 *    X   |   X   |   0   |   0   |   0   |  NONE         | NONE
 *    0   |   0   |   0   |   0   |   1   |  128 KB       | Upper 1/64
 *    0   |   0   |   0   |   1   |   0   |  256 KB       | Upper 1/32
 *    0   |   0   |   0   |   1   |   1   |  512 KB       | Upper 1/16
 *    0   |   0   |   1   |   0   |   0   |  1 MB         | Upper 1/8
 *    0   |   0   |   1   |   0   |   1   |  2 MB         | Upper 1/4
 *    0   |   0   |   1   |   1   |   0   |  4 MB         | Upper 1/2
 *    X   |   X   |   1   |   1   |   1   |  8 MB         | ALL
 *
 * Returns negative on errors, 0 on success.
 */
int stm_lock(struct spi_flash *flash, u32 ofs, size_t len)
{
	u8 status_old, status_new;
	u8 mask = SR_BP2 | SR_BP1 | SR_BP0;
	u8 shift = ffs(mask) - 1, pow, val;
	int ret;

	ret = read_sr(flash, &status_old);
	if (ret < 0)
		return ret;

	/* SPI NOR always locks to the end */
	if (ofs + len != flash->size) {
		/* Does combined region extend to end? */
		if (!stm_is_locked_sr(flash, ofs + len, flash->size - ofs - len,
				      status_old))
			return -EINVAL;
		len = flash->size - ofs;
	}

	/*
	 * Need smallest pow such that:
	 *
	 *   1 / (2^pow) <= (len / size)
	 *
	 * so (assuming power-of-2 size) we do:
	 *
	 *   pow = ceil(log2(size / len)) = log2(size) - floor(log2(len))
	 */
	pow = ilog2(flash->size) - ilog2(len);
	val = mask - (pow << shift);
	if (val & ~mask)
		return -EINVAL;

	/* Don't "lock" with no region! */
	if (!(val & mask))
		return -EINVAL;

	status_new = (status_old & ~mask) | val;

	/* Only modify protection if it will not unlock other areas */
	if ((status_new & mask) <= (status_old & mask))
		return -EINVAL;

	write_sr(flash, status_new);

	return 0;
}

/*
 * Unlock a region of the flash. See stm_lock() for more info
 *
 * Returns negative on errors, 0 on success.
 */
int stm_unlock(struct spi_flash *flash, u32 ofs, size_t len)
{
	uint8_t status_old, status_new;
	u8 mask = SR_BP2 | SR_BP1 | SR_BP0;
	u8 shift = ffs(mask) - 1, pow, val;
	int ret;

	ret = read_sr(flash, &status_old);
	if (ret < 0)
		return ret;

	/* Cannot unlock; would unlock larger region than requested */
	if (stm_is_locked_sr(flash, ofs - flash->erase_size, flash->erase_size,
			     status_old))
		return -EINVAL;
	/*
	 * Need largest pow such that:
	 *
	 *   1 / (2^pow) >= (len / size)
	 *
	 * so (assuming power-of-2 size) we do:
	 *
	 *   pow = floor(log2(size / len)) = log2(size) - ceil(log2(len))
	 */
	pow = ilog2(flash->size) - order_base_2(flash->size - (ofs + len));
	if (ofs + len == flash->size) {
		val = 0; /* fully unlocked */
	} else {
		val = mask - (pow << shift);
		/* Some power-of-two sizes are not supported */
		if (val & ~mask)
			return -EINVAL;
	}

	status_new = (status_old & ~mask) | val;

	/* Only modify protection if it will not lock other areas */
	if ((status_new & mask) >= (status_old & mask))
		return -EINVAL;

	write_sr(flash, status_new);

	return 0;
}
#endif


#if defined(CONFIG_SPI_FLASH_MACRONIX) || defined(CONFIG_SPI_FLASH_ISSI)
static int macronix_quad_enable(struct spi_flash *flash)
{
	u8 qeb_status;
#ifdef CONFIG_SPI_GENERIC
	u8 qeb_status_up;
#endif
	int ret;

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH)
		flash->spi->flags |= SPI_XFER_LOWER;
#endif

	ret = read_sr(flash, &qeb_status);
	if (ret < 0)
		return ret;

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH) {
		flash->spi->flags |= SPI_XFER_UPPER;
		read_sr(flash, &qeb_status_up);
	}
#endif

	if ((qeb_status & STATUS_QEB_MXIC)
#ifdef CONFIG_SPI_GENERIC
	     && (qeb_status_up & STATUS_QEB_MXIC)
#endif
	) {
		debug("SF: mxic: QEB is already set\n");
	} else {
#ifdef CONFIG_SPI_GENERIC
		if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH)
			flash->spi->flags |= SPI_XFER_LOWER;
#endif
		ret = write_sr(flash, STATUS_QEB_MXIC);
		if (ret < 0)
			return ret;
#ifdef CONFIG_SPI_GENERIC
		if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH) {
			flash->spi->flags |= SPI_XFER_UPPER;
			ret = write_sr(flash, STATUS_QEB_MXIC);
			if (ret < 0)
				return ret;
		}
#endif
	}

	return ret;
}
#endif

#if defined(CONFIG_SPI_FLASH_SPANSION) || defined(CONFIG_SPI_FLASH_WINBOND)
static int spansion_quad_enable(struct spi_flash *flash)
{
	u8 qeb_status;
#ifdef CONFIG_SPI_GENERIC
	u8 qeb_status_up;
#endif
	int ret;

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH)
		flash->spi->flags |= SPI_XFER_LOWER;
#endif

	ret = read_cr(flash, &qeb_status);
	if (ret < 0)
		return ret;

#ifdef CONFIG_SPI_GENERIC
	if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH) {
		flash->spi->flags |= SPI_XFER_UPPER;
		ret = read_cr(flash, &qeb_status_up);
	}
#endif

	if ((qeb_status & STATUS_QEB_WINSPAN)
#ifdef CONFIG_SPI_GENERIC
	    && (qeb_status_up & STATUS_QEB_WINSPAN)
#endif
	) {
		debug("SF: winspan: QEB is already set\n");
	} else {
		ret = write_cr(flash, STATUS_QEB_WINSPAN);
		if (ret < 0)
			return ret;
	}

	return ret;
}
#endif


static const struct spi_flash_info *spi_flash_read_id(struct spi_flash *flash)
{
	int				tmp;
	u8				id[SPI_FLASH_MAX_ID_LEN];
	const struct spi_flash_info	*info;

#ifdef CONFIG_SPI_GENERIC
	if (flash->spi->option & SF_DUAL_PARALLEL_FLASH)
			flash->spi->flags |= SPI_XFER_LOWER;
#endif

	tmp = spi_flash_cmd(flash->spi, CMD_READ_ID, id, SPI_FLASH_MAX_ID_LEN);
	if (tmp < 0) {
		printf("SF: error %d reading JEDEC ID\n", tmp);
		return ERR_PTR(tmp);
	}

	info = spi_flash_ids;
	for (; info->name != NULL; info++) {
		if (info->id_len) {
			if (!memcmp(info->id, id, info->id_len))
				return info;
		}
	}

	printf("SF: unrecognized JEDEC id bytes: %02x, %02x, %02x\n",
	       id[0], id[1], id[2]);
	return ERR_PTR(-ENODEV);
}

static int set_quad_mode(struct spi_flash *flash,
			 const struct spi_flash_info *info)
{
	switch (JEDEC_MFR(info)) {
#if defined(CONFIG_SPI_FLASH_MACRONIX) || defined(CONFIG_SPI_FLASH_ISSI)
	case SPI_FLASH_CFI_MFR_MACRONIX:
	case SPI_FLASH_CFI_MFR_ISSI:
		return macronix_quad_enable(flash);
#endif
#if defined(CONFIG_SPI_FLASH_SPANSION) || defined(CONFIG_SPI_FLASH_WINBOND)
	case SPI_FLASH_CFI_MFR_SPANSION:
	case SPI_FLASH_CFI_MFR_WINBOND:
		return spansion_quad_enable(flash);
#endif
#ifdef CONFIG_SPI_FLASH_STMICRO
	case SPI_FLASH_CFI_MFR_STMICRO:
		debug("SF: QEB is volatile for %02x flash\n", JEDEC_MFR(info));
		return 0;
#endif
	default:
		printf("SF: Need set QEB func for %02x flash\n",
		       JEDEC_MFR(info));
		return -1;
	}
}

int spi_flash_cmd_4B_addr_switch(struct spi_flash *flash,
				int enable, u8 idcode0)
{
	int ret;
	u8 cmd;
#ifdef CONFIG_SPI_FLASH_BAR
	u8 bar;
#endif
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
#ifdef CONFIG_SPI_FLASH_BAR
		/* Spansion style */
		bar = enable << 7;
		cmd = CMD_BANKADDR_BRWR;
		ret = spi_flash_cmd_write(flash->spi, &cmd, 1, &bar, 1);
#else
		puts("SF: Bank Address is not set\n");
		ret = -EINVAL;
#endif
	}

	spi_release_bus(flash->spi);

	return ret;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
int spi_flash_decode_fdt(struct spi_flash *flash)
{
#ifdef CONFIG_DM_SPI_FLASH
	fdt_addr_t addr;
	fdt_size_t size;

	addr = dev_read_addr_size(flash->dev, "memory-map", &size);
	if (addr == FDT_ADDR_T_NONE) {
		debug("%s: Cannot decode address\n", __func__);
		return 0;
	}

	if (flash->size > size) {
		debug("%s: Memory map must cover entire device\n", __func__);
		return -1;
	}
	flash->memory_map = map_sysmem(addr, size);
#endif

	return 0;
}
#endif /* CONFIG_IS_ENABLED(OF_CONTROL) */

int spi_flash_scan(struct spi_flash *flash)
{
	struct spi_slave *spi = flash->spi;
	const struct spi_flash_info *info = NULL;
	int ret;

	info = spi_flash_read_id(flash);
	if (IS_ERR_OR_NULL(info))
		return -ENOENT;

	/*
	 * Flash powers up read-only, so clear BP# bits.
	 *
	 * Note on some flash (like Macronix), QE (quad enable) bit is in the
	 * same status register as BP# bits, and we need preserve its original
	 * value during a reboot cycle as this is required by some platforms
	 * (like Intel ICH SPI controller working under descriptor mode).
	 */
	if (JEDEC_MFR(info) == SPI_FLASH_CFI_MFR_ATMEL ||
	   (JEDEC_MFR(info) == SPI_FLASH_CFI_MFR_SST) ||
	   (JEDEC_MFR(info) == SPI_FLASH_CFI_MFR_MACRONIX)) {
		u8 sr = 0;
#ifdef CONFIG_SPI_GENERIC
		u8 sr_up = 0;
#endif

		if (JEDEC_MFR(info) == SPI_FLASH_CFI_MFR_MACRONIX) {
#ifdef CONFIG_SPI_GENERIC
			if (flash->spi->option & SF_DUAL_PARALLEL_FLASH)
				flash->spi->flags |= SPI_XFER_LOWER;
#endif
			read_sr(flash, &sr);
			sr &= STATUS_QEB_MXIC;
#ifdef CONFIG_SPI_GENERIC
			if (flash->spi->option & SF_DUAL_PARALLEL_FLASH) {
				flash->spi->flags |= SPI_XFER_UPPER;
				read_sr(flash, &sr_up);
				sr_up &= STATUS_QEB_MXIC;
			}
#endif
		}

#ifdef CONFIG_SPI_GENERIC
		flash->dual_flash = flash->spi->option;
		if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH)
			flash->spi->flags |= SPI_XFER_LOWER;
#endif
		write_sr(flash, sr);
#ifdef CONFIG_SPI_GENERIC
		if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH) {
			flash->spi->flags |= SPI_XFER_UPPER;
			write_sr(flash, sr_up);
		}
#endif
	}

	flash->name = info->name;
	flash->memory_map = spi->memory_map;
	flash->dual_flash = flash->spi->option;

	if (info->flags & SST_WR)
		flash->flags |= SNOR_F_SST_WR;

#ifndef CONFIG_DM_SPI_FLASH
	flash->write = spi_flash_cmd_write_ops;
#if defined(CONFIG_SPI_FLASH_SST)
	if (flash->flags & SNOR_F_SST_WR) {
		if (spi->mode & SPI_TX_BYTE)
			flash->write = sst_write_bp;
		else
			flash->write = sst_write_wp;
	}
#endif
	flash->erase = spi_flash_cmd_erase_ops;
	flash->read = spi_flash_cmd_read_ops;
#endif

#if defined(CONFIG_SPI_FLASH_STMICRO) || defined(CONFIG_SPI_FLASH_SST)
	/* NOR protection support for STmicro/Micron chips and similar */
	if (JEDEC_MFR(info) == SPI_FLASH_CFI_MFR_STMICRO ||
	    JEDEC_MFR(info) == SPI_FLASH_CFI_MFR_SST) {
		flash->flash_lock = stm_lock;
		flash->flash_unlock = stm_unlock;
		flash->flash_is_locked = stm_is_locked;
	}
#endif

	/* Compute the flash size */
	flash->shift = (flash->dual_flash & SF_DUAL_PARALLEL_FLASH) ? 1 : 0;
	flash->page_size = info->page_size;
	/*
	 * The Spansion S25FL032P and S25FL064P have 256b pages, yet use the
	 * 0x4d00 Extended JEDEC code. The rest of the Spansion flashes with
	 * the 0x4d00 Extended JEDEC code have 512b pages. All of the others
	 * have 256b pages.
	 */
	if (JEDEC_EXT(info) == 0x4d00) {
		if ((JEDEC_ID(info) != 0x0215) &&
		    (JEDEC_ID(info) != 0x0216))
			flash->page_size = 512;
	}
	flash->page_size <<= flash->shift;
	flash->sector_size = info->sector_size << flash->shift;
	flash->size = flash->sector_size * info->n_sectors;

	/*
	 * So far, the 4-byte address mode haven't been supported in U-Boot,
	 * and make sure the chip (> 16MiB) in default 3-byte address mode,
	 * in case of warm bootup, the chip was set to 4-byte mode in kernel.
	 */
	if (((flash->size >> flash->shift) <= SPI_FLASH_16MB_BOUN) &&
	    (flash->spi->bytemode == SPI_4BYTE_MODE))
		/*
		 * Clear the 4-byte support if the flash size is
		 * less than 16MB
		 */
			flash->spi->bytemode = 0;

	if (spi_flash_cmd_4B_addr_switch(flash, flash->spi->bytemode,
					 JEDEC_MFR(info)) < 0)
		printf("SF: enter %s address mode failed\n",
		       flash->spi->bytemode ? "4B" : "3B");
	if (flash->dual_flash & SF_DUAL_STACKED_FLASH) {
		flash->spi->flags = SPI_XFER_U_PAGE;
		if (spi_flash_cmd_4B_addr_switch(flash, flash->spi->bytemode,
						 JEDEC_MFR(info)) < 0)
			printf("SF: enter %s address mode failed\n",
			       flash->spi->bytemode ? "4B" : "3B");
		flash->spi->flags &= ~SPI_XFER_U_PAGE;
	}

#ifdef CONFIG_SF_DUAL_FLASH
	if (flash->dual_flash & SF_DUAL_STACKED_FLASH)
		flash->size <<= 1;
#endif

#ifdef CONFIG_SPI_FLASH_USE_4K_SECTORS
	/* Compute erase sector and command */
	if (info->flags & SECT_4K) {
		flash->erase_cmd = CMD_ERASE_4K;
		flash->erase_size = 4096 << flash->shift;
	} else
#endif
	{
		flash->erase_cmd = CMD_ERASE_64K;
		flash->erase_size = flash->sector_size;
	}

	/* Now erase size becomes valid sector size */
	flash->sector_size = flash->erase_size;

	flash->read_cmd = CMD_READ_ARRAY_FAST;
	if (spi->mode & SPI_RX_SLOW) {
		flash->read_cmd = CMD_READ_ARRAY_SLOW;
	} else if (spi->mode & SPI_RX_QUAD && info->flags & RD_QUAD) {
		flash->read_cmd = CMD_READ_QUAD_OUTPUT_FAST;
		if (((JEDEC_MFR(info) == SPI_FLASH_CFI_MFR_SPANSION) &&
		     (info->id[5] == SPI_FLASH_SPANSION_S25FS_FMLY)) ||
		    ((JEDEC_MFR(info) == SPI_FLASH_CFI_MFR_ISSI) &&
		      info->flags & RD_QUADIO))
			flash->read_cmd = CMD_READ_QUAD_IO_FAST;
	} else if (spi->mode & SPI_RX_DUAL && info->flags & RD_DUAL) {
		flash->read_cmd = CMD_READ_DUAL_OUTPUT_FAST;
	}

	if (spi->dio == SF_DUALIO_FLASH)
		flash->read_cmd = CMD_READ_DUAL_IO_FAST;

	if ((info->flags & WR_QPP) && (spi->mode & SPI_TX_QUAD) &&
	    (spi->dio != SF_DUALIO_FLASH)) {
		if ((JEDEC_MFR(info) == SPI_FLASH_CFI_MFR_SPANSION) &&
		    (info->id[5] == SPI_FLASH_SPANSION_S25FS_FMLY))
			flash->write_cmd = CMD_PAGE_PROGRAM;
		else
			flash->write_cmd = CMD_QUAD_PAGE_PROGRAM;
	} else {
		/* Go for default supported write cmd */
		flash->write_cmd = CMD_PAGE_PROGRAM;
	}


	/* Set the quad enable bit - only for quad commands */
	if ((flash->read_cmd == CMD_READ_QUAD_OUTPUT_FAST) ||
	    (flash->read_cmd == CMD_READ_QUAD_IO_FAST) ||
	    (flash->write_cmd == CMD_QUAD_PAGE_PROGRAM)) {
		ret = set_quad_mode(flash, info);
		if (ret) {
			debug("SF: Fail to set QEB for %02x\n",
			      JEDEC_MFR(info));
			return -EINVAL;
		}
#ifdef CONFIG_SF_DUAL_FLASH
		if (flash->dual_flash & SF_DUAL_STACKED_FLASH) {
			flash->spi->flags |= SPI_XFER_U_PAGE;
			if (set_quad_mode(flash, info)) {
				debug("SF: Fail to set QEB Upper Flash %02x\n",
				      JEDEC_MFR(info));
				return -EINVAL;
			}
			flash->spi->flags &= ~SPI_XFER_U_PAGE;
		}
#endif
	}

	/* Read dummy_byte: dummy byte is determined based on the
	 * dummy cycles of a particular command.
	 * Fast commands - dummy_byte = dummy_cycles/8
	 * I/O commands- dummy_byte = (dummy_cycles * no.of lines)/8
	 * For I/O commands except cmd[0] everything goes on no.of lines
	 * based on particular command but incase of fast commands except
	 * data all go on single line irrespective of command.
	 */
	switch (flash->read_cmd) {
	case CMD_READ_QUAD_IO_FAST:
		if (JEDEC_MFR(info) == SPI_FLASH_CFI_MFR_ISSI)
			flash->dummy_byte = 3;
		else if ((JEDEC_MFR(info) == SPI_FLASH_CFI_MFR_SPANSION) &&
			 (info->id[5] == SPI_FLASH_SPANSION_S25FS_FMLY))
			if (flash->dual_flash & SF_DUAL_PARALLEL_FLASH)
				flash->dummy_byte = 7;
			else
				flash->dummy_byte = 5;
		else
		flash->dummy_byte = 2;
		break;
	case CMD_READ_ARRAY_SLOW:
		flash->dummy_byte = 0;
		break;
	default:
		flash->dummy_byte = 1;
	}

#ifdef CONFIG_SPI_FLASH_STMICRO
	if (info->flags & E_FSR)
		flash->flags |= SNOR_F_USE_FSR;
#endif

	/* Configure the BAR - discover bank cmds and read current bank */
#ifdef CONFIG_SPI_FLASH_BAR
	if (flash->spi->bytemode != SPI_4BYTE_MODE) {
		ret = read_bar(flash, info);
		if (ret < 0)
			return ret;
	}
#endif

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
	ret = spi_flash_decode_fdt(flash);
	if (ret) {
		debug("SF: FDT decode error\n");
		return -EINVAL;
	}
#endif

#ifndef CONFIG_SPL_BUILD
	printf("SF: Detected %s with page size ", flash->name);
	print_size(flash->page_size, ", erase size ");
	print_size(flash->erase_size, ", total ");
	print_size(flash->size, "");
	if (flash->memory_map)
		printf(", mapped at %p", flash->memory_map);
	puts("\n");
#endif

#ifndef CONFIG_SPI_FLASH_BAR
	if (((flash->dual_flash == SF_SINGLE_FLASH) &&
	     (flash->size > SPI_FLASH_16MB_BOUN)) ||
	     ((flash->dual_flash > SF_SINGLE_FLASH) &&
	     (flash->size > SPI_FLASH_16MB_BOUN << 1))) {
		puts("SF: Warning - Only lower 16MiB accessible,");
		puts(" Full access #define CONFIG_SPI_FLASH_BAR\n");
	}
#endif

	return 0;
}
