/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Torsten Demke, FORCE Computers GmbH. torsten.demke@fci.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _PIIX4_PCI_H
#define _PIIX4_PCI_H

#include <common.h>
#include <mpc824x.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>

#define PIIX4_VENDOR_ID         0x8086
#define PIIX4_ISA_DEV_ID        0x7110
#define PIIX4_IDE_DEV_ID        0x7111

/* Function 0 ISA Bridge */
#define PCI_CFG_PIIX4_IORT      0x4C    /* 8 bit ISA Recovery Timer Reg (default 0x4D) */
#define PCI_CFG_PIIX4_XBCS      0x4E    /* 16 bit XBus Chip select reg (default 0x0003) */
#define PCI_CFG_PIIX4_PIRQC     0x60    /* PCI IRQ Route Register 4 x 8bit (default )*/
#define PCI_CFG_PIIX4_SERIRQ    0x64
#define PCI_CFG_PIIX4_TOM       0x69
#define PCI_CFG_PIIX4_MSTAT     0x6A
#define PCI_CFG_PIIX4_MBDMA     0x76
#define PCI_CFG_PIIX4_APICBS    0x80
#define PCI_CFG_PIIX4_DLC       0x82
#define PCI_CFG_PIIX4_PDMACFG   0x90
#define PCI_CFG_PIIX4_DDMABS    0x92
#define PCI_CFG_PIIX4_GENCFG    0xB0
#define PCI_CFG_PIIX4_RTCCFG    0xCB

/* IO Addresses */
#define PIIX4_ISA_DMA1_CH0BA    0x00
#define PIIX4_ISA_DMA1_CH0CA    0x01
#define PIIX4_ISA_DMA1_CH1BA    0x02
#define PIIX4_ISA_DMA1_CH1CA    0x03
#define PIIX4_ISA_DMA1_CH2BA    0x04
#define PIIX4_ISA_DMA1_CH2CA    0x05
#define PIIX4_ISA_DMA1_CH3BA    0x06
#define PIIX4_ISA_DMA1_CH3CA    0x07
#define PIIX4_ISA_DMA1_CMDST    0x08
#define PIIX4_ISA_DMA1_REQ      0x09
#define PIIX4_ISA_DMA1_WSBM     0x0A
#define PIIX4_ISA_DMA1_CH_MOD   0x0B
#define PIIX4_ISA_DMA1_CLR_PT   0x0C
#define PIIX4_ISA_DMA1_M_CLR    0x0D
#define PIIX4_ISA_DMA1_CLR_M    0x0E
#define PIIX4_ISA_DMA1_RWAMB    0x0F

#define PIIX4_ISA_DMA2_CH0BA    0xC0
#define PIIX4_ISA_DMA2_CH0CA    0xC1
#define PIIX4_ISA_DMA2_CH1BA    0xC2
#define PIIX4_ISA_DMA2_CH1CA    0xC3
#define PIIX4_ISA_DMA2_CH2BA    0xC4
#define PIIX4_ISA_DMA2_CH2CA    0xC5
#define PIIX4_ISA_DMA2_CH3BA    0xC6
#define PIIX4_ISA_DMA2_CH3CA    0xC7
#define PIIX4_ISA_DMA2_CMDST    0xD0
#define PIIX4_ISA_DMA2_REQ      0xD2
#define PIIX4_ISA_DMA2_WSBM     0xD4
#define PIIX4_ISA_DMA2_CH_MOD   0xD6
#define PIIX4_ISA_DMA2_CLR_PT   0xD8
#define PIIX4_ISA_DMA2_M_CLR    0xDA
#define PIIX4_ISA_DMA2_CLR_M    0xDC
#define PIIX4_ISA_DMA2_RWAMB    0xDE

#define PIIX4_ISA_INT1_ICW1     0x20
#define PIIX4_ISA_INT1_OCW2     0x20
#define PIIX4_ISA_INT1_OCW3     0x20
#define PIIX4_ISA_INT1_ICW2     0x21
#define PIIX4_ISA_INT1_ICW3     0x21
#define PIIX4_ISA_INT1_ICW4     0x21
#define PIIX4_ISA_INT1_OCW1     0x21

