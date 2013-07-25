/*
 *
 * BRIEF MODULE DESCRIPTION
 *   OMAP730 hardware map
 *
 * Copyright (C) 2004 MPC-Data Limited. (http://www.mpc-data.co.uk)
 * Author: MPC-Data Limited
 *	   Dave Peverley
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __INCLUDED_OMAP730_H
#define __INCLUDED_OMAP730_H

#include <asm/sizes.h>

/***************************************************************************
 * OMAP730 Configuration Registers
 **************************************************************************/

#define PERSEUS2_MPU_DEV_ID               ((unsigned int)(0xFFFE1000))
#define PERSEUS2_GSM_DEV_ID0              ((unsigned int)(0xFFFE1000))
#define PERSEUS2_GDM_DEV_ID1              ((unsigned int)(0xFFFE1002))
#define DSP_CONF                          ((unsigned int)(0xFFFE1004))
#define PERSEUS2_MPU_DIE_ID0              ((unsigned int)(0xFFFE1008))
#define GSM_ASIC_CONF                     ((unsigned int)(0xFFFE1008))
#define PERSEUS2_MPU_DIE_ID1              ((unsigned int)(0xFFFE100C))
#define PERSEUS2_MODE1                    ((unsigned int)(0xFFFE1010))
#define PERSEUS2_GSM_DIE_ID0              ((unsigned int)(0xFFFE1010))
#define PERSEUS2_GSM_DIE_ID1              ((unsigned int)(0xFFFE1012))
#define PERSEUS2_MODE2                    ((unsigned int)(0xFFFE1014))
#define PERSEUS2_GSM_DIE_ID2              ((unsigned int)(0xFFFE1014))
#define PERSEUS2_GSM_DIE_ID3              ((unsigned int)(0xFFFE1016))
#define PERSEUS2_ANALOG_CELLS_CONF        ((unsigned int)(0xFFFE1018))
#define SPECCTL                           ((unsigned int)(0xFFFE101C))
#define SPARE1                            ((unsigned int)(0xFFFE1020))
#define SPARE2                            ((unsigned int)(0xFFFE1024))
#define GSM_PBG_IRQ                       ((unsigned int)(0xFFFE1028))
#define DMA_REQ_CONF                      ((unsigned int)(0xFFFE1030))
#define PE_CONF_NO_DUAL                   ((unsigned int)(0xFFFE1060))
#define PERSEUS2_IO_CONF0                 ((unsigned int)(0xFFFE1070))
#define PERSEUS2_IO_CONF1                 ((unsigned int)(0xFFFE1074))
#define PERSEUS2_IO_CONF2                 ((unsigned int)(0xFFFE1078))
#define PERSEUS2_IO_CONF3                 ((unsigned int)(0xFFFE107C))
#define PERSEUS2_IO_CONF4                 ((unsigned int)(0xFFFE1080))
#define PERSEUS2_IO_CONF5                 ((unsigned int)(0xFFFE1084))
#define PERSEUS2_IO_CONF6                 ((unsigned int)(0xFFFE1088))
#define PERSEUS2_IO_CONF7                 ((unsigned int)(0xFFFE108C))
#define PERSEUS2_IO_CONF8                 ((unsigned int)(0xFFFE1090))
#define PERSEUS2_IO_CONF9                 ((unsigned int)(0xFFFE1094))
#define PERSEUS2_IO_CONF10                ((unsigned int)(0xFFFE1098))
#define PERSEUS2_IO_CONF11                ((unsigned int)(0xFFFE109C))
#define PERSEUS2_IO_CONF12                ((unsigned int)(0xFFFE10A0))
#define PERSEUS2_IO_CONF13                ((unsigned int)(0xFFFE10A4))
#define PERSEUS_PCC_CONF_REG              ((unsigned int)(0xFFFE10B4))
#define BIST_STATUS_INTERNAL              ((unsigned int)(0xFFFE10B8))
#define BIST_CONTROL                      ((unsigned int)(0xFFFE10C0))
#define BOOT_ROM_REG                      ((unsigned int)(0xFFFE10C4))
#define PRODUCTION_ID_REG                 ((unsigned int)(0xFFFE10C8))
#define BIST_SECROM_SIGNATURE1_INTERNAL   ((unsigned int)(0xFFFE10D0))
#define BIST_SECROM_SIGNATURE2_INTERNAL   ((unsigned int)(0xFFFE10D4))
#define BIST_CONTROL_2                    ((unsigned int)(0xFFFE10D8))
#define DEBUG1                            ((unsigned int)(0xFFFE10E0))
#define DEBUG2                            ((unsigned int)(0xFFFE10E4))
#define DEBUG_DMA_IRQ                     ((unsigned int)(0xFFFE10E8))

