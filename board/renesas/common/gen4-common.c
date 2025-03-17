// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/common/gen4-common.c
 *
 * Copyright (C) 2021-2024 Renesas Electronics Corp.
 */

#include <asm/arch/renesas.h>
#include <asm/arch/sys_proto.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/processor.h>
#include <asm/system.h>
#include <image.h>
#include <linux/errno.h>

#define RST_BASE	0xE6160000 /* Domain0 */
#define RST_WDTRSTCR	(RST_BASE + 0x10)
#define RST_RWDT	0xA55A8002

DECLARE_GLOBAL_DATA_PTR;

static void init_generic_timer(void)
{
	const u32 freq = CONFIG_SYS_CLK_FREQ;

	/* Update memory mapped and register based freqency */
	asm volatile ("msr cntfrq_el0, %0" :: "r" (freq));
	writel(freq, CNTFID0);

	/* Enable counter */
	setbits_le32(CNTCR_BASE, CNTCR_EN);
}

static void init_gic_v3(void)
{
	/* GIC v3 power on */
	writel(BIT(1), GICR_LPI_PWRR);

	/* Wait till the WAKER_CA_BIT changes to 0 */
	clrbits_le32(GICR_LPI_WAKER, BIT(1));
	while (readl(GICR_LPI_WAKER) & BIT(2))
		;

	writel(0xffffffff, GICR_SGI_BASE + GICR_IGROUPR0);
}

void s_init(void)
{
	if (current_el() == 3)
		init_generic_timer();
}

int board_early_init_f(void)
{
	/* Unlock CPG access */
	writel(0x5A5AFFFF, CPGWPR);
	writel(0xA5A50000, CPGWPCR);

	return 0;
}

int board_init(void)
{
	if (current_el() != 3)
		return 0;
	init_gic_v3();

	/* Enable RWDT reset on V3U in EL3 */
	if (IS_ENABLED(CONFIG_R8A779A0) &&
	    renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A779A0) {
		writel(RST_RWDT, RST_WDTRSTCR);
	}

	return 0;
}

#define RST_BASE	0xE6160000 /* Domain0 */
#define RST_SRESCR0	(RST_BASE + 0x18)
#define RST_SPRES	0x5AA58000

void __weak reset_cpu(void)
{
	writel(RST_SPRES, RST_SRESCR0);
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}

/* R-Car Gen4 TFA BL31 handoff structure and handling. */
struct param_header {
	u8			type;
	u8			version;
	u16			size;
	u32			attr;
};

struct tfa_image_info {
	struct param_header	h;
	uintptr_t		image_base;
	u32			image_size;
	u32			image_max_size;
};

struct aapcs64_params {
	u64			arg0;
	u64			arg1;
	u64			arg2;
	u64			arg3;
	u64			arg4;
	u64			arg5;
	u64			arg6;
	u64			arg7;
};

struct entry_point_info {
	struct param_header	h;
	uintptr_t		pc;
	u32			spsr;
	struct aapcs64_params	args;
};

struct bl2_to_bl31_params_mem {
	struct tfa_image_info	bl32_image_info;
	struct tfa_image_info	bl33_image_info;
	struct entry_point_info	bl33_ep_info;
	struct entry_point_info	bl32_ep_info;
};

/* Default jump address, return to U-Boot */
#define BL33_BASE	0x44100000
/* Custom parameters address passed to TFA by ICUMXA loader */
#define PARAMS_BASE	0x46422200

/* Usually such a structure is produced by ICUMXA and passed in at 0x46422200 */
static const struct bl2_to_bl31_params_mem blinfo_template = {
	.bl33_ep_info.h.type = 1,	/* PARAM_EP */
	.bl33_ep_info.h.version = 2,	/* Version 2 */
	.bl33_ep_info.h.size = sizeof(struct entry_point_info),
	.bl33_ep_info.h.attr = 0x81,	/* Executable | Non-Secure */
	.bl33_ep_info.spsr = 0x2c9,	/* Mode=EL2, SP=ELX, Exceptions=OFF */
	.bl33_ep_info.pc = BL33_BASE,

	.bl33_image_info.h.type = 1,	/* PARAM_EP */
	.bl33_image_info.h.version = 2,	/* Version 2 */
	.bl33_image_info.h.size = sizeof(struct image_info),
	.bl33_image_info.h.attr = 0,
	.bl33_image_info.image_base = BL33_BASE,
};

static bool tfa_bl31_image_loaded;
static ulong tfa_bl31_image_addr;

static void tfa_bl31_image_process(ulong image, size_t size)
{
	/* Custom parameters address passed to TFA by ICUMXA loader */
	struct bl2_to_bl31_params_mem *blinfo = (struct bl2_to_bl31_params_mem *)PARAMS_BASE;

	/* Not in EL3, do nothing. */
	if (current_el() != 3)
		return;

	/* Clear a page and copy template */
	memset((void *)PARAMS_BASE, 0, PAGE_SIZE);
	memcpy(blinfo, &blinfo_template, sizeof(*blinfo));
	tfa_bl31_image_addr = image;
	tfa_bl31_image_loaded = true;
}

U_BOOT_FIT_LOADABLE_HANDLER(IH_TYPE_TFA_BL31, tfa_bl31_image_process);

void armv8_switch_to_el2_prep(u64 args, u64 mach_nr, u64 fdt_addr,
			      u64 arg4, u64 entry_point, u64 es_flag)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);
	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)(void *)tfa_bl31_image_addr;
	struct bl2_to_bl31_params_mem *blinfo =
		(struct bl2_to_bl31_params_mem *)PARAMS_BASE;

	/* Not in EL3, do nothing. */
	if (current_el() != 3)
		return;

	/*
	 * Destination address in arch/arm/cpu/armv8/transition.S
	 * right past the first bl in armv8_switch_to_el2() to let
	 * the rest of U-Boot pre-Linux code run. The code does run
	 * without stack pointer!
	 */
	const u64 ep = ((u64)(uintptr_t)&armv8_switch_to_el2) + 4;

	/* If TFA BL31 was not part of the fitImage, do regular boot. */
	if (!tfa_bl31_image_loaded)
		return;

	/*
	 * Set up kernel entry point and parameters:
	 * x0 is FDT address, x1..x3 must be 0
	 */
	blinfo->bl33_ep_info.pc = ep;
	blinfo->bl33_ep_info.args.arg0 = args;
	blinfo->bl33_ep_info.args.arg1 = mach_nr;
	blinfo->bl33_ep_info.args.arg2 = fdt_addr;
	blinfo->bl33_ep_info.args.arg3 = arg4;
	blinfo->bl33_ep_info.args.arg4 = entry_point;
	blinfo->bl33_ep_info.args.arg5 = es_flag;
	blinfo->bl33_image_info.image_base = ep;

	/* Jump to TFA BL31 */
	image_entry();
}
