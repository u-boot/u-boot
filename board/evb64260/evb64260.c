/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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

/*
 * evb64260.c - main board support/init for the Galileo Eval board.
 */

#include <common.h>
#include <74xx_7xx.h>
#include <galileo/memory.h>
#include <galileo/pci.h>
#include <galileo/gt64260R.h>
#include <net.h>
#include <netdev.h>

#include <asm/io.h>
#include "eth.h"
#include "mpsc.h"
#include "i2c.h"
#include "64260.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_ZUMA_V2
extern void zuma_mbox_init(void);
#endif

#undef	DEBUG
#define	MAP_PCI

#ifdef DEBUG
#define DP(x) x
#else
#define DP(x)
#endif

/* ------------------------------------------------------------------------- */

/* this is the current GT register space location */
/* it starts at CONFIG_SYS_DFL_GT_REGS but moves later to CONFIG_SYS_GT_REGS */

/* Unfortunately, we cant change it while we are in flash, so we initialize it
 * to the "final" value. This means that any debug_led calls before
 * board_early_init_f wont work right (like in cpu_init_f).
 * See also my_remap_gt_regs below. (NTL)
 */

unsigned int INTERNAL_REG_BASE_ADDR = CONFIG_SYS_GT_REGS;

/* ------------------------------------------------------------------------- */

/*
 * This is a version of the GT register space remapping function that
 * doesn't touch globals (meaning, it's ok to run from flash.)
 *
 * Unfortunately, this has the side effect that a writable
 * INTERNAL_REG_BASE_ADDR is impossible. Oh well.
 */

void
my_remap_gt_regs(u32 cur_loc, u32 new_loc)
{
	u32 temp;

	/* check and see if it's already moved */
	temp = in_le32((u32 *)(new_loc + INTERNAL_SPACE_DECODE));
	if ((temp & 0xffff) == new_loc >> 20)
		return;

	temp = (in_le32((u32 *)(cur_loc + INTERNAL_SPACE_DECODE)) &
		0xffff0000) | (new_loc >> 20);

	out_le32((u32 *)(cur_loc + INTERNAL_SPACE_DECODE), temp);

	while (GTREGREAD(INTERNAL_SPACE_DECODE) != temp);
}

static void
gt_pci_config(void)
{
	/* move PCI stuff out of the way - NTL */
	/* map PCI Host 0 */
	pciMapSpace(PCI_HOST0, PCI_REGION0, CONFIG_SYS_PCI0_0_MEM_SPACE,
		CONFIG_SYS_PCI0_0_MEM_SPACE, CONFIG_SYS_PCI0_MEM_SIZE);

	pciMapSpace(PCI_HOST0, PCI_REGION1, 0, 0, 0);
	pciMapSpace(PCI_HOST0, PCI_REGION2, 0, 0, 0);
	pciMapSpace(PCI_HOST0, PCI_REGION3, 0, 0, 0);

	pciMapSpace(PCI_HOST0, PCI_IO, CONFIG_SYS_PCI0_IO_SPACE_PCI,
		CONFIG_SYS_PCI0_IO_SPACE, CONFIG_SYS_PCI0_IO_SIZE);

	/* map PCI Host 1 */
	pciMapSpace(PCI_HOST1, PCI_REGION0, CONFIG_SYS_PCI1_0_MEM_SPACE,
		CONFIG_SYS_PCI1_0_MEM_SPACE, CONFIG_SYS_PCI1_MEM_SIZE);

	pciMapSpace(PCI_HOST1, PCI_REGION1, 0, 0, 0);
	pciMapSpace(PCI_HOST1, PCI_REGION2, 0, 0, 0);
	pciMapSpace(PCI_HOST1, PCI_REGION3, 0, 0, 0);

	pciMapSpace(PCI_HOST1, PCI_IO, CONFIG_SYS_PCI1_IO_SPACE_PCI,
		CONFIG_SYS_PCI1_IO_SPACE, CONFIG_SYS_PCI1_IO_SIZE);

	/* PCI interface settings */
	GT_REG_WRITE(PCI_0TIMEOUT_RETRY, 0xffff);
	GT_REG_WRITE(PCI_1TIMEOUT_RETRY, 0xffff);
	GT_REG_WRITE(PCI_0BASE_ADDRESS_REGISTERS_ENABLE, 0xfffff80e);
	GT_REG_WRITE(PCI_1BASE_ADDRESS_REGISTERS_ENABLE, 0xfffff80e);


}

