// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <env.h>
#include <env_internal.h>
#include <init.h>
#include <log.h>
#include <net.h>
#include <sata.h>
#include <ahci.h>
#include <scsi.h>
#include <malloc.h>
#include <wdt.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/psu_init_gpl.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <zynqmppl.h>
#include <zynqmp_firmware.h>
#include <g_dnl.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include "../common/board.h"

#include "pm_cfg_obj.h"

#define ZYNQMP_VERSION_SIZE	7
#define EFUSE_VCU_DIS_MASK	0x100
#define EFUSE_VCU_DIS_SHIFT	8
#define EFUSE_GPU_DIS_MASK	0x20
#define EFUSE_GPU_DIS_SHIFT	5
#define IDCODE2_PL_INIT_MASK	0x200
#define IDCODE2_PL_INIT_SHIFT	9

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(FPGA) && defined(CONFIG_FPGA_ZYNQMPPL)
static xilinx_desc zynqmppl = XILINX_ZYNQMP_DESC;

enum {
	ZYNQMP_VARIANT_EG = BIT(0U),
	ZYNQMP_VARIANT_EV = BIT(1U),
	ZYNQMP_VARIANT_CG = BIT(2U),
	ZYNQMP_VARIANT_DR = BIT(3U),
};

static const struct {
	u32 id;
	u8 device;
	u8 variants;
} zynqmp_devices[] = {
	{
		.id = 0x04711093,
		.device = 2,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG,
	},
	{
		.id = 0x04710093,
		.device = 3,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG,
	},
	{
		.id = 0x04721093,
		.device = 4,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG |
			ZYNQMP_VARIANT_EV,
	},
	{
		.id = 0x04720093,
		.device = 5,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG |
			ZYNQMP_VARIANT_EV,
	},
	{
		.id = 0x04739093,
		.device = 6,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG,
	},
	{
		.id = 0x04730093,
		.device = 7,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG |
			ZYNQMP_VARIANT_EV,
	},
	{
		.id = 0x04738093,
		.device = 9,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG,
	},
	{
		.id = 0x04740093,
		.device = 11,
		.variants = ZYNQMP_VARIANT_EG,
	},
	{
		.id = 0x04750093,
		.device = 15,
		.variants = ZYNQMP_VARIANT_EG,
	},
	{
		.id = 0x04759093,
		.device = 17,
		.variants = ZYNQMP_VARIANT_EG,
	},
	{
		.id = 0x04758093,
		.device = 19,
		.variants = ZYNQMP_VARIANT_EG,
	},
	{
		.id = 0x047E1093,
		.device = 21,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E3093,
		.device = 23,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E5093,
		.device = 25,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E4093,
		.device = 27,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E0093,
		.device = 28,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E2093,
		.device = 29,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E6093,
		.device = 39,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047FD093,
		.device = 43,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047F8093,
		.device = 46,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047FF093,
		.device = 47,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047FB093,
		.device = 48,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047FE093,
		.device = 49,
		.variants = ZYNQMP_VARIANT_DR,
	},
};

