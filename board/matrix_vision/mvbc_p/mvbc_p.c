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
#include <asm/io.h>
#include "fpga.h"
#include "mvbc_p.h"

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
	test1 = get_ram_size((long *)CFG_SDRAM_BASE, 0x80000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CFG_SDRAM_BASE, 0x80000000);
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

	out_be32((u32*)&gpio->sint_ode, SINT_ODE);
	out_be32((u32*)&gpio->sint_ddr, SINT_DDR);
	out_be32((u32*)&gpio->sint_dvo, SINT_DVO);
	out_be32((u32*)&gpio->sint_inten, SINT_INTEN);
	out_be32((u32*)&gpio->sint_itype, SINT_ITYPE);
	out_be32((u32*)&gpio->sint_gpioe, SINT_GPIOEN);

	out_8((u8*)MPC5XXX_WU_GPIO_ODE, WKUP_ODE);
	out_8((u8*)MPC5XXX_WU_GPIO_DIR, WKUP_DIR);
	out_8((u8*)MPC5XXX_WU_GPIO_DATA_O, WKUP_DO);
	out_8((u8*)MPC5XXX_WU_GPIO_ENABLE, WKUP_EN);

	printf("simple_gpioe: 0x%08x\n", gpio->simple_gpioe);
	printf("sint_gpioe  : 0x%08x\n", gpio->sint_gpioe);
}

void reset_environment(void)
{
	char *s, sernr[64];

	printf("\n*** RESET ENVIRONMENT ***\n");
	memset(sernr, 0, sizeof(sernr));
	s = getenv("serial#");
	if (s) {
		printf("found serial# : %s\n", s);
		strncpy(sernr, s, 64);
	}
	gd->env_valid = 0;
	env_relocate();
	if (s)
		setenv("serial#", sernr);
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
	reset_environment();
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
	out_be32((u32*)MPC5XXX_BOOTCS_START, START_REG(CFG_BOOTCS_START |
		size));
	out_be32((u32*)MPC5XXX_CS0_START, START_REG(CFG_BOOTCS_START |
		size));
	out_be32((u32*)MPC5XXX_BOOTCS_STOP, STOP_REG(CFG_BOOTCS_START | size,
		size));
	out_be32((u32*)MPC5XXX_CS0_STOP, STOP_REG(CFG_BOOTCS_START | size,
		size));
}

void pci_mvbc_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	unsigned char line = 0xff;
	u32 base;

	if (PCI_BUS(dev) == 0) {
		switch (PCI_DEV (dev)) {
		case 0xa: /* FPGA */
			line = 3;
			pci_hose_read_config_dword(hose, dev, PCI_BASE_ADDRESS_0, &base);
			printf("found FPA - enable arbitration\n");
			writel(0x03, (u32*)(base + 0x80c0));
			writel(0xf0, (u32*)(base + 0x8080));
			break;
		case 0xb: /* LAN */
			line = 2;
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

int mvbc_p_load_fpga(void)
{
	size_t data_size = 0;
	void *fpga_data = NULL;
	char *datastr = getenv("fpgadata");
	char *sizestr = getenv("fpgadatasize");

	if (datastr)
		fpga_data = (void *)simple_strtoul(datastr, NULL, 16);
	if (sizestr)
		data_size = (size_t)simple_strtoul(sizestr, NULL, 16);

	return fpga_load(0, fpga_data, data_size);
}

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	char *s;
	int load_fpga = 1;

	mvbc_p_init_fpga();
	s = getenv("skip_fpga");
	if (s) {
		printf("found 'skip_fpga' -> FPGA _not_ loaded !\n");
		load_fpga = 0;
	}
	if (load_fpga) {
		printf("loading FPGA ... ");
		mvbc_p_load_fpga();
		printf("done\n");
	}
	pci_mpc5xxx_init(&hose);
}

u8 *dhcp_vendorex_prep(u8 *e)
{
	char *ptr;

	/* DHCP vendor-class-identifier = 60 */
	if ((ptr = getenv("dhcp_vendor-class-identifier"))) {
		*e++ = 60;
		*e++ = strlen(ptr);
		while (*ptr)
			*e++ = *ptr++;
	}
	/* DHCP_CLIENT_IDENTIFIER = 61 */
	if ((ptr = getenv("dhcp_client_id"))) {
		*e++ = 61;
		*e++ = strlen(ptr);
		while (*ptr)
			*e++ = *ptr++;
	}

	return e;
}

u8 *dhcp_vendorex_proc (u8 *popt)
{
	return NULL;
}

void show_boot_progress(int val)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio*)MPC5XXX_GPIO;

	switch(val) {
	case 0: /* FPGA ok */
		setbits_be32(&gpio->simple_dvo, 0x80);
		break;
	case 1:
		setbits_be32(&gpio->simple_dvo, 0x40);
		break;
	case 12:
		setbits_be32(&gpio->simple_dvo, 0x20);
		break;
	case 15:
		setbits_be32(&gpio->simple_dvo, 0x10);
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
