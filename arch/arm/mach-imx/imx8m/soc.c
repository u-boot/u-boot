// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2019 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <log.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/hab.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/syscounter.h>
#include <asm/armv8/mmu.h>
#include <dm/uclass.h>
#include <errno.h>
#include <fdt_support.h>
#include <fsl_wdog.h>
#include <imx_sip.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_IMX_HAB)
struct imx_sec_config_fuse_t const imx_sec_config_fuse = {
	.bank = 1,
	.word = 3,
};
#endif

int timer_init(void)
{
#ifdef CONFIG_SPL_BUILD
	struct sctr_regs *sctr = (struct sctr_regs *)SYSCNT_CTRL_BASE_ADDR;
	unsigned long freq = readl(&sctr->cntfid0);

	/* Update with accurate clock frequency */
	asm volatile("msr cntfrq_el0, %0" : : "r" (freq) : "memory");

	clrsetbits_le32(&sctr->cntcr, SC_CNTCR_FREQ0 | SC_CNTCR_FREQ1,
			SC_CNTCR_FREQ0 | SC_CNTCR_ENABLE | SC_CNTCR_HDBG);
#endif

	gd->arch.tbl = 0;
	gd->arch.tbu = 0;

	return 0;
}

void enable_tzc380(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Enable TZASC and lock setting */
	setbits_le32(&gpr->gpr[10], GPR_TZASC_EN);
	setbits_le32(&gpr->gpr[10], GPR_TZASC_EN_LOCK);
	if (is_imx8mm() || is_imx8mn() || is_imx8mp())
		setbits_le32(&gpr->gpr[10], BIT(1));
	/*
	 * set Region 0 attribute to allow secure and non-secure
	 * read/write permission. Found some masters like usb dwc3
	 * controllers can't work with secure memory.
	 */
	writel(0xf0000000, TZASC_BASE_ADDR + 0x108);
}

void set_wdog_reset(struct wdog_regs *wdog)
{
	/*
	 * Output WDOG_B signal to reset external pmic or POR_B decided by
	 * the board design. Without external reset, the peripherals/DDR/
	 * PMIC are not reset, that may cause system working abnormal.
	 * WDZST bit is write-once only bit. Align this bit in kernel,
	 * otherwise kernel code will have no chance to set this bit.
	 */
	setbits_le16(&wdog->wcr, WDOG_WDT_MASK | WDOG_WDZST_MASK);
}

