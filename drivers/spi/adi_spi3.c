// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Converted to driver model by Nathan Barrett-Morrison
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 * Contact: Ian Roberts <ian.roberts@timesys.com>
 * Contact: Piotr Wojtaszczyk <piotr.wojtaszczyk@timesys.com>
 *
 */

#include <clk.h>
#include <dm.h>
#include <mapmem.h>
#include <spi.h>
#include <spi-mem.h>
#include <dm/device_compat.h>
#include <linux/io.h>

#define SPI_IDLE_VAL	0xff

#define MAX_CTRL_CS 7

/* SPI_CONTROL */
#define SPI_CTL_EN          0x00000001 /* Enable */
#define SPI_CTL_MSTR        0x00000002 /* Master/Slave */
#define SPI_CTL_PSSE        0x00000004 /* controls modf error in master mode */
#define SPI_CTL_ODM         0x00000008 /* Open Drain Mode */
#define SPI_CTL_CPHA        0x00000010 /* Clock Phase */
#define SPI_CTL_CPOL        0x00000020 /* Clock Polarity */
#define SPI_CTL_ASSEL       0x00000040 /* Slave Select Pin Control */
#define SPI_CTL_SELST       0x00000080 /* Slave Select Polarity in transfers */
#define SPI_CTL_EMISO       0x00000100 /*Enable MISO */
#define SPI_CTL_SIZE        0x00000600 /*Word Transfer Size */
#define SPI_CTL_SIZE08      0x00000000 /*SIZE: 8 bits */
#define SPI_CTL_SIZE16      0x00000200 /*SIZE: 16 bits */
#define SPI_CTL_SIZE32      0x00000400 /*SIZE: 32 bits */
#define SPI_CTL_LSBF        0x00001000 /*LSB First */
#define SPI_CTL_FCEN        0x00002000 /*Flow-Control Enable */
#define SPI_CTL_FCCH        0x00004000 /*Flow-Control Channel Selection */
#define SPI_CTL_FCPL        0x00008000 /*Flow-Control Polarity */
#define SPI_CTL_FCWM        0x00030000 /*Flow-Control Water-Mark */
#define SPI_CTL_FIFO0       0x00000000 /*FCWM: Tx empty or Rx Full */
#define SPI_CTL_FIFO1       0x00010000 /*FCWM: Tx empty or Rx full (>=75%) */
#define SPI_CTL_FIFO2       0x00020000 /*FCWM: Tx empty or Rx full (>=50%) */
#define SPI_CTL_FMODE       0x00040000 /*Fast-mode Enable */
#define SPI_CTL_MIOM        0x00300000 /*Multiple I/O Mode */
#define SPI_CTL_MIO_DIS     0x00000000 /*MIOM: Disable */
#define SPI_CTL_MIO_DUAL    0x00100000 /*MIOM: Enable DIOM (Dual I/O Mode) */
#define SPI_CTL_MIO_QUAD    0x00200000 /*MIOM: Enable QUAD (Quad SPI Mode) */
#define SPI_CTL_SOSI        0x00400000 /*Start on MOSI */
#define SPI_CTL_MMWEM       0x40000000 /*Start on MMWEM */
#define SPI_CTL_MMSE        0x80000000 /*Start on MMSE */
/* SPI_RX_CONTROL */
#define SPI_RXCTL_REN       0x00000001 /*Receive Channel Enable */
#define SPI_RXCTL_RTI       0x00000004 /*Receive Transfer Initiate */
#define SPI_RXCTL_RWCEN     0x00000008 /*Receive Word Counter Enable */
#define SPI_RXCTL_RDR       0x00000070 /*Receive Data Request */
#define SPI_RXCTL_RDR_DIS   0x00000000 /*RDR: Disabled */
#define SPI_RXCTL_RDR_NE    0x00000010 /*RDR: RFIFO not empty */
#define SPI_RXCTL_RDR_25    0x00000020 /*RDR: RFIFO 25% full */
#define SPI_RXCTL_RDR_50    0x00000030 /*RDR: RFIFO 50% full */
#define SPI_RXCTL_RDR_75    0x00000040 /*RDR: RFIFO 75% full */
#define SPI_RXCTL_RDR_FULL  0x00000050 /*RDR: RFIFO full */
#define SPI_RXCTL_RDO       0x00000100 /*Receive Data Over-Run */
#define SPI_RXCTL_RRWM      0x00003000 /*FIFO Regular Water-Mark */
#define SPI_RXCTL_RWM_0     0x00000000 /*RRWM: RFIFO Empty */
#define SPI_RXCTL_RWM_25    0x00001000 /*RRWM: RFIFO 25% full */
#define SPI_RXCTL_RWM_50    0x00002000 /*RRWM: RFIFO 50% full */
#define SPI_RXCTL_RWM_75    0x00003000 /*RRWM: RFIFO 75% full */
#define SPI_RXCTL_RUWM      0x00070000 /*FIFO Urgent Water-Mark */
#define SPI_RXCTL_UWM_DIS   0x00000000 /*RUWM: Disabled */
#define SPI_RXCTL_UWM_25    0x00010000 /*RUWM: RFIFO 25% full */
#define SPI_RXCTL_UWM_50    0x00020000 /*RUWM: RFIFO 50% full */
#define SPI_RXCTL_UWM_75    0x00030000 /*RUWM: RFIFO 75% full */
#define SPI_RXCTL_UWM_FULL  0x00040000 /*RUWM: RFIFO full */
/* SPI_TX_CONTROL */
#define SPI_TXCTL_TEN       0x00000001 /*Transmit Channel Enable */
#define SPI_TXCTL_TTI       0x00000004 /*Transmit Transfer Initiate */
#define SPI_TXCTL_TWCEN     0x00000008 /*Transmit Word Counter Enable */
#define SPI_TXCTL_TDR       0x00000070 /*Transmit Data Request */
#define SPI_TXCTL_TDR_DIS   0x00000000 /*TDR: Disabled */
#define SPI_TXCTL_TDR_NF    0x00000010 /*TDR: TFIFO not full */
#define SPI_TXCTL_TDR_25    0x00000020 /*TDR: TFIFO 25% empty */
#define SPI_TXCTL_TDR_50    0x00000030 /*TDR: TFIFO 50% empty */
#define SPI_TXCTL_TDR_75    0x00000040 /*TDR: TFIFO 75% empty */
#define SPI_TXCTL_TDR_EMPTY 0x00000050 /*TDR: TFIFO empty */
#define SPI_TXCTL_TDU       0x00000100 /*Transmit Data Under-Run */
#define SPI_TXCTL_TRWM      0x00003000 /*FIFO Regular Water-Mark */
#define SPI_TXCTL_RWM_FULL  0x00000000 /*TRWM: TFIFO full */
#define SPI_TXCTL_RWM_25    0x00001000 /*TRWM: TFIFO 25% empty */
#define SPI_TXCTL_RWM_50    0x00002000 /*TRWM: TFIFO 50% empty */
#define SPI_TXCTL_RWM_75    0x00003000 /*TRWM: TFIFO 75% empty */
#define SPI_TXCTL_TUWM      0x00070000 /*FIFO Urgent Water-Mark */
#define SPI_TXCTL_UWM_DIS   0x00000000 /*TUWM: Disabled */
#define SPI_TXCTL_UWM_25    0x00010000 /*TUWM: TFIFO 25% empty */
#define SPI_TXCTL_UWM_50    0x00020000 /*TUWM: TFIFO 50% empty */
#define SPI_TXCTL_UWM_75    0x00030000 /*TUWM: TFIFO 75% empty */
#define SPI_TXCTL_UWM_EMPTY 0x00040000 /*TUWM: TFIFO empty */
/* SPI_CLOCK */
#define SPI_CLK_BAUD        0x0000FFFF /*Baud Rate */
/* SPI_DELAY */
#define SPI_DLY_STOP        0x000000FF /*Transfer delay time */
#define SPI_DLY_LEADX       0x00000100 /*Extended (1 SCK) LEAD Control */
#define SPI_DLY_LAGX        0x00000200 /*Extended (1 SCK) LAG control */
/* SPI_SSEL */
#define SPI_SLVSEL_SSE1     0x00000002 /*SPISSEL1 Enable */
#define SPI_SLVSEL_SSE2     0x00000004 /*SPISSEL2 Enable */
#define SPI_SLVSEL_SSE3     0x00000008 /*SPISSEL3 Enable */
#define SPI_SLVSEL_SSE4     0x00000010 /*SPISSEL4 Enable */
#define SPI_SLVSEL_SSE5     0x00000020 /*SPISSEL5 Enable */
#define SPI_SLVSEL_SSE6     0x00000040 /*SPISSEL6 Enable */
#define SPI_SLVSEL_SSE7     0x00000080 /*SPISSEL7 Enable */
#define SPI_SLVSEL_SSEL1    0x00000200 /*SPISSEL1 Value */
#define SPI_SLVSEL_SSEL2    0x00000400 /*SPISSEL2 Value */
#define SPI_SLVSEL_SSEL3    0x00000800 /*SPISSEL3 Value */
#define SPI_SLVSEL_SSEL4    0x00001000 /*SPISSEL4 Value */
#define SPI_SLVSEL_SSEL5    0x00002000 /*SPISSEL5 Value */
#define SPI_SLVSEL_SSEL6    0x00004000 /*SPISSEL6 Value */
#define SPI_SLVSEL_SSEL7    0x00008000 /*SPISSEL7 Value */
/* SPI_RWC */
#define SPI_RWC_VALUE       0x0000FFFF /*Received Word-Count */
/* SPI_RWCR */
#define SPI_RWCR_VALUE      0x0000FFFF /*Received Word-Count Reload */
/* SPI_TWC */
#define SPI_TWC_VALUE       0x0000FFFF /*Transmitted Word-Count */
/* SPI_TWCR */
#define SPI_TWCR_VALUE      0x0000FFFF /*Transmitted Word-Count Reload */
/* SPI_IMASK */
#define SPI_IMSK_RUWM       0x00000002 /*Receive Water-Mark Interrupt Mask */
#define SPI_IMSK_TUWM       0x00000004 /*Transmit Water-Mark Interrupt Mask */
#define SPI_IMSK_ROM        0x00000010 /*Receive Over-Run Interrupt Mask */
#define SPI_IMSK_TUM        0x00000020 /*Transmit Under-Run Interrupt Mask */
#define SPI_IMSK_TCM        0x00000040 /*Transmit Collision Interrupt Mask */
#define SPI_IMSK_MFM        0x00000080 /*Mode Fault Interrupt Mask */
#define SPI_IMSK_RSM        0x00000100 /*Receive Start Interrupt Mask */
#define SPI_IMSK_TSM        0x00000200 /*Transmit Start Interrupt Mask */
#define SPI_IMSK_RFM        0x00000400 /*Receive Finish Interrupt Mask */
#define SPI_IMSK_TFM        0x00000800 /*Transmit Finish Interrupt Mask */
/* SPI_IMASKCL */
#define SPI_IMSK_CLR_RUW    0x00000002 /*Receive Water-Mark Interrupt Mask */
#define SPI_IMSK_CLR_TUWM   0x00000004 /*Transmit Water-Mark Interrupt Mask */
#define SPI_IMSK_CLR_ROM    0x00000010 /*Receive Over-Run Interrupt Mask */
#define SPI_IMSK_CLR_TUM    0x00000020 /*Transmit Under-Run Interrupt Mask */
#define SPI_IMSK_CLR_TCM    0x00000040 /*Transmit Collision Interrupt Mask */
#define SPI_IMSK_CLR_MFM    0x00000080 /*Mode Fault Interrupt Mask */
#define SPI_IMSK_CLR_RSM    0x00000100 /*Receive Start Interrupt Mask */
#define SPI_IMSK_CLR_TSM    0x00000200 /*Transmit Start Interrupt Mask */
#define SPI_IMSK_CLR_RFM    0x00000400 /*Receive Finish Interrupt Mask */
#define SPI_IMSK_CLR_TFM    0x00000800 /*Transmit Finish Interrupt Mask */
/* SPI_IMASKST */
#define SPI_IMSK_SET_RUWM   0x00000002 /*Receive Water-Mark Interrupt Mask */
#define SPI_IMSK_SET_TUWM   0x00000004 /*Transmit Water-Mark Interrupt Mask */
#define SPI_IMSK_SET_ROM    0x00000010 /*Receive Over-Run Interrupt Mask */
#define SPI_IMSK_SET_TUM    0x00000020 /*Transmit Under-Run Interrupt Mask */
#define SPI_IMSK_SET_TCM    0x00000040 /*Transmit Collision Interrupt Mask */
#define SPI_IMSK_SET_MFM    0x00000080 /*Mode Fault Interrupt Mask */
#define SPI_IMSK_SET_RSM    0x00000100 /*Receive Start Interrupt Mask */
#define SPI_IMSK_SET_TSM    0x00000200 /*Transmit Start Interrupt Mask */
#define SPI_IMSK_SET_RFM    0x00000400 /*Receive Finish Interrupt Mask */
#define SPI_IMSK_SET_TFM    0x00000800 /*Transmit Finish Interrupt Mask */
/* SPI_STATUS */
#define SPI_STAT_SPIF       0x00000001 /*SPI Finished */
#define SPI_STAT_RUWM       0x00000002 /*Receive Water-Mark Breached */
#define SPI_STAT_TUWM       0x00000004 /*Transmit Water-Mark Breached */
#define SPI_STAT_ROE        0x00000010 /*Receive Over-Run Indication */
#define SPI_STAT_TUE        0x00000020 /*Transmit Under-Run Indication */
#define SPI_STAT_TCE        0x00000040 /*Transmit Collision Indication */
#define SPI_STAT_MODF       0x00000080 /*Mode Fault Indication */
#define SPI_STAT_RS         0x00000100 /*Receive Start Indication */
#define SPI_STAT_TS         0x00000200 /*Transmit Start Indication */
#define SPI_STAT_RF         0x00000400 /*Receive Finish Indication */
#define SPI_STAT_TF         0x00000800 /*Transmit Finish Indication */
#define SPI_STAT_RFS        0x00007000 /*SPI_RFIFO status */
#define SPI_STAT_RFIFO_EMPTY 0x00000000 /*RFS: RFIFO Empty */
#define SPI_STAT_RFIFO_25   0x00001000 /*RFS: RFIFO 25% Full */
#define SPI_STAT_RFIFO_50   0x00002000 /*RFS: RFIFO 50% Full */
#define SPI_STAT_RFIFO_75   0x00003000 /*RFS: RFIFO 75% Full */
#define SPI_STAT_RFIFO_FULL 0x00004000 /*RFS: RFIFO Full */
#define SPI_STAT_TFS        0x00070000 /*SPI_TFIFO status */
#define SPI_STAT_TFIFO_FULL 0x00000000 /*TFS: TFIFO full */
#define SPI_STAT_TFIFO_25   0x00010000 /*TFS: TFIFO 25% empty */
#define SPI_STAT_TFIFO_50   0x00020000 /*TFS: TFIFO 50% empty */
#define SPI_STAT_TFIFO_75   0x00030000 /*TFS: TFIFO 75% empty */
#define SPI_STAT_TFIFO_EMPTY 0x00040000 /*TFS: TFIFO empty */
#define SPI_STAT_FCS        0x00100000 /*Flow-Control Stall Indication */
#define SPI_STAT_RFE        0x00400000 /*SPI_RFIFO Empty */
#define SPI_STAT_TFF        0x00800000 /*SPI_TFIFO Full */
/* SPI_ILAT */
#define SPI_ILAT_RUWMI      0x00000002 /*Receive Water Mark Interrupt */
#define SPI_ILAT_TUWMI      0x00000004 /*Transmit Water Mark Interrupt */
#define SPI_ILAT_ROI        0x00000010 /*Receive Over-Run Indication */
#define SPI_ILAT_TUI        0x00000020 /*Transmit Under-Run Indication */
#define SPI_ILAT_TCI        0x00000040 /*Transmit Collision Indication */
#define SPI_ILAT_MFI        0x00000080 /*Mode Fault Indication */
#define SPI_ILAT_RSI        0x00000100 /*Receive Start Indication */
#define SPI_ILAT_TSI        0x00000200 /*Transmit Start Indication */
#define SPI_ILAT_RFI        0x00000400 /*Receive Finish Indication */
#define SPI_ILAT_TFI        0x00000800 /*Transmit Finish Indication */
/* SPI_ILATCL */
#define SPI_ILAT_CLR_RUWMI  0x00000002 /*Receive Water Mark Interrupt */
#define SPI_ILAT_CLR_TUWMI  0x00000004 /*Transmit Water Mark Interrupt */
#define SPI_ILAT_CLR_ROI    0x00000010 /*Receive Over-Run Indication */
#define SPI_ILAT_CLR_TUI    0x00000020 /*Transmit Under-Run Indication */
#define SPI_ILAT_CLR_TCI    0x00000040 /*Transmit Collision Indication */
#define SPI_ILAT_CLR_MFI    0x00000080 /*Mode Fault Indication */
#define SPI_ILAT_CLR_RSI    0x00000100 /*Receive Start Indication */
#define SPI_ILAT_CLR_TSI    0x00000200 /*Transmit Start Indication */
#define SPI_ILAT_CLR_RFI    0x00000400 /*Receive Finish Indication */
#define SPI_ILAT_CLR_TFI    0x00000800 /*Transmit Finish Indication */
/* SPI_MMRDH */
#define SPI_MMRDH_MERGE     0x04000000 /*Merge Enable */
#define SPI_MMRDH_DMY_SZ    0x00007000 /*Bytes of Dummy */
#define SPI_MMRDH_ADDR_PINS 0x00000800 /*Pins used for Address */
#define SPI_MMRDH_ADDR_SZ   0x00000700 /*Bytes of Read Address */
#define SPI_MMRDH_OPCODE    0x000000FF /*Read Opcode */

