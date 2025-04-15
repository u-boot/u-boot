// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Amlogic Meson Nand Flash Controller Driver
 *
 * Copyright (c) 2018 Amlogic, inc.
 * Author: Liang Yang <liang.yang@amlogic.com>
 *
 * Copyright (c) 2023 SaluteDevices, Inc.
 * Author: Arseniy Krasnov <avkrasnov@salutedevices.com>
 */

#include <nand.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/ofnode.h>
#include <dm/uclass.h>
#include <linux/bug.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/iopoll.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/sizes.h>

#define NFC_CMD_IDLE			(0xc << 14)
#define NFC_CMD_CLE			(0x5 << 14)
#define NFC_CMD_ALE			(0x6 << 14)
#define NFC_CMD_DWR			(0x4 << 14)
#define NFC_CMD_DRD			(0x8 << 14)
#define NFC_CMD_ADL			((0 << 16) | (3 << 20))
#define NFC_CMD_ADH			((1 << 16) | (3 << 20))
#define NFC_CMD_AIL			((2 << 16) | (3 << 20))
#define NFC_CMD_AIH			((3 << 16) | (3 << 20))
#define NFC_CMD_SEED			((8 << 16) | (3 << 20))
#define NFC_CMD_M2N			((0 << 17) | (2 << 20))
#define NFC_CMD_N2M			((1 << 17) | (2 << 20))
#define NFC_CMD_RB			BIT(20)
#define NFC_CMD_SCRAMBLER_ENABLE	BIT(19)
#define NFC_CMD_SCRAMBLER_DISABLE	0
#define NFC_CMD_SHORTMODE_ENABLE	1
#define NFC_CMD_SHORTMODE_DISABLE	0
#define NFC_CMD_RB_INT			BIT(14)
#define NFC_CMD_RB_INT_NO_PIN		((0xb << 10) | BIT(18) | BIT(16))

#define NFC_CMD_GET_SIZE(x)	(((x) >> 22) & GENMASK(4, 0))

#define NFC_REG_CMD		0x00
#define NFC_REG_CFG		0x04
#define NFC_REG_DADR		0x08
#define NFC_REG_IADR		0x0c
#define NFC_REG_BUF		0x10
#define NFC_REG_INFO		0x14
#define NFC_REG_DC		0x18
#define NFC_REG_ADR		0x1c
#define NFC_REG_DL		0x20
#define NFC_REG_DH		0x24
#define NFC_REG_CADR		0x28
#define NFC_REG_SADR		0x2c
#define NFC_REG_PINS		0x30
#define NFC_REG_VER		0x38

#define CMDRWGEN(cmd_dir, ran, bch, short_mode, page_size, pages)	\
	(								\
		(cmd_dir)			|			\
		(ran)				|			\
		((bch) << 14)			|			\
		((short_mode) << 13)		|			\
		(((page_size) & 0x7f) << 6)	|			\
		((pages) & 0x3f)					\
	)

#define GENCMDDADDRL(adl, addr)		((adl) | ((addr) & 0xffff))
#define GENCMDDADDRH(adh, addr)		((adh) | (((addr) >> 16) & 0xffff))
#define GENCMDIADDRL(ail, addr)		((ail) | ((addr) & 0xffff))
#define GENCMDIADDRH(aih, addr)		((aih) | (((addr) >> 16) & 0xffff))

#define DMA_DIR(dir)		((dir) ? NFC_CMD_N2M : NFC_CMD_M2N)

#define NFC_SHORT_MODE_ECC_SZ	384

#define ECC_CHECK_RETURN_FF	-1

#define NAND_CE0		(0xe << 10)
#define NAND_CE1		(0xd << 10)

#define DMA_BUSY_TIMEOUT_US	1000000
#define CMD_DRAIN_TIMEOUT_US	1000
#define ECC_POLL_TIMEOUT_US	15

#define MAX_CE_NUM		2

/* eMMC clock register, misc control */
#define CLK_SELECT_NAND		BIT(31)
#define CLK_ALWAYS_ON_NAND	BIT(24)
#define CLK_ENABLE_VALUE	0x245

#define DIRREAD			1
#define DIRWRITE		0

#define ECC_PARITY_BCH8_512B	14
#define ECC_COMPLETE            BIT(31)
#define ECC_ERR_CNT(x)		(((x) >> 24) & GENMASK(5, 0))
#define ECC_ZERO_CNT(x)		(((x) >> 16) & GENMASK(5, 0))
#define ECC_UNCORRECTABLE	0x3f

#define PER_INFO_BYTE		8

#define NFC_SEND_CMD(host, cmd) \
	(writel((cmd), (host)->reg_base + NFC_REG_CMD))

#define NFC_GET_CMD(host) \
	(readl((host)->reg_base + NFC_REG_CMD))

#define NFC_CMDFIFO_SIZE(host)	((NFC_GET_CMD((host)) >> 22) & GENMASK(4, 0))

#define NFC_CMD_MAKE_IDLE(ce, delay)	((ce) | NFC_CMD_IDLE | ((delay) & 0x3ff))
#define NFC_CMD_MAKE_DRD(ce, size)	((ce) | NFC_CMD_DRD | (size))
#define NFC_CMD_MAKE_DWR(ce, data)	((ce) | NFC_CMD_DWR | ((data) & 0xff))
#define NFC_CMD_MAKE_CLE(ce, cmd_val)	((ce) | NFC_CMD_CLE | ((cmd_val) & 0xff))
#define NFC_CMD_MAKE_ALE(ce, addr)	((ce) | NFC_CMD_ALE | ((addr) & 0xff))