static struct mm_region imx8m_mem_map[] = {
	{
		/* ROM */
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x100000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		/* CAAM */
		.virt = 0x100000UL,
		.phys = 0x100000UL,
		.size = 0x8000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* TCM */
		.virt = 0x7C0000UL,
		.phys = 0x7C0000UL,
		.size = 0x80000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* OCRAM */
		.virt = 0x900000UL,
		.phys = 0x900000UL,
		.size = 0x200000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		/* AIPS */
		.virt = 0xB00000UL,
		.phys = 0xB00000UL,
		.size = 0x3f500000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* DRAM1 */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = PHYS_SDRAM_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
#ifdef PHYS_SDRAM_2_SIZE
	}, {
		/* DRAM2 */
		.virt = 0x100000000UL,
		.phys = 0x100000000UL,
		.size = PHYS_SDRAM_2_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
#endif
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = imx8m_mem_map;

void enable_caches(void)
{
	/*
	 * If OPTEE runs, remove OPTEE memory from MMU table to
	 * avoid speculative prefetch. OPTEE runs at the top of
	 * the first memory bank
	 */
	if (rom_pointer[1])
		imx8m_mem_map[5].size -= rom_pointer[1];

	icache_enable();
	dcache_enable();
}

static u32 get_cpu_variant_type(u32 type)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;

	u32 value = readl(&fuse->tester4);

	if (type == MXC_CPU_IMX8MQ) {
		if ((value & 0x3) == 0x2)
			return MXC_CPU_IMX8MD;
		else if (value & 0x200000)
			return MXC_CPU_IMX8MQL;

	} else if (type == MXC_CPU_IMX8MM) {
		switch (value & 0x3) {
		case 2:
			if (value & 0x1c0000)
				return MXC_CPU_IMX8MMDL;
			else
				return MXC_CPU_IMX8MMD;
		case 3:
			if (value & 0x1c0000)
				return MXC_CPU_IMX8MMSL;
			else
				return MXC_CPU_IMX8MMS;
		default:
			if (value & 0x1c0000)
				return MXC_CPU_IMX8MML;
			break;
		}
	} else if (type == MXC_CPU_IMX8MN) {
		switch (value & 0x3) {
		case 2:
			if (value & 0x1000000)
				return MXC_CPU_IMX8MNDL;
			else
				return MXC_CPU_IMX8MND;
		case 3:
			if (value & 0x1000000)
				return MXC_CPU_IMX8MNSL;
			else
				return MXC_CPU_IMX8MNS;
		default:
			if (value & 0x1000000)
				return MXC_CPU_IMX8MNL;
			break;
		}
	}

	return type;
}

u32 get_cpu_rev(void)
{
	struct anamix_pll *ana_pll = (struct anamix_pll *)ANATOP_BASE_ADDR;
	u32 reg = readl(&ana_pll->digprog);
	u32 type = (reg >> 16) & 0xff;
	u32 major_low = (reg >> 8) & 0xff;
	u32 rom_version;

	reg &= 0xff;

	/* iMX8MP */
	if (major_low == 0x43) {
		return (MXC_CPU_IMX8MP << 12) | reg;
	} else if (major_low == 0x42) {
		/* iMX8MN */
		type = get_cpu_variant_type(MXC_CPU_IMX8MN);
	} else if (major_low == 0x41) {
		type = get_cpu_variant_type(MXC_CPU_IMX8MM);
	} else {
		if (reg == CHIP_REV_1_0) {
			/*
			 * For B0 chip, the DIGPROG is not updated,
			 * it is still TO1.0. we have to check ROM
			 * version or OCOTP_READ_FUSE_DATA.
			 * 0xff0055aa is magic number for B1.
			 */
			if (readl((void __iomem *)(OCOTP_BASE_ADDR + 0x40)) == 0xff0055aa) {
				reg = CHIP_REV_2_1;
			} else {
				rom_version =
					readl((void __iomem *)ROM_VERSION_A0);
				if (rom_version != CHIP_REV_1_0) {
					rom_version = readl((void __iomem *)ROM_VERSION_B0);
					rom_version &= 0xff;
					if (rom_version == CHIP_REV_2_0)
						reg = CHIP_REV_2_0;
				}
			}
		}

		type = get_cpu_variant_type(type);
	}

	return (type << 12) | reg;
}

static void imx_set_wdog_powerdown(bool enable)
{
	struct wdog_regs *wdog1 = (struct wdog_regs *)WDOG1_BASE_ADDR;
	struct wdog_regs *wdog2 = (struct wdog_regs *)WDOG2_BASE_ADDR;
	struct wdog_regs *wdog3 = (struct wdog_regs *)WDOG3_BASE_ADDR;

	/* Write to the PDE (Power Down Enable) bit */
	writew(enable, &wdog1->wmcr);
	writew(enable, &wdog2->wmcr);
	writew(enable, &wdog3->wmcr);
}

int arch_cpu_init_dm(void)
{
	struct udevice *dev;
	int ret;

	if (CONFIG_IS_ENABLED(CLK)) {
		ret = uclass_get_device_by_name(UCLASS_CLK,
						"clock-controller@30380000",
						&dev);
		if (ret < 0) {
			printf("Failed to find clock node. Check device tree\n");
			return ret;
		}
	}

	return 0;
}

int arch_cpu_init(void)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	/*
	 * ROM might disable clock for SCTR,
	 * enable the clock before timer_init.
	 */
	if (IS_ENABLED(CONFIG_SPL_BUILD))
		clock_enable(CCGR_SCTR, 1);
	/*
	 * Init timer at very early state, because sscg pll setting
	 * will use it
	 */
	timer_init();

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		clock_init();
		imx_set_wdog_powerdown(false);
	}

	if (is_imx8mq()) {
		clock_enable(CCGR_OCOTP, 1);
		if (readl(&ocotp->ctrl) & 0x200)
			writel(0x200, &ocotp->ctrl_clr);
	}

	return 0;
}