#define SPI_MMRDH_TRIDMY_OFF	24 /*Bytes of Dummy offset */
#define SPI_MMRDH_DMY_SZ_OFF	12 /*Bytes of Dummy offset */
#define SPI_MMRDH_ADDR_SZ_OFF	8  /*Bytes of Read Address offset */

#define BIT_SSEL_VAL(x) ((1 << 8) << (x)) /* Slave Select input value bit */
#define BIT_SSEL_EN(x) (1 << (x))         /* Slave Select enable bit*/

struct adi_spi_regs {
	u32 revid;
	u32 control;
	u32 rx_control;
	u32 tx_control;
	u32 clock;
	u32 delay;
	u32 ssel;
	u32 rwc;
	u32 rwcr;
	u32 twc;
	u32 twcr;
	u32 reserved0;
	u32 emask;
	u32 emaskcl;
	u32 emaskst;
	u32 reserved1;
	u32 status;
	u32 elat;
	u32 elatcl;
	u32 reserved2;
	u32 rfifo;
	u32 reserved3;
	u32 tfifo;
	u32 reserved4;
	u32 mmrdh;
	u32 mmtop;
};

struct adi_spi_platdata {
	u32 max_hz;
	u32 bus_num;
	struct adi_spi_regs __iomem *regs;
};

