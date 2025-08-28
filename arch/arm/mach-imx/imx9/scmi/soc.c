// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/armv8/mmu.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/setup.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <env_internal.h>
#include <fuse.h>
#include <imx_thermal.h>
#include <linux/iopoll.h>
#include <scmi_agent.h>

DECLARE_GLOBAL_DATA_PTR;

static rom_passover_t rom_passover_data = {0};

uint32_t scmi_get_rom_data(rom_passover_t *rom_data)
{
	/* Read ROM passover data */
	struct scmi_rom_passover_get_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_IMX_MISC,
		.message_id = SCMI_MISC_ROM_PASSOVER_GET,
		.in_msg = (u8 *)NULL,
		.in_msg_sz = 0,
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;
	struct udevice *dev;

	ret = uclass_get_device_by_name(UCLASS_CLK, "protocol@14", &dev);
	if (ret)
		return ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret == 0 && out.status == 0) {
		memcpy(rom_data, (struct rom_passover_t *)out.passover, sizeof(rom_passover_t));
	} else {
		printf("Failed to get ROM passover data, scmi_err = %d, size_of(out) = %ld\n",
		       out.status, sizeof(out));
		return -EINVAL;
	}

	return 0;
}

#if IS_ENABLED(CONFIG_ENV_IS_IN_MMC)
__weak int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int mmc_get_env_dev(void)
{
	int ret;
	u16 boot_type;
	u8 boot_instance;

	gd_t *pgd = gd;
	rom_passover_t *rdata;

#if IS_ENABLED(CONFIG_XPL_BUILD)
	rdata = &rom_passover_data;
#else
	rom_passover_t rom_data = {0};

	if (!pgd->reloc_off)
		rdata = &rom_data;
	else
		rdata = &rom_passover_data;
#endif
	if (rdata->tag == 0) {
		ret = scmi_get_rom_data(rdata);
		if (ret != 0) {
			puts("SCMI: failure at rom_boot_info\n");
			return CONFIG_ENV_MMC_DEVICE_INDEX;
		}
	}
	boot_type = rdata->boot_dev_type;
	boot_instance = rdata->boot_dev_inst;
	set_gd(pgd);

	debug("boot_type %d, instance %d\n", boot_type, boot_instance);

	/* If not boot from sd/mmc, use default value */
	if (boot_type != BOOT_TYPE_SD && boot_type != BOOT_TYPE_MMC)
		return env_get_ulong("mmcdev", 10, CONFIG_ENV_MMC_DEVICE_INDEX);

	return board_mmc_get_env_dev(boot_instance);
}
#endif

u32 get_cpu_speed_grade_hz(void)
{
	u32 speed, max_speed;
	int ret;
	u32 val, word, offset;

	word = 17;
	offset = 14;

	ret = fuse_read(word / 8, word % 8, &val);
	if (ret)
		val = 0; /* If read fuse failed, return as blank fuse */

	val >>= offset;
	val &= 0xf;

	max_speed = 2300000000;
	speed = max_speed - val * 100000000;

	if (is_imx95())
		max_speed = 2000000000;

	/* In case the fuse of speed grade not programmed */
	if (speed > max_speed)
		speed = max_speed;

	return speed;
}

u32 get_cpu_temp_grade(int *minc, int *maxc)
{
	int ret;
	u32 val, word, offset;

	word = 17;
	offset = 12;

	ret = fuse_read(word / 8, word % 8, &val);
	if (ret)
		val = 0; /* If read fuse failed, return as blank fuse */

	val >>= offset;
	val &= 0x3;

	if (minc && maxc) {
		if (val == TEMP_AUTOMOTIVE) {
			*minc = -40;
			*maxc = 125;
		} else if (val == TEMP_INDUSTRIAL) {
			*minc = -40;
			*maxc = 105;
		} else if (val == TEMP_EXTCOMMERCIAL) {
			*minc = -20;
			*maxc = 105;
		} else {
			*minc = 0;
			*maxc = 95;
		}
	}
	return val;
}

static void set_cpu_info(struct ele_get_info_data *info)
{
	gd->arch.soc_rev = info->soc;
	gd->arch.lifecycle = info->lc;
	memcpy((void *)&gd->arch.uid, &info->uid, 4 * sizeof(u32));
}

