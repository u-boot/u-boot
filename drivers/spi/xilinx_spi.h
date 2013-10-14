/*
 * Xilinx SPI driver
 *
 * XPS/AXI bus interface
 *
 * based on bfin_spi.c, by way of altera_spi.c
 * Copyright (c) 2005-2008 Analog Devices Inc.
 * Copyright (c) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Copyright (c) 2010 Graeme Smecher <graeme.smecher@mail.mcgill.ca>
 * Copyright (c) 2012 Stephan Linz <linz@li-pro.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [S]:	[0]/ip_documentation/xps_spi.pdf
 *	[0]/ip_documentation/axi_spi_ds742.pdf
 */
#ifndef _XILINX_SPI_
#define _XILINX_SPI_

#include <asm/types.h>
#include <asm/io.h>

/*
 * Xilinx SPI Register Definition
 *
 * [1]:	[0]/ip_documentation/xps_spi.pdf
 *	page 8, Register Descriptions
 * [2]:	[0]/ip_documentation/axi_spi_ds742.pdf
 *	page 7, Register Overview Table
 */
struct xilinx_spi_reg {
	u32 __space0__[7];
	u32 dgier;	/* Device Global Interrupt Enable Register (DGIER) */
	u32 ipisr;	/* IP Interrupt Status Register (IPISR) */
	u32 __space1__;
	u32 ipier;	/* IP Interrupt Enable Register (IPIER) */
	u32 __space2__[5];
	u32 srr;	/* Softare Reset Register (SRR) */
	u32 __space3__[7];
	u32 spicr;	/* SPI Control Register (SPICR) */
	u32 spisr;	/* SPI Status Register (SPISR) */
	u32 spidtr;	/* SPI Data Transmit Register (SPIDTR) */
	u32 spidrr;	/* SPI Data Receive Register (SPIDRR) */
	u32 spissr;	/* SPI Slave Select Register (SPISSR) */
	u32 spitfor;	/* SPI Transmit FIFO Occupancy Register (SPITFOR) */
	u32 spirfor;	/* SPI Receive FIFO Occupancy Register (SPIRFOR) */
};

/* Device Global Interrupt Enable Register (dgier), [1] p15, [2] p15 */
#define DGIER_GIE		(1 << 31)

/* IP Interrupt Status Register (ipisr), [1] p15, [2] p15 */
#define IPISR_DRR_NOT_EMPTY	(1 << 8)
#define IPISR_SLAVE_SELECT	(1 << 7)
#define IPISR_TXF_HALF_EMPTY	(1 << 6)
#define IPISR_DRR_OVERRUN	(1 << 5)
#define IPISR_DRR_FULL		(1 << 4)
#define IPISR_DTR_UNDERRUN	(1 << 3)
#define IPISR_DTR_EMPTY		(1 << 2)
#define IPISR_SLAVE_MODF	(1 << 1)
#define IPISR_MODF		(1 << 0)

/* IP Interrupt Enable Register (ipier), [1] p17, [2] p18 */
#define IPIER_DRR_NOT_EMPTY	(1 << 8)
#define IPIER_SLAVE_SELECT	(1 << 7)
#define IPIER_TXF_HALF_EMPTY	(1 << 6)
#define IPIER_DRR_OVERRUN	(1 << 5)
#define IPIER_DRR_FULL		(1 << 4)
#define IPIER_DTR_UNDERRUN	(1 << 3)
#define IPIER_DTR_EMPTY		(1 << 2)
#define IPIER_SLAVE_MODF	(1 << 1)
#define IPIER_MODF		(1 << 0)

/* Softare Reset Register (srr), [1] p9, [2] p8 */
#define SRR_RESET_CODE		0x0000000A

/* SPI Control Register (spicr), [1] p9, [2] p8 */
#define SPICR_LSB_FIRST		(1 << 9)
#define SPICR_MASTER_INHIBIT	(1 << 8)
#define SPICR_MANUAL_SS		(1 << 7)
#define SPICR_RXFIFO_RESEST	(1 << 6)
#define SPICR_TXFIFO_RESEST	(1 << 5)
#define SPICR_CPHA		(1 << 4)
#define SPICR_CPOL		(1 << 3)
#define SPICR_MASTER_MODE	(1 << 2)
#define SPICR_SPE		(1 << 1)
#define SPICR_LOOP		(1 << 0)

/* SPI Status Register (spisr), [1] p11, [2] p10 */
#define SPISR_SLAVE_MODE_SELECT	(1 << 5)
#define SPISR_MODF		(1 << 4)
#define SPISR_TX_FULL		(1 << 3)
#define SPISR_TX_EMPTY		(1 << 2)
#define SPISR_RX_FULL		(1 << 1)
#define SPISR_RX_EMPTY		(1 << 0)

/* SPI Data Transmit Register (spidtr), [1] p12, [2] p12 */
#define SPIDTR_8BIT_MASK	(0xff << 0)
#define SPIDTR_16BIT_MASK	(0xffff << 0)
#define SPIDTR_32BIT_MASK	(0xffffffff << 0)

/* SPI Data Receive Register (spidrr), [1] p12, [2] p12 */
#define SPIDRR_8BIT_MASK	(0xff << 0)
#define SPIDRR_16BIT_MASK	(0xffff << 0)
#define SPIDRR_32BIT_MASK	(0xffffffff << 0)

/* SPI Slave Select Register (spissr), [1] p13, [2] p13 */
#define SPISSR_MASK(cs)		(1 << (cs))
#define SPISSR_ACT(cs)		~SPISSR_MASK(cs)
#define SPISSR_OFF		~0UL

/* SPI Transmit FIFO Occupancy Register (spitfor), [1] p13, [2] p14 */
#define SPITFOR_OCYVAL_POS	0
#define SPITFOR_OCYVAL_MASK	(0xf << SPITFOR_OCYVAL_POS)

/* SPI Receive FIFO Occupancy Register (spirfor), [1] p14, [2] p14 */
#define SPIRFOR_OCYVAL_POS	0
#define SPIRFOR_OCYVAL_MASK	(0xf << SPIRFOR_OCYVAL_POS)

/* SPI Software Reset Register (ssr) */
#define SPISSR_RESET_VALUE	0x0a

struct xilinx_spi_slave {
	struct spi_slave slave;
	struct xilinx_spi_reg *regs;
	unsigned int freq;
	unsigned int mode;
};

static inline struct xilinx_spi_slave *to_xilinx_spi_slave(
					struct spi_slave *slave)
{
	return container_of(slave, struct xilinx_spi_slave, slave);
}

#endif /* _XILINX_SPI_ */