/* Setup CPU interface paramaters */
static void
gt_cpu_config(void)
{
	cpu_t cpu = get_cpu_type();
	ulong tmp;

	/* cpu configuration register */
	tmp = GTREGREAD(CPU_CONFIGURATION);

	/* set the AACK delay bit
	 * see Res#14 */
	tmp |= CPU_CONF_AACK_DELAY;
	tmp &= ~CPU_CONF_AACK_DELAY_2; /* New RGF */

	/* Galileo claims this is necessary for all busses >= 100 MHz */
	tmp |= CPU_CONF_FAST_CLK;

	if (cpu == CPU_750CX) {
		tmp &= ~CPU_CONF_DP_VALID; /* Safer, needed for CXe. RGF */
		tmp &= ~CPU_CONF_AP_VALID;
	} else {
		tmp |= CPU_CONF_DP_VALID;
		tmp |= CPU_CONF_AP_VALID;
	}

	/* this only works with the MPX bus */
	tmp &= ~CPU_CONF_RD_OOO; /* Safer RGF */
	tmp |= CPU_CONF_PIPELINE;
	tmp |= CPU_CONF_TA_DELAY;

	GT_REG_WRITE(CPU_CONFIGURATION, tmp);

	/* CPU master control register */
	tmp = GTREGREAD(CPU_MASTER_CONTROL);

	tmp |= CPU_MAST_CTL_ARB_EN;

	if ((cpu == CPU_7400) ||
	    (cpu == CPU_7410) ||
	    (cpu == CPU_7450)) {

		tmp |= CPU_MAST_CTL_CLEAN_BLK;
		tmp |= CPU_MAST_CTL_FLUSH_BLK;

	} else {
		/* cleanblock must be cleared for CPUs
		 * that do not support this command
		 * see Res#1 */
		tmp &= ~CPU_MAST_CTL_CLEAN_BLK;
		tmp &= ~CPU_MAST_CTL_FLUSH_BLK;
	}
	GT_REG_WRITE(CPU_MASTER_CONTROL, tmp);
}

/*
 * board_early_init_f.
 *
 * set up gal. device mappings, etc.
 */