u32 get_cpu_rev(void)
{
	u32 rev = (gd->arch.soc_rev >> 24) - 0xa0;

	return (MXC_CPU_IMX95 << 12) | (CHIP_REV_1_0 + rev);
}

#define UNLOCK_WORD 0xD928C520
#define REFRESH_WORD 0xB480A602

static void disable_wdog(void __iomem *wdog_base)
{
	u32 val_cs = readl(wdog_base + 0x00);
	int ret = 0;

	if (!(val_cs & 0x80))
		return;

	/* default is 32bits cmd */
	writel(REFRESH_WORD, (wdog_base + 0x04)); /* Refresh the CNT */

	if (!(val_cs & 0x800)) {
		writel(UNLOCK_WORD, (wdog_base + 0x04));
		while (!(readl(wdog_base + 0x00) & 0x800))
			;
	}
	writel(0x0, (wdog_base + 0x0C)); /* Set WIN to 0 */
	writel(0x400, (wdog_base + 0x08)); /* Set timeout to default 0x400 */
	writel(0x2120, (wdog_base + 0x00)); /* Disable it and set update */

	ret = readl_poll_timeout(wdog_base, val_cs, val_cs & 0x400, 100000);
	if (ret < 0)
		debug("%s timeout\n", __func__);
}

static struct mm_region imx9_mem_map[] = {
	{
		/* M7 TCM */
		.virt = 0x203c0000UL,
		.phys = 0x203c0000UL,
		.size = 0x80000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* OCRAM */
		.virt = 0x20480000UL,
		.phys = 0x20480000UL,
		.size = 0xA0000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		/* AIPS */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* Flexible Serial Peripheral Interface */
		.virt = 0x28000000UL,
		.phys = 0x28000000UL,
		.size = 0x8000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* DRAM1 */
		.virt = PHYS_SDRAM,
		.phys = PHYS_SDRAM,
		.size = PHYS_SDRAM_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				 PTE_BLOCK_OUTER_SHARE
	}, {
#ifdef PHYS_SDRAM_2_SIZE
		/* DRAM2 */
		.virt = 0x100000000UL,
		.phys = 0x100000000UL,
		.size = PHYS_SDRAM_2_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
#endif
		/* empty entry to split table entry 5 if needed when TEEs are used */
		0,
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = imx9_mem_map;

static unsigned int imx9_find_dram_entry_in_mem_map(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(imx9_mem_map); i++)
		if (imx9_mem_map[i].phys == CFG_SYS_SDRAM_BASE)
			return i;

	hang();	/* Entry not found, this must never happen. */
}

void enable_caches(void)
{
	/* If OPTEE runs, remove OPTEE memory from MMU table to avoid speculative prefetch
	 * If OPTEE does not run, still update the MMU table according to dram banks structure
	 * to set correct dram size from board_phys_sdram_size
	 */
	int i = 0;
	/*
	 * please make sure that entry initial value matches
	 * imx9_mem_map for DRAM1
	 */
	int entry = imx9_find_dram_entry_in_mem_map();
	u64 attrs = imx9_mem_map[entry].attrs;

	while (i < CONFIG_NR_DRAM_BANKS &&
	       entry < ARRAY_SIZE(imx9_mem_map)) {
		if (gd->bd->bi_dram[i].start == 0)
			break;
		imx9_mem_map[entry].phys = gd->bd->bi_dram[i].start;
		imx9_mem_map[entry].virt = gd->bd->bi_dram[i].start;
		imx9_mem_map[entry].size = gd->bd->bi_dram[i].size;
		imx9_mem_map[entry].attrs = attrs;
		debug("Added memory mapping (%d): %llx %llx\n", entry,
		      imx9_mem_map[entry].phys, imx9_mem_map[entry].size);
		i++; entry++;
	}

	icache_enable();
	dcache_enable();
}

__weak int board_phys_sdram_size(phys_size_t *size)
{
	phys_size_t start, end;
	phys_size_t val;

	if (!size)
		return -EINVAL;

	val = readl(REG_DDR_CS0_BNDS);
	start = (val >> 16) << 24;
	end   = (val & 0xFFFF);
	end   = end ? end + 1 : 0;
	end   = end << 24;
	*size = end - start;

	val = readl(REG_DDR_CS1_BNDS);
	start = (val >> 16) << 24;
	end   = (val & 0xFFFF);
	end   = end ? end + 1 : 0;
	end   = end << 24;
	*size += end - start;

	return 0;
}

int dram_init(void)
{
	phys_size_t sdram_size;
	int ret;

	ret = board_phys_sdram_size(&sdram_size);
	if (ret)
		return ret;

	/* rom_pointer[1] contains the size of TEE occupies */
	if (rom_pointer[1] && PHYS_SDRAM < (phys_addr_t)rom_pointer[0])
		gd->ram_size = sdram_size - rom_pointer[1];
	else
		gd->ram_size = sdram_size;

	return 0;
}

int dram_init_banksize(void)
{
	int bank = 0;
	int ret;
	phys_size_t sdram_size;
	phys_size_t sdram_b1_size, sdram_b2_size;

	ret = board_phys_sdram_size(&sdram_size);
	if (ret)
		return ret;

	/* Bank 1 can't cross over 4GB space */
	if (sdram_size > 0x80000000) {
		sdram_b1_size = 0x100000000UL - PHYS_SDRAM;
		sdram_b2_size = sdram_size - sdram_b1_size;
	} else {
		sdram_b1_size = sdram_size;
		sdram_b2_size = 0;
	}

	gd->bd->bi_dram[bank].start = PHYS_SDRAM;
	if (rom_pointer[1] && PHYS_SDRAM < (phys_addr_t)rom_pointer[0]) {
		phys_addr_t optee_start = (phys_addr_t)rom_pointer[0];
		phys_size_t optee_size = (size_t)rom_pointer[1];

		gd->bd->bi_dram[bank].size = optee_start - gd->bd->bi_dram[bank].start;
		if ((optee_start + optee_size) < (PHYS_SDRAM + sdram_b1_size)) {
			if (++bank >= CONFIG_NR_DRAM_BANKS) {
				puts("CONFIG_NR_DRAM_BANKS is not enough\n");
				return -1;
			}

			gd->bd->bi_dram[bank].start = optee_start + optee_size;
			gd->bd->bi_dram[bank].size = PHYS_SDRAM +
				sdram_b1_size - gd->bd->bi_dram[bank].start;
		}
	} else {
		gd->bd->bi_dram[bank].size = sdram_b1_size;
	}

	if (sdram_b2_size) {
		if (++bank >= CONFIG_NR_DRAM_BANKS) {
			puts("CONFIG_NR_DRAM_BANKS is not enough for SDRAM_2\n");
			return -1;
		}
		gd->bd->bi_dram[bank].start = 0x100000000UL;
		gd->bd->bi_dram[bank].size = sdram_b2_size;
	}

	return 0;
}

phys_size_t get_effective_memsize(void)
{
	int ret;
	phys_size_t sdram_size;
	phys_size_t sdram_b1_size;

	ret = board_phys_sdram_size(&sdram_size);
	if (!ret) {
		/* Bank 1 can't cross over 4GB space */
		if (sdram_size > 0x80000000)
			sdram_b1_size = 0x100000000UL - PHYS_SDRAM;
		else
			sdram_b1_size = sdram_size;

		if (rom_pointer[1]) {
			/* We will relocate u-boot to Top of dram1. Tee position has three cases:
			 * 1. At the top of dram1,  Then return the size removed optee size.
			 * 2. In the middle of dram1, return the size of dram1.
			 * 3. Not in the scope of dram1, return the size of dram1.
			 */
			if ((rom_pointer[0] + rom_pointer[1]) == (PHYS_SDRAM + sdram_b1_size))
				return ((phys_addr_t)rom_pointer[0] - PHYS_SDRAM);
		}

		return sdram_b1_size;
	} else {
		return PHYS_SDRAM_SIZE;
	}
}

void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	u32 val[2] = {};
	int ret, num_of_macs;

