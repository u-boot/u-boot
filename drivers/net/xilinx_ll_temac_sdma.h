/*
 * Xilinx xps_ll_temac ethernet driver for u-boot
 *
 * SDMA sub-controller interface
 *
 * Copyright (C) 2011 - 2012 Stephan Linz <linz@li-pro.net>
 * Copyright (C) 2008 - 2011 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2008 - 2011 PetaLogix
 *
 * Based on Yoshio Kashiwagi kashiwagi@co-nss.co.jp driver
 * Copyright (C) 2008 Nissin Systems Co.,Ltd.
 * March 2008 created
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [S]:	[0]/ip_documentation/xps_ll_temac.pdf
 * [A]:	[0]/application_notes/xapp1041.pdf
 */
#ifndef _XILINX_LL_TEMAC_SDMA_
#define _XILINX_LL_TEMAC_SDMA_

#include <net.h>

#include <asm/types.h>
#include <asm/byteorder.h>

#include <linux/compiler.h>

#if !defined(__BIG_ENDIAN)
# error LL_TEMAC requires big endianess
#endif

/*
 * DMA Buffer Descriptor for CDMAC
 *
 * Used for data connection from and to (Rx/Tx) the LocalLink (LL) TEMAC via
 * the Communications Direct Memory Access Controller (CDMAC) -- one for each.
 *
 * overview:
 *      ftp://ftp.xilinx.com/pub/documentation/misc/mpmc_getting_started.pdf
 *
 * [1]: [0]/ip_documentation/mpmc.pdf
 *      page 140, DMA Operation Descriptors
 *
 * [2]:	[0]/user_guides/ug200.pdf
 *	page 229, DMA Controller -- Descriptor Format
 *
 * [3]:	[0]/ip_documentation/xps_ll_temac.pdf
 *	page 72, Transmit LocalLink Frame Format
 *	page 73, Receive LocalLink Frame Format
 */
struct cdmac_bd {
	struct cdmac_bd *next_p;	/* Next Descriptor Pointer */
	u8 *phys_buf_p;			/* Buffer Address */
	u32 buf_len;			/* Buffer Length */
	union {
		u8 stctrl;		/* Status/Control the DMA transfer */
		u32 app[5];		/* application specific data */
	} __packed __aligned(1) sca;
};

/* CDMAC Descriptor Status and Control (stctrl), [1] p140, [2] p230 */
#define CDMAC_BD_STCTRL_ERROR		(1 << 7)
#define CDMAC_BD_STCTRL_IRQ_ON_END	(1 << 6)
#define CDMAC_BD_STCTRL_STOP_ON_END	(1 << 5)
#define CDMAC_BD_STCTRL_COMPLETED	(1 << 4)
#define CDMAC_BD_STCTRL_SOP		(1 << 3)
#define CDMAC_BD_STCTRL_EOP		(1 << 2)
#define CDMAC_BD_STCTRL_DMACHBUSY	(1 << 1)

/* CDMAC Descriptor APP0: Transmit LocalLink Footer Word 3, [3] p72 */
#define CDMAC_BD_APP0_TXCSCNTRL		(1 << 0)

/* CDMAC Descriptor APP1: Transmit LocalLink Footer Word 4, [3] p73 */
#define CDMAC_BD_APP1_TXCSBEGIN_POS	16
#define CDMAC_BD_APP1_TXCSBEGIN_MASK	(0xFFFF << CDMAC_BD_APP1_TXCSBEGIN_POS)
#define CDMAC_BD_APP1_TXCSINSERT_POS	0
#define CDMAC_BD_APP1_TXCSINSERT_MASK	(0xFFFF << CDMAC_BD_APP1_TXCSINSERT_POS)

/* CDMAC Descriptor APP2: Transmit LocalLink Footer Word 5, [3] p73 */
#define CDMAC_BD_APP2_TXCSINIT_POS	0
#define CDMAC_BD_APP2_TXCSINIT_MASK	(0xFFFF << CDMAC_BD_APP2_TXCSINIT_POS)

