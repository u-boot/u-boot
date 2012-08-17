/*
 * include/asm-arm/arch-ixp425/ixp425.h
 *
 * Register definitions for IXP425
 *
 * Copyright (C) 2002 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _ASM_ARM_IXP425_H_
#define _ASM_ARM_IXP425_H_

#define BIT(x)	(1<<(x))

/* FIXME: Only this does work for u-boot... find out why... [RS] */
#define UBOOT_REG_FIX 1
#ifdef UBOOT_REG_FIX
# undef	io_p2v
# undef __REG
# ifndef __ASSEMBLY__
#  define io_p2v(PhAdd)    (PhAdd)
#  define __REG(x)	(*((volatile u32 *)io_p2v(x)))
#  define __REG2(x,y)	(*(volatile u32 *)((u32)&__REG(x) + (y)))
# else
#  define __REG(x) (x)
# endif
#endif /* UBOOT_REG_FIX */

/*
 *
 * IXP425 Memory map:
 *
 * Phy		Phy Size	Map Size	Virt		Description
 * =========================================================================
 *
 * 0x00000000	0x10000000					SDRAM 1
 *
 * 0x10000000	0x10000000					SDRAM 2
 *
 * 0x20000000	0x10000000					SDRAM 3
 *
 * 0x30000000	0x10000000					SDRAM 4
 *
 * The above four are aliases to the same memory location  (0x00000000)
 *
 * 0x48000000	 0x4000000					PCI Memory
 *
 * 0x50000000	0x10000000			Not Mapped	EXP BUS
 *
 * 0x6000000	0x00004000	    0x4000	0xFFFEB000	QMgr
 *
 * 0xC0000000	     0x100	    0x1000	0xFFFDD000	PCI CFG
 *
 * 0xC4000000	     0x100          0x1000	0xFFFDE000	EXP CFG
 *
 * 0xC8000000	    0xC000          0xC000	0xFFFDF000	PERIPHERAL
 *
 * 0xCC000000	     0x100	    0x1000	Not Mapped	SDRAM CFG
 */

/*
 * SDRAM
 */
#define IXP425_SDRAM_BASE		(0x00000000)
#define IXP425_SDRAM_BASE_ALT		(0x10000000)


/*
 * PCI Configuration space
 */
#define IXP425_PCI_CFG_BASE_PHYS	(0xC0000000)
#define IXP425_PCI_CFG_REGION_SIZE	(0x00001000)

/*
 * Expansion BUS Configuration registers
 */
#define IXP425_EXP_CFG_BASE_PHYS	(0xC4000000)
#define IXP425_EXP_CFG_REGION_SIZE	(0x00001000)

/*
 * Peripheral space
 */
#define IXP425_PERIPHERAL_BASE_PHYS	(0xC8000000)
#define IXP425_PERIPHERAL_REGION_SIZE	(0x0000C000)

/*
 * SDRAM configuration registers
 */
#define IXP425_SDRAM_CFG_BASE_PHYS	(0xCC000000)

/*
 * Q Manager space .. not static mapped
 */
#define IXP425_QMGR_BASE_PHYS		(0x60000000)
#define IXP425_QMGR_REGION_SIZE		(0x00004000)

/*
 * Expansion BUS
 *
 * Expansion Bus 'lives' at either base1 or base 2 depending on the value of
 * Exp Bus config registers:
 *
 * Setting bit 31 of IXP425_EXP_CFG0 puts SDRAM at zero,
 * and The expansion bus to IXP425_EXP_BUS_BASE2
 */
#define IXP425_EXP_BUS_BASE1_PHYS	(0x00000000)
#define IXP425_EXP_BUS_BASE2_PHYS	(0x50000000)

#define IXP425_EXP_BUS_BASE_PHYS	IXP425_EXP_BUS_BASE2_PHYS

#define IXP425_EXP_BUS_REGION_SIZE	(0x08000000)
#define IXP425_EXP_BUS_CSX_REGION_SIZE	(0x01000000)