#define PIIX4_ISA_INT1_ELCR     0x4D0

#define PIIX4_ISA_INT2_ICW1     0xA0
#define PIIX4_ISA_INT2_OCW2     0xA0
#define PIIX4_ISA_INT2_OCW3     0xA0
#define PIIX4_ISA_INT2_ICW2     0xA1
#define PIIX4_ISA_INT2_ICW3     0xA1
#define PIIX4_ISA_INT2_ICW4     0xA1
#define PIIX4_ISA_INT2_OCW1     0xA1
#define PIIX4_ISA_INT2_IMR      0xA1 /* read only */

#define PIIX4_ISA_INT2_ELCR     0x4D1

#define PIIX4_ISA_TMR0_CNT_ST   0x40
#define PIIX4_ISA_TMR1_CNT_ST   0x41
#define PIIX4_ISA_TMR2_CNT_ST   0x42
#define PIIX4_ISA_TMR_TCW       0x43

#define PIIX4_ISA_RST_XBUS      0x60

#define PIIX4_ISA_NMI_CNT_ST    0x61
#define PIIX4_ISA_NMI_ENABLE    0x70

#define PIIX4_ISA_RTC_INDEX     0x70
#define PIIX4_ISA_RTC_DATA      0x71
#define PIIX4_ISA_RTCEXT_IND    0x70
#define PIIX4_ISA_RTCEXT_DATA   0x71

#define PIIX4_ISA_DMA1_CH2LPG   0x81
#define PIIX4_ISA_DMA1_CH3LPG   0x82
#define PIIX4_ISA_DMA1_CH1LPG   0x83
#define PIIX4_ISA_DMA1_CH0LPG   0x87
#define PIIX4_ISA_DMA2_CH2LPG   0x89
#define PIIX4_ISA_DMA2_CH3LPG   0x8A
#define PIIX4_ISA_DMA2_CH1LPG   0x8B
#define PIIX4_ISA_DMA2_LPGRFR   0x8F

#define PIIX4_ISA_PORT_92       0x92

#define PIIX4_ISA_APM_CONTRL    0xB2
#define PIIX4_ISA_APM_STATUS    0xB3

#define PIIX4_ISA_COCPU_ERROR   0xF0

/* Function 1 IDE Controller */
#define PCI_CFG_PIIX4_BMIBA     0x20
#define PCI_CFG_PIIX4_IDETIM    0x40
#define PCI_CFG_PIIX4_SIDETIM   0x44
#define PCI_CFG_PIIX4_UDMACTL   0x48
#define PCI_CFG_PIIX4_UDMATIM   0x4A

/* Function 2 USB Controller */
#define PCI_CFG_PIIX4_SBRNUM    0x60
#define PCI_CFG_PIIX4_LEGSUP    0xC0

/* Function 3 Power Management */
#define PCI_CFG_PIIX4_PMAB      0x40
#define PCI_CFG_PIIX4_CNTA      0x44
#define PCI_CFG_PIIX4_CNTB      0x48
#define PCI_CFG_PIIX4_GPICTL    0x4C
#define PCI_CFG_PIIX4_DEVRESD   0x50
#define PCI_CFG_PIIX4_DEVACTA   0x54
#define PCI_CFG_PIIX4_DEVACTB   0x58
#define PCI_CFG_PIIX4_DEVRESA   0x5C
#define PCI_CFG_PIIX4_DEVRESB   0x60
#define PCI_CFG_PIIX4_DEVRESC   0x64
#define PCI_CFG_PIIX4_DEVRESE   0x68
#define PCI_CFG_PIIX4_DEVRESF   0x6C
#define PCI_CFG_PIIX4_DEVRESG   0x70
#define PCI_CFG_PIIX4_DEVRESH   0x74
#define PCI_CFG_PIIX4_DEVRESI   0x78
#define PCI_CFG_PIIX4_PMMISC    0x80
#define PCI_CFG_PIIX4_SMBBA     0x90


#endif  /* _PIIX4_PCI_H */