int board_early_init_f (void)
{
	uchar sram_boot = 0;

	/*
	 * set up the GT the way the kernel wants it
	 * the call to move the GT register space will obviously
	 * fail if it has already been done, but we're going to assume
	 * that if it's not at the power-on location, it's where we put
	 * it last time. (huber)
	 */
	my_remap_gt_regs(CONFIG_SYS_DFL_GT_REGS, CONFIG_SYS_GT_REGS);

	gt_pci_config();

	/* mask all external interrupt sources */
	GT_REG_WRITE(CPU_INTERRUPT_MASK_REGISTER_LOW, 0);
	GT_REG_WRITE(CPU_INTERRUPT_MASK_REGISTER_HIGH, 0);
	GT_REG_WRITE(PCI_0INTERRUPT_CAUSE_MASK_REGISTER_LOW, 0);
	GT_REG_WRITE(PCI_0INTERRUPT_CAUSE_MASK_REGISTER_HIGH, 0);
	GT_REG_WRITE(PCI_1INTERRUPT_CAUSE_MASK_REGISTER_LOW, 0);
	GT_REG_WRITE(PCI_1INTERRUPT_CAUSE_MASK_REGISTER_HIGH, 0);
	GT_REG_WRITE(CPU_INT_0_MASK, 0);
	GT_REG_WRITE(CPU_INT_1_MASK, 0);
	GT_REG_WRITE(CPU_INT_2_MASK, 0);
	GT_REG_WRITE(CPU_INT_3_MASK, 0);

	/* now, onto the configuration */
	GT_REG_WRITE(SDRAM_CONFIGURATION, CONFIG_SYS_SDRAM_CONFIG);

	/* ----- DEVICE BUS SETTINGS ------ */

	/*
	 * EVB
	 * 0 - SRAM
	 * 1 - RTC
	 * 2 - UART
	 * 3 - Flash
	 * boot - BootCS
	 *
	 * Zuma
	 * 0 - Flash
	 * boot - BootCS
	 */

	/*
	 * the dual 7450 module requires burst access to the boot
	 * device, so the serial rom copies the boot device to the
	 * on-board sram on the eval board, and updates the correct
	 * registers to boot from the sram. (device0)
	 */
#if defined(CONFIG_ZUMA_V2) || defined(CONFIG_P3G4)
	/* Zuma has no SRAM */
	sram_boot = 0;
#else
	if (memoryGetDeviceBaseAddress(DEVICE0) && 0xfff00000 == CONFIG_SYS_MONITOR_BASE)
		sram_boot = 1;
#endif

		memoryMapDeviceSpace(DEVICE0, CONFIG_SYS_DEV0_SPACE, CONFIG_SYS_DEV0_SIZE);

	memoryMapDeviceSpace(DEVICE1, CONFIG_SYS_DEV1_SPACE, CONFIG_SYS_DEV1_SIZE);
	memoryMapDeviceSpace(DEVICE2, CONFIG_SYS_DEV2_SPACE, CONFIG_SYS_DEV2_SIZE);
	memoryMapDeviceSpace(DEVICE3, CONFIG_SYS_DEV3_SPACE, CONFIG_SYS_DEV3_SIZE);

	/* configure device timing */
#ifdef CONFIG_SYS_DEV0_PAR
	if (!sram_boot)
		GT_REG_WRITE(DEVICE_BANK0PARAMETERS, CONFIG_SYS_DEV0_PAR);
#endif

#ifdef CONFIG_SYS_DEV1_PAR
	GT_REG_WRITE(DEVICE_BANK1PARAMETERS, CONFIG_SYS_DEV1_PAR);
#endif
#ifdef CONFIG_SYS_DEV2_PAR
	GT_REG_WRITE(DEVICE_BANK2PARAMETERS, CONFIG_SYS_DEV2_PAR);
#endif

#ifdef CONFIG_EVB64260
#ifdef CONFIG_SYS_32BIT_BOOT_PAR
	/* detect if we are booting from the 32 bit flash */
	if (GTREGREAD(DEVICE_BOOT_BANK_PARAMETERS) & (0x3 << 20)) {
		/* 32 bit boot flash */
		GT_REG_WRITE(DEVICE_BANK3PARAMETERS, CONFIG_SYS_8BIT_BOOT_PAR);
		GT_REG_WRITE(DEVICE_BOOT_BANK_PARAMETERS, CONFIG_SYS_32BIT_BOOT_PAR);
	} else {
		/* 8 bit boot flash */
		GT_REG_WRITE(DEVICE_BANK3PARAMETERS, CONFIG_SYS_32BIT_BOOT_PAR);
		GT_REG_WRITE(DEVICE_BOOT_BANK_PARAMETERS, CONFIG_SYS_8BIT_BOOT_PAR);
	}
#else
	/* 8 bit boot flash only */
	GT_REG_WRITE(DEVICE_BOOT_BANK_PARAMETERS, CONFIG_SYS_8BIT_BOOT_PAR);
#endif
#else /* CONFIG_EVB64260 not defined */
		/* We are booting from 16-bit flash.
		 */
	GT_REG_WRITE(DEVICE_BOOT_BANK_PARAMETERS, CONFIG_SYS_16BIT_BOOT_PAR);
#endif

	gt_cpu_config();

	/* MPP setup */
	GT_REG_WRITE(MPP_CONTROL0, CONFIG_SYS_MPP_CONTROL_0);
	GT_REG_WRITE(MPP_CONTROL1, CONFIG_SYS_MPP_CONTROL_1);
	GT_REG_WRITE(MPP_CONTROL2, CONFIG_SYS_MPP_CONTROL_2);
	GT_REG_WRITE(MPP_CONTROL3, CONFIG_SYS_MPP_CONTROL_3);

	GT_REG_WRITE(GPP_LEVEL_CONTROL, CONFIG_SYS_GPP_LEVEL_CONTROL);
	GT_REG_WRITE(SERIAL_PORT_MULTIPLEX, CONFIG_SYS_SERIAL_PORT_MUX);

	return 0;
}

/* various things to do after relocation */

int misc_init_r (void)
{
	icache_enable();
#ifdef CONFIG_SYS_L2
	l2cache_enable();
#endif

#ifdef CONFIG_MPSC
	mpsc_init2();
#endif

#ifdef CONFIG_ZUMA_V2
	zuma_mbox_init();
#endif
	return (0);
}