#define IXP425_EXP_BUS_CS0_BASE_PHYS	(IXP425_EXP_BUS_BASE2_PHYS + 0x00000000)
#define IXP425_EXP_BUS_CS1_BASE_PHYS	(IXP425_EXP_BUS_BASE2_PHYS + 0x01000000)
#define IXP425_EXP_BUS_CS2_BASE_PHYS	(IXP425_EXP_BUS_BASE2_PHYS + 0x02000000)
#define IXP425_EXP_BUS_CS3_BASE_PHYS	(IXP425_EXP_BUS_BASE2_PHYS + 0x03000000)
#define IXP425_EXP_BUS_CS4_BASE_PHYS	(IXP425_EXP_BUS_BASE2_PHYS + 0x04000000)
#define IXP425_EXP_BUS_CS5_BASE_PHYS	(IXP425_EXP_BUS_BASE2_PHYS + 0x05000000)
#define IXP425_EXP_BUS_CS6_BASE_PHYS	(IXP425_EXP_BUS_BASE2_PHYS + 0x06000000)
#define IXP425_EXP_BUS_CS7_BASE_PHYS	(IXP425_EXP_BUS_BASE2_PHYS + 0x07000000)

#define IXP425_FLASH_WRITABLE	(0x2)
#define IXP425_FLASH_DEFAULT	(0xbcd23c40)
#define IXP425_FLASH_WRITE	(0xbcd23c42)

#define IXP425_EXP_CS0_OFFSET	0x00
#define IXP425_EXP_CS1_OFFSET   0x04
#define IXP425_EXP_CS2_OFFSET   0x08
#define IXP425_EXP_CS3_OFFSET   0x0C
#define IXP425_EXP_CS4_OFFSET   0x10
#define IXP425_EXP_CS5_OFFSET   0x14
#define IXP425_EXP_CS6_OFFSET   0x18
#define IXP425_EXP_CS7_OFFSET   0x1C
#define IXP425_EXP_CFG0_OFFSET	0x20
#define IXP425_EXP_CFG1_OFFSET	0x24
#define IXP425_EXP_CFG2_OFFSET	0x28
#define IXP425_EXP_CFG3_OFFSET	0x2C

/*
 * Expansion Bus Controller registers.
 */
#ifndef __ASSEMBLY__
#define IXP425_EXP_REG(x) ((volatile u32 *)(IXP425_EXP_CFG_BASE_PHYS+(x)))
#else
#define IXP425_EXP_REG(x) (IXP425_EXP_CFG_BASE_PHYS+(x))
#endif

#define IXP425_EXP_CS0      IXP425_EXP_REG(IXP425_EXP_CS0_OFFSET)
#define IXP425_EXP_CS1      IXP425_EXP_REG(IXP425_EXP_CS1_OFFSET)
#define IXP425_EXP_CS2      IXP425_EXP_REG(IXP425_EXP_CS2_OFFSET)
#define IXP425_EXP_CS3      IXP425_EXP_REG(IXP425_EXP_CS3_OFFSET)
#define IXP425_EXP_CS4      IXP425_EXP_REG(IXP425_EXP_CS4_OFFSET)
#define IXP425_EXP_CS5      IXP425_EXP_REG(IXP425_EXP_CS5_OFFSET)
#define IXP425_EXP_CS6      IXP425_EXP_REG(IXP425_EXP_CS6_OFFSET)
#define IXP425_EXP_CS7      IXP425_EXP_REG(IXP425_EXP_CS7_OFFSET)

#define IXP425_EXP_CFG0     IXP425_EXP_REG(IXP425_EXP_CFG0_OFFSET)
#define IXP425_EXP_CFG1     IXP425_EXP_REG(IXP425_EXP_CFG1_OFFSET)
#define IXP425_EXP_CFG2     IXP425_EXP_REG(IXP425_EXP_CFG2_OFFSET)
#define IXP425_EXP_CFG3     IXP425_EXP_REG(IXP425_EXP_CFG3_OFFSET)

/*
 * SDRAM Controller registers.
 */
#define IXP425_SDR_CONFIG_OFFSET	0x00
#define IXP425_SDR_REFRESH_OFFSET	0x04
#define IXP425_SDR_IR_OFFSET		0x08

#define IXP425_SDRAM_REG(x)	(IXP425_SDRAM_CFG_BASE_PHYS+(x))

#define IXP425_SDR_CONFIG	IXP425_SDRAM_REG(IXP425_SDR_CONFIG_OFFSET)
#define IXP425_SDR_REFRESH	IXP425_SDRAM_REG(IXP425_SDR_REFRESH_OFFSET)
#define IXP425_SDR_IR		IXP425_SDRAM_REG(IXP425_SDR_IR_OFFSET)

/*
 * UART registers
 */
