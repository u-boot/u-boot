// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/armv8/mmu.h>
#include <asm/mach-imx/boot_mode.h>
#include <efi_loader.h>
#include <spl.h>
#include <asm/arch/s400_api.h>
#include <asm/arch/mu_hal.h>
#include <cpu_func.h>
#include <asm/setup.h>

DECLARE_GLOBAL_DATA_PTR;

struct rom_api *g_rom_api = (struct rom_api *)0x1980;

u32 get_cpu_rev(void)
{
	return (MXC_CPU_IMX8ULP << 12) | CHIP_REV_1_0;
}

enum bt_mode get_boot_mode(void)
{
	u32 bt0_cfg = 0;

	bt0_cfg = readl(CMC1_BASE_ADDR + 0xa0);
	bt0_cfg &= (BT0CFG_LPBOOT_MASK | BT0CFG_DUALBOOT_MASK);

	if (!(bt0_cfg & BT0CFG_LPBOOT_MASK)) {
		/* No low power boot */
		if (bt0_cfg & BT0CFG_DUALBOOT_MASK)
			return DUAL_BOOT;
		else
			return SINGLE_BOOT;
	}

	return LOW_POWER_BOOT;
}

#define CMC_SRS_TAMPER                    BIT(31)
#define CMC_SRS_SECURITY                  BIT(30)
#define CMC_SRS_TZWDG                     BIT(29)
#define CMC_SRS_JTAG_RST                  BIT(28)
#define CMC_SRS_CORE1                     BIT(16)
#define CMC_SRS_LOCKUP                    BIT(15)
#define CMC_SRS_SW                        BIT(14)
#define CMC_SRS_WDG                       BIT(13)
#define CMC_SRS_PIN_RESET                 BIT(8)
#define CMC_SRS_WARM                      BIT(4)
#define CMC_SRS_HVD                       BIT(3)
#define CMC_SRS_LVD                       BIT(2)
#define CMC_SRS_POR                       BIT(1)
#define CMC_SRS_WUP                       BIT(0)

static u32 reset_cause = -1;