/* CDMAC Descriptor APP0: Receive LocalLink Footer Word 3, [3] p73 */
#define CDMAC_BD_APP0_MADDRU_POS	0
#define CDMAC_BD_APP0_MADDRU_MASK	(0xFFFF << CDMAC_BD_APP0_MADDRU_POS)

/* CDMAC Descriptor APP1: Receive LocalLink Footer Word 4, [3] p74 */
#define CDMAC_BD_APP1_MADDRL_POS	0
#define CDMAC_BD_APP1_MADDRL_MASK	(~0UL << CDMAC_BD_APP1_MADDRL_POS)

/* CDMAC Descriptor APP2: Receive LocalLink Footer Word 5, [3] p74 */
#define CDMAC_BD_APP2_BCAST_FRAME	(1 << 2)
#define CDMAC_BD_APP2_IPC_MCAST_FRAME	(1 << 1)
#define CDMAC_BD_APP2_MAC_MCAST_FRAME	(1 << 0)

/* CDMAC Descriptor APP3: Receive LocalLink Footer Word 6, [3] p74 */
#define CDMAC_BD_APP3_TLTPID_POS	16
#define CDMAC_BD_APP3_TLTPID_MASK	(0xFFFF << CDMAC_BD_APP3_TLTPID_POS)
#define CDMAC_BD_APP3_RXCSRAW_POS	0
#define CDMAC_BD_APP3_RXCSRAW_MASK	(0xFFFF << CDMAC_BD_APP3_RXCSRAW_POS)

/* CDMAC Descriptor APP4: Receive LocalLink Footer Word 7, [3] p74 */
#define CDMAC_BD_APP4_VLANTAG_POS	16
#define CDMAC_BD_APP4_VLANTAG_MASK	(0xFFFF << CDMAC_BD_APP4_VLANTAG_POS)
#define CDMAC_BD_APP4_RXBYTECNT_POS	0
#define CDMAC_BD_APP4_RXBYTECNT_MASK	(0x3FFF << CDMAC_BD_APP4_RXBYTECNT_POS)

/*
 * SDMA Register Definition
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [1]:	[0]/ip_documentation/mpmc.pdf
 *	page 54, SDMA Register Summary
 *	page 160, SDMA Registers
 *
 * [2]:	[0]/user_guides/ug200.pdf
 *	page 244, DMA Controller -- Programming Interface and Registers
 */
#define SDMA_CTRL_REGTYPE	u32
#define SDMA_CTRL_REGSIZE	sizeof(SDMA_CTRL_REGTYPE)
struct sdma_ctrl {
	/* Transmit Registers */
	SDMA_CTRL_REGTYPE tx_nxtdesc_ptr;   /* TX Next Description Pointer */
	SDMA_CTRL_REGTYPE tx_curbuf_addr;   /* TX Current Buffer Address */
	SDMA_CTRL_REGTYPE tx_curbuf_length; /* TX Current Buffer Length */
	SDMA_CTRL_REGTYPE tx_curdesc_ptr;   /* TX Current Descriptor Pointer */
	SDMA_CTRL_REGTYPE tx_taildesc_ptr;  /* TX Tail Descriptor Pointer */
	SDMA_CTRL_REGTYPE tx_chnl_ctrl;     /* TX Channel Control */
	SDMA_CTRL_REGTYPE tx_irq_reg;       /* TX Interrupt Register */
	SDMA_CTRL_REGTYPE tx_chnl_sts;      /* TX Status Register */
	/* Receive Registers */
	SDMA_CTRL_REGTYPE rx_nxtdesc_ptr;   /* RX Next Descriptor Pointer */
	SDMA_CTRL_REGTYPE rx_curbuf_addr;   /* RX Current Buffer Address */
	SDMA_CTRL_REGTYPE rx_curbuf_length; /* RX Current Buffer Length */
	SDMA_CTRL_REGTYPE rx_curdesc_ptr;   /* RX Current Descriptor Pointer */
	SDMA_CTRL_REGTYPE rx_taildesc_ptr;  /* RX Tail Descriptor Pointer */
	SDMA_CTRL_REGTYPE rx_chnl_ctrl;     /* RX Channel Control */
	SDMA_CTRL_REGTYPE rx_irq_reg;       /* RX Interrupt Register */
	SDMA_CTRL_REGTYPE rx_chnl_sts;      /* RX Status Register */
	/* Control Registers */
	SDMA_CTRL_REGTYPE dma_control_reg;  /* DMA Control Register */
};