struct adi_spi_priv {
	u32 control;
	u32 clock;
	u32 bus_num;
	u32 max_cs;
	struct adi_spi_regs __iomem *regs;
};

/**
 * By convention, this driver uses the same CS numbering that is used with the SSEL bit
 * definitions (both here and in the TRM on which this is based), which are 1-indexed not
 * 0-indexed. The valid CS range is therefore [1,max_cs], in contrast with other drivers
 * where it is [0,max_cs-1].
 */
static int adi_spi_cs_info(struct udevice *bus, uint cs,
			   struct spi_cs_info *info)
{
	struct adi_spi_priv *priv = dev_get_priv(bus);

	if (cs == 0 || cs > priv->max_cs) {
		dev_err(bus, "invalid chipselect %u\n", cs);
		return -EINVAL;
	}

	return 0;
}

static int adi_spi_of_to_plat(struct udevice *bus)
{
	struct adi_spi_platdata *plat = dev_get_plat(bus);
	fdt_addr_t addr;

	plat->max_hz = dev_read_u32_default(bus, "spi-max-frequency", 500000);
	plat->bus_num = dev_read_u32_default(bus, "bus-num", 0);
	addr = dev_read_addr(bus);

	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->regs = map_sysmem(addr, sizeof(*plat->regs));

	return 0;
}

