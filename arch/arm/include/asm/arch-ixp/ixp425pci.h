/*
 * IXP PCI Init
 * (C) Copyright 2004 eslab.whut.edu.cn
 * Yue Hu(huyue_whut@yahoo.com.cn), Ligong Xue(lgxue@hotmail.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _IXP425PCI_H
#define _IXP425PCI_H

#define OK	0
#define ERROR	-1

struct pci_controller;
extern void pci_ixp_init(struct pci_controller *hose);

/* Mask definitions*/
#define IXP425_PCI_BOTTOM_NIBBLE_OF_LONG_MASK	0x0000000f

#define PCI_NP_CBE_BESL	 (4)
#define PCI_NP_AD_FUNCSL (8)

/*Register addressing definitions for PCI controller configuration
  and status registers*/

#define PCI_CSR_BASE (0xC0000000)
/*
#define PCI_NP_AD_OFFSET       (0x00)
#define PCI_NP_CBE_OFFSET      (0x04)
#define PCI_NP_WDATA_OFFSET    (0x08)
#define PCI_NP_RDATA_OFFSET    (0x0C)
#define PCI_CRP_OFFSET	       (0x10)
#define PCI_CRP_WDATA_OFFSET   (0x14)
#define PCI_CRP_RDATA_OFFSET   (0x18)
#define PCI_CSR_OFFSET	       (0x1C)
#define PCI_ISR_OFFSET	       (0x20)
#define PCI_INTEN_OFFSET       (0x24)
#define PCI_DMACTRL_OFFSET     (0x28)
#define PCI_AHBMEMBASE_OFFSET  (0x2C)
#define PCI_AHBIOBASE_OFFSET   (0x30)
#define PCI_PCIMEMBASE_OFFSET  (0x34)
#define PCI_AHBDOORBELL_OFFSET (0x38)
#define PCI_PCIDOORBELL_OFFSET (0x3C)
#define PCI_ATPDMA0_AHBADDR    (0x40)
#define PCI_ATPDMA0_PCIADDR    (0x44)
#define PCI_ATPDMA0_LENADDR    (0x48)
#define PCI_ATPDMA1_AHBADDR    (0x4C)
#define PCI_ATPDMA1_PCIADDR    (0x50)
#define PCI_ATPDMA1_LENADDR    (0x54)
#define PCI_PTADMA0_AHBADDR    (0x58)
#define PCI_PTADMA0_PCIADDR    (0x5C)
#define PCI_PTADMA0_LENADDR    (0x60)
#define PCI_PTADMA1_AHBADDR    (0x64)
#define PCI_PTADMA1_PCIADDR    (0x68)
#define PCI_PTADMA1_LENADDR    (0x6C)
*/
/*Non prefetch registers bit definitions*/
/*
#define NP_CMD_INTACK	   (0x0)
#define NP_CMD_SPECIAL	   (0x1)
#define NP_CMD_IOREAD	   (0x2)
#define NP_CMD_IOWRITE	   (0x3)
#define NP_CMD_MEMREAD	   (0x6)
#define NP_CMD_MEMWRITE	   (0x7)
#define NP_CMD_CONFIGREAD  (0xa)
#define NP_CMD_CONFIGWRITE (0xb)
*/

/*Configuration Port register bit definitions*/
#define PCI_CRP_WRITE BIT(16)

/*ISR (Interrupt status) Register bit definitions*/
#define PCI_ISR_PSE   BIT(0)
#define PCI_ISR_PFE   BIT(1)
#define PCI_ISR_PPE   BIT(2)
#define PCI_ISR_AHBE  BIT(3)
#define PCI_ISR_APDC  BIT(4)
#define PCI_ISR_PADC  BIT(5)
#define PCI_ISR_ADB   BIT(6)
#define PCI_ISR_PDB   BIT(7)

/*INTEN (Interrupt Enable) Register bit definitions*/
#define PCI_INTEN_PSE	BIT(0)
#define PCI_INTEN_PFE	BIT(1)
#define PCI_INTEN_PPE	BIT(2)
#define PCI_INTEN_AHBE	BIT(3)
#define PCI_INTEN_APDC	BIT(4)
#define PCI_INTEN_PADC	BIT(5)
#define PCI_INTEN_ADB	BIT(6)
#define PCI_INTEN_PDB	BIT(7)

/*PCI configuration regs.*/

