/*
 * Copyright (C) 2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <asm/processor.h>

#include "../soc-info.h"

#define MMC_CMD_SWITCH			6
#define MMC_CMD_SELECT_CARD		7
#define MMC_CMD_SEND_CSD		9
#define MMC_CMD_READ_MULTIPLE_BLOCK	18

#define EXT_CSD_PART_CONF		179	/* R/W */

#define MMC_RSP_PRESENT BIT(0)
#define MMC_RSP_136	BIT(1)		/* 136 bit response */
#define MMC_RSP_CRC	BIT(2)		/* expect valid crc */
#define MMC_RSP_BUSY	BIT(3)		/* card may send busy */
#define MMC_RSP_OPCODE	BIT(4)		/* response contains opcode */

#define MMC_RSP_NONE	(0)
#define MMC_RSP_R1	(MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)
#define MMC_RSP_R1b	(MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE | \
			MMC_RSP_BUSY)
#define MMC_RSP_R2	(MMC_RSP_PRESENT | MMC_RSP_136 | MMC_RSP_CRC)
#define MMC_RSP_R3	(MMC_RSP_PRESENT)
#define MMC_RSP_R4	(MMC_RSP_PRESENT)
#define MMC_RSP_R5	(MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)
#define MMC_RSP_R6	(MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)
#define MMC_RSP_R7	(MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE)

#define SDHCI_DMA_ADDRESS	0x00
#define SDHCI_BLOCK_SIZE	0x04
#define  SDHCI_MAKE_BLKSZ(dma, blksz) ((((dma) & 0x7) << 12) | ((blksz) & 0xFFF))
#define SDHCI_BLOCK_COUNT	0x06
#define SDHCI_ARGUMENT		0x08
#define SDHCI_TRANSFER_MODE	0x0C
#define  SDHCI_TRNS_DMA		BIT(0)
#define  SDHCI_TRNS_BLK_CNT_EN	BIT(1)
#define  SDHCI_TRNS_ACMD12	BIT(2)
#define  SDHCI_TRNS_READ	BIT(4)
#define  SDHCI_TRNS_MULTI	BIT(5)
#define SDHCI_COMMAND		0x0E
#define  SDHCI_CMD_RESP_MASK	0x03
#define  SDHCI_CMD_CRC		0x08
#define  SDHCI_CMD_INDEX	0x10
#define  SDHCI_CMD_DATA		0x20
#define  SDHCI_CMD_ABORTCMD	0xC0
#define  SDHCI_CMD_RESP_NONE	0x00
#define  SDHCI_CMD_RESP_LONG	0x01
#define  SDHCI_CMD_RESP_SHORT	0x02
#define  SDHCI_CMD_RESP_SHORT_BUSY 0x03
#define  SDHCI_MAKE_CMD(c, f) ((((c) & 0xff) << 8) | ((f) & 0xff))
#define SDHCI_RESPONSE		0x10
#define SDHCI_HOST_CONTROL	0x28
#define  SDHCI_CTRL_DMA_MASK	0x18
#define   SDHCI_CTRL_SDMA	0x00
#define SDHCI_BLOCK_GAP_CONTROL	0x2A
#define SDHCI_SOFTWARE_RESET	0x2F
#define  SDHCI_RESET_CMD	0x02
#define  SDHCI_RESET_DATA	0x04
#define SDHCI_INT_STATUS	0x30
#define  SDHCI_INT_RESPONSE	BIT(0)
#define  SDHCI_INT_DATA_END	BIT(1)
#define  SDHCI_INT_ERROR	BIT(15)
#define SDHCI_SIGNAL_ENABLE	0x38

/* RCA assigned by Boot ROM */
#define UNIPHIER_EMMC_RCA	0x1000

struct uniphier_mmc_cmd {
	unsigned int cmdidx;
	unsigned int resp_type;
	unsigned int cmdarg;
	unsigned int is_data;
};

static int uniphier_emmc_send_cmd(void __iomem *host_base,
				  struct uniphier_mmc_cmd *cmd)
{
	u32 mode = 0;
	u32 mask = SDHCI_INT_RESPONSE;
	u32 stat, flags;

	writel(U32_MAX, host_base + SDHCI_INT_STATUS);
	writel(0, host_base + SDHCI_SIGNAL_ENABLE);
	writel(cmd->cmdarg, host_base + SDHCI_ARGUMENT);

	if (cmd->is_data)
		mode = SDHCI_TRNS_DMA | SDHCI_TRNS_BLK_CNT_EN |
			SDHCI_TRNS_ACMD12 | SDHCI_TRNS_READ |
			SDHCI_TRNS_MULTI;

	writew(mode, host_base + SDHCI_TRANSFER_MODE);

	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags = SDHCI_CMD_RESP_NONE;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = SDHCI_CMD_RESP_LONG;
	else if (cmd->resp_type & MMC_RSP_BUSY)
		flags = SDHCI_CMD_RESP_SHORT_BUSY;
	else
		flags = SDHCI_CMD_RESP_SHORT;

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= SDHCI_CMD_CRC;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= SDHCI_CMD_INDEX;
	if (cmd->is_data)
		flags |= SDHCI_CMD_DATA;