#define SDMA_CTRL_REGNUMS	sizeof(struct sdma_ctrl)/SDMA_CTRL_REGSIZE

/*
 * DMAC Register Index Enumeration
 *
 * [2]:	http://www.xilinx.com/support/documentation/user_guides/ug200.pdf
 *	page 244, DMA Controller -- Programming Interface and Registers
 */
enum dmac_ctrl {
	/* Transmit Registers */
	TX_NXTDESC_PTR = 0,	/* TX Next Description Pointer */
	TX_CURBUF_ADDR,		/* TX Current Buffer Address */
	TX_CURBUF_LENGTH,	/* TX Current Buffer Length */
	TX_CURDESC_PTR,		/* TX Current Descriptor Pointer */
	TX_TAILDESC_PTR,	/* TX Tail Descriptor Pointer */
	TX_CHNL_CTRL,		/* TX Channel Control */
	TX_IRQ_REG,		/* TX Interrupt Register */
	TX_CHNL_STS,		/* TX Status Register */
	/* Receive Registers */
	RX_NXTDESC_PTR,		/* RX Next Descriptor Pointer */
	RX_CURBUF_ADDR,		/* RX Current Buffer Address */
	RX_CURBUF_LENGTH,	/* RX Current Buffer Length */
	RX_CURDESC_PTR,		/* RX Current Descriptor Pointer */
	RX_TAILDESC_PTR,	/* RX Tail Descriptor Pointer */
	RX_CHNL_CTRL,		/* RX Channel Control */
	RX_IRQ_REG,		/* RX Interrupt Register */
	RX_CHNL_STS,		/* RX Status Register */
	/* Control Registers */
	DMA_CONTROL_REG		/* DMA Control Register */
};

/* Rx/Tx Channel Control Register (*_chnl_ctrl), [1] p163, [2] p246/p252 */
#define CHNL_CTRL_ITO_POS	24
#define CHNL_CTRL_ITO_MASK	(0xFF << CHNL_CTRL_ITO_POS)
#define CHNL_CTRL_IC_POS	16
#define CHNL_CTRL_IC_MASK	(0xFF << CHNL_CTRL_IC_POS)
#define CHNL_CTRL_MSBADDR_POS	12
#define CHNL_CTRL_MSBADDR_MASK	(0xF << CHNL_CTRL_MSBADDR_POS)
#define CHNL_CTRL_AME		(1 << 11)
#define CHNL_CTRL_OBWC		(1 << 10)
#define CHNL_CTRL_IOE		(1 << 9)
#define CHNL_CTRL_LIC		(1 << 8)
#define CHNL_CTRL_IE		(1 << 7)
#define CHNL_CTRL_IEE		(1 << 2)
#define CHNL_CTRL_IDE		(1 << 1)
#define CHNL_CTRL_ICE		(1 << 0)

/* All interrupt enable bits */
#define CHNL_CTRL_IRQ_MASK	(CHNL_CTRL_IE | \
				 CHNL_CTRL_IEE | \
				 CHNL_CTRL_IDE | \
				 CHNL_CTRL_ICE)

