/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2005-2007
 * Andre Schwarz, Matrix Vision GmbH, andre.schwarz@matrix-vision.de
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <mpc5xxx.h>
#include <malloc.h>
#include <pci.h>
#include <i2c.h>
#include <fpga.h>
#include <environment.h>
#include <fdt_support.h>
#include <netdev.h>
#include <asm/io.h>
#include "fpga.h"
#include "mvbc_p.h"
#include "../common/mv_common.h"

#define SDRAM_MODE	0x00CD0000
#define SDRAM_CONTROL	0x504F0000
#define SDRAM_CONFIG1	0xD2322800
#define SDRAM_CONFIG2	0x8AD70000

DECLARE_GLOBAL_DATA_PTR;

static void sdram_start (int hi_addr)
{
	long hi_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	out_be32((u32*)MPC5XXX_SDRAM_CTRL, SDRAM_CONTROL | 0x80000000 | hi_bit);

	/* precharge all banks */
	out_be32((u32*)MPC5XXX_SDRAM_CTRL, SDRAM_CONTROL | 0x80000002 | hi_bit);

	/* precharge all banks */
	out_be32((u32*)MPC5XXX_SDRAM_CTRL, SDRAM_CONTROL | 0x80000002 | hi_bit);

	/* auto refresh */
	out_be32((u32*)MPC5XXX_SDRAM_CTRL, SDRAM_CONTROL | 0x80000004 | hi_bit);

	/* set mode register */
	out_be32((u32*)MPC5XXX_SDRAM_MODE, SDRAM_MODE);

	/* normal operation */
	out_be32((u32*)MPC5XXX_SDRAM_CTRL, SDRAM_CONTROL | hi_bit);
}

phys_addr_t initdram (int board_type)
{
	ulong dramsize = 0;
	ulong test1,
	      test2;

	/* setup SDRAM chip selects */
	out_be32((u32*)MPC5XXX_SDRAM_CS0CFG, 0x0000001e);

	/* setup config registers */
	out_be32((u32*)MPC5XXX_SDRAM_CONFIG1, SDRAM_CONFIG1);
	out_be32((u32*)MPC5XXX_SDRAM_CONFIG2, SDRAM_CONFIG2);

	/* find RAM size using SDRAM CS0 only */
	sdram_start(0);
	test1 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	if (test1 > test2) {
		sdram_start(0);
		dramsize = test1;
	} else
		dramsize = test2;

	if (dramsize < (1 << 20))
		dramsize = 0;

	if (dramsize > 0)
		out_be32((u32*)MPC5XXX_SDRAM_CS0CFG, 0x13 +
			__builtin_ffs(dramsize >> 20) - 1);
	else
		out_be32((u32*)MPC5XXX_SDRAM_CS0CFG, 0);

	return dramsize;
}

void mvbc_init_gpio(void)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio*)MPC5XXX_GPIO;

	printf("Ports : 0x%08x\n", gpio->port_config);
	printf("PORCFG: 0x%08lx\n", *(vu_long*)MPC5XXX_CDM_PORCFG);

	out_be32(&gpio->simple_ddr, SIMPLE_DDR);
	out_be32(&gpio->simple_dvo, SIMPLE_DVO);
	out_be32(&gpio->simple_ode, SIMPLE_ODE);
	out_be32(&gpio->simple_gpioe, SIMPLE_GPIOEN);

	out_8(&gpio->sint_ode, SINT_ODE);
	out_8(&gpio->sint_ddr, SINT_DDR);
	out_8(&gpio->sint_dvo, SINT_DVO);
	out_8(&gpio->sint_inten, SINT_INTEN);
	out_be16(&gpio->sint_itype, SINT_ITYPE);
	out_8(&gpio->sint_gpioe, SINT_GPIOEN);

	out_8((u8*)MPC5XXX_WU_GPIO_ODE, WKUP_ODE);
	out_8((u8*)MPC5XXX_WU_GPIO_DIR, WKUP_DIR);
	out_8((u8*)MPC5XXX_WU_GPIO_DATA_O, WKUP_DO);
	out_8((u8*)MPC5XXX_WU_GPIO_ENABLE, WKUP_EN);

	printf("simple_gpioe: 0x%08x\n", gpio->simple_gpioe);
	printf("sint_gpioe  : 0x%08x\n", gpio->sint_gpioe);
}