/***************************************************************************
 * OMAP730 EMIFS Registers                                       (TRM 2.5.7)
 **************************************************************************/

#define TCMIF_BASE                 0xFFFECC00

#define EMIFS_LRUREG               (TCMIF_BASE + 0x04)
#define EMIFS_CONFIG               (TCMIF_BASE + 0x0C)
#define FLASH_CFG_0                (TCMIF_BASE + 0x10)
#define FLASH_CFG_1                (TCMIF_BASE + 0x14)
#define FLASH_CFG_2                (TCMIF_BASE + 0x18)
#define FLASH_CFG_3                (TCMIF_BASE + 0x1C)
#define FL_CFG_DYN_WAIT            (TCMIF_BASE + 0x40)
#define EMIFS_TIMEOUT1_REG         (TCMIF_BASE + 0x28)
#define EMIFS_TIMEOUT2_REG         (TCMIF_BASE + 0x2C)
#define EMIFS_TIMEOUT3_REG         (TCMIF_BASE + 0x30)
#define EMIFS_ABORT_ADDR           (TCMIF_BASE + 0x44)
#define EMIFS_ABORT_TYPE           (TCMIF_BASE + 0x48)
#define EMIFS_ABORT_TOUT           (TCMIF_BASE + 0x4C)
#define FLASH_ACFG_0_1             (TCMIF_BASE + 0x50)
#define FLASH_ACFG_1_1             (TCMIF_BASE + 0x54)
#define FLASH_ACFG_2_1             (TCMIF_BASE + 0x58)
#define FLASH_ACFG_3_1             (TCMIF_BASE + 0x5C)

/***************************************************************************
 * OMAP730 Interrupt handlers
 **************************************************************************/

#define OMAP_IH1_BASE		0xFFFECB00     /* MPU Level 1 IRQ handler */
#define OMAP_IH2_BASE           0xfffe0000

/***************************************************************************
 * OMAP730 Timers
 *
 * There are three general purpose OS timers in the 730 that can be
 * configured in autoreload or one-shot modes.
 **************************************************************************/

#define OMAP730_32kHz_TIMER_BASE  0xFFFB9000

/* 32k Timer Registers */
#define TIMER32k_CR               0x08
#define TIMER32k_TVR              0x00
#define TIMER32k_TCR              0x04

/* 32k Timer Control Register definition */
#define TIMER32k_TSS              (1<<0)
#define TIMER32k_TRB              (1<<1)
#define TIMER32k_INT              (1<<2)
#define TIMER32k_ARL              (1<<3)

/* MPU Timer base addresses  */
#define OMAP730_MPUTIMER_BASE	0xfffec500
#define OMAP730_MPUTIMER_OFF	0x00000100

#define OMAP730_TIMER1_BASE	0xFFFEC500
#define OMAP730_TIMER2_BASE	0xFFFEC600
#define OMAP730_TIMER3_BASE	0xFFFEC700

/* MPU Timer Register offsets */
#define CNTL_TIMER	           0x00   /* MPU_CNTL_TIMER */
#define LOAD_TIM	           0x04   /* MPU_LOAD_TIMER */
#define READ_TIM	           0x08   /* MPU_READ_TIMER */

/* MPU_CNTL_TIMER register bits */
#define MPUTIM_FREE               (1<<6)
#define MPUTIM_CLOCK_ENABLE       (1<<5)
#define MPUTIM_PTV_MASK           (0x7<<MPUTIM_PTV_BIT)
#define MPUTIM_PTV_BIT            2
#define MPUTIM_AR                 (1<<1)
#define MPUTIM_ST                 (1<<0)

/***************************************************************************
 * OMAP730 GPIO
 *
 * The GPIO control is split over 6 register bases in the OMAP730 to allow
 * access to all the (6 x 32) GPIO pins!
 **************************************************************************/

