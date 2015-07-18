/*
 * LPC32xx SLC NAND flash controller driver
 *
 * (C) Copyright 2015 Vladimir Zapolskiy <vz@mleia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <nand.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/sys_proto.h>

struct lpc32xx_nand_slc_regs {
	u32 data;
	u32 addr;
	u32 cmd;
	u32 stop;
	u32 ctrl;
	u32 cfg;
	u32 stat;
	u32 int_stat;
	u32 ien;
	u32 isr;
	u32 icr;
	u32 tac;
	u32 tc;
	u32 ecc;
	u32 dma_data;
};

/* CFG register */
#define CFG_CE_LOW		(1 << 5)

/* CTRL register */
#define CTRL_SW_RESET		(1 << 2)

/* STAT register */
#define STAT_NAND_READY		(1 << 0)

/* INT_STAT register */
#define INT_STAT_TC		(1 << 1)
#define INT_STAT_RDY		(1 << 0)

/* TAC register bits, be aware of overflows */
#define TAC_W_RDY(n)		(max_t(uint32_t, (n), 0xF) << 28)
#define TAC_W_WIDTH(n)		(max_t(uint32_t, (n), 0xF) << 24)
#define TAC_W_HOLD(n)		(max_t(uint32_t, (n), 0xF) << 20)
#define TAC_W_SETUP(n)		(max_t(uint32_t, (n), 0xF) << 16)
#define TAC_R_RDY(n)		(max_t(uint32_t, (n), 0xF) << 12)
#define TAC_R_WIDTH(n)		(max_t(uint32_t, (n), 0xF) << 8)
#define TAC_R_HOLD(n)		(max_t(uint32_t, (n), 0xF) << 4)
#define TAC_R_SETUP(n)		(max_t(uint32_t, (n), 0xF) << 0)

static struct lpc32xx_nand_slc_regs __iomem *lpc32xx_nand_slc_regs
	= (struct lpc32xx_nand_slc_regs __iomem *)SLC_NAND_BASE;

static void lpc32xx_nand_init(void)
{
	uint32_t hclk = get_hclk_clk_rate();

	/* Reset SLC NAND controller */
	writel(CTRL_SW_RESET, &lpc32xx_nand_slc_regs->ctrl);

	/* 8-bit bus, no DMA, no ECC, ordinary CE signal */
	writel(0, &lpc32xx_nand_slc_regs->cfg);

	/* Interrupts disabled and cleared */
	writel(0, &lpc32xx_nand_slc_regs->ien);
	writel(INT_STAT_TC | INT_STAT_RDY,
	       &lpc32xx_nand_slc_regs->icr);

	/* Configure NAND flash timings */
	writel(TAC_W_RDY(CONFIG_LPC32XX_NAND_SLC_WDR_CLKS) |
	       TAC_W_WIDTH(hclk / CONFIG_LPC32XX_NAND_SLC_WWIDTH) |
	       TAC_W_HOLD(hclk / CONFIG_LPC32XX_NAND_SLC_WHOLD) |
	       TAC_W_SETUP(hclk / CONFIG_LPC32XX_NAND_SLC_WSETUP) |
	       TAC_R_RDY(CONFIG_LPC32XX_NAND_SLC_RDR_CLKS) |
	       TAC_R_WIDTH(hclk / CONFIG_LPC32XX_NAND_SLC_RWIDTH) |
	       TAC_R_HOLD(hclk / CONFIG_LPC32XX_NAND_SLC_RHOLD) |
	       TAC_R_SETUP(hclk / CONFIG_LPC32XX_NAND_SLC_RSETUP),
	       &lpc32xx_nand_slc_regs->tac);
}

static void lpc32xx_nand_cmd_ctrl(struct mtd_info *mtd,
				  int cmd, unsigned int ctrl)
{
	debug("ctrl: 0x%08x, cmd: 0x%08x\n", ctrl, cmd);

	if (ctrl & NAND_NCE)
		setbits_le32(&lpc32xx_nand_slc_regs->cfg, CFG_CE_LOW);
	else
		clrbits_le32(&lpc32xx_nand_slc_regs->cfg, CFG_CE_LOW);

	if (cmd == NAND_CMD_NONE)
		return;

	if (ctrl & NAND_CLE)
		writel(cmd & 0xFF, &lpc32xx_nand_slc_regs->cmd);
	else if (ctrl & NAND_ALE)
		writel(cmd & 0xFF, &lpc32xx_nand_slc_regs->addr);
}

static int lpc32xx_nand_dev_ready(struct mtd_info *mtd)
{
	return readl(&lpc32xx_nand_slc_regs->stat) & STAT_NAND_READY;
}

static void lpc32xx_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	while (len-- > 0)
		*buf++ = readl(&lpc32xx_nand_slc_regs->data);
}

static uint8_t lpc32xx_read_byte(struct mtd_info *mtd)
{
	return readl(&lpc32xx_nand_slc_regs->data);
}

static void lpc32xx_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	while (len-- > 0)
		writel(*buf++, &lpc32xx_nand_slc_regs->data);
}

static void lpc32xx_write_byte(struct mtd_info *mtd, uint8_t byte)
{
	writel(byte, &lpc32xx_nand_slc_regs->data);
}

/*
 * LPC32xx has only one SLC NAND controller, don't utilize
 * CONFIG_SYS_NAND_SELF_INIT to be able to reuse this function
 * both in SPL NAND and U-boot images.
 */
int board_nand_init(struct nand_chip *lpc32xx_chip)
{
	lpc32xx_chip->cmd_ctrl  = lpc32xx_nand_cmd_ctrl;
	lpc32xx_chip->dev_ready = lpc32xx_nand_dev_ready;

	/*
	 * Hardware ECC calculation is not supported by the driver,
	 * because it requires DMA support, see LPC32x0 User Manual,
	 * note after SLC_ECC register description (UM10326, p.198)
	 */
	lpc32xx_chip->ecc.mode = NAND_ECC_SOFT;

	/*
	 * The implementation of these functions is quite common, but
	 * they MUST be defined, because access to data register
	 * is strictly 32-bit aligned.
	 */
	lpc32xx_chip->read_buf   = lpc32xx_read_buf;
	lpc32xx_chip->read_byte  = lpc32xx_read_byte;
	lpc32xx_chip->write_buf  = lpc32xx_write_buf;
	lpc32xx_chip->write_byte = lpc32xx_write_byte;

	/*
	 * Use default ECC layout, but these values are predefined
	 * for both small and large page NAND flash devices.
	 */
	lpc32xx_chip->ecc.size     = 256;
	lpc32xx_chip->ecc.bytes    = 3;
	lpc32xx_chip->ecc.strength = 1;

#if defined(CONFIG_SYS_NAND_USE_FLASH_BBT)
	lpc32xx_chip->bbt_options |= NAND_BBT_USE_FLASH;
#endif

	/* Initialize NAND interface */
	lpc32xx_nand_init();

	return 0;
}