#define NAND_TWB_TIME_CYCLE	10

#define NFC_DEV_READY_TICK_MAX	5000

/* Both values are recommended by vendor, as the most
 * tested with almost all SLC NAND flash. Second value
 * could be calculated dynamically from timing parameters,
 * but we need both values for initial start of the NAND
 * controller (e.g. before NAND subsystem processes timings),
 * so use hardcoded constants.
 */
#define NFC_DEFAULT_BUS_CYCLE	6
#define NFC_DEFAULT_BUS_TIMING	7

#define NFC_SEED_OFFSET		0xc2
#define NFC_SEED_MASK		0x7fff

#define DMA_ADDR_ALIGN		8

struct meson_nfc_nand_chip {
	struct list_head node;
	struct nand_chip nand;
	u32 boot_pages;
	u32 boot_page_step;

	u32 bch_mode;
	u8 *data_buf;
	__le64 *info_buf;
	u32 nsels;
	u8 sels[];
};

struct meson_nfc_param {
	u32 chip_select;
	u32 rb_select;
};

struct meson_nfc {
	void __iomem *reg_base;
	void __iomem *reg_clk;
	struct list_head chips;
	struct meson_nfc_param param;
	struct udevice *dev;
	dma_addr_t daddr;
	dma_addr_t iaddr;
	u32 data_bytes;
	u32 info_bytes;
	u64 assigned_cs;
};

struct meson_nand_ecc {
	u32 bch;
	u32 strength;
	u32 size;
};

enum {
	NFC_ECC_BCH8_512 = 1,
	NFC_ECC_BCH8_1K,
	NFC_ECC_BCH24_1K,
	NFC_ECC_BCH30_1K,
	NFC_ECC_BCH40_1K,
	NFC_ECC_BCH50_1K,
	NFC_ECC_BCH60_1K,
};

#define MESON_ECC_DATA(b, s, sz) { .bch = (b), .strength = (s), .size = (sz) }

static struct meson_nand_ecc meson_ecc[] = {
	MESON_ECC_DATA(NFC_ECC_BCH8_512, 8,  512),
	MESON_ECC_DATA(NFC_ECC_BCH8_1K,  8,  1024),
};

static int meson_nand_calc_ecc_bytes(int step_size, int strength)
{
	int ecc_bytes;

	if (step_size == 512 && strength == 8)
		return ECC_PARITY_BCH8_512B;

	ecc_bytes = DIV_ROUND_UP(strength * fls(step_size * 8), 8);
	ecc_bytes = ALIGN(ecc_bytes, 2);

	return ecc_bytes;
}

static struct meson_nfc_nand_chip *to_meson_nand(struct nand_chip *nand)
{
	return container_of(nand, struct meson_nfc_nand_chip, nand);
}

static void meson_nfc_nand_select_chip(struct mtd_info *mtd, int chip)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
	struct meson_nfc *nfc = nand_get_controller_data(nand);

	nfc->param.chip_select = meson_chip->sels[chip] ? NAND_CE1 : NAND_CE0;
}

static void meson_nfc_cmd_idle(struct meson_nfc *nfc, u32 time)
{
	writel(NFC_CMD_MAKE_IDLE(nfc->param.chip_select, time),
	       nfc->reg_base + NFC_REG_CMD);
}

static void meson_nfc_cmd_seed(const struct meson_nfc *nfc, u32 seed)
{
	writel(NFC_CMD_SEED | (NFC_SEED_OFFSET + (seed & NFC_SEED_MASK)),
	       nfc->reg_base + NFC_REG_CMD);
}

static int meson_nfc_is_boot_page(struct nand_chip *nand, int page)
{
	const struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);

	return (nand->options & NAND_IS_BOOT_MEDIUM) &&
	       !(page % meson_chip->boot_page_step) &&
	       (page < meson_chip->boot_pages);
}

static void meson_nfc_cmd_access(struct nand_chip *nand, bool raw, bool dir, int page)
{
	const struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
	struct mtd_info *mtd = nand_to_mtd(nand);
	const struct meson_nfc *nfc = nand_get_controller_data(mtd_to_nand(mtd));
	int len = mtd->writesize, pagesize, pages;
	unsigned int scrambler;
	u32 cmd;

	if (nand->options & NAND_NEED_SCRAMBLING)
		scrambler = NFC_CMD_SCRAMBLER_ENABLE;
	else
		scrambler = NFC_CMD_SCRAMBLER_DISABLE;

	if (raw) {
		len = mtd->writesize + mtd->oobsize;
		cmd = len | scrambler | DMA_DIR(dir);
	} else if (meson_nfc_is_boot_page(nand, page)) {
		pagesize = NFC_SHORT_MODE_ECC_SZ >> 3;
		pages = mtd->writesize / 512;

		scrambler = NFC_CMD_SCRAMBLER_ENABLE;
		cmd = CMDRWGEN(DMA_DIR(dir), scrambler, NFC_ECC_BCH8_1K,
			       NFC_CMD_SHORTMODE_ENABLE, pagesize, pages);
	} else {
		pagesize = nand->ecc.size >> 3;
		pages = len / nand->ecc.size;

		cmd = CMDRWGEN(DMA_DIR(dir), scrambler, meson_chip->bch_mode,
			       NFC_CMD_SHORTMODE_DISABLE, pagesize, pages);
	}

	if (scrambler == NFC_CMD_SCRAMBLER_ENABLE)
		meson_nfc_cmd_seed(nfc, page);

	writel(cmd, nfc->reg_base + NFC_REG_CMD);
}