static int adi_spi_probe(struct udevice *bus)
{
	struct adi_spi_platdata *plat = dev_get_plat(bus);
	struct adi_spi_priv *priv = dev_get_priv(bus);

	priv->bus_num = plat->bus_num;
	priv->regs = plat->regs;
	priv->max_cs = dev_read_u32_default(bus, "num-cs", MAX_CTRL_CS);

	iowrite32(0x0, &plat->regs->control);
	iowrite32(0x0, &plat->regs->rx_control);
	iowrite32(0x0, &plat->regs->tx_control);

	return 0;
}

static int adi_spi_remove(struct udevice *dev)
{
	return -ENODEV;
}

static int adi_spi_claim_bus(struct udevice *dev)
{
	struct adi_spi_priv *priv;
	struct udevice *bus = dev->parent;

	priv = dev_get_priv(bus);

	debug("%s: control:%i clock:%i\n",
	      __func__, priv->control, priv->clock);

	iowrite32(priv->control, &priv->regs->control);
	iowrite32(priv->clock, &priv->regs->clock);
	iowrite32(0x0, &priv->regs->delay);

	return 0;
}

static int adi_spi_release_bus(struct udevice *dev)
{
	struct adi_spi_priv *priv;
	struct udevice *bus = dev->parent;

	priv = dev_get_priv(bus);

	debug("%s: control:%i clock:%i\n",
	      __func__, priv->control, priv->clock);

	iowrite32(0x0, &priv->regs->rx_control);
	iowrite32(0x0, &priv->regs->tx_control);
	iowrite32(0x0, &priv->regs->control);

	return 0;
}

