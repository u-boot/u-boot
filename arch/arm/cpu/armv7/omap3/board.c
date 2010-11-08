/*
 *
 * Common board functions for OMAP3 based boards.
 *
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *      Sunil Kumar <sunilsaini05@gmail.com>
 *      Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *      Richard Woodruff <r-woodruff2@ti.com>
 *      Syed Mohammed Khasim <khasim@ti.com>
 *
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
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/cache.h>

extern omap3_sysinfo sysinfo;

/******************************************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 *****************************************************************************/
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}

/******************************************************************************
 * Routine: secure_unlock
 * Description: Setup security registers for access
 *              (GP Device only)
 *****************************************************************************/
void secure_unlock_mem(void)
{
	struct pm *pm_rt_ape_base = (struct pm *)PM_RT_APE_BASE_ADDR_ARM;
	struct pm *pm_gpmc_base = (struct pm *)PM_GPMC_BASE_ADDR_ARM;
	struct pm *pm_ocm_ram_base = (struct pm *)PM_OCM_RAM_BASE_ADDR_ARM;
	struct pm *pm_iva2_base = (struct pm *)PM_IVA2_BASE_ADDR_ARM;
	struct sms *sms_base = (struct sms *)OMAP34XX_SMS_BASE;

	/* Protection Module Register Target APE (PM_RT) */
	writel(UNLOCK_1, &pm_rt_ape_base->req_info_permission_1);
	writel(UNLOCK_1, &pm_rt_ape_base->read_permission_0);
	writel(UNLOCK_1, &pm_rt_ape_base->wirte_permission_0);
	writel(UNLOCK_2, &pm_rt_ape_base->addr_match_1);

	writel(UNLOCK_3, &pm_gpmc_base->req_info_permission_0);
	writel(UNLOCK_3, &pm_gpmc_base->read_permission_0);
	writel(UNLOCK_3, &pm_gpmc_base->wirte_permission_0);

	writel(UNLOCK_3, &pm_ocm_ram_base->req_info_permission_0);
	writel(UNLOCK_3, &pm_ocm_ram_base->read_permission_0);
	writel(UNLOCK_3, &pm_ocm_ram_base->wirte_permission_0);
	writel(UNLOCK_2, &pm_ocm_ram_base->addr_match_2);

	/* IVA Changes */
	writel(UNLOCK_3, &pm_iva2_base->req_info_permission_0);
	writel(UNLOCK_3, &pm_iva2_base->read_permission_0);
	writel(UNLOCK_3, &pm_iva2_base->wirte_permission_0);

	/* SDRC region 0 public */
	writel(UNLOCK_1, &sms_base->rg_att0);
}

/******************************************************************************
 * Routine: secureworld_exit()
 * Description: If chip is EMU and boot type is external
 *		configure secure registers and exit secure world
 *              general use.
 *****************************************************************************/
void secureworld_exit()
{
	unsigned long i;

	/* configrue non-secure access control register */
	__asm__ __volatile__("mrc p15, 0, %0, c1, c1, 2":"=r"(i));
	/* enabling co-processor CP10 and CP11 accesses in NS world */
	__asm__ __volatile__("orr %0, %0, #0xC00":"=r"(i));
	/*
	 * allow allocation of locked TLBs and L2 lines in NS world
	 * allow use of PLE registers in NS world also
	 */
	__asm__ __volatile__("orr %0, %0, #0x70000":"=r"(i));
	__asm__ __volatile__("mcr p15, 0, %0, c1, c1, 2":"=r"(i));

	/* Enable ASA in ACR register */
	__asm__ __volatile__("mrc p15, 0, %0, c1, c0, 1":"=r"(i));
	__asm__ __volatile__("orr %0, %0, #0x10":"=r"(i));
	__asm__ __volatile__("mcr p15, 0, %0, c1, c0, 1":"=r"(i));

	/* Exiting secure world */
	__asm__ __volatile__("mrc p15, 0, %0, c1, c1, 0":"=r"(i));
	__asm__ __volatile__("orr %0, %0, #0x31":"=r"(i));
	__asm__ __volatile__("mcr p15, 0, %0, c1, c1, 0":"=r"(i));
}

/******************************************************************************
 * Routine: try_unlock_sram()
 * Description: If chip is GP/EMU(special) type, unlock the SRAM for
 *              general use.
 *****************************************************************************/