/* Rx/Tx Interrupt Status Register (*_irq_reg), [1] p164, [2] p247/p253 */
#define IRQ_REG_DTV_POS		24
#define IRQ_REG_DTV_MASK	(0xFF << IRQ_REG_DTV_POS)
#define IRQ_REG_CCV_POS		16
#define IRQ_REG_CCV_MASK	(0xFF << IRQ_REG_CCV_POS)
#define IRQ_REG_WRCQ_EMPTY	(1 << 14)
#define IRQ_REG_CIC_POS		10
#define IRQ_REG_CIC_MASK	(0xF << IRQ_REG_CIC_POS)
#define IRQ_REG_DIC_POS		8
#define IRQ_REG_DIC_MASK	(3 << 8)
#define IRQ_REG_PLB_RD_NMI	(1 << 4)
#define IRQ_REG_PLB_WR_NMI	(1 << 3)
#define IRQ_REG_EI		(1 << 2)
#define IRQ_REG_DI		(1 << 1)
#define IRQ_REG_CI		(1 << 0)

/* All interrupt bits */
#define IRQ_REG_IRQ_MASK	(IRQ_REG_PLB_RD_NMI | \
				 IRQ_REG_PLB_WR_NMI | \
				 IRQ_REG_EI | IRQ_REG_DI | IRQ_REG_CI)

/* Rx/Tx Channel Status Register (*_chnl_sts), [1] p165, [2] p249/p255 */
#define CHNL_STS_ERROR_TAIL	(1 << 21)
#define CHNL_STS_ERROR_CMP	(1 << 20)
#define CHNL_STS_ERROR_ADDR	(1 << 19)
#define CHNL_STS_ERROR_NXTP	(1 << 18)
#define CHNL_STS_ERROR_CURP	(1 << 17)
#define CHNL_STS_ERROR_BSYWR	(1 << 16)
#define CHNL_STS_ERROR		(1 << 7)
#define CHNL_STS_IOE		(1 << 6)
#define CHNL_STS_SOE		(1 << 5)
#define CHNL_STS_CMPLT		(1 << 4)
#define CHNL_STS_SOP		(1 << 3)
#define CHNL_STS_EOP		(1 << 2)
#define CHNL_STS_EBUSY		(1 << 1)

/* DMA Control Register (dma_control_reg), [1] p166, [2] p256 */
#define DMA_CONTROL_PLBED	(1 << 5)
#define DMA_CONTROL_RXOCEID	(1 << 4)
#define DMA_CONTROL_TXOCEID	(1 << 3)
#define DMA_CONTROL_TPE		(1 << 2)
#define DMA_CONTROL_RESET	(1 << 0)

#if defined(CONFIG_XILINX_440) || defined(CONFIG_XILINX_405)

/* Xilinx Device Control Register (DCR) in/out accessors */
unsigned ll_temac_xldcr_in32(phys_addr_t addr);
void ll_temac_xldcr_out32(phys_addr_t addr, unsigned value);

/* collect all register addresses for Xilinx DCR in/out accessors */
void ll_temac_collect_xldcr_sdma_reg_addr(struct eth_device *dev);

#endif /* CONFIG_XILINX_440 || CONFIG_XILINX_405 */

/* Xilinx Processor Local Bus (PLB) in/out accessors */
unsigned ll_temac_xlplb_in32(phys_addr_t base);
void ll_temac_xlplb_out32(phys_addr_t base, unsigned value);

/* collect all register addresses for Xilinx PLB in/out accessors */
void ll_temac_collect_xlplb_sdma_reg_addr(struct eth_device *dev);

/* initialize both Rx/Tx buffer descriptors */
int ll_temac_init_sdma(struct eth_device *dev);

/* halt both Rx/Tx transfers */
int ll_temac_halt_sdma(struct eth_device *dev);

/* reset SDMA and IRQ, disable interrupts and errors */
int ll_temac_reset_sdma(struct eth_device *dev);

/* receive buffered data from SDMA (polling ISR) */
int ll_temac_recv_sdma(struct eth_device *dev);

/* send buffered data to SDMA */
int ll_temac_send_sdma(struct eth_device *dev, volatile void *packet,
							int length);

#endif /* _XILINX_LL_TEMAC_SDMA_ */