#define IXP425_UART1		0
#define IXP425_UART2		0x1000

#define IXP425_UART_RBR_OFFSET	0x00
#define IXP425_UART_THR_OFFSET	0x00
#define IXP425_UART_DLL_OFFSET	0x00
#define IXP425_UART_IER_OFFSET	0x04
#define IXP425_UART_DLH_OFFSET	0x04
#define IXP425_UART_IIR_OFFSET	0x08
#define IXP425_UART_FCR_OFFSET	0x00
#define IXP425_UART_LCR_OFFSET	0x0c
#define IXP425_UART_MCR_OFFSET	0x10
#define IXP425_UART_LSR_OFFSET	0x14
#define IXP425_UART_MSR_OFFSET	0x18
#define IXP425_UART_SPR_OFFSET	0x1c
#define IXP425_UART_ISR_OFFSET	0x20

#define IXP425_UART_CFG_BASE_PHYS	(0xc8000000)

#define RBR(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_RBR_OFFSET)
#define THR(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_THR_OFFSET)
#define DLL(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_DLL_OFFSET)
#define IER(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_IER_OFFSET)
#define DLH(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_DLH_OFFSET)
#define IIR(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_IIR_OFFSET)
#define FCR(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_FCR_OFFSET)
#define LCR(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_LCR_OFFSET)
#define MCR(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_MCR_OFFSET)
#define LSR(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_LSR_OFFSET)
#define MSR(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_MSR_OFFSET)
#define SPR(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_SPR_OFFSET)
#define ISR(x)		__REG(IXP425_UART_CFG_BASE_PHYS+(x)+IXP425_UART_ISR_OFFSET)

#define IER_DMAE	(1 << 7)	/* DMA Requests Enable */
#define IER_UUE		(1 << 6)	/* UART Unit Enable */
#define IER_NRZE	(1 << 5)	/* NRZ coding Enable */
#define IER_RTIOE	(1 << 4)	/* Receiver Time Out Interrupt Enable */
#define IER_MIE		(1 << 3)	/* Modem Interrupt Enable */
#define IER_RLSE	(1 << 2)	/* Receiver Line Status Interrupt Enable */
#define IER_TIE		(1 << 1)	/* Transmit Data request Interrupt Enable */
#define IER_RAVIE	(1 << 0)	/* Receiver Data Available Interrupt Enable */

#define IIR_FIFOES1	(1 << 7)	/* FIFO Mode Enable Status */
#define IIR_FIFOES0	(1 << 6)	/* FIFO Mode Enable Status */
#define IIR_TOD		(1 << 3)	/* Time Out Detected */
#define IIR_IID2	(1 << 2)	/* Interrupt Source Encoded */
#define IIR_IID1	(1 << 1)	/* Interrupt Source Encoded */
#define IIR_IP		(1 << 0)	/* Interrupt Pending (active low) */

#define FCR_ITL2	(1 << 7)	/* Interrupt Trigger Level */
#define FCR_ITL1	(1 << 6)	/* Interrupt Trigger Level */
#define FCR_RESETTF	(1 << 2)	/* Reset Transmitter FIFO */
#define FCR_RESETRF	(1 << 1)	/* Reset Receiver FIFO */
#define FCR_TRFIFOE	(1 << 0)	/* Transmit and Receive FIFO Enable */
#define FCR_ITL_1	(0)
#define FCR_ITL_8	(FCR_ITL1)
#define FCR_ITL_16	(FCR_ITL2)
#define FCR_ITL_32	(FCR_ITL2|FCR_ITL1)

#define LCR_DLAB	(1 << 7)	/* Divisor Latch Access Bit */
#define LCR_SB		(1 << 6)	/* Set Break */
#define LCR_STKYP	(1 << 5)	/* Sticky Parity */
#define LCR_EPS		(1 << 4)	/* Even Parity Select */
#define LCR_PEN		(1 << 3)	/* Parity Enable */
#define LCR_STB		(1 << 2)	/* Stop Bit */
#define LCR_WLS1	(1 << 1)	/* Word Length Select */
#define LCR_WLS0	(1 << 0)	/* Word Length Select */

