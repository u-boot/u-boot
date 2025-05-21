/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 */

#ifndef __DWMMC_HW_H
#define __DWMMC_HW_H

#include <asm/io.h>
#include <mmc.h>
#include <linux/bitops.h>

#define DWMCI_CTRL		0x000
#define	DWMCI_PWREN		0x004
#define DWMCI_CLKDIV		0x008
#define DWMCI_CLKSRC		0x00c
#define DWMCI_CLKENA		0x010
#define DWMCI_TMOUT		0x014
#define DWMCI_CTYPE		0x018
#define DWMCI_BLKSIZ		0x01c
#define DWMCI_BYTCNT		0x020
#define DWMCI_INTMASK		0x024
#define DWMCI_CMDARG		0x028
#define DWMCI_CMD		0x02c
#define DWMCI_RESP0		0x030
#define DWMCI_RESP1		0x034
#define DWMCI_RESP2		0x038
#define DWMCI_RESP3		0x03c
#define DWMCI_MINTSTS		0x040
#define DWMCI_RINTSTS		0x044
#define DWMCI_STATUS		0x048
#define DWMCI_FIFOTH		0x04c
#define DWMCI_CDETECT		0x050
#define DWMCI_WRTPRT		0x054
#define DWMCI_GPIO		0x058
#define DWMCI_TCMCNT		0x05c
#define DWMCI_TBBCNT		0x060
#define DWMCI_DEBNCE		0x064
#define DWMCI_USRID		0x068
#define DWMCI_VERID		0x06c
#define DWMCI_HCON		0x070
#define DWMCI_UHS_REG		0x074
#define DWMCI_BMOD		0x080
#define DWMCI_PLDMND		0x084
#define DWMCI_DATA		0x200
/* Registers to support IDMAC 32-bit address mode */
#define DWMCI_DBADDR		0x088
#define DWMCI_IDSTS		0x08c
#define DWMCI_IDINTEN		0x090
#define DWMCI_DSCADDR		0x094
#define DWMCI_BUFADDR		0x098
/* Registers to support IDMAC 64-bit address mode */
#define DWMCI_DBADDRL		0x088
#define DWMCI_DBADDRU		0x08c
#define DWMCI_IDSTS64		0x090
#define DWMCI_IDINTEN64		0x094
#define DWMCI_DSCADDRL		0x098
#define DWMCI_DSCADDRU		0x09c
#define DWMCI_BUFADDRL		0x0a0
#define DWMCI_BUFADDRU		0x0a4

/* Interrupt Mask register */
#define DWMCI_INTMSK_ALL	0xffffffff
#define DWMCI_INTMSK_RE		BIT(1)
#define DWMCI_INTMSK_CDONE	BIT(2)
#define DWMCI_INTMSK_DTO	BIT(3)
#define DWMCI_INTMSK_TXDR	BIT(4)
#define DWMCI_INTMSK_RXDR	BIT(5)
#define DWMCI_INTMSK_RCRC	BIT(6)
#define DWMCI_INTMSK_DCRC	BIT(7)
#define DWMCI_INTMSK_RTO	BIT(8)
#define DWMCI_INTMSK_DRTO	BIT(9)
#define DWMCI_INTMSK_HTO	BIT(10)
#define DWMCI_INTMSK_FRUN	BIT(11)
#define DWMCI_INTMSK_HLE	BIT(12)
#define DWMCI_INTMSK_SBE	BIT(13)
#define DWMCI_INTMSK_ACD	BIT(14)
#define DWMCI_INTMSK_EBE	BIT(15)

/* Raw interrupt register */
#define DWMCI_DATA_ERR		(DWMCI_INTMSK_EBE | DWMCI_INTMSK_SBE | \
				 DWMCI_INTMSK_HLE | DWMCI_INTMSK_FRUN | \
				 DWMCI_INTMSK_EBE | DWMCI_INTMSK_DCRC)
#define DWMCI_DATA_TOUT		(DWMCI_INTMSK_HTO | DWMCI_INTMSK_DRTO)

/* CTRL register */
#define DWMCI_CTRL_RESET	BIT(0)
#define DWMCI_CTRL_FIFO_RESET	BIT(1)
#define DWMCI_CTRL_DMA_RESET	BIT(2)
#define DWMCI_DMA_EN		BIT(5)
#define DWMCI_CTRL_SEND_AS_CCSD	BIT(10)
#define DWMCI_IDMAC_EN		BIT(25)
#define DWMCI_RESET_ALL		(DWMCI_CTRL_RESET | DWMCI_CTRL_FIFO_RESET |\
				DWMCI_CTRL_DMA_RESET)