	ret = fuse_read(40, 5, &val[0]);
	if (ret)
		goto err;

	ret = fuse_read(40, 6, &val[1]);
	if (ret)
		goto err;

	num_of_macs = (val[1] >> 24) & 0xff;
	if (num_of_macs <= (dev_id * 3)) {
		printf("WARNING: no MAC address assigned for MAC%d\n", dev_id);
		goto err;
	}

	mac[0] = val[0] & 0xff;
	mac[1] = (val[0] >> 8) & 0xff;
	mac[2] = (val[0] >> 16) & 0xff;
	mac[3] = (val[0] >> 24) & 0xff;
	mac[4] = val[1] & 0xff;
	mac[5] = (val[1] >> 8) & 0xff;
	if (dev_id == 1)
		mac[5] = mac[5] + 3;
	if (dev_id == 2)
		mac[5] = mac[5] + 6;

	debug("%s: MAC%d: %pM\n", __func__, dev_id, mac);
	return;
err:
	memset(mac, 0, 6);
	printf("%s: fuse read err: %d\n", __func__, ret);
}

const char *get_imx_type(u32 imxtype)
{
	switch (imxtype) {
	case MXC_CPU_IMX95:
		return "95";/* iMX95 FULL */
	default:
		return "??";
	}
}

void build_info(void)
{
	u32 fw_version, sha1, res = 0, status;
	int ret;

	printf("\nBuildInfo:\n");

	ret = ele_get_fw_status(&status, &res);
	if (ret) {
		printf("  - ELE firmware status failed %d, 0x%x\n", ret, res);
	} else if ((status & 0xff) == 1) {
		ret = ele_get_fw_version(&fw_version, &sha1, &res);
		if (ret) {
			printf("  - ELE firmware version failed %d, 0x%x\n", ret, res);
		} else {
			printf("  - ELE firmware version %u.%u.%u-%x",
			       (fw_version & (0x00ff0000)) >> 16,
			       (fw_version & (0x0000fff0)) >> 4,
			       (fw_version & (0x0000000f)), sha1);
			((fw_version & (0x80000000)) >> 31) == 1 ? puts("-dirty\n") : puts("\n");
		}
	} else {
		printf("  - ELE firmware not included\n");
	}
	puts("\n");
}