void
after_reloc(ulong dest_addr)
{
	/* check to see if we booted from the sram.  If so, move things
	 * back to the way they should be. (we're running from main
	 * memory at this point now */

	if (memoryGetDeviceBaseAddress(DEVICE0) == CONFIG_SYS_MONITOR_BASE) {
		memoryMapDeviceSpace(DEVICE0, CONFIG_SYS_DEV0_SPACE, CONFIG_SYS_DEV0_SIZE);
		memoryMapDeviceSpace(BOOT_DEVICE, CONFIG_SYS_FLASH_BASE, _1M);
	}

	/* now, jump to the main U-Boot board init code */
	board_init_r ((gd_t *)gd, dest_addr);

	/* NOTREACHED */
}

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int
checkboard (void)
{
	puts ("Board: " CONFIG_SYS_BOARD_NAME "\n");
	return (0);
}

/* utility functions */
void
debug_led(int led, int mode)
{
#if !defined(CONFIG_ZUMA_V2) && !defined(CONFIG_P3G4)
	volatile int *addr = NULL;
	int dummy;

	if (mode == 1) {
		switch (led) {
		case 0:
			addr = (int *)((unsigned int)CONFIG_SYS_DEV1_SPACE | 0x08000);
			break;

		case 1:
			addr = (int *)((unsigned int)CONFIG_SYS_DEV1_SPACE | 0x0c000);
			break;

		case 2:
			addr = (int *)((unsigned int)CONFIG_SYS_DEV1_SPACE | 0x10000);
			break;
		}
	} else if (mode == 0) {
		switch (led) {
		case 0:
			addr = (int *)((unsigned int)CONFIG_SYS_DEV1_SPACE | 0x14000);
			break;

		case 1:
			addr = (int *)((unsigned int)CONFIG_SYS_DEV1_SPACE | 0x18000);
			break;

		case 2:
			addr = (int *)((unsigned int)CONFIG_SYS_DEV1_SPACE | 0x1c000);
			break;
		}
	}
	WRITE_CHAR(addr, 0);
	dummy = *addr;
#endif /* CONFIG_ZUMA_V2 */
}

void
display_mem_map(void)
{
    int i,j;
    unsigned int base,size,width;
    /* SDRAM */
    printf("SDRAM\n");
    for(i=0;i<=BANK3;i++) {
	base = memoryGetBankBaseAddress(i);
	size = memoryGetBankSize(i);
	if(size !=0)
	{
	    printf("BANK%d: base - 0x%08x\tsize - %dM bytes\n",i,base,size>>20);
	}
    }

    /* CPU's PCI windows */
    for(i=0;i<=PCI_HOST1;i++) {
	printf("\nCPU's PCI %d windows\n", i);
	base=pciGetSpaceBase(i,PCI_IO);
	size=pciGetSpaceSize(i,PCI_IO);
	printf("      IO: base - 0x%08x\tsize - %dM bytes\n",base,size>>20);
	for(j=0;j<=PCI_REGION3;j++) {
	    base = pciGetSpaceBase(i,j);
	    size = pciGetSpaceSize(i,j);
	    printf("MEMORY %d: base - 0x%08x\tsize - %dM bytes\n",j,base,
		    size>>20);
	}
    }

    /* Devices */
    printf("\nDEVICES\n");
	for(i=0;i<=DEVICE3;i++) {
	base = memoryGetDeviceBaseAddress(i);
	size = memoryGetDeviceSize(i);
	width= memoryGetDeviceWidth(i) * 8;
	printf("DEV %d:  base - 0x%08x\tsize - %dM bytes\twidth - %d bits\n",
	       i, base, size>>20, width);
    }

    /* Bootrom */
    base = memoryGetDeviceBaseAddress(BOOT_DEVICE); /* Boot */
    size = memoryGetDeviceSize(BOOT_DEVICE);
    width= memoryGetDeviceWidth(BOOT_DEVICE) * 8;
    printf(" BOOT:  base - 0x%08x\tsize - %dM bytes\twidth - %d bits\n",
	   base, size>>20, width);
}

int board_eth_init(bd_t *bis)
{
	gt6426x_eth_initialize(bis);
	return 0;
}
