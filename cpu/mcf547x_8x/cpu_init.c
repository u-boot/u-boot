/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <MCD_dma.h>
#include <asm/immap.h>

#if defined(CONFIG_CMD_NET)
#include <config.h>
#include <net.h>
#include <asm/fsl_mcdmafec.h>
#endif

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	volatile fbcs_t *fbcs = (fbcs_t *) MMAP_FBCS;
	volatile xlbarb_t *xlbarb = (volatile xlbarb_t *) MMAP_XARB;

	xlbarb->adrto = 0x2000;
	xlbarb->datto = 0x2500;
	xlbarb->busto = 0x3000;

	xlbarb->cfg = XARB_CFG_AT | XARB_CFG_DT;

	/* Master Priority Enable */
	xlbarb->prien = 0xff;
	xlbarb->pri = 0;

#if (defined(CONFIG_SYS_CS0_BASE) && defined(CONFIG_SYS_CS0_MASK) && defined(CONFIG_SYS_CS0_CTRL))
	fbcs->csar0 = CONFIG_SYS_CS0_BASE;
	fbcs->cscr0 = CONFIG_SYS_CS0_CTRL;
	fbcs->csmr0 = CONFIG_SYS_CS0_MASK;
#endif

#if (defined(CONFIG_SYS_CS1_BASE) && defined(CONFIG_SYS_CS1_MASK) && defined(CONFIG_SYS_CS1_CTRL))
	fbcs->csar1 = CONFIG_SYS_CS1_BASE;
	fbcs->cscr1 = CONFIG_SYS_CS1_CTRL;
	fbcs->csmr1 = CONFIG_SYS_CS1_MASK;
#endif

#if (defined(CONFIG_SYS_CS2_BASE) && defined(CONFIG_SYS_CS2_MASK) && defined(CONFIG_SYS_CS2_CTRL))
	fbcs->csar2 = CONFIG_SYS_CS2_BASE;
	fbcs->cscr2 = CONFIG_SYS_CS2_CTRL;
	fbcs->csmr2 = CONFIG_SYS_CS2_MASK;
#endif

#if (defined(CONFIG_SYS_CS3_BASE) && defined(CONFIG_SYS_CS3_MASK) && defined(CONFIG_SYS_CS3_CTRL))
	fbcs->csar3 = CONFIG_SYS_CS3_BASE;
	fbcs->cscr3 = CONFIG_SYS_CS3_CTRL;
	fbcs->csmr3 = CONFIG_SYS_CS3_MASK;
#endif

#if (defined(CONFIG_SYS_CS4_BASE) && defined(CONFIG_SYS_CS4_MASK) && defined(CONFIG_SYS_CS4_CTRL))
	fbcs->csar4 = CONFIG_SYS_CS4_BASE;
	fbcs->cscr4 = CONFIG_SYS_CS4_CTRL;
	fbcs->csmr4 = CONFIG_SYS_CS4_MASK;
#endif

#if (defined(CONFIG_SYS_CS5_BASE) && defined(CONFIG_SYS_CS5_MASK) && defined(CONFIG_SYS_CS5_CTRL))
	fbcs->csar5 = CONFIG_SYS_CS5_BASE;
	fbcs->cscr5 = CONFIG_SYS_CS5_CTRL;
	fbcs->csmr5 = CONFIG_SYS_CS5_MASK;
#endif

#ifdef CONFIG_FSL_I2C
	gpio->par_feci2cirq = GPIO_PAR_FECI2CIRQ_SCL | GPIO_PAR_FECI2CIRQ_SDA;
#endif

	icache_enable();
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
#if defined(CONFIG_CMD_NET) && defined(CONFIG_FSLDMAFEC)
	MCD_initDma((dmaRegs *) (MMAP_MCDMA), (void *)(MMAP_SRAM + 512),
		    MCD_RELOC_TASKS);
#endif
	return (0);
}

void uart_port_conf(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	volatile u8 *pscsicr = (u8 *) (CONFIG_SYS_UART_BASE + 0x40);

	/* Setup Ports: */
	switch (CONFIG_SYS_UART_PORT) {
	case 0:
		gpio->par_psc0 = (GPIO_PAR_PSC0_TXD0 | GPIO_PAR_PSC0_RXD0);
		break;
	case 1:
		gpio->par_psc1 = (GPIO_PAR_PSC1_TXD1 | GPIO_PAR_PSC1_RXD1);
		break;
	case 2:
		gpio->par_psc2 = (GPIO_PAR_PSC2_TXD2 | GPIO_PAR_PSC2_RXD2);
		break;
	case 3:
		gpio->par_psc3 = (GPIO_PAR_PSC3_TXD3 | GPIO_PAR_PSC3_RXD3);
		break;
	}

	*pscsicr &= 0xF8;
}

#if defined(CONFIG_CMD_NET)
int fecpin_setclear(struct eth_device *dev, int setclear)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	struct fec_info_dma *info = (struct fec_info_dma *)dev->priv;

	if (setclear) {
		if (info->iobase == CONFIG_SYS_FEC0_IOBASE)
			gpio->par_feci2cirq |= 0xF000;
		else
			gpio->par_feci2cirq |= 0x0FC0;
	} else {
		if (info->iobase == CONFIG_SYS_FEC0_IOBASE)
			gpio->par_feci2cirq &= 0x0FFF;
		else
			gpio->par_feci2cirq &= 0xF03F;
	}
	return 0;
}
#endif