#define OMAP730_GPIO_BASE_1        0xFFFBC000
#define OMAP730_GPIO_BASE_2        0xFFFBC800
#define OMAP730_GPIO_BASE_3        0xFFFBD000
#define OMAP730_GPIO_BASE_4        0xFFFBD800
#define OMAP730_GPIO_BASE_5        0xFFFBE000
#define OMAP730_GPIO_BASE_6        0xFFFBE800

#define GPIO_DATA_INPUT            0x00
#define GPIO_DATA_OUTPUT           0x04
#define GPIO_DIRECTION_CONTROL     0x08
#define GPIO_INTERRUPT_CONTROL     0x0C
#define GPIO_INTERRUPT_MASK        0x10
#define GPIO_INTERRUPT_STATUS      0x14

#define GPIO_DATA_INPUT_1            ((unsigned int)(OMAP730_GPIO_BASE_1 + GPIO_DATA_INPUT))
#define GPIO_DATA_OUTPUT_1           ((unsigned int)(OMAP730_GPIO_BASE_1 + GPIO_DATA_OUTPUT))
#define GPIO_DIRECTION_CONTROL_1     ((unsigned int)(OMAP730_GPIO_BASE_1 + GPIO_DIRECTION_CONTROL))
#define GPIO_INTERRUPT_CONTROL_1     ((unsigned int)(OMAP730_GPIO_BASE_1 + GPIO_INTERRUPT_CONTROL))
#define GPIO_INTERRUPT_MASK_1        ((unsigned int)(OMAP730_GPIO_BASE_1 + GPIO_INTERRUPT_MASK))
#define GPIO_INTERRUPT_STATUS_1      ((unsigned int)(OMAP730_GPIO_BASE_1 + GPIO_INTERRUPT_STATUS))

#define GPIO_DATA_INPUT_2            ((unsigned int)(OMAP730_GPIO_BASE_2 + GPIO_DATA_INPUT))
#define GPIO_DATA_OUTPUT_2           ((unsigned int)(OMAP730_GPIO_BASE_2 + GPIO_DATA_OUTPUT))
#define GPIO_DIRECTION_CONTROL_2     ((unsigned int)(OMAP730_GPIO_BASE_2 + GPIO_DIRECTION_CONTROL))
#define GPIO_INTERRUPT_CONTROL_2     ((unsigned int)(OMAP730_GPIO_BASE_2 + GPIO_INTERRUPT_CONTROL))
#define GPIO_INTERRUPT_MASK_2        ((unsigned int)(OMAP730_GPIO_BASE_2 + GPIO_INTERRUPT_MASK))
#define GPIO_INTERRUPT_STATUS_2      ((unsigned int)(OMAP730_GPIO_BASE_2 + GPIO_INTERRUPT_STATUS))

#define GPIO_DATA_INPUT_3            ((unsigned int)(OMAP730_GPIO_BASE_3 + GPIO_DATA_INPUT))
#define GPIO_DATA_OUTPUT_3           ((unsigned int)(OMAP730_GPIO_BASE_3 + GPIO_DATA_OUTPUT))
#define GPIO_DIRECTION_CONTROL_3     ((unsigned int)(OMAP730_GPIO_BASE_3 + GPIO_DIRECTION_CONTROL))
#define GPIO_INTERRUPT_CONTROL_3     ((unsigned int)(OMAP730_GPIO_BASE_3 + GPIO_INTERRUPT_CONTROL))
#define GPIO_INTERRUPT_MASK_3        ((unsigned int)(OMAP730_GPIO_BASE_3 + GPIO_INTERRUPT_MASK))
#define GPIO_INTERRUPT_STATUS_3      ((unsigned int)(OMAP730_GPIO_BASE_3 + GPIO_INTERRUPT_STATUS))

#define GPIO_DATA_INPUT_4            ((unsigned int)(OMAP730_GPIO_BASE_4 + GPIO_DATA_INPUT))
#define GPIO_DATA_OUTPUT_4           ((unsigned int)(OMAP730_GPIO_BASE_4 + GPIO_DATA_OUTPUT))
#define GPIO_DIRECTION_CONTROL_4     ((unsigned int)(OMAP730_GPIO_BASE_4 + GPIO_DIRECTION_CONTROL))
#define GPIO_INTERRUPT_CONTROL_4     ((unsigned int)(OMAP730_GPIO_BASE_4 + GPIO_INTERRUPT_CONTROL))
#define GPIO_INTERRUPT_MASK_4        ((unsigned int)(OMAP730_GPIO_BASE_4 + GPIO_INTERRUPT_MASK))
#define GPIO_INTERRUPT_STATUS_4      ((unsigned int)(OMAP730_GPIO_BASE_4 + GPIO_INTERRUPT_STATUS))