static void meson_nfc_drain_cmd(struct meson_nfc *nfc)
{
	/*
	 * Insert two commands to make sure all valid commands are finished.
	 *
	 * The Nand flash controller is designed as two stages pipleline -
	 *  a) fetch and b) execute.
	 * There might be cases when the driver see command queue is empty,
	 * but the Nand flash controller still has two commands buffered,
	 * one is fetched into NFC request queue (ready to run), and another
	 * is actively executing. So pushing 2 "IDLE" commands guarantees that
	 * the pipeline is emptied.
	 */
	meson_nfc_cmd_idle(nfc, 0);
	meson_nfc_cmd_idle(nfc, 0);
}

static int meson_nfc_wait_cmd_finish(const struct meson_nfc *nfc,
				     unsigned int timeout_us)
{
	u32 cmd_size = 0;

	/* wait cmd fifo is empty */
	return readl_relaxed_poll_timeout(nfc->reg_base + NFC_REG_CMD, cmd_size,
					  !NFC_CMD_GET_SIZE(cmd_size),
					  timeout_us);
}

static int meson_nfc_wait_dma_finish(struct meson_nfc *nfc)
{
	meson_nfc_drain_cmd(nfc);

	return meson_nfc_wait_cmd_finish(nfc, DMA_BUSY_TIMEOUT_US);
}

static u8 *meson_nfc_oob_ptr(struct nand_chip *nand, int i)
{
	const struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
	int len;

	len = nand->ecc.size * (i + 1) + (nand->ecc.bytes + 2) * i;

	return meson_chip->data_buf + len;
}

static u8 *meson_nfc_data_ptr(struct nand_chip *nand, int i)
{
	const struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
	int len, temp;

	temp = nand->ecc.size + nand->ecc.bytes;
	len = (temp + 2) * i;

	return meson_chip->data_buf + len;
}

static void meson_nfc_get_data_oob(struct nand_chip *nand,
				   u8 *buf, u8 *oobbuf)
{
	u8 *dsrc, *osrc;
	int i, oob_len;

	oob_len = nand->ecc.bytes + 2;
	for (i = 0; i < nand->ecc.steps; i++) {
		if (buf) {
			dsrc = meson_nfc_data_ptr(nand, i);
			memcpy(buf, dsrc, nand->ecc.size);
			buf += nand->ecc.size;
		}

		if (oobbuf) {
			osrc = meson_nfc_oob_ptr(nand, i);
			memcpy(oobbuf, osrc, oob_len);
			oobbuf += oob_len;
		}
	}
}

static void meson_nfc_set_data_oob(struct nand_chip *nand,
				   const u8 *buf, u8 *oobbuf)
{
	int i, oob_len;

	oob_len = nand->ecc.bytes + 2;
	for (i = 0; i < nand->ecc.steps; i++) {
		u8 *osrc;

		if (buf) {
			u8 *dsrc;

			dsrc = meson_nfc_data_ptr(nand, i);
			memcpy(dsrc, buf, nand->ecc.size);
			buf += nand->ecc.size;
		}

		osrc = meson_nfc_oob_ptr(nand, i);
		memcpy(osrc, oobbuf, oob_len);
		oobbuf += oob_len;
	}
}

static void meson_nfc_set_user_byte(struct nand_chip *nand, const u8 *oob_buf)
{
	struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
	int i, count;

	for (i = 0, count = 0; i < nand->ecc.steps; i++, count += (2 + nand->ecc.bytes)) {
		__le64 *info = &meson_chip->info_buf[i];

		*info |= oob_buf[count];
		*info |= oob_buf[count + 1] << 8;
	}
}

static void meson_nfc_get_user_byte(struct nand_chip *nand, u8 *oob_buf)
{
	struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
	int i, count;

	for (i = 0, count = 0; i < nand->ecc.steps; i++, count += (2 + nand->ecc.bytes)) {
		const __le64 *info = &meson_chip->info_buf[i];

		oob_buf[count] = *info;
		oob_buf[count + 1] = *info >> 8;
	}
}

static int meson_nfc_ecc_correct(struct nand_chip *nand, u32 *bitflips,
				 u64 *correct_bitmap)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	int ret = 0, i;

	for (i = 0; i < nand->ecc.steps; i++) {
		struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
		const __le64 *info = &meson_chip->info_buf[i];

		if (ECC_ERR_CNT(*info) != ECC_UNCORRECTABLE) {
			mtd->ecc_stats.corrected += ECC_ERR_CNT(*info);
			*bitflips = max_t(u32, *bitflips, ECC_ERR_CNT(*info));
			*correct_bitmap |= BIT_ULL(i);
			continue;
		}

		if ((nand->options & NAND_NEED_SCRAMBLING) &&
		    ECC_ZERO_CNT(*info) < nand->ecc.strength) {
			mtd->ecc_stats.corrected += ECC_ZERO_CNT(*info);
			*bitflips = max_t(u32, *bitflips,
					  ECC_ZERO_CNT(*info));
			ret = ECC_CHECK_RETURN_FF;
		} else {
			ret = -EBADMSG;
		}
	}

	return ret;
}