#define LSR_FIFOE	(1 << 7)	/* FIFO Error Status */
#define LSR_TEMT	(1 << 6)	/* Transmitter Empty */
#define LSR_TDRQ	(1 << 5)	/* Transmit Data Request */
#define LSR_BI		(1 << 4)	/* Break Interrupt */
#define LSR_FE		(1 << 3)	/* Framing Error */
#define LSR_PE		(1 << 2)	/* Parity Error */
#define LSR_OE		(1 << 1)	/* Overrun Error */
#define LSR_DR		(1 << 0)	/* Data Ready */

#define MCR_LOOP	(1 << 4)	*/
#define MCR_OUT2	(1 << 3)	/* force MSR_DCD in loopback mode */
#define MCR_OUT1	(1 << 2)	/* force MSR_RI in loopback mode */
#define MCR_RTS		(1 << 1)	/* Request to Send */
#define MCR_DTR		(1 << 0)	/* Data Terminal Ready */

#define MSR_DCD		(1 << 7)	/* Data Carrier Detect */
#define MSR_RI		(1 << 6)	/* Ring Indicator */
#define MSR_DSR		(1 << 5)	/* Data Set Ready */
#define MSR_CTS		(1 << 4)	/* Clear To Send */
#define MSR_DDCD	(1 << 3)	/* Delta Data Carrier Detect */
#define MSR_TERI	(1 << 2)	/* Trailing Edge Ring Indicator */
#define MSR_DDSR	(1 << 1)	/* Delta Data Set Ready */
#define MSR_DCTS	(1 << 0)	/* Delta Clear To Send */

#define IXP425_CONSOLE_UART_BASE_PHYS IXP425_UART1_BASE_PHYS
/*
 * Peripheral Space Registers
 */
#define IXP425_UART1_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0x0000)
#define IXP425_UART2_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0x1000)
#define IXP425_PMU_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0x2000)
#define IXP425_INTC_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0x3000)
#define IXP425_GPIO_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0x4000)
#define IXP425_TIMER_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0x5000)
#define IXP425_NPEA_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0x6000)
#define IXP425_NPEB_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0x7000)
#define IXP425_NPEC_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0x8000)
#define IXP425_EthA_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0x9000)
#define IXP425_EthB_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0xA000)
#define IXP425_USB_BASE_PHYS	(IXP425_PERIPHERAL_BASE_PHYS + 0xB000)

/*
 * UART Register Definitions , Offsets only as there are 2 UARTS.
 *   IXP425_UART1_BASE , IXP425_UART2_BASE.
 */

#undef  UART_NO_RX_INTERRUPT

#define IXP425_UART_XTAL        14745600

/*
 * Constants to make it easy to access  Interrupt Controller registers
 */
#define IXP425_ICPR_OFFSET	0x00 /* Interrupt Status */
#define IXP425_ICMR_OFFSET	0x04 /* Interrupt Enable */
#define IXP425_ICLR_OFFSET	0x08 /* Interrupt IRQ/FIQ Select */
#define IXP425_ICIP_OFFSET      0x0C /* IRQ Status */
#define IXP425_ICFP_OFFSET	0x10 /* FIQ Status */
#define IXP425_ICHR_OFFSET	0x14 /* Interrupt Priority */
#define IXP425_ICIH_OFFSET	0x18 /* IRQ Highest Pri Int */
#define IXP425_ICFH_OFFSET	0x1C /* FIQ Highest Pri Int */

#define N_IRQS			32
#define IXP425_TIMER_2_IRQ	11

/*
 * Interrupt Controller Register Definitions.
 */
#ifndef __ASSEMBLY__
#define IXP425_INTC_REG(x) ((volatile u32 *)(IXP425_INTC_BASE_PHYS+(x)))
#else
#define IXP425_INTC_REG(x) (IXP425_INTC_BASE_PHYS+(x))
#endif

#define IXP425_ICPR     IXP425_INTC_REG(IXP425_ICPR_OFFSET)
#define IXP425_ICMR     IXP425_INTC_REG(IXP425_ICMR_OFFSET)
#define IXP425_ICLR     IXP425_INTC_REG(IXP425_ICLR_OFFSET)
#define IXP425_ICIP     IXP425_INTC_REG(IXP425_ICIP_OFFSET)
#define IXP425_ICFP     IXP425_INTC_REG(IXP425_ICFP_OFFSET)
#define IXP425_ICHR     IXP425_INTC_REG(IXP425_ICHR_OFFSET)
#define IXP425_ICIH     IXP425_INTC_REG(IXP425_ICIH_OFFSET)
#define IXP425_ICFH     IXP425_INTC_REG(IXP425_ICFH_OFFSET)