#if defined(CONFIG_IMX8MN) || defined(CONFIG_IMX8MP)
struct rom_api *g_rom_api = (struct rom_api *)0x980;

enum boot_device get_boot_device(void)
{
	volatile gd_t *pgd = gd;
	int ret;
	u32 boot;
	u16 boot_type;
	u8 boot_instance;
	enum boot_device boot_dev = SD1_BOOT;

	ret = g_rom_api->query_boot_infor(QUERY_BT_DEV, &boot,
					  ((uintptr_t)&boot) ^ QUERY_BT_DEV);
	gd = pgd;

	if (ret != ROM_API_OKAY) {
		puts("ROMAPI: failure at query_boot_info\n");
		return -1;
	}

	boot_type = boot >> 16;
	boot_instance = (boot >> 8) & 0xff;

	switch (boot_type) {
	case BT_DEV_TYPE_SD:
		boot_dev = boot_instance + SD1_BOOT;
		break;
	case BT_DEV_TYPE_MMC:
		boot_dev = boot_instance + MMC1_BOOT;
		break;
	case BT_DEV_TYPE_NAND:
		boot_dev = NAND_BOOT;
		break;
	case BT_DEV_TYPE_FLEXSPINOR:
		boot_dev = QSPI_BOOT;
		break;
	case BT_DEV_TYPE_USB:
		boot_dev = USB_BOOT;
		break;
	default:
		break;
	}

	return boot_dev;
}
#endif

bool is_usb_boot(void)
{
	return get_boot_device() == USB_BOOT;
}

#ifdef CONFIG_OF_SYSTEM_SETUP
int ft_system_setup(void *blob, bd_t *bd)
{
	int i = 0;
	int rc;
	int nodeoff;

	/* Disable the CPU idle for A0 chip since the HW does not support it */
	if (is_soc_rev(CHIP_REV_1_0)) {
		static const char * const nodes_path[] = {
			"/cpus/cpu@0",
			"/cpus/cpu@1",
			"/cpus/cpu@2",
			"/cpus/cpu@3",
		};

		for (i = 0; i < ARRAY_SIZE(nodes_path); i++) {
			nodeoff = fdt_path_offset(blob, nodes_path[i]);
			if (nodeoff < 0)
				continue; /* Not found, skip it */

			debug("Found %s node\n", nodes_path[i]);

			rc = fdt_delprop(blob, nodeoff, "cpu-idle-states");
			if (rc == -FDT_ERR_NOTFOUND)
				continue;
			if (rc) {
				printf("Unable to update property %s:%s, err=%s\n",
				       nodes_path[i], "status", fdt_strerror(rc));
				return rc;
			}

			debug("Remove %s:%s\n", nodes_path[i],
			       "cpu-idle-states");
		}
	}

	return 0;
}
#endif

#if !CONFIG_IS_ENABLED(SYSRESET)
void reset_cpu(ulong addr)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	/* Clear WDA to trigger WDOG_B immediately */
	writew((SET_WCR_WT(1) | WCR_WDT | WCR_WDE | WCR_SRS), &wdog->wcr);

	while (1) {
		/*
		 * spin for .5 seconds before reset
		 */
	}
}
#endif

#if defined(CONFIG_ARCH_MISC_INIT)
static void acquire_buildinfo(void)
{
	u64 atf_commit = 0;

	/* Get ARM Trusted Firmware commit id */
	atf_commit = call_imx_sip(IMX_SIP_BUILDINFO,
				  IMX_SIP_BUILDINFO_GET_COMMITHASH, 0, 0, 0);
	if (atf_commit == 0xffffffff) {
		debug("ATF does not support build info\n");
		atf_commit = 0x30; /* Display 0, 0 ascii is 0x30 */
	}

	printf("\n BuildInfo:\n  - ATF %s\n\n", (char *)&atf_commit);
}