void adi_spi_enable_ssel(struct adi_spi_priv *priv, int cs)
{
	setbits_32(&priv->regs->ssel, BIT_SSEL_EN(cs));
}

void adi_spi_set_ssel(struct adi_spi_priv *priv, int cs, int high)
{
	if (high)
		setbits_32(&priv->regs->ssel, BIT_SSEL_VAL(cs));
	else
		clrbits_32(&priv->regs->ssel, BIT_SSEL_VAL(cs));
}

void adi_spi_cs_activate(struct adi_spi_priv *priv, struct dm_spi_slave_plat *slave_plat)
{
	bool high = slave_plat->mode & SPI_CS_HIGH;

	adi_spi_set_ssel(priv, slave_plat->cs[0], high);
	adi_spi_enable_ssel(priv, slave_plat->cs[0]);
}

void adi_spi_cs_deactivate(struct adi_spi_priv *priv, struct dm_spi_slave_plat *slave_plat)
{
	bool high = slave_plat->mode & SPI_CS_HIGH;

	/* invert CS for matching SSEL to deactivate */
	adi_spi_set_ssel(priv, slave_plat->cs[0], !high);
}

static void discard_rx_fifo_contents(struct adi_spi_regs *regs)
{
	while (!(ioread32(&regs->status) & SPI_STAT_RFE))
		ioread32(&regs->rfifo);
}