/*
 * Constants to make it easy to access GPIO registers
 */
#define IXP425_GPIO_GPOUTR_OFFSET       0x00
#define IXP425_GPIO_GPOER_OFFSET        0x04
#define IXP425_GPIO_GPINR_OFFSET        0x08
#define IXP425_GPIO_GPISR_OFFSET        0x0C
#define IXP425_GPIO_GPIT1R_OFFSET	0x10
#define IXP425_GPIO_GPIT2R_OFFSET	0x14
#define IXP425_GPIO_GPCLKR_OFFSET	0x18
#define IXP425_GPIO_GPDBSELR_OFFSET	0x1C

/*
 * GPIO Register Definitions.
 * [Only perform 32bit reads/writes]
 */
#define IXP425_GPIO_REG(x) ((volatile u32 *)(IXP425_GPIO_BASE_PHYS+(x)))

#define IXP425_GPIO_GPOUTR	IXP425_GPIO_REG(IXP425_GPIO_GPOUTR_OFFSET)
#define IXP425_GPIO_GPOER       IXP425_GPIO_REG(IXP425_GPIO_GPOER_OFFSET)
#define IXP425_GPIO_GPINR       IXP425_GPIO_REG(IXP425_GPIO_GPINR_OFFSET)
#define IXP425_GPIO_GPISR       IXP425_GPIO_REG(IXP425_GPIO_GPISR_OFFSET)
#define IXP425_GPIO_GPIT1R      IXP425_GPIO_REG(IXP425_GPIO_GPIT1R_OFFSET)
#define IXP425_GPIO_GPIT2R      IXP425_GPIO_REG(IXP425_GPIO_GPIT2R_OFFSET)
#define IXP425_GPIO_GPCLKR      IXP425_GPIO_REG(IXP425_GPIO_GPCLKR_OFFSET)
#define IXP425_GPIO_GPDBSELR    IXP425_GPIO_REG(IXP425_GPIO_GPDBSELR_OFFSET)

#define IXP425_GPIO_GPITR(line)	(((line) >= 8) ? \
				IXP425_GPIO_GPIT2R : IXP425_GPIO_GPIT1R)

/*
 * Macros to make it easy to access the GPIO registers
 */
#define GPIO_OUTPUT_ENABLE(line)	*IXP425_GPIO_GPOER &= ~(1 << (line))
#define GPIO_OUTPUT_DISABLE(line)	*IXP425_GPIO_GPOER |= (1 << (line))
#define GPIO_OUTPUT_SET(line)		*IXP425_GPIO_GPOUTR |= (1 << (line))
#define GPIO_OUTPUT_CLEAR(line)		*IXP425_GPIO_GPOUTR &= ~(1 << (line))
#define GPIO_INT_ACT_LOW_SET(line)				\
	*IXP425_GPIO_GPITR(line) =				\
			(*IXP425_GPIO_GPITR(line) &		\
			~(0x7 << (((line) & 0x7) * 3))) |	\
			(0x1 << (((line) & 0x7) * 3))		\

/*
 * Constants to make it easy to access Timer Control/Status registers
 */
#define IXP425_OSTS_OFFSET	0x00  /* Continious TimeStamp */
#define IXP425_OST1_OFFSET	0x04  /* Timer 1 Timestamp */
#define IXP425_OSRT1_OFFSET	0x08  /* Timer 1 Reload */
#define IXP425_OST2_OFFSET	0x0C  /* Timer 2 Timestamp */
#define IXP425_OSRT2_OFFSET	0x10  /* Timer 2 Reload */
#define IXP425_OSWT_OFFSET	0x14  /* Watchdog Timer */
#define IXP425_OSWE_OFFSET	0x18  /* Watchdog Enable */
#define IXP425_OSWK_OFFSET	0x1C  /* Watchdog Key */
#define IXP425_OSST_OFFSET	0x20  /* Timer Status */

/*
 * Operating System Timer Register Definitions.
 */

#ifndef __ASSEMBLY__
#define IXP425_TIMER_REG(x) ((volatile u32 *)(IXP425_TIMER_BASE_PHYS+(x)))
#else
#define IXP425_TIMER_REG(x) (IXP425_TIMER_BASE_PHYS+(x))
#endif

