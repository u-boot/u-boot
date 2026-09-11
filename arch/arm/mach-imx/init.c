// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/crm_regs.h>

void init_aips(void)
{
	struct aipstz_regs *aips1, *aips2, *aips3;

	aips1 = (struct aipstz_regs *)AIPS1_BASE_ADDR;
	aips2 = (struct aipstz_regs *)AIPS2_BASE_ADDR;
	aips3 = (struct aipstz_regs *)AIPS3_BASE_ADDR;

	/*
	 * Set all MPROTx to be non-bufferable, trusted for R/W,
	 * not forced to user-mode.
	 */
	writel(0x77777777, &aips1->mprot0);
	writel(0x77777777, &aips1->mprot1);
	writel(0x77777777, &aips2->mprot0);
	writel(0x77777777, &aips2->mprot1);

	/*
	 * Set all OPACRx to be non-bufferable, not require
	 * supervisor privilege level for access,allow for
	 * write access and untrusted master access.
	 */
	writel(0x00000000, &aips1->opacr0);
	writel(0x00000000, &aips1->opacr1);
	writel(0x00000000, &aips1->opacr2);
	writel(0x00000000, &aips1->opacr3);
	writel(0x00000000, &aips1->opacr4);
	writel(0x00000000, &aips2->opacr0);
	writel(0x00000000, &aips2->opacr1);
	writel(0x00000000, &aips2->opacr2);
	writel(0x00000000, &aips2->opacr3);
	writel(0x00000000, &aips2->opacr4);

	if (is_mx6ull() || is_mx6sx() || is_mx7()) {
		/*
		 * Set all MPROTx to be non-bufferable, trusted for R/W,
		 * not forced to user-mode.
		 */
		writel(0x77777777, &aips3->mprot0);
		writel(0x77777777, &aips3->mprot1);

		/*
		 * Set all OPACRx to be non-bufferable, not require
		 * supervisor privilege level for access,allow for
		 * write access and untrusted master access.
		 */
		writel(0x00000000, &aips3->opacr0);
		writel(0x00000000, &aips3->opacr1);
		writel(0x00000000, &aips3->opacr2);
		writel(0x00000000, &aips3->opacr3);
		writel(0x00000000, &aips3->opacr4);
	}
}

void imx_wdog_disable_powerdown(void)
{
	struct wdog_regs *wdog1 = (struct wdog_regs *)WDOG1_BASE_ADDR;
	struct wdog_regs *wdog2 = (struct wdog_regs *)WDOG2_BASE_ADDR;
	struct wdog_regs *wdog3 = (struct wdog_regs *)WDOG3_BASE_ADDR;
#ifdef CONFIG_MX7D
	struct wdog_regs *wdog4 = (struct wdog_regs *)WDOG4_BASE_ADDR;
#endif

	/* Write to the PDE (Power Down Enable) bit */
	writew(0, &wdog1->wmcr);
	writew(0, &wdog2->wmcr);

	if (is_mx6sx() || is_mx6ul() || is_mx6ull() || is_mx7())
		writew(0, &wdog3->wmcr);
#ifdef CONFIG_MX7D
	writew(0, &wdog4->wmcr);
#endif
}

#define SRC_SCR_WARM_RESET_ENABLE	0

void init_src(void)
{
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;
	u32 val;

	/*
	 * force warm reset sources to generate cold reset
	 * for a more reliable restart
	 */
	val = readl(&src_regs->scr);
	val &= ~(1 << SRC_SCR_WARM_RESET_ENABLE);
	writel(val, &src_regs->scr);
}

#ifdef CONFIG_CMD_BMODE
void boot_mode_apply(unsigned cfg_val)
{
#ifdef CONFIG_MX6
	const u32 persist_sec = IMX6_SRC_GPR10_PERSIST_SECONDARY_BOOT;
	const u32 bmode = IMX6_SRC_GPR10_BMODE;
#elif CONFIG_MX7
	const u32 persist_sec = IMX7_SRC_GPR10_PERSIST_SECONDARY_BOOT;
	const u32 bmode = IMX7_SRC_GPR10_BMODE;
#endif
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned reg;

	if (cfg_val == MAKE_CFGVAL_PRIMARY_BOOT)
		clrbits_le32(&psrc->gpr10, persist_sec);
	else if (cfg_val == MAKE_CFGVAL_SECONDARY_BOOT)
		setbits_le32(&psrc->gpr10, persist_sec);
	else {
		writel(cfg_val, &psrc->gpr9);
		reg = readl(&psrc->gpr10);
		if (cfg_val)
			reg |= bmode;
		else
			reg &= ~bmode;
		writel(reg, &psrc->gpr10);
	}
}
#endif

#if defined(CONFIG_MX6)
u32 imx6_src_get_boot_mode(void)
{
	if (readl(&src_base->gpr10) & IMX6_SRC_GPR10_BMODE)
		return readl(&src_base->gpr9);
	else
		return readl(&src_base->sbmr1);
}

/*
 * The boot ROM records what it did in an event log buffer in OCRAM. The
 * address of that buffer is stored at a fixed, SoC specific location inside
 * the ROM, see AN12853 "i.MX ROMs Log Events". i.MX6 uses the version 0 log
 * format, where an entry consists of a 32-bit word holding a 24-bit event
 * ID, optionally followed by up to three parameter words.
 */
#define IMX6_ROM_LOG_ID_MASK		GENMASK(23, 0)
#define IMX6_ROM_LOG_MAX_ENTRIES	128

static const u32 *imx6_get_rom_log(void)
{
	uintptr_t log;
	ulong addr;

	if (is_mx6dq() || is_mx6dqp() || is_mx6sl())
		addr = 0xd4;
	else if (is_mx6sdl())
		addr = 0xd8;
	else if (is_mx6sx() || is_mx6sll() || is_mx6ul() || is_mx6ull())
		addr = 0x1e0;
	else
		return NULL;

	log = readl(addr);

	/* The log buffer lives in OCRAM, ignore an invalid pointer */
	if (log < IRAM_BASE_ADDR || log >= IRAM_BASE_ADDR + IRAM_SIZE ||
	    log & 0x3)
		return NULL;

	return (const u32 *)log;
}

/*
 * If BOOT_CFG4[6] is fused, the boot ROM does not fall back to the serial
 * downloader right away when the primary boot device fails, but first tries
 * a recovery boot from the serial ROM selected by BOOT_CFG4[2:0]. Neither
 * SRC_SBMR1 nor SRC_SBMR2 reflect that this happened, so ask the ROM event
 * log instead.
 */
bool imx6_is_ecspi_recovery_boot(void)
{
	const u32 *log = imx6_get_rom_log();
	u32 event_id;
	int i;

	if (!log)
		return false;

	for (i = 0; i < IMX6_ROM_LOG_MAX_ENTRIES; i++) {
		event_id = log[i] & IMX6_ROM_LOG_ID_MASK;

		switch (event_id) {
		case 0x000000: /* End of list */
			return false;
		/* Recovery boot from ECSPI NOR device */
		case 0x061004:
			return true;
		/* Log entries with 1 parameter, skip 1 */
		case 0x080000: /* Start to read data from boot device */
		case 0x090000: /* Image authentication result */
		case 0x0a0000: /* Start to execute the plugin program */
		case 0x0b0000: /* Jump to the boot image soon */
		case 0x0d0000: /* Jump to the SDP boot image soon */
			i += 1;
			continue;
		default:
			continue;
		}
	}

	return false;
}
#endif