static char *zynqmp_get_silicon_idcode_name(void)
{
	u32 i;
	u32 idcode, idcode2;
	char name[ZYNQMP_VERSION_SIZE];
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	ret = xilinx_pm_request(PM_GET_CHIPID, 0, 0, 0, 0, ret_payload);
	if (ret) {
		debug("%s: Getting chipid failed\n", __func__);
		return "unknown";
	}

	/*
	 * Firmware returns:
	 * payload[0][31:0]  = status of the operation
	 * payload[1]] = IDCODE
	 * payload[2][19:0]  = Version
	 * payload[2][28:20] = EXTENDED_IDCODE
	 * payload[2][29] = PL_INIT
	 */

	idcode  = ret_payload[1];
	idcode2 = ret_payload[2] >> ZYNQMP_CSU_VERSION_EMPTY_SHIFT;
	debug("%s, IDCODE: 0x%0x, IDCODE2: 0x%0x\r\n", __func__, idcode,
	      idcode2);

	for (i = 0; i < ARRAY_SIZE(zynqmp_devices); i++) {
		if (zynqmp_devices[i].id == (idcode & 0x0FFFFFFF))
			break;
	}

	if (i >= ARRAY_SIZE(zynqmp_devices))
		return "unknown";

	/* Add device prefix to the name */
	ret = snprintf(name, ZYNQMP_VERSION_SIZE, "zu%d",
		       zynqmp_devices[i].device);
	if (ret < 0)
		return "unknown";

	if (zynqmp_devices[i].variants & ZYNQMP_VARIANT_EV) {
		/* Devices with EV variant might be EG/CG/EV family */
		if (idcode2 & IDCODE2_PL_INIT_MASK) {
			u32 family = ((idcode2 & EFUSE_VCU_DIS_MASK) >>
				      EFUSE_VCU_DIS_SHIFT) << 1 |
				     ((idcode2 & EFUSE_GPU_DIS_MASK) >>
				      EFUSE_GPU_DIS_SHIFT);

			/*
			 * Get family name based on extended idcode values as
			 * determined on UG1087, EXTENDED_IDCODE register
			 * description
			 */
			switch (family) {
			case 0x00:
				strncat(name, "ev", 2);
				break;
			case 0x10:
				strncat(name, "eg", 2);
				break;
			case 0x11:
				strncat(name, "cg", 2);
				break;
			default:
				/* Do not append family name*/
				break;
			}
		} else {
			/*
			 * When PL powered down the VCU Disable efuse cannot be
			 * read. So, ignore the bit and just findout if it is CG
			 * or EG/EV variant.
			 */
			strncat(name, (idcode2 & EFUSE_GPU_DIS_MASK) ? "cg" :
				"e", 2);
		}
	} else if (zynqmp_devices[i].variants & ZYNQMP_VARIANT_CG) {
		/* Devices with CG variant might be EG or CG family */
		strncat(name, (idcode2 & EFUSE_GPU_DIS_MASK) ? "cg" : "eg", 2);
	} else if (zynqmp_devices[i].variants & ZYNQMP_VARIANT_EG) {
		strncat(name, "eg", 2);
	} else if (zynqmp_devices[i].variants & ZYNQMP_VARIANT_DR) {
		strncat(name, "dr", 2);
	} else {
		debug("Variant not identified\n");
	}

	return strdup(name);
}
#endif

int board_early_init_f(void)
{
#if defined(CONFIG_ZYNQMP_PSU_INIT_ENABLED)
	int ret;

	ret = psu_init();
	if (ret)
		return ret;

	/* Delay is required for clocks to be propagated */
	udelay(1000000);
#endif

#ifdef CONFIG_DEBUG_UART
	/* Uart debug for sure */
	debug_uart_init();
	puts("Debug uart enabled\n"); /* or printch() */
#endif

	return 0;
}

static int multi_boot(void)
{
	u32 multiboot;

	multiboot = readl(&csu_base->multi_boot);

	printf("Multiboot:\t%d\n", multiboot);

	return 0;
}

#define PS_SYSMON_ANALOG_BUS_VAL	0x3210
#define PS_SYSMON_ANALOG_BUS_REG	0xFFA50914

int board_init(void)
{
#if defined(CONFIG_ZYNQMP_FIRMWARE)
	struct udevice *dev;

	uclass_get_device_by_name(UCLASS_FIRMWARE, "zynqmp-power", &dev);
	if (!dev)
		panic("PMU Firmware device not found - Enable it");
#endif

#if defined(CONFIG_SPL_BUILD)
	/* Check *at build time* if the filename is an non-empty string */
	if (sizeof(CONFIG_ZYNQMP_SPL_PM_CFG_OBJ_FILE) > 1)
		zynqmp_pmufw_load_config_object(zynqmp_pm_cfg_obj,
						zynqmp_pm_cfg_obj_size);
	printf("Silicon version:\t%d\n", zynqmp_get_silicon_version());
#else
	if (CONFIG_IS_ENABLED(DM_I2C) && CONFIG_IS_ENABLED(I2C_EEPROM))
		xilinx_read_eeprom();
#endif

	printf("EL Level:\tEL%d\n", current_el());

	/* Bug in ROM sets wrong value in this register */
	writel(PS_SYSMON_ANALOG_BUS_VAL, PS_SYSMON_ANALOG_BUS_REG);

#if CONFIG_IS_ENABLED(FPGA) && defined(CONFIG_FPGA_ZYNQMPPL)
	zynqmppl.name = zynqmp_get_silicon_idcode_name();
	printf("Chip ID:\t%s\n", zynqmppl.name);
	fpga_init();
	fpga_add(fpga_xilinx, &zynqmppl);
#endif

	if (current_el() == 3)
		multi_boot();

	return 0;
}