/* _B to avoid collision: also defined in npe/include/... */
#define IXP425_OSTS_B	IXP425_TIMER_REG(IXP425_OSTS_OFFSET)
#define IXP425_OST1	IXP425_TIMER_REG(IXP425_OST1_OFFSET)
#define IXP425_OSRT1	IXP425_TIMER_REG(IXP425_OSRT1_OFFSET)
#define IXP425_OST2	IXP425_TIMER_REG(IXP425_OST2_OFFSET)
#define IXP425_OSRT2	IXP425_TIMER_REG(IXP425_OSRT2_OFFSET)
#define IXP425_OSWT	IXP425_TIMER_REG(IXP425_OSWT_OFFSET)
#define IXP425_OSWE	IXP425_TIMER_REG(IXP425_OSWE_OFFSET)
#define IXP425_OSWK	IXP425_TIMER_REG(IXP425_OSWK_OFFSET)
#define IXP425_OSST	IXP425_TIMER_REG(IXP425_OSST_OFFSET)

/*
 * Timer register values and bit definitions
 */
#define IXP425_OST_ENABLE              BIT(0)
#define IXP425_OST_ONE_SHOT            BIT(1)
/* Low order bits of reload value ignored */
#define IXP425_OST_RELOAD_MASK         (0x3)
#define IXP425_OST_DISABLED            (0x0)
#define IXP425_OSST_TIMER_1_PEND       BIT(0)
#define IXP425_OSST_TIMER_2_PEND       BIT(1)
#define IXP425_OSST_TIMER_TS_PEND      BIT(2)
#define IXP425_OSST_TIMER_WDOG_PEND    BIT(3)
#define IXP425_OSST_TIMER_WARM_RESET   BIT(4)

/*
 * Constants to make it easy to access PCI Control/Status registers
 */
#define PCI_NP_AD_OFFSET            0x00
#define PCI_NP_CBE_OFFSET           0x04
#define PCI_NP_WDATA_OFFSET         0x08
#define PCI_NP_RDATA_OFFSET         0x0c
#define PCI_CRP_AD_CBE_OFFSET       0x10
#define PCI_CRP_WDATA_OFFSET        0x14
#define PCI_CRP_RDATA_OFFSET        0x18
#define PCI_CSR_OFFSET              0x1c
#define PCI_ISR_OFFSET              0x20
#define PCI_INTEN_OFFSET            0x24
#define PCI_DMACTRL_OFFSET          0x28
#define PCI_AHBMEMBASE_OFFSET       0x2c
#define PCI_AHBIOBASE_OFFSET        0x30
#define PCI_PCIMEMBASE_OFFSET       0x34
#define PCI_AHBDOORBELL_OFFSET      0x38
#define PCI_PCIDOORBELL_OFFSET      0x3C
#define PCI_ATPDMA0_AHBADDR_OFFSET  0x40
#define PCI_ATPDMA0_PCIADDR_OFFSET  0x44
#define PCI_ATPDMA0_LENADDR_OFFSET  0x48
#define PCI_ATPDMA1_AHBADDR_OFFSET  0x4C
#define PCI_ATPDMA1_PCIADDR_OFFSET  0x50
#define PCI_ATPDMA1_LENADDR_OFFSET  0x54

/*
 * PCI Control/Status Registers
 */
#define IXP425_PCI_CSR(x) ((volatile u32 *)(IXP425_PCI_CFG_BASE_PHYS+(x)))

