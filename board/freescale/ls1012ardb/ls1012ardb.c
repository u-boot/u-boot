/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fsl_esdhc.h>
#include <environment.h>
#include <fsl_mmdc.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	u8 in1;

	puts("Board: LS1012ARDB ");

	/* Initialize i2c early for Serial flash bank information */
	i2c_set_bus_num(0);

	if (i2c_read(I2C_MUX_IO1_ADDR, 1, 1, &in1, 1) < 0) {
		printf("Error reading i2c boot information!\n");
		return 0; /* Don't want to hang() on this error */
	}

	puts("Version");
	if ((in1 & (~__SW_REV_MASK)) == __SW_REV_A)
		puts(": RevA");
	else if ((in1 & (~__SW_REV_MASK)) == __SW_REV_B)
		puts(": RevB");
	else
		puts(": unknown");

	printf(", boot from QSPI");
	if ((in1 & (~__SW_BOOT_MASK)) == __SW_BOOT_EMU)
		puts(": emu\n");
	else if ((in1 & (~__SW_BOOT_MASK)) == __SW_BOOT_BANK1)
		puts(": bank1\n");
	else if ((in1 & (~__SW_BOOT_MASK)) == __SW_BOOT_BANK2)
		puts(": bank2\n");
	else
		puts("unknown\n");

	return 0;
}

int dram_init(void)
{
	mmdc_init();

	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

int board_init(void)
{
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)CONFIG_SYS_CCI400_ADDR;
	/*
	 * Set CCI-400 control override register to enable barrier
	 * transaction
	 */
	out_le32(&cci->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
	erratum_a010315();
#endif

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif

	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	arch_fixup_fdt(blob);

	ft_cpu_setup(blob, bd);

	return 0;
}