static int meson_nfc_dma_buffer_setup(struct nand_chip *nand, void *databuf,
				      int datalen, void *infobuf, int infolen,
				      enum dma_data_direction dir)
{
	struct meson_nfc *nfc = nand_get_controller_data(nand);
	int ret;
	u32 cmd;

	nfc->daddr = dma_map_single(databuf, datalen, DMA_BIDIRECTIONAL);
	ret = dma_mapping_error(nfc->dev, nfc->daddr);
	if (ret)
		return ret;

	cmd = GENCMDDADDRL(NFC_CMD_ADL, nfc->daddr);
	writel(cmd, nfc->reg_base + NFC_REG_CMD);

	cmd = GENCMDDADDRH(NFC_CMD_ADH, nfc->daddr);
	writel(cmd, nfc->reg_base + NFC_REG_CMD);

	if (infobuf) {
		nfc->iaddr = dma_map_single(infobuf, infolen,
					    DMA_BIDIRECTIONAL);
		ret = dma_mapping_error(nfc->dev, nfc->iaddr);
		if (ret) {
			dma_unmap_single(nfc->daddr, datalen, dir);
			return ret;
		}

		nfc->info_bytes = infolen;
		cmd = GENCMDIADDRL(NFC_CMD_AIL, nfc->iaddr);
		writel(cmd, nfc->reg_base + NFC_REG_CMD);

		cmd = GENCMDIADDRH(NFC_CMD_AIH, nfc->iaddr);
		writel(cmd, nfc->reg_base + NFC_REG_CMD);
	}

	return 0;
}

static void meson_nfc_dma_buffer_release(struct nand_chip *nand,
					 int datalen, int infolen,
					 enum dma_data_direction dir)
{
	struct meson_nfc *nfc = nand_get_controller_data(nand);

	dma_unmap_single(nfc->daddr, datalen, dir);

	if (infolen) {
		dma_unmap_single(nfc->iaddr, infolen, dir);
		nfc->info_bytes = 0;
	}
}

static void meson_nfc_read_buf(struct mtd_info *mtd, u8 *buf, int size)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct meson_nfc *nfc = nand_get_controller_data(nand);
	struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
	u8 *dma_buf;
	int ret;
	u32 cmd;

	if ((uintptr_t)buf % DMA_ADDR_ALIGN) {
		unsigned long tmp_addr;

		dma_buf = dma_alloc_coherent(size, &tmp_addr);
		if (!dma_buf)
			return;
	} else {
		dma_buf = buf;
	}

	ret = meson_nfc_dma_buffer_setup(nand, dma_buf, size, meson_chip->info_buf,
					 PER_INFO_BYTE, DMA_FROM_DEVICE);
	if (ret) {
		pr_err("Failed to setup DMA buffer %p/%p\n", dma_buf,
		       meson_chip->info_buf);
		return;
	}

	cmd = NFC_CMD_N2M | size;
	writel(cmd, nfc->reg_base + NFC_REG_CMD);

	meson_nfc_drain_cmd(nfc);
	meson_nfc_wait_cmd_finish(nfc, CMD_DRAIN_TIMEOUT_US);
	meson_nfc_dma_buffer_release(nand, size, PER_INFO_BYTE, DMA_FROM_DEVICE);

	if (buf != dma_buf) {
		memcpy(buf, dma_buf, size);
		dma_free_coherent(dma_buf);
	}
}

static void meson_nfc_write_buf(struct mtd_info *mtd, const u8 *buf, int size)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct meson_nfc *nfc = nand_get_controller_data(nand);
	u8 *dma_buf;
	int ret;
	u32 cmd;

	if ((uintptr_t)buf % DMA_ADDR_ALIGN) {
		unsigned long tmp_addr;

		dma_buf = dma_alloc_coherent(size, &tmp_addr);
		if (!dma_buf)
			return;

		memcpy(dma_buf, buf, size);
	} else {
		dma_buf = (u8 *)buf;
	}

	ret = meson_nfc_dma_buffer_setup(nand, (void *)dma_buf, size, NULL,
					 0, DMA_TO_DEVICE);
	if (ret) {
		pr_err("Failed to setup DMA buffer %p\n", dma_buf);
		return;
	}

	cmd = NFC_CMD_M2N | size;
	writel(cmd, nfc->reg_base + NFC_REG_CMD);

	meson_nfc_drain_cmd(nfc);
	meson_nfc_wait_cmd_finish(nfc, CMD_DRAIN_TIMEOUT_US);
	meson_nfc_dma_buffer_release(nand, size, 0, DMA_TO_DEVICE);

	if (buf != dma_buf)
		dma_free_coherent(dma_buf);
}

static int meson_nfc_write_page_sub(struct nand_chip *nand,
				    int page, bool raw)
{
	const struct mtd_info *mtd = nand_to_mtd(nand);
	struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
	struct meson_nfc *nfc = nand_get_controller_data(nand);
	int data_len, info_len;
	int ret;
	u32 cmd;

	data_len =  mtd->writesize + mtd->oobsize;
	info_len = nand->ecc.steps * PER_INFO_BYTE;

	ret = meson_nfc_dma_buffer_setup(nand, meson_chip->data_buf,
					 data_len, meson_chip->info_buf,
					 info_len, DMA_TO_DEVICE);
	if (ret) {
		pr_err("Failed to setup DMA buffer %p/%p\n",
		       meson_chip->data_buf, meson_chip->info_buf);
		return ret;
	}

	meson_nfc_cmd_access(nand, raw, DIRWRITE, page);

	cmd = nfc->param.chip_select | NFC_CMD_CLE | NAND_CMD_PAGEPROG;
	writel(cmd, nfc->reg_base + NFC_REG_CMD);

	meson_nfc_dma_buffer_release(nand, data_len, info_len, DMA_TO_DEVICE);

	return 0;
}