#define PCI_NP_AD               IXP425_PCI_CSR(PCI_NP_AD_OFFSET)
#define PCI_NP_CBE              IXP425_PCI_CSR(PCI_NP_CBE_OFFSET)
#define PCI_NP_WDATA            IXP425_PCI_CSR(PCI_NP_WDATA_OFFSET)
#define PCI_NP_RDATA            IXP425_PCI_CSR(PCI_NP_RDATA_OFFSET)
#define PCI_CRP_AD_CBE          IXP425_PCI_CSR(PCI_CRP_AD_CBE_OFFSET)
#define PCI_CRP_WDATA           IXP425_PCI_CSR(PCI_CRP_WDATA_OFFSET)
#define PCI_CRP_RDATA           IXP425_PCI_CSR(PCI_CRP_RDATA_OFFSET)
#define PCI_CSR                 IXP425_PCI_CSR(PCI_CSR_OFFSET)
#define PCI_ISR                 IXP425_PCI_CSR(PCI_ISR_OFFSET)
#define PCI_INTEN               IXP425_PCI_CSR(PCI_INTEN_OFFSET)
#define PCI_DMACTRL             IXP425_PCI_CSR(PCI_DMACTRL_OFFSET)
#define PCI_AHBMEMBASE          IXP425_PCI_CSR(PCI_AHBMEMBASE_OFFSET)
#define PCI_AHBIOBASE           IXP425_PCI_CSR(PCI_AHBIOBASE_OFFSET)
#define PCI_PCIMEMBASE          IXP425_PCI_CSR(PCI_PCIMEMBASE_OFFSET)
#define PCI_AHBDOORBELL         IXP425_PCI_CSR(PCI_AHBDOORBELL_OFFSET)
#define PCI_PCIDOORBELL         IXP425_PCI_CSR(PCI_PCIDOORBELL_OFFSET)
#define PCI_ATPDMA0_AHBADDR     IXP425_PCI_CSR(PCI_ATPDMA0_AHBADDR_OFFSET)
#define PCI_ATPDMA0_PCIADDR     IXP425_PCI_CSR(PCI_ATPDMA0_PCIADDR_OFFSET)
#define PCI_ATPDMA0_LENADDR     IXP425_PCI_CSR(PCI_ATPDMA0_LENADDR_OFFSET)
#define PCI_ATPDMA1_AHBADDR     IXP425_PCI_CSR(PCI_ATPDMA1_AHBADDR_OFFSET)
#define PCI_ATPDMA1_PCIADDR     IXP425_PCI_CSR(PCI_ATPDMA1_PCIADDR_OFFSET)
#define PCI_ATPDMA1_LENADDR     IXP425_PCI_CSR(PCI_ATPDMA1_LENADDR_OFFSET)

/*
 * PCI register values and bit definitions
 */

/* CSR bit definitions */
#define PCI_CSR_HOST		BIT(0)
#define PCI_CSR_ARBEN		BIT(1)
#define PCI_CSR_ADS		BIT(2)
#define PCI_CSR_PDS		BIT(3)
#define PCI_CSR_ABE		BIT(4)
#define PCI_CSR_DBT		BIT(5)
#define PCI_CSR_ASE		BIT(8)
#define PCI_CSR_IC		BIT(15)

/* ISR (Interrupt status) Register bit definitions */
#define PCI_ISR_PSE		BIT(0)
#define PCI_ISR_PFE		BIT(1)
#define PCI_ISR_PPE		BIT(2)
#define PCI_ISR_AHBE		BIT(3)
#define PCI_ISR_APDC		BIT(4)
#define PCI_ISR_PADC		BIT(5)
#define PCI_ISR_ADB		BIT(6)
#define PCI_ISR_PDB		BIT(7)

/* INTEN (Interrupt Enable) Register bit definitions */
#define PCI_INTEN_PSE		BIT(0)
#define PCI_INTEN_PFE		BIT(1)
#define PCI_INTEN_PPE		BIT(2)
#define PCI_INTEN_AHBE		BIT(3)
#define PCI_INTEN_APDC		BIT(4)
#define PCI_INTEN_PADC		BIT(5)
#define PCI_INTEN_ADB		BIT(6)
#define PCI_INTEN_PDB		BIT(7)

/*
 * Shift value for byte enable on NP cmd/byte enable register
 */
#define IXP425_PCI_NP_CBE_BESL	4

/*
 * PCI commands supported by NP access unit
 */
#define NP_CMD_IOREAD		0x2
#define NP_CMD_IOWRITE		0x3
#define NP_CMD_CONFIGREAD	0xa
#define NP_CMD_CONFIGWRITE	0xb
#define NP_CMD_MEMREAD		0x6
#define	NP_CMD_MEMWRITE		0x7

#if 0
#ifndef __ASSEMBLY__
extern int ixp425_pci_read(u32 addr, u32 cmd, u32* data);
extern int ixp425_pci_write(u32 addr, u32 cmd, u32 data);
extern void ixp425_pci_init(void *);
#endif
#endif

/*
 * Constants for CRP access into local config space
 */
#define CRP_AD_CBE_BESL         20
#define CRP_AD_CBE_WRITE        BIT(16)

/*
 * Clock Speed Definitions.
 */
#define IXP425_PERIPHERAL_BUS_CLOCK (66) /* 66Mhzi APB BUS   */


#endif