void try_unlock_memory()
{
	int mode;
	int in_sdram = is_running_in_sdram();

	/*
	 * if GP device unlock device SRAM for general use
	 * secure code breaks for Secure/Emulation device - HS/E/T
	 */
	mode = get_device_type();
	if (mode == GP_DEVICE)
		secure_unlock_mem();

	/*
	 * If device is EMU and boot is XIP external booting
	 * Unlock firewalls and disable L2 and put chip
	 * out of secure world
	 *
	 * Assuming memories are unlocked by the demon who put us in SDRAM
	 */
	if ((mode <= EMU_DEVICE) && (get_boot_type() == 0x1F)
	    && (!in_sdram)) {
		secure_unlock_mem();
		secureworld_exit();
	}

	return;
}

/******************************************************************************
 * Routine: s_init
 * Description: Does early system init of muxing and clocks.
 *              - Called path is with SRAM stack.
 *****************************************************************************/
void s_init(void)
{
	int in_sdram = is_running_in_sdram();

	watchdog_init();

	try_unlock_memory();

	/*
	 * Right now flushing at low MPU speed.
	 * Need to move after clock init
	 */
	invalidate_dcache(get_device_type());
#ifndef CONFIG_ICACHE_OFF
	icache_enable();
#endif

#ifdef CONFIG_L2_OFF
	l2_cache_disable();
#else
	l2_cache_enable();
#endif
	/*
	 * Writing to AuxCR in U-boot using SMI for GP DEV
	 * Currently SMI in Kernel on ES2 devices seems to have an issue
	 * Once that is resolved, we can postpone this config to kernel
	 */
	if (get_device_type() == GP_DEVICE)
		setup_auxcr();

	set_muxconf_regs();
	delay(100);

	prcm_init();

	per_clocks_enable();

	if (!in_sdram)
		mem_init();
}

/******************************************************************************
 * Routine: wait_for_command_complete
 * Description: Wait for posting to finish on watchdog
 *****************************************************************************/
void wait_for_command_complete(struct watchdog *wd_base)
{
	int pending = 1;
	do {
		pending = readl(&wd_base->wwps);
	} while (pending);
}

/******************************************************************************
 * Routine: watchdog_init
 * Description: Shut down watch dogs
 *****************************************************************************/
void watchdog_init(void)
{
	struct watchdog *wd2_base = (struct watchdog *)WD2_BASE;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;

	/*
	 * There are 3 watch dogs WD1=Secure, WD2=MPU, WD3=IVA. WD1 is
	 * either taken care of by ROM (HS/EMU) or not accessible (GP).
	 * We need to take care of WD2-MPU or take a PRCM reset. WD3
	 * should not be running and does not generate a PRCM reset.
	 */

	sr32(&prcm_base->fclken_wkup, 5, 1, 1);
	sr32(&prcm_base->iclken_wkup, 5, 1, 1);
	wait_on_value(ST_WDT2, 0x20, &prcm_base->idlest_wkup, 5);

	writel(WD_UNLOCK1, &wd2_base->wspr);
	wait_for_command_complete(wd2_base);
	writel(WD_UNLOCK2, &wd2_base->wspr);
}

/******************************************************************************
 * Dummy function to handle errors for EABI incompatibility
 *****************************************************************************/
void abort(void)
{
}

#ifdef CONFIG_NAND_OMAP_GPMC
/******************************************************************************
 * OMAP3 specific command to switch between NAND HW and SW ecc
 *****************************************************************************/
static int do_switch_ecc(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 2)
		goto usage;
	if (strncmp(argv[1], "hw", 2) == 0)
		omap_nand_switch_ecc(1);
	else if (strncmp(argv[1], "sw", 2) == 0)
		omap_nand_switch_ecc(0);
	else
		goto usage;

	return 0;

usage:
	printf ("Usage: nandecc %s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	nandecc, 2, 1,	do_switch_ecc,
	"switch OMAP3 NAND ECC calculation algorithm",
	"[hw/sw] - Switch between NAND hardware (hw) or software (sw) ecc algorithm"
);

#endif /* CONFIG_NAND_OMAP_GPMC */

#ifdef CONFIG_DISPLAY_BOARDINFO
/**
 * Print board information
 */
int checkboard (void)
{
	char *mem_s ;

	if (is_mem_sdr())
		mem_s = "mSDR";
	else
		mem_s = "LPDDR";

	printf("%s + %s/%s\n", sysinfo.board_string, mem_s,
			sysinfo.nand_string);

	return 0;
}
#endif	/* CONFIG_DISPLAY_BOARDINFO */