static int meson_nfc_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
				    const u8 *buf, int oob_required, int page)
{
	meson_nfc_set_data_oob(chip, buf, oob_required ? chip->oob_poi : NULL);

	return meson_nfc_write_page_sub(chip, page, true);
}

static int meson_nfc_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				      const u8 *buf, int oob_required, int page)
{
	struct meson_nfc_nand_chip *meson_chip = to_meson_nand(chip);

	if (buf)
		memcpy(meson_chip->data_buf, buf, mtd->writesize);

	memset(meson_chip->info_buf, 0, chip->ecc.steps * PER_INFO_BYTE);
	meson_nfc_set_user_byte(chip, chip->oob_poi);

	return meson_nfc_write_page_sub(chip, page, false);
}

static void meson_nfc_check_ecc_pages_valid(struct meson_nfc *nfc,
					    struct nand_chip *nand, bool raw)
{
	struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
	__le64 *info;
	u32 neccpages;
	int ret;

	neccpages = raw ? 1 : nand->ecc.steps;
	info = &meson_chip->info_buf[neccpages - 1];
	do {
		udelay(ECC_POLL_TIMEOUT_US);
		/* info is updated by nfc dma engine*/
		rmb();
		invalidate_dcache_range(nfc->iaddr, nfc->iaddr + nfc->info_bytes);
		ret = *info & ECC_COMPLETE;
	} while (!ret);
}

static int meson_nfc_read_page_sub(struct nand_chip *nand,
				   int page, bool raw)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct meson_nfc *nfc = nand_get_controller_data(nand);
	struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
	u32 data_len, info_len;
	int ret;

	data_len = mtd->writesize + mtd->oobsize;
	info_len = nand->ecc.steps * PER_INFO_BYTE;

	ret = meson_nfc_dma_buffer_setup(nand, meson_chip->data_buf, data_len,
					 meson_chip->info_buf, info_len,
					 DMA_FROM_DEVICE);
	if (ret)
		return ret;

	meson_nfc_cmd_access(nand, raw, DIRREAD, page);

	meson_nfc_wait_dma_finish(nfc);
	meson_nfc_check_ecc_pages_valid(nfc, nand, raw);

	meson_nfc_dma_buffer_release(nand, data_len, info_len,
				     DMA_FROM_DEVICE);

	return 0;
}

static int meson_nfc_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
				   u8 *buf, int oob_required, int page)
{
	int ret;

	ret = meson_nfc_read_page_sub(chip, page, true);
	if (ret)
		return ret;

	meson_nfc_get_data_oob(chip, buf, oob_required ? chip->oob_poi : NULL);

	return 0;
}

static int meson_nfc_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				     u8 *buf, int oob_required, int page)
{
	const struct meson_nfc_nand_chip *meson_chip = to_meson_nand(chip);
	u64 correct_bitmap = 0;
	u32 bitflips = 0;
	int ret;

	ret = meson_nfc_read_page_sub(chip, page, false);
	if (ret)
		return ret;

	if (oob_required)
		meson_nfc_get_user_byte(chip, chip->oob_poi);

	ret = meson_nfc_ecc_correct(chip, &bitflips, &correct_bitmap);

	if (ret == ECC_CHECK_RETURN_FF) {
		if (buf)
			memset(buf, 0xff, mtd->writesize);

		if (oob_required)
			memset(chip->oob_poi, 0xff, mtd->oobsize);
	} else if (ret < 0) {
		struct nand_ecc_ctrl *ecc;
		int i;

		if ((chip->options & NAND_NEED_SCRAMBLING) || !buf) {
			mtd->ecc_stats.failed++;
			return bitflips;
		}

		chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);

		ret = meson_nfc_read_page_raw(mtd, chip, buf, 1, page);
		if (ret)
			return ret;

		ecc = &chip->ecc;

		for (i = 0; i < chip->ecc.steps ; i++) {
			u8 *data = buf + i * ecc->size;
			u8 *oob = chip->oob_poi + i * (ecc->bytes + 2);

			if (correct_bitmap & BIT_ULL(i))
				continue;

			ret = nand_check_erased_ecc_chunk(data,	ecc->size,
							  oob, ecc->bytes + 2,
							  NULL, 0,
							  ecc->strength);
			if (ret < 0) {
				mtd->ecc_stats.failed++;
			} else {
				mtd->ecc_stats.corrected += ret;
				bitflips =  max_t(u32, bitflips, ret);
			}
		}
	} else if (buf && buf != meson_chip->data_buf) {
		memcpy(buf, meson_chip->data_buf, mtd->writesize);
	}

	return bitflips;
}

static int meson_nfc_read_oob_raw(struct mtd_info *mtd, struct nand_chip *chip,
				  int page)
{
	int ret;

	ret = nand_read_page_op(chip, page, 0, NULL, 0);
	if (ret)
		return ret;

	return meson_nfc_read_page_raw(mtd, chip, NULL, 1, page);
}

static int meson_nfc_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
			      int page)
{
	int ret;

	ret = nand_read_page_op(chip, page, 0, NULL, 0);
	if (ret)
		return ret;

	return meson_nfc_read_page_hwecc(mtd, chip, NULL, 1, page);
}