int board_early_init_r(void)
{
	u32 val;

	if (current_el() != 3)
		return 0;

	val = readl(&crlapb_base->timestamp_ref_ctrl);
	val &= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;

	if (!val) {
		val = readl(&crlapb_base->timestamp_ref_ctrl);
		val |= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;
		writel(val, &crlapb_base->timestamp_ref_ctrl);

		/* Program freq register in System counter */
		writel(zynqmp_get_system_timer_freq(),
		       &iou_scntr_secure->base_frequency_id_register);
		/* And enable system counter */
		writel(ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN,
		       &iou_scntr_secure->counter_control_register);
	}
	return 0;
}

unsigned long do_go_exec(ulong (*entry)(int, char * const []), int argc,
			 char *const argv[])
{
	int ret = 0;

	if (current_el() > 1) {
		smp_kick_all_cpus();
		dcache_disable();
		armv8_switch_to_el1(0x0, 0, 0, 0, (unsigned long)entry,
				    ES_TO_AARCH64);
	} else {
		printf("FAIL: current EL is not above EL1\n");
		ret = EINVAL;
	}
	return ret;
}

#if !defined(CONFIG_SYS_SDRAM_BASE) && !defined(CONFIG_SYS_SDRAM_SIZE)
int dram_init_banksize(void)
{
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		return ret;

	mem_map_fill();

	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}
#else
int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = get_effective_memsize();

	mem_map_fill();

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);

	return 0;
}
#endif

void reset_cpu(void)
{
}

static u8 __maybe_unused zynqmp_get_bootmode(void)
{
	u8 bootmode;
	u32 reg = 0;
	int ret;

	ret = zynqmp_mmio_read((ulong)&crlapb_base->boot_mode, &reg);
	if (ret)
		return -EINVAL;

	if (reg >> BOOT_MODE_ALT_SHIFT)
		reg >>= BOOT_MODE_ALT_SHIFT;

	bootmode = reg & BOOT_MODES_MASK;

	return bootmode;
}

#if defined(CONFIG_BOARD_LATE_INIT)
static const struct {
	u32 bit;
	const char *name;
} reset_reasons[] = {
	{ RESET_REASON_DEBUG_SYS, "DEBUG" },
	{ RESET_REASON_SOFT, "SOFT" },
	{ RESET_REASON_SRST, "SRST" },
	{ RESET_REASON_PSONLY, "PS-ONLY" },
	{ RESET_REASON_PMU, "PMU" },
	{ RESET_REASON_INTERNAL, "INTERNAL" },
	{ RESET_REASON_EXTERNAL, "EXTERNAL" },
	{}
};

static int reset_reason(void)
{
	u32 reg;
	int i, ret;
	const char *reason = NULL;

	ret = zynqmp_mmio_read((ulong)&crlapb_base->reset_reason, &reg);
	if (ret)
		return -EINVAL;

	puts("Reset reason:\t");

	for (i = 0; i < ARRAY_SIZE(reset_reasons); i++) {
		if (reg & reset_reasons[i].bit) {
			reason = reset_reasons[i].name;
			printf("%s ", reset_reasons[i].name);
			break;
		}
	}

	puts("\n");

	env_set("reset_reason", reason);

	return 0;
}

static int set_fdtfile(void)
{
	char *compatible, *fdtfile;
	const char *suffix = ".dtb";
	const char *vendor = "xilinx/";
	int fdt_compat_len;

	if (env_get("fdtfile"))
		return 0;

	compatible = (char *)fdt_getprop(gd->fdt_blob, 0, "compatible",
					 &fdt_compat_len);
	if (compatible && fdt_compat_len) {
		char *name;

		debug("Compatible: %s\n", compatible);

		name = strchr(compatible, ',');
		if (!name)
			return -EINVAL;

		name++;

		fdtfile = calloc(1, strlen(vendor) + strlen(name) +
				 strlen(suffix) + 1);
		if (!fdtfile)
			return -ENOMEM;

		sprintf(fdtfile, "%s%s%s", vendor, name, suffix);

		env_set("fdtfile", fdtfile);
		free(fdtfile);
	}

	return 0;
}