#define GPIO_DATA_INPUT_5            ((unsigned int)(OMAP730_GPIO_BASE_5 + GPIO_DATA_INPUT))
#define GPIO_DATA_OUTPUT_5           ((unsigned int)(OMAP730_GPIO_BASE_5 + GPIO_DATA_OUTPUT))
#define GPIO_DIRECTION_CONTROL_5     ((unsigned int)(OMAP730_GPIO_BASE_5 + GPIO_DIRECTION_CONTROL))
#define GPIO_INTERRUPT_CONTROL_5     ((unsigned int)(OMAP730_GPIO_BASE_5 + GPIO_INTERRUPT_CONTROL))
#define GPIO_INTERRUPT_MASK_5        ((unsigned int)(OMAP730_GPIO_BASE_5 + GPIO_INTERRUPT_MASK))
#define GPIO_INTERRUPT_STATUS_5      ((unsigned int)(OMAP730_GPIO_BASE_5 + GPIO_INTERRUPT_STATUS))

#define GPIO_DATA_INPUT_6            ((unsigned int)(OMAP730_GPIO_BASE_6 + GPIO_DATA_INPUT))
#define GPIO_DATA_OUTPUT_6           ((unsigned int)(OMAP730_GPIO_BASE_6 + GPIO_DATA_OUTPUT))
#define GPIO_DIRECTION_CONTROL_6     ((unsigned int)(OMAP730_GPIO_BASE_6 + GPIO_DIRECTION_CONTROL))
#define GPIO_INTERRUPT_CONTROL_6     ((unsigned int)(OMAP730_GPIO_BASE_6 + GPIO_INTERRUPT_CONTROL))
#define GPIO_INTERRUPT_MASK_6        ((unsigned int)(OMAP730_GPIO_BASE_6 + GPIO_INTERRUPT_MASK))
#define GPIO_INTERRUPT_STATUS_6      ((unsigned int)(OMAP730_GPIO_BASE_6 + GPIO_INTERRUPT_STATUS))

/***************************************************************************
 * OMAP730 Watchdog timers
 **************************************************************************/

#define WDTIM_BASE                 0xFFFEC800
#define WDTIM_CONTROL              (WDTIM_BASE + 0x00)    /* MPU_CNTL_TIMER */
#define WDTIM_LOAD                 (WDTIM_BASE + 0x04)    /* MPU_LOAD_TIMER */
#define WDTIM_READ                 (WDTIM_BASE + 0x04)    /* MPU_READ_TIMER */
#define WDTIM_MODE                 (WDTIM_BASE + 0x08)    /* MPU_TIMER_MODE */

/***************************************************************************
 * OMAP730 Interrupt Registers
 **************************************************************************/

/* Interrupt Register offsets */

#define IRQ_ITR                               0x00
#define IRQ_MIR                               0x04
#define IRQ_SIR_IRQ                           0x10
#define IRQ_SIR_FIQ                           0x14
#define IRQ_CONTROL_REG                       0x18
#define IRQ_ILR0                              0x1C  /* ILRx == ILR0 + (0x4 * x) */
#define IRQ_SIR                               0x9C  /* a.k.a.IRQ_ISR */
#define IRQ_GMIR                              0xA0

#define REG_IHL1_MIR  (OMAP_IH1_BASE + IRQ_MIR)
#define REG_IHL2_MIR  (OMAP_IH2_BASE + IRQ_MIR)

/***************************************************************************
 * OMAP730 Intersystem Communication Register                      (TRM 4.5)
 **************************************************************************/

#define ICR_BASE                   0xFFFBB800

#define M_ICR                      (ICR_BASE + 0x00)
#define G_ICR                      (ICR_BASE + 0x02)
#define M_CTL                      (ICR_BASE + 0x04)
#define G_CTL                      (ICR_BASE + 0x06)
#define PM_BA                      (ICR_BASE + 0x0A)
#define DM_BA                      (ICR_BASE + 0x0C)
#define RM_BA                      (ICR_BASE + 0x0E)
#define SSPI_TAS                   (ICR_BASE + 0x12)

#endif /* ! __INCLUDED_OMAP730_H */