int misc_init_r(void)
{
	char *s = getenv("reset_env");

	if (!s) {
		if (in_8((u8*)MPC5XXX_WU_GPIO_DATA_I) & MPC5XXX_GPIO_WKUP_6)
			return 0;
		udelay(50000);
		if (in_8((u8*)MPC5XXX_WU_GPIO_DATA_I) & MPC5XXX_GPIO_WKUP_6)
			return 0;
		udelay(50000);
		if (in_8((u8*)MPC5XXX_WU_GPIO_DATA_I) & MPC5XXX_GPIO_WKUP_6)
			return 0;
	}
	printf(" === FACTORY RESET ===\n");
	mv_reset_environment();
	saveenv();

	return -1;
}

int checkboard(void)
{
	mvbc_init_gpio();
	printf("Board: Matrix Vision mvBlueCOUGAR-P\n");

	return 0;
}

void flash_preinit(void)
{
	/*
	 * Now, when we are in RAM, enable flash write
	 * access for detection process.
	 * Note that CS_BOOT cannot be cleared when
	 * executing in flash.
	 */
	clrbits_be32((u32*)MPC5XXX_BOOTCS_CFG, 0x1);
}

void flash_afterinit(ulong size)
{
	out_be32((u32*)MPC5XXX_BOOTCS_START, START_REG(CONFIG_SYS_BOOTCS_START |
		size));
	out_be32((u32*)MPC5XXX_CS0_START, START_REG(CONFIG_SYS_BOOTCS_START |
		size));
	out_be32((u32*)MPC5XXX_BOOTCS_STOP, STOP_REG(CONFIG_SYS_BOOTCS_START | size,
		size));
	out_be32((u32*)MPC5XXX_CS0_STOP, STOP_REG(CONFIG_SYS_BOOTCS_START | size,
		size));
}

void pci_mvbc_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	unsigned char line = 0xff;
	char *s = getenv("pci_latency");
	u32 base;
	u8 val = 0;

	if (s)
		val = simple_strtoul(s, NULL, 16);

	if (PCI_BUS(dev) == 0) {
		switch (PCI_DEV (dev)) {
		case 0xa: /* FPGA */
			line = 3;
			pci_hose_read_config_dword(hose, dev, PCI_BASE_ADDRESS_0, &base);
			printf("found FPGA - enable arbitration\n");
			writel(0x03, (u32*)(base + 0x80c0));
			writel(0xf0, (u32*)(base + 0x8080));
			if (val)
				pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, val);
			break;
		case 0xb: /* LAN */
			line = 2;
			if (val)
				pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, val);
			break;
		case 0x1a:
			break;
		default:
			printf ("***pci_scan: illegal dev = 0x%08x\n", PCI_DEV (dev));
			break;
		}
		pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE, line);
	}
}

struct pci_controller hose = {
	fixup_irq:pci_mvbc_fixup_irq
};

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	mvbc_p_init_fpga();
	mv_load_fpga();
	pci_mpc5xxx_init(&hose);
}

void show_boot_progress(int val)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio*)MPC5XXX_GPIO;

	switch(val) {
	case 0: /* FPGA ok */
		setbits_be32(&gpio->simple_dvo, LED_G0);
		break;
	case 65:
		setbits_be32(&gpio->simple_dvo, LED_G1);
		break;
	case 12:
		setbits_be32(&gpio->simple_dvo, LED_Y);
		break;
	case 15:
		setbits_be32(&gpio->simple_dvo, LED_R);
		break;
	default:
		break;
	}

}

void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);
}

int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis); /* Built in FEC comes first */
	return pci_eth_init(bis);
}
