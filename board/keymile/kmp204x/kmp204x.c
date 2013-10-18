/*
 * (C) Copyright 2013 Keymile AG
 * Valentin Longchamp <valentin.longchamp@keymile.com>
 *
 * Copyright 2011,2012 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>

#include "../common/common.h"
#include "kmp204x.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: Keymile %s\n", CONFIG_KM_BOARD_NAME);

	return 0;
}

/* TODO: implement the I2C deblocking function */
int i2c_make_abort(void)
{
	return 1;
}

#define ZL30158_RST	8
#define ZL30343_RST	9

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* board only uses the DDR_MCK0, so disable the DDR_MCK1/2/3 */
	setbits_be32(&gur->ddrclkdr, 0x001f000f);

	/* take the Zarlinks out of reset as soon as possible */
	qrio_prst(ZL30158_RST, false, false);
	qrio_prst(ZL30343_RST, false, false);

	/* and set their reset to power-up only */
	qrio_prstcfg(ZL30158_RST, PRSTCFG_POWUP_RST);
	qrio_prstcfg(ZL30343_RST, PRSTCFG_POWUP_RST);

	return 0;
}

int board_early_init_r(void)
{
	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	set_liodns();
	setup_portals();

	return 0;
}

unsigned long get_board_sys_clk(unsigned long dummy)
{
	return 66666666;
}

#define WDMASK_OFF	0x16

static void qrio_wdmask(u8 bit, bool wden)
{
	u16 wdmask;
	void __iomem *qrio_base = (void *)CONFIG_SYS_QRIO_BASE;

	wdmask = in_be16(qrio_base + WDMASK_OFF);

	if (wden)
		wdmask |= (1 << bit);
	else
		wdmask &= ~(1 << bit);

	out_be16(qrio_base + WDMASK_OFF, wdmask);
}

#define PRST_OFF	0x1a

void qrio_prst(u8 bit, bool en, bool wden)
{
	u16 prst;
	void __iomem *qrio_base = (void *)CONFIG_SYS_QRIO_BASE;

	qrio_wdmask(bit, wden);

	prst = in_be16(qrio_base + PRST_OFF);

	if (en)
		prst &= ~(1 << bit);
	else
		prst |= (1 << bit);

	out_be16(qrio_base + PRST_OFF, prst);
}

#define PRSTCFG_OFF	0x1c

void qrio_prstcfg(u8 bit, u8 mode)
{
	u32 prstcfg;
	u8 i;
	void __iomem *qrio_base = (void *)CONFIG_SYS_QRIO_BASE;

	prstcfg = in_be32(qrio_base + PRSTCFG_OFF);

	for (i = 0; i < 2; i++) {
		if (mode & (1<<i))
			set_bit(2*bit+i, &prstcfg);
		else
			clear_bit(2*bit+i, &prstcfg);
	}

	out_be32(qrio_base + PRSTCFG_OFF, prstcfg);
}


#define BOOTCOUNT_OFF	0x12

void bootcount_store(ulong counter)
{
	u8 val;
	void __iomem *qrio_base = (void *)CONFIG_SYS_QRIO_BASE;

	val = (counter <= 255) ? (u8)counter : 255;
	out_8(qrio_base + BOOTCOUNT_OFF, val);
}

ulong bootcount_load(void)
{
	u8 val;
	void __iomem *qrio_base = (void *)CONFIG_SYS_QRIO_BASE;
	val = in_8(qrio_base + BOOTCOUNT_OFF);
	return val;
}

#define NUM_SRDS_BANKS	2
#define PHY_RST		15

int misc_init_r(void)
{
	serdes_corenet_t *regs = (void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	u32 expected[NUM_SRDS_BANKS] = {SRDS_PLLCR0_RFCK_SEL_100,
		SRDS_PLLCR0_RFCK_SEL_125};
	unsigned int i;

	/* check SERDES reference clocks */
	for (i = 0; i < NUM_SRDS_BANKS; i++) {
		u32 actual = in_be32(&regs->bank[i].pllcr0);
		actual &= SRDS_PLLCR0_RFCK_SEL_MASK;
		if (actual != expected[i]) {
			printf("Warning: SERDES bank %u expects reference \
			       clock %sMHz, but actual is %sMHz\n", i + 1,
			       serdes_clock_to_string(expected[i]),
			       serdes_clock_to_string(actual));
		}
	}

	/* take the mgmt eth phy out of reset */
	qrio_prst(PHY_RST, false, false);

	return 0;
}

#if defined(CONFIG_HUSH_INIT_VAR)
int hush_init_var(void)
{
	ivm_read_eeprom();
	return 0;
}
#endif

#if defined(CONFIG_LAST_STAGE_INIT)
int last_stage_init(void)
{
	set_km_env();
	return 0;
}
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
void fdt_fixup_fman_mac_addresses(void *blob)
{
	int node, i, ret;
	char *tmp, *end;
	unsigned char mac_addr[6];

	/* get the mac addr from env */
	tmp = getenv("ethaddr");
	if (!tmp) {
		printf("ethaddr env variable not defined\n");
		return;
	}
	for (i = 0; i < 6; i++) {
		mac_addr[i] = tmp ? simple_strtoul(tmp, &end, 16) : 0;
		if (tmp)
			tmp = (*end) ? end+1 : end;
	}

	/* find the correct fdt ethernet path and correct it */
	node = fdt_path_offset(blob, "/soc/fman/ethernet@e8000");
	if (node < 0) {
		printf("no /soc/fman/ethernet path offset\n");
		return;
	}
	ret = fdt_setprop(blob, node, "local-mac-address", &mac_addr, 6);
	if (ret) {
		printf("error setting local-mac-address property\n");
		return;
	}
}
#endif

void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#if defined(CONFIG_HAS_FSL_DR_USB) || defined(CONFIG_HAS_FSL_MPH_USB)
	fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);
#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_fman_mac_addresses(blob);
#endif
}