int arch_misc_init(void)
{
	acquire_buildinfo();

	return 0;
}
#endif

void imx_tmu_arch_init(void *reg_base)
{
	if (is_imx8mm() || is_imx8mn()) {
		/* Load TCALIV and TASR from fuses */
		struct ocotp_regs *ocotp =
			(struct ocotp_regs *)OCOTP_BASE_ADDR;
		struct fuse_bank *bank = &ocotp->bank[3];
		struct fuse_bank3_regs *fuse =
			(struct fuse_bank3_regs *)bank->fuse_regs;

		u32 tca_rt, tca_hr, tca_en;
		u32 buf_vref, buf_slope;

		tca_rt = fuse->ana0 & 0xFF;
		tca_hr = (fuse->ana0 & 0xFF00) >> 8;
		tca_en = (fuse->ana0 & 0x2000000) >> 25;

		buf_vref = (fuse->ana0 & 0x1F00000) >> 20;
		buf_slope = (fuse->ana0 & 0xF0000) >> 16;

		writel(buf_vref | (buf_slope << 16), (ulong)reg_base + 0x28);
		writel((tca_en << 31) | (tca_hr << 16) | tca_rt,
		       (ulong)reg_base + 0x30);
	}
#ifdef CONFIG_IMX8MP
	/* Load TCALIV0/1/m40 and TRIM from fuses */
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[38];
	struct fuse_bank38_regs *fuse =
		(struct fuse_bank38_regs *)bank->fuse_regs;
	struct fuse_bank *bank2 = &ocotp->bank[39];
	struct fuse_bank39_regs *fuse2 =
		(struct fuse_bank39_regs *)bank2->fuse_regs;
	u32 buf_vref, buf_slope, bjt_cur, vlsb, bgr;
	u32 reg;
	u32 tca40[2], tca25[2], tca105[2];

	/* For blank sample */
	if (!fuse->ana_trim2 && !fuse->ana_trim3 &&
	    !fuse->ana_trim4 && !fuse2->ana_trim5) {
		/* Use a default 25C binary codes */
		tca25[0] = 1596;
		tca25[1] = 1596;
		writel(tca25[0], (ulong)reg_base + 0x30);
		writel(tca25[1], (ulong)reg_base + 0x34);
		return;
	}

	buf_vref = (fuse->ana_trim2 & 0xc0) >> 6;
	buf_slope = (fuse->ana_trim2 & 0xF00) >> 8;
	bjt_cur = (fuse->ana_trim2 & 0xF000) >> 12;
	bgr = (fuse->ana_trim2 & 0xF0000) >> 16;
	vlsb = (fuse->ana_trim2 & 0xF00000) >> 20;
	writel(buf_vref | (buf_slope << 16), (ulong)reg_base + 0x28);

	reg = (bgr << 28) | (bjt_cur << 20) | (vlsb << 12) | (1 << 7);
	writel(reg, (ulong)reg_base + 0x3c);

	tca40[0] = (fuse->ana_trim3 & 0xFFF0000) >> 16;
	tca25[0] = (fuse->ana_trim3 & 0xF0000000) >> 28;
	tca25[0] |= ((fuse->ana_trim4 & 0xFF) << 4);
	tca105[0] = (fuse->ana_trim4 & 0xFFF00) >> 8;
	tca40[1] = (fuse->ana_trim4 & 0xFFF00000) >> 20;
	tca25[1] = fuse2->ana_trim5 & 0xFFF;
	tca105[1] = (fuse2->ana_trim5 & 0xFFF000) >> 12;

	/* use 25c for 1p calibration */
	writel(tca25[0] | (tca105[0] << 16), (ulong)reg_base + 0x30);
	writel(tca25[1] | (tca105[1] << 16), (ulong)reg_base + 0x34);
	writel(tca40[0] | (tca40[1] << 16), (ulong)reg_base + 0x38);
#endif
}