static int adi_spi_fifo_mio_xfer(struct adi_spi_priv *priv, const u8 *tx, u8 *rx,
				 uint bytes, uint32_t mio_mode)
{
	u8 value;

	/* switch current SPI transfer to mio SPI mode */
	clrsetbits_32(&priv->regs->control, SPI_CTL_SOSI, mio_mode);
	/*
	 * Data can only be transferred in one direction in multi-io SPI
	 * modes, trigger the transfer in respective direction.
	 */
	if (rx) {
		iowrite32(0x0, &priv->regs->tx_control);
		iowrite32(SPI_RXCTL_REN | SPI_RXCTL_RTI, &priv->regs->rx_control);

		while (bytes--) {
			while (ioread32(&priv->regs->status) &
				SPI_STAT_RFE)
				if (ctrlc())
					return -1;
			value = ioread32(&priv->regs->rfifo);
			*rx++ = value;
		}
	} else if (tx) {
		iowrite32(0x0, &priv->regs->rx_control);
		iowrite32(SPI_TXCTL_TEN | SPI_TXCTL_TTI, &priv->regs->tx_control);

		while (bytes--) {
			value = *tx++;
			iowrite32(value, &priv->regs->tfifo);
			while (ioread32(&priv->regs->status) &
				SPI_STAT_TFF)
				if (ctrlc())
					return -1;
		}

		/* Wait till the tfifo is empty */
		while ((ioread32(&priv->regs->status) & SPI_STAT_TFS) != SPI_STAT_TFIFO_EMPTY)
			if (ctrlc())
				return -1;
	} else {
		return -1;
	}
	return 0;
}