int board_late_init(void)
{
	u8 bootmode;
	struct udevice *dev;
	int bootseq = -1;
	int bootseq_len = 0;
	int env_targets_len = 0;
	const char *mode;
	char *new_targets;
	char *env_targets;
	int ret;

#if defined(CONFIG_USB_ETHER) && !defined(CONFIG_USB_GADGET_DOWNLOAD)
	usb_ether_init();
#endif

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	if (!CONFIG_IS_ENABLED(ENV_VARS_UBOOT_RUNTIME_CONFIG))
		return 0;

	ret = set_fdtfile();
	if (ret)
		return ret;

	bootmode = zynqmp_get_bootmode();

	puts("Bootmode: ");
	switch (bootmode) {
	case USB_MODE:
		puts("USB_MODE\n");
		mode = "usb";
		env_set("modeboot", "usb_dfu_spl");
		break;
	case JTAG_MODE:
		puts("JTAG_MODE\n");
		mode = "jtag pxe dhcp";
		env_set("modeboot", "jtagboot");
		break;
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		mode = "qspi0";
		puts("QSPI_MODE\n");
		env_set("modeboot", "qspiboot");
		break;
	case EMMC_MODE:
		puts("EMMC_MODE\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@ff160000", &dev) &&
		    uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@ff160000", &dev)) {
			puts("Boot from EMMC but without SD0 enabled!\n");
			return -1;
		}
		debug("mmc0 device found at %p, seq %d\n", dev, dev_seq(dev));

		mode = "mmc";
		bootseq = dev_seq(dev);
		break;
	case SD_MODE:
		puts("SD_MODE\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@ff160000", &dev) &&
		    uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@ff160000", &dev)) {
			puts("Boot from SD0 but without SD0 enabled!\n");
			return -1;
		}
		debug("mmc0 device found at %p, seq %d\n", dev, dev_seq(dev));

		mode = "mmc";
		bootseq = dev_seq(dev);
		env_set("modeboot", "sdboot");
		break;
	case SD1_LSHFT_MODE:
		puts("LVL_SHFT_");
		/* fall through */
	case SD_MODE1:
		puts("SD_MODE1\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@ff170000", &dev) &&
		    uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@ff170000", &dev)) {
			puts("Boot from SD1 but without SD1 enabled!\n");
			return -1;
		}
		debug("mmc1 device found at %p, seq %d\n", dev, dev_seq(dev));

		mode = "mmc";
		bootseq = dev_seq(dev);
		env_set("modeboot", "sdboot");
		break;
	case NAND_MODE:
		puts("NAND_MODE\n");
		mode = "nand0";
		env_set("modeboot", "nandboot");
		break;
	default:
		mode = "";
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	if (bootseq >= 0) {
		bootseq_len = snprintf(NULL, 0, "%i", bootseq);
		debug("Bootseq len: %x\n", bootseq_len);
		env_set_hex("bootseq", bootseq);
	}

	/*
	 * One terminating char + one byte for space between mode
	 * and default boot_targets
	 */
	env_targets = env_get("boot_targets");
	if (env_targets)
		env_targets_len = strlen(env_targets);

	new_targets = calloc(1, strlen(mode) + env_targets_len + 2 +
			     bootseq_len);
	if (!new_targets)
		return -ENOMEM;

	if (bootseq >= 0)
		sprintf(new_targets, "%s%x %s", mode, bootseq,
			env_targets ? env_targets : "");
	else
		sprintf(new_targets, "%s %s", mode,
			env_targets ? env_targets : "");

	env_set("boot_targets", new_targets);

	reset_reason();

	return board_late_init_xilinx();
}
#endif

int checkboard(void)
{
	puts("Board: Xilinx ZynqMP\n");
	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 bootmode = zynqmp_get_bootmode();

	if (prio)
		return ENVL_UNKNOWN;

	switch (bootmode) {
	case EMMC_MODE:
	case SD_MODE:
	case SD1_LSHFT_MODE:
	case SD_MODE1:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_FAT))
			return ENVL_FAT;
		if (IS_ENABLED(CONFIG_ENV_IS_IN_EXT4))
			return ENVL_EXT4;
		return ENVL_UNKNOWN;
	case NAND_MODE:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_NAND))
			return ENVL_NAND;
		if (IS_ENABLED(CONFIG_ENV_IS_IN_UBI))
			return ENVL_UBI;
		return ENVL_UNKNOWN;
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH))
			return ENVL_SPI_FLASH;
		return ENVL_UNKNOWN;
	case JTAG_MODE:
	default:
		return ENVL_NOWHERE;
	}
}