static int meson_nfc_write_oob_raw(struct mtd_info *mtd, struct nand_chip *chip,
				   int page)
{
	int ret;

	ret = nand_prog_page_begin_op(chip, page, 0, NULL, 0);
	if (ret)
		return ret;

	ret = meson_nfc_write_page_raw(mtd, chip, NULL, 1, page);
	if (ret)
		return ret;

	return nand_prog_page_end_op(chip);
}

static int meson_nfc_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
			       int page)
{
	int ret;

	ret = nand_prog_page_begin_op(chip, page, 0, NULL, 0);
	if (ret)
		return ret;

	ret = meson_nfc_write_page_hwecc(mtd, chip, NULL, 1, page);
	if (ret)
		return ret;

	return nand_prog_page_end_op(chip);
}

static void meson_nfc_nand_cmd_function(struct mtd_info *mtd, unsigned int command,
					int column, int page_addr)
{
	struct nand_chip *chip = mtd_to_nand(mtd);

	chip->cmd_ctrl(mtd, command, NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);

	if (column != -1 || page_addr != -1) {
		int ctrl = NAND_CTRL_CHANGE | NAND_NCE | NAND_ALE;

		/* Serially input address */
		if (column != -1) {
			/* Adjust columns for 16 bit buswidth */
			if (chip->options & NAND_BUSWIDTH_16 &&
			    !nand_opcode_8bits(command))
				column >>= 1;

			chip->cmd_ctrl(mtd, column, ctrl);
			ctrl &= ~NAND_CTRL_CHANGE;
			/* Only output a single addr cycle for 8bits
			 * opcodes.
			 */
			if (!nand_opcode_8bits(command))
				chip->cmd_ctrl(mtd, column >> 8, ctrl);
		}

		if (page_addr != -1) {
			chip->cmd_ctrl(mtd, page_addr, ctrl);
			chip->cmd_ctrl(mtd, page_addr >> 8, NAND_NCE |
							    NAND_ALE);
			/* One more address cycle for devices > 128MiB */
			if (chip->chipsize > SZ_128M)
				chip->cmd_ctrl(mtd, page_addr >> 16,
					       NAND_NCE | NAND_ALE);
		}

		switch (command) {
		case NAND_CMD_READ0:
			chip->cmd_ctrl(mtd, NAND_CMD_READSTART,
				       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
			fallthrough;
		case NAND_CMD_PARAM:
			nand_wait_ready(mtd);
			nand_exit_status_op(chip);
		}
	}
}

static void meson_nfc_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct meson_nfc *nfc = nand_get_controller_data(nand);

	if (cmd == NAND_CMD_NONE)
		return;

	if (ctrl & NAND_CLE)
		cmd = NFC_CMD_MAKE_CLE(nfc->param.chip_select, cmd);
	else
		cmd = NFC_CMD_MAKE_ALE(nfc->param.chip_select, cmd);

	writel(cmd, nfc->reg_base + NFC_REG_CMD);
}

static void meson_nfc_wait_cmd_fifo(struct meson_nfc *nfc)
{
	while ((NFC_GET_CMD(nfc) >> 22) & GENMASK(4, 0))
		;
}

static u8 meson_nfc_nand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct meson_nfc *nfc = nand_get_controller_data(nand);

	writel(NFC_CMD_MAKE_DRD(nfc->param.chip_select, 0), nfc->reg_base + NFC_REG_CMD);

	meson_nfc_cmd_idle(nfc, NAND_TWB_TIME_CYCLE);
	meson_nfc_cmd_idle(nfc, 0);
	meson_nfc_cmd_idle(nfc, 0);

	meson_nfc_wait_cmd_fifo(nfc);

	return readl(nfc->reg_base + NFC_REG_BUF);
}

static void meson_nfc_nand_write_byte(struct mtd_info *mtd, u8 val)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct meson_nfc *nfc = nand_get_controller_data(nand);

	meson_nfc_cmd_idle(nfc, NAND_TWB_TIME_CYCLE);

	writel(NFC_CMD_MAKE_DWR(nfc->param.chip_select, val), nfc->reg_base + NFC_REG_CMD);

	meson_nfc_cmd_idle(nfc, NAND_TWB_TIME_CYCLE);
	meson_nfc_cmd_idle(nfc, 0);
	meson_nfc_cmd_idle(nfc, 0);

	meson_nfc_wait_cmd_fifo(nfc);
}

static int meson_nfc_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	unsigned int time_out_cnt = 0;

	chip->select_chip(mtd, 0);

	chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);

	do {
		int status;

		status = (int)chip->read_byte(mtd);
		if (status & NAND_STATUS_READY)
			break;
	} while (time_out_cnt++ < NFC_DEV_READY_TICK_MAX);

	return time_out_cnt != NFC_DEV_READY_TICK_MAX;
}

static int meson_chip_buffer_init(struct nand_chip *nand)
{
	const struct mtd_info *mtd = nand_to_mtd(nand);
	struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);
	u32 page_bytes, info_bytes, nsectors;
	unsigned long tmp_addr;

	nsectors = mtd->writesize / nand->ecc.size;

	page_bytes =  mtd->writesize + mtd->oobsize;
	info_bytes = nsectors * PER_INFO_BYTE;

	meson_chip->data_buf = dma_alloc_coherent(page_bytes, &tmp_addr);
	if (!meson_chip->data_buf)
		return -ENOMEM;

	meson_chip->info_buf = dma_alloc_coherent(info_bytes, &tmp_addr);
	if (!meson_chip->info_buf) {
		dma_free_coherent(meson_chip->data_buf);
		return -ENOMEM;
	}

	return 0;
}

