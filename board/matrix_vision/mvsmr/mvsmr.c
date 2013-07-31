/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2005-2010
 * Andre Schwarz, Matrix Vision GmbH, andre.schwarz@matrix-vision.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc5xxx.h>
#include <malloc.h>
#include <pci.h>
#include <i2c.h>
#include <fpga.h>
#include <environment.h>
#include <netdev.h>
#include <asm/io.h>
#include "fpga.h"
#include "mvsmr.h"
#include "../common/mv_common.h"

#define SDRAM_DDR	1
#define SDRAM_MODE	0x018D0000
#define SDRAM_EMODE	0x40090000
#define SDRAM_CONTROL	0x715f0f00
#define SDRAM_CONFIG1	0xd3722930
#define SDRAM_CONFIG2	0x46770000

DECLARE_GLOBAL_DATA_PTR;

static void sdram_start(int hi_addr)
{
	long hi_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	out_be32((u32 *)MPC5XXX_SDRAM_CTRL, SDRAM_CONTROL | 0x80000000 |
		hi_bit);

	/* precharge all banks */
	out_be32((u32 *)MPC5XXX_SDRAM_CTRL, SDRAM_CONTROL | 0x80000002 |
		hi_bit);

	/* set mode register: extended mode */
	out_be32((u32 *)MPC5XXX_SDRAM_MODE, SDRAM_EMODE);

	/* set mode register: reset DLL */
	out_be32((u32 *)MPC5XXX_SDRAM_MODE, SDRAM_MODE | 0x04000000);

	/* precharge all banks */
	out_be32((u32 *)MPC5XXX_SDRAM_CTRL, SDRAM_CONTROL | 0x80000002 |
		hi_bit);

	/* auto refresh */
	out_be32((u32 *)MPC5XXX_SDRAM_CTRL, SDRAM_CONTROL | 0x80000004 |
		hi_bit);

	/* set mode register */
	out_be32((u32 *)MPC5XXX_SDRAM_MODE, SDRAM_MODE);

	/* normal operation */
	out_be32((u32 *)MPC5XXX_SDRAM_CTRL, SDRAM_CONTROL | hi_bit);
}

phys_addr_t initdram(int board_type)
{
	ulong dramsize = 0;
	ulong test1,
	      test2;

	/* setup SDRAM chip selects */
	out_be32((u32 *)MPC5XXX_SDRAM_CS0CFG, 0x0000001e);

	/* setup config registers */
	out_be32((u32 *)MPC5XXX_SDRAM_CONFIG1, SDRAM_CONFIG1);
	out_be32((u32 *)MPC5XXX_SDRAM_CONFIG2, SDRAM_CONFIG2);

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
		out_be32((u32 *)MPC5XXX_SDRAM_CS0CFG, 0x13 +
			__builtin_ffs(dramsize >> 20) - 1);
	else
		out_be32((u32 *)MPC5XXX_SDRAM_CS0CFG, 0);

	return dramsize;
}

void mvsmr_init_gpio(void)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	struct mpc5xxx_wu_gpio *wu_gpio =
		(struct mpc5xxx_wu_gpio *)MPC5XXX_WU_GPIO;
	struct mpc5xxx_gpt_0_7 *timers = (struct mpc5xxx_gpt_0_7 *)MPC5XXX_GPT;

	printf("Ports : 0x%08x\n", gpio->port_config);
	printf("PORCFG: 0x%08x\n", in_be32((unsigned *)MPC5XXX_CDM_PORCFG));

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

	out_8(&wu_gpio->ode, WKUP_ODE);
	out_8(&wu_gpio->ddr, WKUP_DIR);
	out_8(&wu_gpio->dvo, WKUP_DO);
	out_8(&wu_gpio->enable, WKUP_EN);

	out_be32(&timers->gpt0.emsr, 0x00000234); /* OD output high */
	out_be32(&timers->gpt1.emsr, 0x00000234);
	out_be32(&timers->gpt2.emsr, 0x00000234);
	out_be32(&timers->gpt3.emsr, 0x00000234);
	out_be32(&timers->gpt4.emsr, 0x00000234);
	out_be32(&timers->gpt5.emsr, 0x00000234);
	out_be32(&timers->gpt6.emsr, 0x00000024); /* push-pull output low */
	out_be32(&timers->gpt7.emsr, 0x00000024);
}

int misc_init_r(void)
{
	char *s = getenv("reset_env");

	if (s) {
		printf(" === FACTORY RESET ===\n");
		mv_reset_environment();
		saveenv();
	}

	return -1;
}

void mvsmr_get_dbg_present(void)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)MPC5XXX_PSC1;

	if (in_be32(&gpio->simple_ival) & COP_PRESENT) {
		setenv("dbg_present", "no\0");
		setenv("bootstopkey", "abcdefghijklmnopqrstuvwxyz\0");
	} else {
		setenv("dbg_present", "yes\0");
		setenv("bootstopkey", "s\0");
		setbits_8(&psc->command, PSC_RX_ENABLE);
	}
}

void mvsmr_get_service_mode(void)
{
	struct mpc5xxx_wu_gpio *wu_gpio =
		(struct mpc5xxx_wu_gpio *)MPC5XXX_WU_GPIO;

	if (in_8(&wu_gpio->ival) & SERVICE_MODE)
		setenv("servicemode", "no\0");
	else
		setenv("servicemode", "yes\0");
}

int mvsmr_get_mac(void)
{
	unsigned char mac[6];
	struct mpc5xxx_wu_gpio *wu_gpio =
		(struct mpc5xxx_wu_gpio *)MPC5XXX_WU_GPIO;

	if (in_8(&wu_gpio->ival) & LAN_PRSNT) {
		setenv("lan_present", "no\0");
		return -1;
	} else
		setenv("lan_present", "yes\0");

	i2c_read(0x50, 0, 1, mac, 6);

	eth_setenv_enetaddr("ethaddr", mac);

	return 0;
}

int checkboard(void)
{
	mvsmr_init_gpio();
	printf("Board: Matrix Vision mvSMR\n");

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
	clrbits_be32((u32 *)MPC5XXX_BOOTCS_CFG, 0x1);
}

void flash_afterinit(ulong size)
{
	out_be32((u32 *)MPC5XXX_BOOTCS_START,
		START_REG(CONFIG_SYS_BOOTCS_START | size));
	out_be32((u32 *)MPC5XXX_CS0_START,
		START_REG(CONFIG_SYS_BOOTCS_START | size));
	out_be32((u32 *)MPC5XXX_BOOTCS_STOP,
		STOP_REG(CONFIG_SYS_BOOTCS_START | size, size));
	out_be32((u32 *)MPC5XXX_CS0_STOP,
		STOP_REG(CONFIG_SYS_BOOTCS_START | size, size));
}

struct pci_controller hose;

void pci_init_board(void)
{
	mvsmr_get_dbg_present();
	mvsmr_get_service_mode();
	mvsmr_init_fpga();
	mv_load_fpga();
	pci_mpc5xxx_init(&hose);
}

int board_eth_init(bd_t *bis)
{
	if (!mvsmr_get_mac())
		return cpu_eth_init(bis);

	return pci_eth_init(bis);
}