static int adi_spi_fifo_1x_xfer(struct adi_spi_priv *priv, const u8 *tx, u8 *rx,
				uint bytes)
{
	u8 value;

	/*
	 * Set current SPI transfer in normal mode and trigger
	 * the bi-direction transfer by tx write operation.
	 */
	iowrite32(priv->control, &priv->regs->control);
	iowrite32(SPI_RXCTL_REN, &priv->regs->rx_control);
	iowrite32(SPI_TXCTL_TEN | SPI_TXCTL_TTI, &priv->regs->tx_control);

	while (bytes--) {
		value = (tx ? *tx++ : SPI_IDLE_VAL);
		debug("%s: tx:%x ", __func__, value);
		iowrite32(value, &priv->regs->tfifo);
		while (ioread32(&priv->regs->status) & SPI_STAT_RFE)
			if (ctrlc())
				return -1;
		value = ioread32(&priv->regs->rfifo);
		if (rx)
			*rx++ = value;
		debug("rx:%x\n", value);
	}
	return 0;
}

static int adi_spi_fifo_xfer(struct adi_spi_priv *priv, int buswidth,
			     const u8 *tx, u8 *rx, uint bytes)
{
	switch (buswidth) {
	case 1:
		return adi_spi_fifo_1x_xfer(priv, tx, rx, bytes);
	case 2:
		return adi_spi_fifo_mio_xfer(priv, tx, rx, bytes, SPI_CTL_MIO_DUAL);
	case 4:
		return adi_spi_fifo_mio_xfer(priv, tx, rx, bytes, SPI_CTL_MIO_QUAD);
	default:
		return -ENOTSUPP;
	}
}

static int adi_spi_xfer(struct udevice *dev, unsigned int bitlen,
			const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct adi_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);

	const u8 *tx = dout;
	u8 *rx = din;
	uint bytes = bitlen / 8;
	int ret = 0;

	debug("%s: bus_num:%i cs:%i bitlen:%i bytes:%i flags:%lx\n", __func__,
	      priv->bus_num, slave_plat->cs[0], bitlen, bytes, flags);

	if (flags & SPI_XFER_BEGIN)
		adi_spi_cs_activate(priv, slave_plat);

	if (bitlen == 0)
		goto done;

	/* we can only do 8 bit transfers */
	if (bitlen % 8) {
		flags |= SPI_XFER_END;
		goto done;
	}

	/* Discard invalid rx data and empty rfifo */
	discard_rx_fifo_contents(priv->regs);

	ret = adi_spi_fifo_1x_xfer(priv, tx, rx, bytes);

 done:
	if (flags & SPI_XFER_END)
		adi_spi_cs_deactivate(priv, slave_plat);

	return ret;
}

static int adi_spi_set_speed(struct udevice *bus, uint speed)
{
	struct adi_spi_platdata *plat = dev_get_plat(bus);
	struct adi_spi_priv *priv = dev_get_priv(bus);
	int ret;
	u32 clock, spi_base_clk;
	struct clk spi_clk;

	ret = clk_get_by_name(bus, "spi", &spi_clk);
	if (ret < 0) {
		dev_err(bus, "Can't get SPI clk: %d\n", ret);
		return ret;
	}
	spi_base_clk = clk_get_rate(&spi_clk);

	if (speed > plat->max_hz)
		speed = plat->max_hz;

	if (speed > spi_base_clk)
		return -ENODEV;

	clock = spi_base_clk / speed;
	if (clock)
		clock--;

	priv->clock = clock;

	debug("%s: priv->clock: %x, speed: %x, get_spi_clk(): %x\n",
	      __func__, clock, speed, spi_base_clk);

	return 0;
}