/* CMD register */
#define DWMCI_CMD_RESP_EXP	BIT(6)
#define DWMCI_CMD_RESP_LENGTH	BIT(7)
#define DWMCI_CMD_CHECK_CRC	BIT(8)
#define DWMCI_CMD_DATA_EXP	BIT(9)
#define DWMCI_CMD_RW		BIT(10)
#define DWMCI_CMD_SEND_STOP	BIT(12)
#define DWMCI_CMD_ABORT_STOP	BIT(14)
#define DWMCI_CMD_PRV_DAT_WAIT	BIT(13)
#define DWMCI_CMD_UPD_CLK	BIT(21)
#define DWMCI_CMD_USE_HOLD_REG	BIT(29)
#define DWMCI_CMD_START		BIT(31)

/* CLKENA register */
#define DWMCI_CLKEN_ENABLE	BIT(0)
#define DWMCI_CLKEN_LOW_PWR	BIT(16)

/* Card type register */
#define DWMCI_CTYPE_1BIT	0
#define DWMCI_CTYPE_4BIT	BIT(0)
#define DWMCI_CTYPE_8BIT	BIT(16)

/* Status register */
#define DWMCI_FIFO_EMPTY	BIT(2)
#define DWMCI_FIFO_FULL		BIT(3)
#define DWMCI_BUSY		BIT(9)
#define DWMCI_FIFO_MASK		0x1fff
#define DWMCI_FIFO_SHIFT	17

/* FIFOTH register */
#define MSIZE(x)		((x) << 28)
#define RX_WMARK(x)		((x) << 16)
#define TX_WMARK(x)		(x)
#define RX_WMARK_SHIFT		16
#define RX_WMARK_MASK		(0xfff << RX_WMARK_SHIFT)

#define DWMCI_IDMAC_OWN		BIT(31)
#define DWMCI_IDMAC_CH		BIT(4)
#define DWMCI_IDMAC_FS		BIT(3)
#define DWMCI_IDMAC_LD		BIT(2)

/* Bus Mode register */
#define DWMCI_BMOD_IDMAC_RESET	BIT(0)
#define DWMCI_BMOD_IDMAC_FB	BIT(1)
#define DWMCI_BMOD_IDMAC_EN	BIT(7)

/* UHS register */
#define DWMCI_DDR_MODE		BIT(16)

/* Internal IDMAC interrupt defines */
#define DWMCI_IDINTEN_RI	BIT(1)
#define DWMCI_IDINTEN_TI	BIT(0)
#define DWMCI_IDINTEN_MASK	(DWMCI_IDINTEN_TI | DWMCI_IDINTEN_RI)

/**
 * struct dwmci_idmac_regs - Offsets of IDMAC registers
 *
 * @dbaddrl:	Descriptor base address, lower 32 bits
 * @dbaddru:	Descriptor base address, upper 32 bits
 * @idsts:	Internal DMA status
 * @idinten:	Internal DMA interrupt enable
 * @dscaddrl:	IDMAC descriptor address, lower 32 bits
 * @dscaddru:	IDMAC descriptor address, upper 32 bits
 * @bufaddrl:	Current data buffer address, lower 32 bits
 * @bufaddru:	Current data buffer address, upper 32 bits
 */
struct dwmci_idmac_regs {
	u32 dbaddrl;
	u32 dbaddru;
	u32 idsts;
	u32 idinten;
	u32 dscaddrl;
	u32 dscaddru;
	u32 bufaddrl;
	u32 bufaddru;
};

/**
 * struct dwmci_host - Information about a designware MMC host
 *
 * @name:	Device name
 * @ioaddr:	Base I/O address of controller
 * @caps:	Capabilities - see MMC_MODE_...
 * @clock:	Current clock frequency (after internal divider), Hz
 * @bus_hz:	Bus speed in Hz, if @get_mmc_clk() is NULL
 * @dev_index:	Arbitrary device index for use by controller
 * @dev_id:	Arbitrary device ID for use by controller
 * @buswidth:	Bus width in bits (8 or 4)
 * @fifo_depth:	Depth of FIFO, bytes (or 0 for automatic detection)
 * @mmc:	Pointer to generic MMC structure for this device
 * @priv:	Private pointer for use by controller
 * @clksel:	(Optional) Platform function to run when speed/width is changed
 * @board_init:	(Optional) Platform function to run on init
 * @cfg:	Internal MMC configuration, for !CONFIG_BLK cases
 * @fifo_mode:	Use FIFO mode (not DMA) to read and write data
 * @dma_64bit_address: Whether DMA supports 64-bit address mode or not
 * @regs:	Registers that can vary for different DW MMC block versions
 */
struct dwmci_host {
	const char *name;
	void *ioaddr;
	unsigned int caps;
	unsigned int clock;
	unsigned int bus_hz;
	int dev_index;
	int dev_id;
	int buswidth;
	u32 fifo_depth;
	struct mmc *mmc;
	void *priv;