	if (cmd->resp_type & MMC_RSP_BUSY || cmd->is_data)
		mask |= SDHCI_INT_DATA_END;

	writew(SDHCI_MAKE_CMD(cmd->cmdidx, flags), host_base + SDHCI_COMMAND);

	do {
		stat = readl(host_base + SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR)
			return -EIO;

	} while ((stat & mask) != mask);

	return 0;
}

static int uniphier_emmc_switch_part(void __iomem *host_base, int part_num)
{
	struct uniphier_mmc_cmd cmd = {};

	cmd.cmdidx = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = (EXT_CSD_PART_CONF << 16) | (part_num << 8) | (3 << 24);

	return uniphier_emmc_send_cmd(host_base, &cmd);
}

static int uniphier_emmc_is_over_2gb(void __iomem *host_base)
{
	struct uniphier_mmc_cmd cmd = {};
	u32 csd40, csd72;	/* CSD[71:40], CSD[103:72] */
	int ret;

	cmd.cmdidx = MMC_CMD_SEND_CSD;
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = UNIPHIER_EMMC_RCA << 16;

	ret = uniphier_emmc_send_cmd(host_base, &cmd);
	if (ret)
		return ret;

	csd40 = readl(host_base + SDHCI_RESPONSE + 4);
	csd72 = readl(host_base + SDHCI_RESPONSE + 8);

	return !(~csd40 & 0xffc00380) && !(~csd72 & 0x3);
}

static int uniphier_emmc_load_image(void __iomem *host_base, u32 dev_addr,
				    unsigned long load_addr, u32 block_cnt)
{
	struct uniphier_mmc_cmd cmd = {};
	u8 tmp;

	WARN_ON(load_addr >> 32);

	writel(load_addr, host_base + SDHCI_DMA_ADDRESS);
	writew(SDHCI_MAKE_BLKSZ(7, 512), host_base + SDHCI_BLOCK_SIZE);
	writew(block_cnt, host_base + SDHCI_BLOCK_COUNT);

	tmp = readb(host_base + SDHCI_HOST_CONTROL);
	tmp &= ~SDHCI_CTRL_DMA_MASK;
	tmp |= SDHCI_CTRL_SDMA;
	writeb(tmp, host_base + SDHCI_HOST_CONTROL);

	tmp = readb(host_base + SDHCI_BLOCK_GAP_CONTROL);
	tmp &= ~1;		/* clear Stop At Block Gap Request */
	writeb(tmp, host_base + SDHCI_BLOCK_GAP_CONTROL);

	cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = dev_addr;
	cmd.is_data = 1;

	return uniphier_emmc_send_cmd(host_base, &cmd);
}

static int spl_board_load_image(struct spl_image_info *spl_image,
				struct spl_boot_device *bootdev)
{
	u32 dev_addr = CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
	void __iomem *host_base = (void __iomem *)0x5a000200;
	struct uniphier_mmc_cmd cmd = {};
	int ret;

	/*
	 * deselect card before SEND_CSD command.
	 * Do not check the return code.  It fails, but it is OK.
	 */
	cmd.cmdidx = MMC_CMD_SELECT_CARD;
	cmd.resp_type = MMC_RSP_R1;

	uniphier_emmc_send_cmd(host_base, &cmd); /* CMD7 (arg=0) */

	/* reset CMD Line */
	writeb(SDHCI_RESET_CMD | SDHCI_RESET_DATA,
	       host_base + SDHCI_SOFTWARE_RESET);
	while (readb(host_base + SDHCI_SOFTWARE_RESET))
		cpu_relax();

	ret = uniphier_emmc_is_over_2gb(host_base);
	if (ret < 0)
		return ret;
	if (ret) {
		debug("card is block addressing\n");
	} else {
		debug("card is byte addressing\n");
		dev_addr *= 512;
	}

	cmd.cmdarg = UNIPHIER_EMMC_RCA << 16;

	/* select card again */
	ret = uniphier_emmc_send_cmd(host_base, &cmd);
	if (ret)
		printf("failed to select card\n");

	/* Switch to Boot Partition 1 */
	ret = uniphier_emmc_switch_part(host_base, 1);
	if (ret)
		printf("failed to switch partition\n");

	ret = uniphier_emmc_load_image(host_base, dev_addr,
				       CONFIG_SYS_TEXT_BASE, 1);
	if (ret) {
		printf("failed to load image\n");
		return ret;
	}

	ret = spl_parse_image_header(spl_image, (void *)CONFIG_SYS_TEXT_BASE);
	if (ret)
		return ret;

	ret = uniphier_emmc_load_image(host_base, dev_addr,
				       spl_image->load_addr,
				       spl_image->size / 512);
	if (ret) {
		printf("failed to load image\n");
		return ret;
	}

	return 0;
}
SPL_LOAD_IMAGE_METHOD("eMMC", 0, BOOT_DEVICE_BOARD, spl_board_load_image);
