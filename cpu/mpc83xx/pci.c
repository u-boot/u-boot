/*
 * Copyright 2004 Freescale Semiconductor.
 * Copyright (C) 2003 Motorola Inc.
 * Xianghua Xiao (x.xiao@motorola.com)
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
 *
 * Change log:
 *
 * 20050101: Eran Liberty (liberty@freescale.com)
 *           Initial file creating (porting from 85XX & 8260)
 */

/*
 * PCI Configuration space access support for MPC85xx PCI Bridge
 */
#include <asm/mmu.h>
#include <asm/io.h>
#include <common.h>
#include <pci.h>

#ifdef CONFIG_MPC8349ADS
#include <asm/i2c.h>
#endif

#if defined(CONFIG_PCI)

void
pci_mpc83xx_init(volatile struct pci_controller *hose)
{
	volatile immap_t *	immr;
	volatile clk8349_t *	clk;
	volatile law8349_t *	pci_law;
	volatile pot8349_t *	pci_pot;
	volatile pcictrl8349_t *	pci_ctrl;
	volatile pciconf8349_t *	pci_conf;

	u8 val8,tmp8,ret;
	u16 reg16,tmp16;
	u32 val32,tmp32;

	immr = (immap_t *)CFG_IMMRBAR;
	clk = (clk8349_t *)&immr->clk;
	pci_law = immr->sysconf.pcilaw;
	pci_pot = immr->ios.pot;
	pci_ctrl = immr->pci_ctrl;
	pci_conf = immr->pci_conf;

	/*
	 * Configure PCI controller and PCI_CLK_OUTPUT both in 66M mode
	 */
	val32 = clk->occr;
	udelay(2000);
	clk->occr = 0xff000000;
	udelay(2000);

	/*
	 * Configure PCI Local Access Windows
	 */
	pci_law[0].bar = CFG_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LAWAR_EN | LAWAR_SIZE_1G;
	pci_law[1].bar = CFG_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LAWAR_EN | LAWAR_SIZE_32M;

	/*
	 * Configure PCI Outbound Translation Windows
	 */
	pci_pot[0].potar = (CFG_PCI1_MEM_BASE >> 12) & POTAR_TA_MASK;
	pci_pot[0].pobar = (CFG_PCI1_MEM_PHYS >> 12) & POBAR_BA_MASK;
	pci_pot[0].pocmr = POCMR_EN | (POCMR_CM_512M & POCMR_CM_MASK);

	/* mapped to PCI1 IO space 0x0 to local 0xe2000000  */
	pci_pot[1].potar = (CFG_PCI1_IO_BASE >> 12) & POTAR_TA_MASK;
	pci_pot[1].pobar = (CFG_PCI1_IO_PHYS >> 12) & POBAR_BA_MASK;
	pci_pot[1].pocmr = POCMR_EN | POCMR_IO | (POCMR_CM_16M & POCMR_CM_MASK);

	pci_pot[3].potar = (CFG_PCI2_MEM_BASE >> 12) & POTAR_TA_MASK;
	pci_pot[3].pobar = (CFG_PCI2_MEM_PHYS >> 12) & POBAR_BA_MASK;
	pci_pot[3].pocmr = POCMR_EN | POCMR_DST | (POCMR_CM_512M & POCMR_CM_MASK);

	/* mapped to PCI2 IO space 0x0 to local 0xe3000000  */
	pci_pot[4].potar = (CFG_PCI2_IO_BASE >> 12) & POTAR_TA_MASK;
	pci_pot[4].pobar = (CFG_PCI2_IO_PHYS >> 12) & POBAR_BA_MASK;
	pci_pot[4].pocmr = POCMR_EN | POCMR_DST | POCMR_IO | (POCMR_CM_16M & POCMR_CM_MASK);

	/*
	 * Configure PCI Inbound Translation Windows
	 */
	pci_ctrl[0].pitar1 = 0x0;
	pci_ctrl[0].pibar1 = 0x0;
	pci_ctrl[0].piebar1 = 0x0;
	pci_ctrl[0].piwar1 = PIWAR_EN | PIWAR_PF | PIWAR_RTT_SNOOP | PIWAR_WTT_SNOOP | PIWAR_IWS_2G;

	pci_ctrl[1].pitar1 = 0x0;
	pci_ctrl[1].pibar1 = 0x0;
	pci_ctrl[1].piebar1 = 0x0;
	pci_ctrl[1].piwar1 = PIWAR_EN | PIWAR_PF | PIWAR_RTT_SNOOP | PIWAR_WTT_SNOOP | PIWAR_IWS_2G;
	/*
	 * Assign PIB PMC slot to desired PCI bus
	 */
#ifdef CONFIG_MPC8349ADS
	mpc8349_i2c = (i2c_t*)(CFG_IMMRBAR + CFG_I2C2_OFFSET);
	i2c_init(CFG_I2C_SPEED,CFG_I2C_SLAVE);
#endif
	val8 = 0;
	ret = i2c_write(0x23,0x6,1,&val8,1);
	ret = i2c_write(0x23,0x7,1,&val8,1);
	val8 = 0xff;
	ret = i2c_write(0x23,0x2,1,&val8,1);
	ret = i2c_write(0x23,0x3,1,&val8,1);

	val8 = 0;
	ret = i2c_write(0x26,0x6,1,&val8,1);
	val8 = 0x34;
	ret = i2c_write(0x26,0x7,1,&val8,1);
#if defined(PCI_64BIT)
	val8 = 0xf4;	/* PMC2<->PCI1  64bit */
#elif defined(PCI_ALL_PCI1)
	val8 = 0xf3;	/* PMC1<->PCI1,PMC2<->PCI1,PMC3<->PCI1  32bit */
#elif defined(PCI_ONE_PCI1)
	val8 = 0xf9;	/* PMC1<->PCI1,PMC2<->PCI2,PMC3<->PCI2  32bit */
#elif defined(PCI_TWO_PCI1)
	val8 = 0xf5;	/* PMC1<->PCI1,PMC2<->PCI1,PMC3<->PCI2 32bit */
#else
	val8 = 0xf5;
#endif
	ret = i2c_write(0x26,0x2,1,&val8,1);
	val8 = 0xff;
	ret = i2c_write(0x26,0x3,1,&val8,1);
	val8 = 0;
	ret = i2c_write(0x27,0x6,1,&val8,1);
	ret = i2c_write(0x27,0x7,1,&val8,1);
	val8 = 0xff;
	ret = i2c_write(0x27,0x2,1,&val8,1);
	val8 = 0xef;
	ret = i2c_write(0x27,0x3,1,&val8,1);
	asm("eieio");

	/*
	 * Release PCI RST Output signal
	 */
	udelay(2000);
	pci_ctrl[0].gcr = 1;
#ifndef PCI_64BIT
	pci_ctrl[1].gcr = 1;
#endif
	udelay(2000);

	hose[0].first_busno = 0;
	hose[0].last_busno = 0xff;

	pci_set_region(hose[0].regions + 0,
		       CFG_PCI1_MEM_BASE,
		       CFG_PCI1_MEM_PHYS,
		       CFG_PCI1_MEM_SIZE,
		       PCI_REGION_MEM);

	pci_set_region(hose[0].regions + 1,
		       CFG_PCI1_IO_BASE,
		       CFG_PCI1_IO_PHYS,
		       CFG_PCI1_IO_SIZE,
		       PCI_REGION_IO);

	hose[0].region_count = 2;

	pci_setup_indirect(&hose[0],
			   (CFG_IMMRBAR+0x8300),
			   (CFG_IMMRBAR+0x8304));
#define PCI_CLASS_BRIDGE	0x06
	reg16 = 0xff;
	tmp32 = 0xffff;
	pci_hose_write_config_byte(&hose[0],PCI_BDF(0,0,0),PCI_CLASS_CODE,PCI_CLASS_BRIDGE);

	pci_hose_read_config_word (&hose[0],PCI_BDF(0,0,0),PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_hose_write_config_word(&hose[0],PCI_BDF(0,0,0), PCI_COMMAND, reg16);

	/*
	 * Clear non-reserved bits in status register.
	 */
	pci_hose_write_config_word(&hose[0],PCI_BDF(0,0,0), PCI_STATUS, 0xffff);
	pci_hose_write_config_byte(&hose[0],PCI_BDF(0,0,0), PCI_LATENCY_TIMER,0x80);
#ifndef PCI_64BIT
	hose[1].first_busno = 0;
	hose[1].last_busno = 0xff;

	pci_set_region(hose[1].regions + 0,
		       CFG_PCI2_MEM_BASE,
		       CFG_PCI2_MEM_PHYS,
		       CFG_PCI2_MEM_SIZE,
		       PCI_REGION_MEM);

	pci_set_region(hose[1].regions + 1,
		       CFG_PCI2_IO_BASE,
		       CFG_PCI2_IO_PHYS,
		       CFG_PCI2_IO_SIZE,
		       PCI_REGION_IO);

	hose[1].region_count = 2;

	pci_setup_indirect(&hose[1],
			   (CFG_IMMRBAR+0x8380),
			   (CFG_IMMRBAR+0x8384));

	pci_hose_write_config_byte(&hose[1],PCI_BDF(0,0,0),PCI_CLASS_CODE,PCI_CLASS_BRIDGE);
	pci_hose_read_config_word (&hose[1],PCI_BDF(0,0,0), PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_hose_write_config_word(&hose[1],PCI_BDF(0,0,0), PCI_COMMAND, reg16);

	/*
	 * Clear non-reserved bits in status register.
	 */
	pci_hose_write_config_word(&hose[1],PCI_BDF(0,0,0), PCI_STATUS, 0xffff);
	pci_hose_write_config_byte(&hose[1],PCI_BDF(0,0,0), PCI_LATENCY_TIMER,0x80);
#endif

#if defined(PCI_64BIT)
	printf("PCI1 64bit on PMC2\n");
#elif defined(PCI_ALL_PCI1)
	printf("PCI1 32bit on PMC1 & PMC2 & PMC3\n");
#elif defined(PCI_ONE_PCI1)
	printf("PCI1 32bit on PMC1,PCI2 32bit on PMC2 & PMC3\n");
#else
	printf("PCI1 32bit on PMC1 & PMC2 & PMC3 in default\n");
#endif

#if 1
	/*
	 * Hose scan.
	 */
	pci_register_hose(hose);
	hose->last_busno = pci_hose_scan(hose);
#endif
}

#endif /* CONFIG_PCI */