static char *get_reset_cause(char *ret)
{
	u32 cause1, cause = 0, srs = 0;
	void __iomem *reg_ssrs = (void __iomem *)(CMC1_BASE_ADDR + 0x88);
	void __iomem *reg_srs = (void __iomem *)(CMC1_BASE_ADDR + 0x80);

	if (!ret)
		return "null";

	srs = readl(reg_srs);
	cause1 = readl(reg_ssrs);

	reset_cause = cause1;

	cause = cause1 & (CMC_SRS_POR | CMC_SRS_WUP | CMC_SRS_WARM);

	switch (cause) {
	case CMC_SRS_POR:
		sprintf(ret, "%s", "POR");
		break;
	case CMC_SRS_WUP:
		sprintf(ret, "%s", "WUP");
		break;
	case CMC_SRS_WARM:
		cause = cause1 & (CMC_SRS_WDG | CMC_SRS_SW |
			CMC_SRS_JTAG_RST);
		switch (cause) {
		case CMC_SRS_WDG:
			sprintf(ret, "%s", "WARM-WDG");
			break;
		case CMC_SRS_SW:
			sprintf(ret, "%s", "WARM-SW");
			break;
		case CMC_SRS_JTAG_RST:
			sprintf(ret, "%s", "WARM-JTAG");
			break;
		default:
			sprintf(ret, "%s", "WARM-UNKN");
			break;
		}
		break;
	default:
		sprintf(ret, "%s-%X", "UNKN", cause1);
		break;
	}

	debug("[%X] SRS[%X] %X - ", cause1, srs, srs ^ cause1);
	return ret;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
const char *get_imx_type(u32 imxtype)
{
	return "8ULP";
}

int print_cpuinfo(void)
{
	u32 cpurev;
	char cause[18];

	cpurev = get_cpu_rev();

	printf("CPU:   Freescale i.MX%s rev%d.%d at %d MHz\n",
	       get_imx_type((cpurev & 0xFF000) >> 12),
	       (cpurev & 0x000F0) >> 4, (cpurev & 0x0000F) >> 0,
	       mxc_get_clock(MXC_ARM_CLK) / 1000000);

	printf("Reset cause: %s\n", get_reset_cause(cause));

	printf("Boot mode: ");
	switch (get_boot_mode()) {
	case LOW_POWER_BOOT:
		printf("Low power boot\n");
		break;
	case DUAL_BOOT:
		printf("Dual boot\n");
		break;
	case SINGLE_BOOT:
	default:
		printf("Single boot\n");
		break;
	}

	return 0;
}
#endif

#define UNLOCK_WORD0 0xC520 /* 1st unlock word */
#define UNLOCK_WORD1 0xD928 /* 2nd unlock word */
#define REFRESH_WORD0 0xA602 /* 1st refresh word */
#define REFRESH_WORD1 0xB480 /* 2nd refresh word */

static void disable_wdog(void __iomem *wdog_base)
{
	u32 val_cs = readl(wdog_base + 0x00);

	if (!(val_cs & 0x80))
		return;

	dmb();
	__raw_writel(REFRESH_WORD0, (wdog_base + 0x04)); /* Refresh the CNT */
	__raw_writel(REFRESH_WORD1, (wdog_base + 0x04));
	dmb();

	if (!(val_cs & 800)) {
		dmb();
		__raw_writel(UNLOCK_WORD0, (wdog_base + 0x04));
		__raw_writel(UNLOCK_WORD1, (wdog_base + 0x04));
		dmb();

		while (!(readl(wdog_base + 0x00) & 0x800))
			;
	}
	writel(0x0, (wdog_base + 0x0C)); /* Set WIN to 0 */
	writel(0x400, (wdog_base + 0x08)); /* Set timeout to default 0x400 */
	writel(0x120, (wdog_base + 0x00)); /* Disable it and set update */

	while (!(readl(wdog_base + 0x00) & 0x400))
		;
}

void init_wdog(void)
{
	disable_wdog((void __iomem *)WDG3_RBASE);
}

static struct mm_region imx8ulp_arm64_mem_map[] = {
	{
		/* ROM */
		.virt = 0x0,
		.phys = 0x0,
		.size = 0x40000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	},
	{
		/* FLEXSPI0 */
		.virt = 0x04000000,
		.phys = 0x04000000,
		.size = 0x08000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		/* SSRAM (align with 2M) */
		.virt = 0x1FE00000UL,
		.phys = 0x1FE00000UL,
		.size = 0x400000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* SRAM1 (align with 2M) */
		.virt = 0x21000000UL,
		.phys = 0x21000000UL,
		.size = 0x200000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* SRAM0 (align with 2M) */
		.virt = 0x22000000UL,
		.phys = 0x22000000UL,
		.size = 0x200000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* Peripherals */
		.virt = 0x27000000UL,
		.phys = 0x27000000UL,
		.size = 0x3000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* Peripherals */
		.virt = 0x2D000000UL,
		.phys = 0x2D000000UL,
		.size = 0x1600000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* FLEXSPI1-2 */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* DRAM1 */
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = PHYS_SDRAM_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		/*
		 * empty entrie to split table entry 5
		 * if needed when TEEs are used
		 */
		0,
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = imx8ulp_arm64_mem_map;

/* simplify the page table size to enhance boot speed */
#define MAX_PTE_ENTRIES		512
#define MAX_MEM_MAP_REGIONS	16
u64 get_page_table_size(void)
{
	u64 one_pt = MAX_PTE_ENTRIES * sizeof(u64);
	u64 size = 0;

	/*
	 * For each memory region, the max table size:
	 * 2 level 3 tables + 2 level 2 tables + 1 level 1 table
	 */
	size = (2 + 2 + 1) * one_pt * MAX_MEM_MAP_REGIONS + one_pt;

	/*
	 * We need to duplicate our page table once to have an emergency pt to
	 * resort to when splitting page tables later on
	 */
	size *= 2;

	/*
	 * We may need to split page tables later on if dcache settings change,
	 * so reserve up to 4 (random pick) page tables for that.
	 */
	size += one_pt * 4;

	return size;
}

void enable_caches(void)
{
	/* TODO: add TEE memmap region */

	icache_enable();
	dcache_enable();
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	/* TODO */
}
#endif

static void set_core0_reset_vector(u32 entry)
{
	/* Update SIM1 DGO8 for reset vector base */
	writel(entry, SIM1_BASE_ADDR + 0x5c);

	/* set update bit */
	setbits_le32(SIM1_BASE_ADDR + 0x8, 0x1 << 24);

	/* polling the ack */
	while ((readl(SIM1_BASE_ADDR + 0x8) & (0x1 << 26)) == 0)
		;

	/* clear the update */
	clrbits_le32(SIM1_BASE_ADDR + 0x8, (0x1 << 24));

	/* clear the ack by set 1 */
	setbits_le32(SIM1_BASE_ADDR + 0x8, (0x1 << 26));
}

enum rdc_type {
	RDC_TRDC,
	RDC_XRDC,
};

static int release_rdc(enum rdc_type type)
{
	ulong s_mu_base = 0x27020000UL;
	struct imx8ulp_s400_msg msg;
	int ret;
	u32 rdc_id = (type == RDC_XRDC) ? 0x78 : 0x74;

	msg.version = AHAB_VERSION;
	msg.tag = AHAB_CMD_TAG;
	msg.size = 2;
	msg.command = AHAB_RELEASE_RDC_REQ_CID;
	msg.data[0] = (rdc_id << 8) | 0x2; /* A35 XRDC */

	mu_hal_init(s_mu_base);
	mu_hal_sendmsg(s_mu_base, 0, *((u32 *)&msg));
	mu_hal_sendmsg(s_mu_base, 1, msg.data[0]);

	ret = mu_hal_receivemsg(s_mu_base, 0, (u32 *)&msg);
	if (!ret) {
		ret = mu_hal_receivemsg(s_mu_base, 1, &msg.data[0]);
		if (!ret) {
			if ((msg.data[0] & 0xff) == 0xd6)
				return 0;
		}

		return -EIO;
	}

	return ret;
}

struct mbc_mem_dom {
	u32 mem_glbcfg[4];
	u32 nse_blk_index;
	u32 nse_blk_set;
	u32 nse_blk_clr;
	u32 nsr_blk_clr_all;
	u32 memn_glbac[8];
	/* The upper only existed in the beginning of each MBC */
	u32 mem0_blk_cfg_w[64];
	u32 mem0_blk_nse_w[16];
	u32 mem1_blk_cfg_w[8];
	u32 mem1_blk_nse_w[2];
	u32 mem2_blk_cfg_w[8];
	u32 mem2_blk_nse_w[2];
	u32 mem3_blk_cfg_w[8];
	u32 mem3_blk_nse_w[2];/*0x1F0, 0x1F4 */
	u32 reserved[2];
};

struct trdc {
	u8 res0[0x1000];
	struct mbc_mem_dom mem_dom[4][8];
};

/* MBC[m]_[d]_MEM[s]_BLK_CFG_W[w] */
int trdc_mbc_set_access(u32 mbc_x, u32 dom_x, u32 mem_x, u32 blk_x, u32 perm)
{
	struct trdc *trdc_base = (struct trdc *)0x28031000U;
	struct mbc_mem_dom *mbc_dom;
	u32 *cfg_w, *nse_w;
	u32 index, offset, val;

	mbc_dom = &trdc_base->mem_dom[mbc_x][dom_x];

	switch (mem_x) {
	case 0:
		cfg_w = &mbc_dom->mem0_blk_cfg_w[blk_x / 8];
		nse_w = &mbc_dom->mem0_blk_nse_w[blk_x / 32];
		break;
	case 1:
		cfg_w = &mbc_dom->mem1_blk_cfg_w[blk_x / 8];
		nse_w = &mbc_dom->mem1_blk_nse_w[blk_x / 32];
		break;
	case 2:
		cfg_w = &mbc_dom->mem2_blk_cfg_w[blk_x / 8];
		nse_w = &mbc_dom->mem2_blk_nse_w[blk_x / 32];
		break;
	case 3:
		cfg_w = &mbc_dom->mem3_blk_cfg_w[blk_x / 8];
		nse_w = &mbc_dom->mem3_blk_nse_w[blk_x / 32];
		break;
	default:
		return -EINVAL;
	};

	index = blk_x % 8;
	offset = index * 4;

	val = readl((void __iomem *)cfg_w);

	val &= ~(0xFU << offset);

	if (perm == 0x7700) {
		val |= (0x0 << offset);
		writel(perm, (void __iomem *)cfg_w);
	} else if (perm == 0x0077) {
		val |= (0x8 << offset); /* nse bit set */
		writel(val, (void __iomem *)cfg_w);
	} else {
		return -EINVAL;
	}

	return 0;
}

int trdc_set_access(void)
{
	/*
	 * CGC0: PBridge0 slot 47
	 * trdc_mbc_set_access(2, 7, 0, 47, 0x7700);
	 * For secure access, default single boot already support,
	 * For non-secure access, need add in future per usecase.
	 */
	 trdc_mbc_set_access(2, 7, 0, 49, 0x7700);
	 trdc_mbc_set_access(2, 7, 0, 50, 0x7700);
	 trdc_mbc_set_access(2, 7, 0, 51, 0x7700);
	 trdc_mbc_set_access(2, 7, 0, 52, 0x7700);

	 trdc_mbc_set_access(2, 7, 0, 47, 0x0077);

	 /* iomuxc 0 */
	 trdc_mbc_set_access(2, 7, 1, 33, 0x7700);

	return 0;
}

static void xrdc_mrc_region_set_access(int mrc_index, u32 addr, u32 access)
{
	ulong xrdc_base = 0x292f0000, off;
	u32 mrgd[5];
	u8 mrcfg, j, region_num;
	u8 dsel;

	mrcfg = readb(xrdc_base + 0x140 + mrc_index);
	region_num = mrcfg & 0x1f;

	for (j = 0; j < region_num; j++) {
		off = 0x2000 + mrc_index * 0x200 + j * 0x20;

		mrgd[0] = readl(xrdc_base + off);
		mrgd[1] = readl(xrdc_base + off + 4);
		mrgd[2] = readl(xrdc_base + off + 8);
		mrgd[3] = readl(xrdc_base + off + 0xc);
		mrgd[4] = readl(xrdc_base + off + 0x10);

		debug("MRC [%u][%u]\n", mrc_index, j);
		debug("0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		      mrgd[0], mrgd[1], mrgd[2], mrgd[3], mrgd[4]);

		/* hit */
		if (addr >= mrgd[0] && addr <= mrgd[1]) {
			/* find domain 7 DSEL */
			dsel = (mrgd[2] >> 21) & 0x7;
			if (dsel == 1) {
				mrgd[4] &= ~0xFFF;
				mrgd[4] |= (access & 0xFFF);
			} else if (dsel == 2) {
				mrgd[4] &= ~0xFFF0000;
				mrgd[4] |= ((access & 0xFFF) << 16);
			}

			/* not handle other cases, since S400 only set ACCESS1 and 2 */
			writel(mrgd[4], xrdc_base + off + 0x10);
			return;
		}
	}
}

int arch_cpu_init(void)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		/* Disable wdog */
		init_wdog();

		if (get_boot_mode() == SINGLE_BOOT) {
			release_rdc(RDC_TRDC);
			trdc_set_access();
			/* LPAV to APD */
			setbits_le32(0x2802B044, BIT(7));
			/* GPU 2D/3D to APD */
			setbits_le32(0x2802B04C, BIT(1) | BIT(2));
		}

		/* release xrdc, then allow A35 to write SRAM2 */
		release_rdc(RDC_XRDC);
		xrdc_mrc_region_set_access(2, CONFIG_SPL_TEXT_BASE, 0xE00);

		clock_init();
	} else {
		/* reconfigure core0 reset vector to ROM */
		set_core0_reset_vector(0x1000);
	}

	return 0;
}

#if defined(CONFIG_SPL_BUILD)
__weak void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	debug("image entry point: 0x%lx\n", spl_image->entry_point);

	set_core0_reset_vector((u32)spl_image->entry_point);

	/* Enable the 512KB cache */
	setbits_le32(SIM1_BASE_ADDR + 0x30, (0x1 << 4));

	/* reset core */
	setbits_le32(SIM1_BASE_ADDR + 0x30, (0x1 << 16));

	while (1)
		;
}
#endif