static int adi_spi_set_mode(struct udevice *bus, uint mode)
{
	struct adi_spi_priv *priv = dev_get_priv(bus);
	u32 reg;

	reg = SPI_CTL_EN | SPI_CTL_MSTR;
	if (mode & SPI_CPHA)
		reg |= SPI_CTL_CPHA;
	if (mode & SPI_CPOL)
		reg |= SPI_CTL_CPOL;
	if (mode & SPI_LSB_FIRST)
		reg |= SPI_CTL_LSBF;
	reg &= ~SPI_CTL_ASSEL;

	priv->control = reg;

	debug("%s: control=%d, cs_pol=%d\n", __func__, reg, mode & SPI_CS_HIGH ? 1 : 0);

	return 0;
}

/**
 * U-boot's version of spi-mem does not support mixed bus-width
 * commands nor anything more than 1x mode.
 * Using a custom exec_op implementation, we can support it.
 */
static int adi_spi_mem_exec_op(struct spi_slave *slave,
			       const struct spi_mem_op *op)
{
	int rv = 0;
	struct udevice *bus = slave->dev->parent;
	struct adi_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(slave->dev);
	u8 tmpbuf[64];
	int i;

	if ((op->cmd.nbytes + op->addr.nbytes + op->dummy.nbytes) >
	    sizeof(tmpbuf))
		return -ENOMEM;

	for (i = 0; i < op->cmd.nbytes; i++)
		tmpbuf[i] = op->cmd.opcode >>
				(8 * (op->cmd.nbytes - i - 1));
	for (i = 0; i < op->addr.nbytes; i++)
		tmpbuf[i + op->cmd.nbytes] = op->addr.val >>
				(8 * (op->addr.nbytes - i - 1));
	memset(tmpbuf + op->addr.nbytes + op->cmd.nbytes, 0xff,
	       op->dummy.nbytes);

	adi_spi_cs_activate(priv, slave_plat);
	discard_rx_fifo_contents(priv->regs);

	if (op->cmd.nbytes) {
		rv = adi_spi_fifo_xfer(priv, op->cmd.buswidth,
				       tmpbuf, NULL, op->cmd.nbytes);
		if (rv != 0)
			goto cleanup;
	}

	if (op->addr.nbytes) {
		rv = adi_spi_fifo_xfer(priv, op->addr.buswidth,
				       tmpbuf + op->cmd.nbytes, NULL,
				       op->addr.nbytes);
		if (rv != 0)
			goto cleanup;
	}

	if (op->dummy.nbytes) {
		rv = adi_spi_fifo_xfer(priv, op->dummy.buswidth,
				       tmpbuf + op->cmd.nbytes +
				       op->addr.nbytes,
				       NULL, op->dummy.nbytes);
		if (rv != 0)
			goto cleanup;
	}

	if (op->data.dir == SPI_MEM_DATA_IN)
		rv = adi_spi_fifo_xfer(priv, op->data.buswidth,
				       NULL, op->data.buf.in,
				       op->data.nbytes);
	else if (op->data.dir == SPI_MEM_DATA_OUT)
		rv = adi_spi_fifo_xfer(priv, op->data.buswidth,
				       op->data.buf.out, NULL,
				       op->data.nbytes);

cleanup:
	adi_spi_cs_deactivate(priv, slave_plat);
	return rv;
}

static const struct spi_controller_mem_ops adi_spi_mem_ops = {
	.exec_op = adi_spi_mem_exec_op,
};

static const struct dm_spi_ops adi_spi_ops = {
	.claim_bus = adi_spi_claim_bus,
	.release_bus = adi_spi_release_bus,
	.xfer = adi_spi_xfer,
	.set_speed = adi_spi_set_speed,
	.set_mode = adi_spi_set_mode,
	.cs_info = adi_spi_cs_info,
	.mem_ops = &adi_spi_mem_ops,
};

static const struct udevice_id adi_spi_ids[] = {
	{ .compatible = "adi,spi3" },
	{ }
};

U_BOOT_DRIVER(adi_spi3) = {
	.name = "adi_spi3",
	.id = UCLASS_SPI,
	.of_match = adi_spi_ids,
	.ops = &adi_spi_ops,
	.of_to_plat = adi_spi_of_to_plat,
	.probe = adi_spi_probe,
	.remove = adi_spi_remove,
	.plat_auto = sizeof(struct adi_spi_platdata),
	.priv_auto = sizeof(struct adi_spi_priv),
	.per_child_auto = sizeof(struct spi_slave),
};