static const int axg_stepinfo_strengths[] = { 8 };
static const struct nand_ecc_step_info axg_stepinfo_1024 = {
	.stepsize = 1024,
	.strengths = axg_stepinfo_strengths,
	.nstrengths = ARRAY_SIZE(axg_stepinfo_strengths)
};

static const struct nand_ecc_step_info axg_stepinfo_512 = {
	.stepsize = 512,
	.strengths = axg_stepinfo_strengths,
	.nstrengths = ARRAY_SIZE(axg_stepinfo_strengths)
};

static const struct nand_ecc_step_info axg_stepinfo[] = { axg_stepinfo_1024, axg_stepinfo_512 };

static const struct nand_ecc_caps meson_axg_ecc_caps = {
	.stepinfos = axg_stepinfo,
	.nstepinfos = ARRAY_SIZE(axg_stepinfo),
	.calc_ecc_bytes = meson_nand_calc_ecc_bytes,
};

/*
 * OOB layout:
 *
 * For ECC with 512 bytes step size:
 * 0x00: AA AA BB BB BB BB BB BB BB BB BB BB BB BB BB BB
 * 0x10: AA AA CC CC CC CC CC CC CC CC CC CC CC CC CC CC
 * 0x20:
 * 0x30:
 *
 * For ECC with 1024 bytes step size:
 * 0x00: AA AA BB BB BB BB BB BB BB BB BB BB BB BB BB BB
 * 0x10: AA AA CC CC CC CC CC CC CC CC CC CC CC CC CC CC
 * 0x20: AA AA DD DD DD DD DD DD DD DD DD DD DD DD DD DD
 * 0x30: AA AA EE EE EE EE EE EE EE EE EE EE EE EE EE EE
 *
 * AA - user bytes.
 * BB, CC, DD, EE - ECC code bytes for each step.
 */
static struct nand_ecclayout nand_oob;

static void meson_nfc_init_nand_oob(struct nand_chip *nand)
{
	int section_size = 2 + nand->ecc.bytes;
	int i;
	int k;

	nand_oob.eccbytes = nand->ecc.steps * nand->ecc.bytes;
	k = 0;

	for (i = 0; i < nand->ecc.steps; i++) {
		int j;

		for (j = 0; j < nand->ecc.bytes; j++)
			nand_oob.eccpos[k++] = (i * section_size) + 2 + j;

		nand_oob.oobfree[i].offset = (i * section_size);
		nand_oob.oobfree[i].length = 2;
	}

	nand_oob.oobavail = 2 * nand->ecc.steps;
	nand->ecc.layout = &nand_oob;
}

static int meson_nfc_init_ecc(struct nand_chip *nand, ofnode node)
{
	const struct mtd_info *mtd = nand_to_mtd(nand);
	int ret;
	int i;

	ret = nand_check_ecc_caps(nand, &meson_axg_ecc_caps, mtd->oobsize - 2);
	if (ret)
		return ret;

	for (i = 0; i < ARRAY_SIZE(meson_ecc); i++) {
		if (meson_ecc[i].strength == nand->ecc.strength &&
		    meson_ecc[i].size == nand->ecc.size) {
			struct meson_nfc_nand_chip *meson_chip = to_meson_nand(nand);

			nand->ecc.steps = mtd->writesize / nand->ecc.size;
			meson_chip->bch_mode = meson_ecc[i].bch;

			meson_nfc_init_nand_oob(nand);

			return 0;
		}
	}

	return -EINVAL;
}