#define PCI_CFG_VENDOR_ID	0x00
#define PCI_CFG_DEVICE_ID	0x02
#define PCI_CFG_COMMAND		0x04
#define PCI_CFG_STATUS		0x06
#define PCI_CFG_REVISION	0x08
#define PCI_CFG_PROGRAMMING_IF	0x09
#define PCI_CFG_SUBCLASS	0x0a
#define PCI_CFG_CLASS		0x0b
#define PCI_CFG_CACHE_LINE_SIZE 0x0c
#define PCI_CFG_LATENCY_TIMER	0x0d
#define PCI_CFG_HEADER_TYPE	0x0e
#define PCI_CFG_BIST		0x0f
#define PCI_CFG_BASE_ADDRESS_0	0x10
#define PCI_CFG_BASE_ADDRESS_1	0x14
#define PCI_CFG_BASE_ADDRESS_2	0x18
#define PCI_CFG_BASE_ADDRESS_3	0x1c
#define PCI_CFG_BASE_ADDRESS_4	0x20
#define PCI_CFG_BASE_ADDRESS_5	0x24
#define PCI_CFG_CIS		0x28
#define PCI_CFG_SUB_VENDOR_ID	0x2c
#define PCI_CFG_SUB_SYSTEM_ID	0x2e
#define PCI_CFG_EXPANSION_ROM	0x30
#define PCI_CFG_RESERVED_0	0x34
#define PCI_CFG_RESERVED_1	0x38
#define PCI_CFG_DEV_INT_LINE	0x3c
#define PCI_CFG_DEV_INT_PIN	0x3d
#define PCI_CFG_MIN_GRANT	0x3e
#define PCI_CFG_MAX_LATENCY	0x3f
#define PCI_CFG_SPECIAL_USE	0x41
#define PCI_CFG_MODE		0x43

#define PCI_CMD_IO_ENABLE	0x0001	/* IO access enable */
#define PCI_CMD_MEM_ENABLE	0x0002	/* memory access enable */
#define PCI_CMD_MASTER_ENABLE	0x0004	/* bus master enable */
#define PCI_CMD_MON_ENABLE	0x0008	/* monitor special cycles enable */
#define PCI_CMD_WI_ENABLE	0x0010	/* write and invalidate enable */
#define PCI_CMD_SNOOP_ENABLE	0x0020	/* palette snoop enable */
#define PCI_CMD_PERR_ENABLE	0x0040	/* parity error enable */
#define PCI_CMD_WC_ENABLE	0x0080	/* wait cycle enable */
#define PCI_CMD_SERR_ENABLE	0x0100	/* system error enable */
#define PCI_CMD_FBTB_ENABLE	0x0200	/* fast back to back enable */


/*CSR Register bit definitions*/
#define PCI_CSR_HOST  BIT(0)
#define PCI_CSR_ARBEN BIT(1)
#define PCI_CSR_ADS   BIT(2)
#define PCI_CSR_PDS   BIT(3)
#define PCI_CSR_ABE   BIT(4)
#define PCI_CSR_DBT   BIT(5)
#define PCI_CSR_ASE   BIT(8)
#define PCI_CSR_IC    BIT(15)

/*Configuration command bit definitions*/
#define PCI_CFG_CMD_IOAE BIT(0)
#define PCI_CFG_CMD_MAE	 BIT(1)
#define PCI_CFG_CMD_BME	 BIT(2)
#define PCI_CFG_CMD_MWIE BIT(4)
#define PCI_CFG_CMD_SER	 BIT(8)
#define PCI_CFG_CMD_FBBE BIT(9)
#define PCI_CFG_CMD_MDPE BIT(24)
#define PCI_CFG_CMD_STA	 BIT(27)
#define PCI_CFG_CMD_RTA	 BIT(28)
#define PCI_CFG_CMD_RMA	 BIT(29)
#define PCI_CFG_CMD_SSE	 BIT(30)
#define PCI_CFG_CMD_DPE	 BIT(31)

/*DMACTRL DMA Control and status Register*/
#define PCI_DMACTRL_APDCEN  BIT(0)
#define PCI_DMACTRL_APDC0   BIT(4)
#define PCI_DMACTRL_APDE0   BIT(5)
#define PCI_DMACTRL_APDC1   BIT(6)
#define PCI_DMACTRL_APDE1   BIT(7)
#define PCI_DMACTRL_PADCEN  BIT(8)
#define PCI_DMACTRL_PADC0   BIT(12)
#define PCI_DMACTRL_PADE0   BIT(13)
#define PCI_DMACTRL_PADC1   BIT(14)
#define PCI_DMACTRL_PADE1   BIT(15)

#endif
