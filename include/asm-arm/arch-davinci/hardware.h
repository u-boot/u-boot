/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Based on:
 *
 * -------------------------------------------------------------------------
 *
 *  linux/include/asm-arm/arch-davinci/hardware.h
 *
 *  Copyright (C) 2006 Texas Instruments.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <config.h>
#include <asm/sizes.h>

#define	REG(addr)	(*(volatile unsigned int *)(addr))
#define REG_P(addr)	((volatile unsigned int *)(addr))

typedef volatile unsigned int	dv_reg;
typedef volatile unsigned int *	dv_reg_p;

/*
 * Base register addresses
 */
#define DAVINCI_DMA_3PCC_BASE			(0x01c00000)
#define DAVINCI_DMA_3PTC0_BASE			(0x01c10000)
#define DAVINCI_DMA_3PTC1_BASE			(0x01c10400)
#define DAVINCI_UART0_BASE			(0x01c20000)
#define DAVINCI_UART1_BASE			(0x01c20400)
#define DAVINCI_UART2_BASE			(0x01c20800)
#define DAVINCI_I2C_BASE			(0x01c21000)
#define DAVINCI_TIMER0_BASE			(0x01c21400)
#define DAVINCI_TIMER1_BASE			(0x01c21800)
#define DAVINCI_WDOG_BASE			(0x01c21c00)
#define DAVINCI_PWM0_BASE			(0x01c22000)
#define DAVINCI_PWM1_BASE			(0x01c22400)
#define DAVINCI_PWM2_BASE			(0x01c22800)
#define DAVINCI_SYSTEM_MODULE_BASE		(0x01c40000)
#define DAVINCI_PLL_CNTRL0_BASE			(0x01c40800)
#define DAVINCI_PLL_CNTRL1_BASE			(0x01c40c00)
#define DAVINCI_PWR_SLEEP_CNTRL_BASE		(0x01c41000)
#define DAVINCI_SYSTEM_DFT_BASE			(0x01c42000)
#define DAVINCI_ARM_INTC_BASE			(0x01c48000)
#define DAVINCI_IEEE1394_BASE			(0x01c60000)
#define DAVINCI_USB_OTG_BASE			(0x01c64000)
#define DAVINCI_CFC_ATA_BASE			(0x01c66000)
#define DAVINCI_SPI_BASE			(0x01c66800)
#define DAVINCI_GPIO_BASE			(0x01c67000)
#define DAVINCI_UHPI_BASE			(0x01c67800)
#define DAVINCI_VPSS_REGS_BASE			(0x01c70000)
#define DAVINCI_EMAC_CNTRL_REGS_BASE		(0x01c80000)
#define DAVINCI_EMAC_WRAPPER_CNTRL_REGS_BASE	(0x01c81000)
#define DAVINCI_EMAC_WRAPPER_RAM_BASE		(0x01c82000)
#define DAVINCI_MDIO_CNTRL_REGS_BASE		(0x01c84000)
#define DAVINCI_IMCOP_BASE			(0x01cc0000)
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE		(0x01e00000)
#define DAVINCI_VLYNQ_BASE			(0x01e01000)
#define DAVINCI_MCBSP_BASE			(0x01e02000)
#define DAVINCI_MMC_SD_BASE			(0x01e10000)
#define DAVINCI_MS_BASE				(0x01e20000)
#define DAVINCI_ASYNC_EMIF_DATA_CE0_BASE	(0x02000000)
#define DAVINCI_ASYNC_EMIF_DATA_CE1_BASE	(0x04000000)
#define DAVINCI_ASYNC_EMIF_DATA_CE2_BASE	(0x06000000)
#define DAVINCI_ASYNC_EMIF_DATA_CE3_BASE	(0x08000000)
#define DAVINCI_VLYNQ_REMOTE_BASE		(0x0c000000)

/* Power and Sleep Controller (PSC) Domains */
#define DAVINCI_GPSC_ARMDOMAIN		0
#define DAVINCI_GPSC_DSPDOMAIN		1

#define DAVINCI_LPSC_VPSSMSTR		0
#define DAVINCI_LPSC_VPSSSLV		1
#define DAVINCI_LPSC_TPCC		2
#define DAVINCI_LPSC_TPTC0		3
#define DAVINCI_LPSC_TPTC1		4
#define DAVINCI_LPSC_EMAC		5
#define DAVINCI_LPSC_EMAC_WRAPPER	6
#define DAVINCI_LPSC_MDIO		7
#define DAVINCI_LPSC_IEEE1394		8
#define DAVINCI_LPSC_USB		9
#define DAVINCI_LPSC_ATA		10
#define DAVINCI_LPSC_VLYNQ		11
#define DAVINCI_LPSC_UHPI		12
#define DAVINCI_LPSC_DDR_EMIF		13
#define DAVINCI_LPSC_AEMIF		14
#define DAVINCI_LPSC_MMC_SD		15
#define DAVINCI_LPSC_MEMSTICK		16
#define DAVINCI_LPSC_McBSP		17
#define DAVINCI_LPSC_I2C		18
#define DAVINCI_LPSC_UART0		19
#define DAVINCI_LPSC_UART1		20
#define DAVINCI_LPSC_UART2		21
#define DAVINCI_LPSC_SPI		22
#define DAVINCI_LPSC_PWM0		23
#define DAVINCI_LPSC_PWM1		24
#define DAVINCI_LPSC_PWM2		25
#define DAVINCI_LPSC_GPIO		26
#define DAVINCI_LPSC_TIMER0		27
#define DAVINCI_LPSC_TIMER1		28
#define DAVINCI_LPSC_TIMER2		29
#define DAVINCI_LPSC_SYSTEM_SUBSYS	30
#define DAVINCI_LPSC_ARM		31
#define DAVINCI_LPSC_SCR2		32
#define DAVINCI_LPSC_SCR3		33
#define DAVINCI_LPSC_SCR4		34
#define DAVINCI_LPSC_CROSSBAR		35
#define DAVINCI_LPSC_CFG27		36
#define DAVINCI_LPSC_CFG3		37
#define DAVINCI_LPSC_CFG5		38
#define DAVINCI_LPSC_GEM		39
#define DAVINCI_LPSC_IMCOP		40

/* Some PSC defines */
#define PSC_CHP_SHRTSW			(0x01c40038)
#define PSC_GBLCTL			(0x01c41010)
#define PSC_EPCPR			(0x01c41070)
#define PSC_EPCCR			(0x01c41078)
#define PSC_PTCMD			(0x01c41120)
#define PSC_PTSTAT			(0x01c41128)
#define PSC_PDSTAT			(0x01c41200)
#define PSC_PDSTAT1			(0x01c41204)
#define PSC_PDCTL			(0x01c41300)
#define PSC_PDCTL1			(0x01c41304)

#define PSC_MDCTL_BASE			(0x01c41a00)
#define PSC_MDSTAT_BASE			(0x01c41800)

#define VDD3P3V_PWDN			(0x01c40048)
#define UART0_PWREMU_MGMT		(0x01c20030)
#define UART1_PWREMU_MGMT		(0x01c20430)
#define UART2_PWREMU_MGMT		(0x01c20830)

#define PSC_SILVER_BULLET		(0x01c41a20)

/* Some PLL defines */
#define PLL1_PLLM			(0x01c40910)
#define PLL2_PLLM			(0x01c40d10)
#define PLL2_DIV2			(0x01c40d1c)

/* Miscellania... */
#define VBPR				(0x20000020)
#define PINMUX0				(0x01c40000)
#define PINMUX1				(0x01c40004)

#endif /* __ASM_ARCH_HARDWARE_H */