int arch_misc_init(void)
{
	build_info();
	return 0;
}

#if defined(CONFIG_OF_BOARD_FIXUP) && !defined(CONFIG_SPL_BUILD)
int board_fix_fdt(void *fdt)
{
	return 0;
}
#endif

int ft_system_setup(void *blob, struct bd_info *bd)
{
	return 0;
}

#if IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)
void get_board_serial(struct tag_serialnr *serialnr)
{
	printf("UID: %08x%08x%08x%08x\n", __be32_to_cpu(gd->arch.uid[0]),
	       __be32_to_cpu(gd->arch.uid[1]), __be32_to_cpu(gd->arch.uid[2]),
	       __be32_to_cpu(gd->arch.uid[3]));

	serialnr->low = __be32_to_cpu(gd->arch.uid[1]);
	serialnr->high = __be32_to_cpu(gd->arch.uid[0]);
}
#endif

static void gpio_reset(ulong gpio_base)
{
	writel(0, gpio_base + 0x10);
	writel(0, gpio_base + 0x14);
	writel(0, gpio_base + 0x18);
	writel(0, gpio_base + 0x1c);
}

int arch_cpu_init(void)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		disable_wdog((void __iomem *)WDG3_BASE_ADDR);
		disable_wdog((void __iomem *)WDG4_BASE_ADDR);

		gpio_reset(GPIO2_BASE_ADDR);
		gpio_reset(GPIO3_BASE_ADDR);
		gpio_reset(GPIO4_BASE_ADDR);
		gpio_reset(GPIO5_BASE_ADDR);
	}

	return 0;
}

int imx9_probe_mu(void)
{
	struct udevice *dev;
	int ret;
	u32 res;
	struct ele_get_info_data info;

	ret = uclass_get_device_by_driver(UCLASS_SCMI_AGENT, DM_DRIVER_GET(scmi_mbox), &dev);
	if (ret)
		return ret;

	ret = uclass_get_device_by_name(UCLASS_CLK, "protocol@14", &dev);
	if (ret)
		return ret;

	ret = devm_scmi_of_get_channel(dev);
	if (ret)
		return ret;

	ret = uclass_get_device_by_name(UCLASS_PINCTRL, "protocol@19", &dev);
	if (ret)
		return ret;

#if defined(CONFIG_SPL_BUILD)
	ret = uclass_get_device_by_name(UCLASS_MISC, "mailbox@47530000", &dev);
#else
	ret = uclass_get_device_by_name(UCLASS_MISC, "mailbox@47550000", &dev);
#endif
	if (ret)
		return ret;

	if (gd->flags & GD_FLG_RELOC)
		return 0;

	ret = ele_get_info(&info, &res);
	if (ret)
		return ret;

	set_cpu_info(&info);

	return 0;
}