	int (*clksel)(struct dwmci_host *host);
	void (*board_init)(struct dwmci_host *host);
	/**
	 * @get_mmc_clk: (Optional) Platform function to get/set a particular
	 * MMC clock frequency
	 *
	 * @host:	DWMMC host
	 * @freq:	Frequency the host is trying to achieve
	 *
	 * This is used to request the current clock frequency of the clock
	 * that drives the DWMMC peripheral. The caller will then use this
	 * information to work out the divider it needs to achieve the
	 * required MMC bus clock frequency. If you want to handle the
	 * clock external to DWMMC, use @freq to select the frequency and
	 * return that value too. Then DWMMC will put itself in bypass mode.
	 */
	unsigned int (*get_mmc_clk)(struct dwmci_host *host, uint freq);

#ifndef CONFIG_BLK
	struct mmc_config cfg;
#endif

	bool fifo_mode;
	bool dma_64bit_address;
	const struct dwmci_idmac_regs *regs;
};

static inline void dwmci_writel(struct dwmci_host *host, int reg, u32 val)
{
	writel(val, host->ioaddr + reg);
}

static inline void dwmci_writew(struct dwmci_host *host, int reg, u16 val)
{
	writew(val, host->ioaddr + reg);
}

static inline void dwmci_writeb(struct dwmci_host *host, int reg, u8 val)
{
	writeb(val, host->ioaddr + reg);
}

static inline u32 dwmci_readl(struct dwmci_host *host, int reg)
{
	return readl(host->ioaddr + reg);
}

static inline u16 dwmci_readw(struct dwmci_host *host, int reg)
{
	return readw(host->ioaddr + reg);
}

static inline u8 dwmci_readb(struct dwmci_host *host, int reg)
{
	return readb(host->ioaddr + reg);
}

#ifdef CONFIG_BLK

/**
 * dwmci_setup_cfg() - Set up the configuration for DWMMC
 * @cfg:	Configuration structure to fill in (generally &plat->mmc)
 * @host:	DWMMC host
 * @max_clk:	Maximum supported clock speed in Hz (e.g. 150000000)
 * @min_clk:	Minimum supported clock speed in Hz (e.g. 400000)
 *
 * This is used to set up a DWMMC device when you are using CONFIG_BLK.
 *
 * This should be called from your MMC driver's probe() method once you have
 * the information required.
 *
 * Generally your driver will have a platform data structure which holds both
 * the configuration (struct mmc_config) and the MMC device info (struct mmc).
 * For example:
 *
 * struct rockchip_mmc_plat {
 *	struct mmc_config cfg;
 *	struct mmc mmc;
 * };
 *
 * ...
 *
 * Inside U_BOOT_DRIVER():
 *	.plat_auto	= sizeof(struct rockchip_mmc_plat),
 *
 * To access platform data:
 *	struct rockchip_mmc_plat *plat = dev_get_plat(dev);
 *
 * See rockchip_dw_mmc.c for an example.
 */
void dwmci_setup_cfg(struct mmc_config *cfg, struct dwmci_host *host,
		     u32 max_clk, u32 min_clk);

/**
 * dwmci_bind() - Set up a new MMC block device
 * @dev:	Device to set up
 * @mmc:	Pointer to mmc structure (normally &plat->mmc)
 * @cfg:	Empty configuration structure (generally &plat->cfg). This is
 *		normally all zeroes at this point. The only purpose of passing
 *		this in is to set mmc->cfg to it.
 *
 * This is used to set up a DWMMC block device when you are using CONFIG_BLK.
 * It should be called from your driver's bind() method.
 *
 * See rockchip_dw_mmc.c for an example.
 *
 * Return: 0 if OK, -ve if the block device could not be created
 */
int dwmci_bind(struct udevice *dev, struct mmc *mmc, struct mmc_config *cfg);

#else

/**
 * add_dwmci() - Add a new DWMMC interface
 * @host:	DWMMC host structure
 * @max_clk:	Maximum supported clock speed in Hz (e.g. 150000000)
 * @min_clk:	Minimum supported clock speed in Hz (e.g. 400000)
 *
 * This is used when you are not using CONFIG_BLK. Convert your driver over!
 *
 * Return: 0 if OK, -ve on error
 */
int add_dwmci(struct dwmci_host *host, u32 max_clk, u32 min_clk);

#endif /* !CONFIG_BLK */

#ifdef CONFIG_DM_MMC
/* Export the operations to drivers */
int dwmci_probe(struct udevice *dev);
extern const struct dm_mmc_ops dm_dwmci_ops;
#endif

#endif	/* __DWMMC_HW_H */