static int meson_nfc_nand_chip_init(struct udevice *dev, struct meson_nfc *nfc,
				    ofnode node)
{
	struct meson_nfc_nand_chip *meson_chip;
	struct nand_chip *nand;
	struct mtd_info *mtd;
	u32 cs[MAX_CE_NUM];
	u32 nsels;
	int ret;
	int i;

	if (!ofnode_get_property(node, "reg", &nsels)) {
		dev_err(dev, "\"reg\" property is not found\n");
		return -ENODEV;
	}

	nsels /= sizeof(u32);
	if (nsels >= MAX_CE_NUM) {
		dev_err(dev, "invalid size of CS array, max is %d\n",
			MAX_CE_NUM);
		return -EINVAL;
	}

	ret = ofnode_read_u32_array(node, "reg", cs, nsels);
	if (ret < 0) {
		dev_err(dev, "failed to read \"reg\" property\n");
		return ret;
	}

	for (i = 0; i < nsels; i++) {
		if (test_and_set_bit(cs[i], &nfc->assigned_cs)) {
			dev_err(dev, "CS %d already assigned\n", cs[i]);
			return -EINVAL;
		}
	}

	meson_chip = malloc(sizeof(*meson_chip) + nsels * sizeof(meson_chip->sels[0]));
	if (!meson_chip) {
		dev_err(dev, "failed to allocate memory for chip\n");
		return -ENOMEM;
	}

	meson_chip->nsels = nsels;
	nand = &meson_chip->nand;

	nand->flash_node = node;
	nand_set_controller_data(nand, nfc);
	/* Set the driver entry points for MTD */
	nand->cmdfunc = meson_nfc_nand_cmd_function;
	nand->cmd_ctrl = meson_nfc_cmd_ctrl;
	nand->select_chip = meson_nfc_nand_select_chip;
	nand->read_byte = meson_nfc_nand_read_byte;
	nand->write_byte = meson_nfc_nand_write_byte;
	nand->dev_ready = meson_nfc_dev_ready;

	/* Buffer read/write routines */
	nand->read_buf = meson_nfc_read_buf;
	nand->write_buf = meson_nfc_write_buf;
	nand->options |= NAND_NO_SUBPAGE_WRITE;

	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.hwctl = NULL;
	nand->ecc.read_page = meson_nfc_read_page_hwecc;
	nand->ecc.write_page = meson_nfc_write_page_hwecc;
	nand->ecc.read_page_raw = meson_nfc_read_page_raw;
	nand->ecc.write_page_raw = meson_nfc_write_page_raw;

	nand->ecc.read_oob = meson_nfc_read_oob;
	nand->ecc.write_oob = meson_nfc_write_oob;
	nand->ecc.read_oob_raw = meson_nfc_read_oob_raw;
	nand->ecc.write_oob_raw = meson_nfc_write_oob_raw;

	nand->ecc.algo = NAND_ECC_BCH;

	mtd = nand_to_mtd(nand);

	ret = nand_scan_ident(mtd, 1, NULL);
	if (ret) {
		dev_err(dev, "'nand_scan_ident()' failed: %d\n", ret);
		goto err_chip_free;
	}

	ret = meson_nfc_init_ecc(nand, node);
	if (ret) {
		dev_err(dev, "failed to init ECC settings: %d\n", ret);
		goto err_chip_free;
	}

	ret = meson_chip_buffer_init(nand);
	if (ret) {
		dev_err(dev, "failed to init DMA buffers: %d\n", ret);
		goto err_chip_free;
	}

	/* 'nand_scan_tail()' needs ECC parameters to be already
	 * set and correct.
	 */
	ret = nand_scan_tail(mtd);
	if (ret) {
		dev_err(dev, "'nand_scan_tail()' failed: %d\n", ret);
		goto err_chip_buf_free;
	}

	if (nand->options & NAND_IS_BOOT_MEDIUM) {
		ret = ofnode_read_u32(node, "amlogic,boot-pages",
				      &meson_chip->boot_pages);
		if (ret) {
			dev_err(dev, "could not retrieve 'amlogic,boot-pages' property: %d",
				ret);
			goto err_chip_buf_free;
		}

		ret = ofnode_read_u32(node, "amlogic,boot-page-step",
				      &meson_chip->boot_page_step);
		if (ret) {
			dev_err(dev, "could not retrieve 'amlogic,boot-page-step' property: %d",
				ret);
			goto err_chip_buf_free;
		}
	}

	ret = nand_register(0, mtd);
	if (ret) {
		dev_err(dev, "'nand_register()' failed: %d\n", ret);
		goto err_chip_buf_free;
	}

	list_add_tail(&meson_chip->node, &nfc->chips);

	return 0;

err_chip_buf_free:
	dma_free_coherent(meson_chip->info_buf);
	dma_free_coherent(meson_chip->data_buf);

err_chip_free:
	free(meson_chip);

	return ret;
}

static int meson_nfc_nand_chips_init(struct udevice *dev,
				     struct meson_nfc *nfc)
{
	ofnode parent = dev_ofnode(dev);
	ofnode node;

	ofnode_for_each_subnode(node, parent) {
		int ret = meson_nfc_nand_chip_init(dev, nfc, node);

		if (ret)
			return ret;
	}

	return 0;
}

static void meson_nfc_clk_init(struct meson_nfc *nfc)
{
	u32 bus_cycle = NFC_DEFAULT_BUS_CYCLE;
	u32 bus_timing = NFC_DEFAULT_BUS_TIMING;
	u32 bus_cfg_val;

	writel(CLK_ALWAYS_ON_NAND | CLK_SELECT_NAND | CLK_ENABLE_VALUE, nfc->reg_clk);
	writel(0, nfc->reg_base + NFC_REG_CFG);

	bus_cfg_val = (((bus_cycle - 1) & 31) | ((bus_timing & 31) << 5));
	writel(bus_cfg_val, nfc->reg_base + NFC_REG_CFG);
	writel(BIT(31), nfc->reg_base + NFC_REG_CMD);
}

static int meson_probe(struct udevice *dev)
{
	struct meson_nfc *nfc = dev_get_priv(dev);
	void *addr;
	int ret;

	addr = dev_read_addr_ptr(dev);
	if (!addr) {
		dev_err(dev, "base register address not found\n");
		return -EINVAL;
	}

	nfc->reg_base = addr;

	addr = dev_read_addr_index_ptr(dev, 1);
	if (!addr) {
		dev_err(dev, "clk register address not found\n");
		return -EINVAL;
	}

	nfc->reg_clk = addr;
	nfc->dev = dev;

	meson_nfc_clk_init(nfc);

	ret = meson_nfc_nand_chips_init(dev, nfc);
	if (ret) {
		dev_err(nfc->dev, "failed to init chips\n");
		return ret;
	}

	return 0;
}

static const struct udevice_id meson_nand_dt_ids[] = {
	{.compatible = "amlogic,meson-axg-nfc",},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(meson_nand) = {
	.name = "meson_nand",
	.id = UCLASS_MTD,
	.of_match = meson_nand_dt_ids,
	.probe = meson_probe,
	.priv_auto = sizeof(struct meson_nfc),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(meson_nand), &dev);

	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize: %d\n", ret);
}