EVENT_SPY_SIMPLE(EVT_DM_POST_INIT_F, imx9_probe_mu);
EVENT_SPY_SIMPLE(EVT_DM_POST_INIT_R, imx9_probe_mu);

int timer_init(void)
{
	gd->arch.tbl = 0;
	gd->arch.tbu = 0;

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		unsigned long freq = 24000000;

		asm volatile("msr cntfrq_el0, %0" : : "r" (freq) : "memory");

		/* Clear the compare frame interrupt */
		unsigned long sctr_cmpcr_addr = SYSCNT_CMP_BASE_ADDR + 0x2c;
		unsigned long sctr_cmpcr = readl(sctr_cmpcr_addr);

		sctr_cmpcr &= ~0x1;
		writel(sctr_cmpcr, sctr_cmpcr_addr);
	}

	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	enum boot_device dev = get_boot_device();
	enum env_location env_loc = ENVL_UNKNOWN;

	if (prio)
		return env_loc;

	switch (dev) {
	case QSPI_BOOT:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH))
			env_loc = ENVL_SPI_FLASH;
		break;
	case SD1_BOOT:
	case SD2_BOOT:
	case SD3_BOOT:
	case MMC1_BOOT:
	case MMC2_BOOT:
	case MMC3_BOOT:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
			env_loc =  ENVL_MMC;
		break;
	default:
		if (IS_ENABLED(CONFIG_ENV_IS_NOWHERE))
			env_loc = ENVL_NOWHERE;
		else if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH))
			env_loc = ENVL_SPI_FLASH;
		else if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
			env_loc = ENVL_MMC;
		break;
	}

	return env_loc;
}

enum imx9_soc_voltage_mode soc_target_voltage_mode(void)
{
	u32 speed = get_cpu_speed_grade_hz();
	enum imx9_soc_voltage_mode voltage = VOLT_OVER_DRIVE;

	if (is_imx95()) {
		if (speed == 2000000000)
			voltage = VOLT_SUPER_OVER_DRIVE;
		else if (speed == 1800000000)
			voltage = VOLT_OVER_DRIVE;
		else if (speed == 1400000000)
			voltage = VOLT_NOMINAL_DRIVE;
		else /* boot not support low drive mode according to AS */
			printf("Unexpected A55 freq %u, default to OD\n", speed);
	}

	return voltage;
}

#if IS_ENABLED(CONFIG_SCMI_FIRMWARE)
enum boot_device get_boot_device(void)
{
	gd_t *pgd = gd;
	int ret;
	u16 boot_type;
	u8 boot_instance;
	enum boot_device boot_dev = 0;
	rom_passover_t *rdata;

#if IS_ENABLED(CONFIG_SPL_BUILD)
	rdata = &rom_passover_data;
#else
	rom_passover_t rom_data = {0};

	if (pgd->reloc_off == 0)
		rdata = &rom_data;
	else
		rdata = &rom_passover_data;
#endif
	if (rdata->tag == 0) {
		ret = scmi_get_rom_data(rdata);
		if (ret != 0) {
			puts("SCMI: failure at rom_boot_info\n");
			return -1;
		}
	}
	boot_type = rdata->boot_dev_type;
	boot_instance = rdata->boot_dev_inst;

	set_gd(pgd);

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
		boot_dev = boot_instance + USB_BOOT;
		if (IS_ENABLED(CONFIG_IMX95))
			boot_dev -= 3; //iMX95 usb instance start at 3
		break;
	default:
		break;
	}

	return boot_dev;
}
#endif

bool arch_check_dst_in_secure(void *start, ulong size)
{
	ulong ns_end = CFG_SYS_SDRAM_BASE + PHYS_SDRAM_SIZE;
#ifdef PHYS_SDRAM_2_SIZE
	ns_end += PHYS_SDRAM_2_SIZE;
#endif

	if ((ulong)start < CFG_SYS_SDRAM_BASE || (ulong)start + size > ns_end)
		return true;

	return false;
}

void *arch_get_container_trampoline(void)
{
	return (void *)((ulong)CFG_SYS_SDRAM_BASE + PHYS_SDRAM_SIZE - SZ_16M);
}
