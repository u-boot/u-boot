/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <uboot_aes.h>
#include <sata.h>
#include <ahci.h>
#include <scsi.h>
#include <malloc.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/psu_init_gpl.h>
#include <asm/io.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <zynqmppl.h>
#include <i2c.h>
#include <g_dnl.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FPGA) && defined(CONFIG_FPGA_ZYNQMPPL) && \
    !defined(CONFIG_SPL_BUILD)
static xilinx_desc zynqmppl = XILINX_ZYNQMP_DESC;

static const struct {
	u32 id;
	u32 ver;
	char *name;
	bool evexists;
} zynqmp_devices[] = {
	{
		.id = 0x10,
		.name = "3eg",
	},
	{
		.id = 0x10,
		.ver = 0x2c,
		.name = "3cg",
	},
	{
		.id = 0x11,
		.name = "2eg",
	},
	{
		.id = 0x11,
		.ver = 0x2c,
		.name = "2cg",
	},
	{
		.id = 0x20,
		.name = "5ev",
		.evexists = 1,
	},
	{
		.id = 0x20,
		.ver = 0x100,
		.name = "5eg",
		.evexists = 1,
	},
	{
		.id = 0x20,
		.ver = 0x12c,
		.name = "5cg",
	},
	{
		.id = 0x21,
		.name = "4ev",
		.evexists = 1,
	},
	{
		.id = 0x21,
		.ver = 0x100,
		.name = "4eg",
		.evexists = 1,
	},
	{
		.id = 0x21,
		.ver = 0x12c,
		.name = "4cg",
	},
	{
		.id = 0x30,
		.name = "7ev",
		.evexists = 1,
	},
	{
		.id = 0x30,
		.ver = 0x100,
		.name = "7eg",
		.evexists = 1,
	},
	{
		.id = 0x30,
		.ver = 0x12c,
		.name = "7cg",
	},
	{
		.id = 0x38,
		.name = "9eg",
	},
	{
		.id = 0x38,
		.ver = 0x2c,
		.name = "9cg",
	},
	{
		.id = 0x39,
		.name = "6eg",
	},
	{
		.id = 0x39,
		.ver = 0x2c,
		.name = "6cg",
	},
	{
		.id = 0x40,
		.name = "11eg",
	},
	{ /* For testing purpose only */
		.id = 0x50,
		.ver = 0x2c,
		.name = "15cg",
	},
	{
		.id = 0x50,
		.name = "15eg",
	},
	{
		.id = 0x58,
		.name = "19eg",
	},
	{
		.id = 0x59,
		.name = "17eg",
	},
	{
		.id = 0x61,
		.name = "21dr",
	},
	{
		.id = 0x63,
		.name = "23dr",
	},
	{
		.id = 0x65,
		.name = "25dr",
	},
	{
		.id = 0x64,
		.name = "27dr",
	},
	{
		.id = 0x60,
		.name = "28dr",
	},
	{
		.id = 0x62,
		.name = "29dr",
	},
};
#endif

int chip_id(unsigned char id)
{
	struct pt_regs regs;
	int val = -EINVAL;

	if (current_el() != 3) {
		regs.regs[0] = ZYNQMP_SIP_SVC_CSU_DMA_CHIPID;
		regs.regs[1] = 0;
		regs.regs[2] = 0;
		regs.regs[3] = 0;

		smc_call(&regs);

		/*
		 * SMC returns:
		 * regs[0][31:0]  = status of the operation
		 * regs[0][63:32] = CSU.IDCODE register
		 * regs[1][31:0]  = CSU.version register
		 * regs[1][63:32] = CSU.IDCODE2 register
		 */
		switch (id) {
		case IDCODE:
			regs.regs[0] = upper_32_bits(regs.regs[0]);
			regs.regs[0] &= ZYNQMP_CSU_IDCODE_DEVICE_CODE_MASK |
					ZYNQMP_CSU_IDCODE_SVD_MASK;
			regs.regs[0] >>= ZYNQMP_CSU_IDCODE_SVD_SHIFT;
			val = regs.regs[0];
			break;
		case VERSION:
			regs.regs[1] = lower_32_bits(regs.regs[1]);
			regs.regs[1] &= ZYNQMP_CSU_SILICON_VER_MASK;
			val = regs.regs[1];
			break;
		case IDCODE2:
			regs.regs[1] = lower_32_bits(regs.regs[1]);
			regs.regs[1] >>= ZYNQMP_CSU_VERSION_EMPTY_SHIFT;
			val = regs.regs[1];
			break;
		default:
			printf("%s, Invalid Req:0x%x\n", __func__, id);
		}
	} else {
		switch (id) {
		case IDCODE:
			val = readl(ZYNQMP_CSU_IDCODE_ADDR);
			val &= ZYNQMP_CSU_IDCODE_DEVICE_CODE_MASK |
			       ZYNQMP_CSU_IDCODE_SVD_MASK;
			val >>= ZYNQMP_CSU_IDCODE_SVD_SHIFT;
			break;
		case VERSION:
			val = readl(ZYNQMP_CSU_VER_ADDR);
			val &= ZYNQMP_CSU_SILICON_VER_MASK;
			break;
		default:
			printf("%s, Invalid Req:0x%x\n", __func__, id);
		}
	}

	return val;
}

#define ZYNQMP_VERSION_SIZE		9
#define ZYNQMP_PL_STATUS_BIT		9
#define ZYNQMP_PL_STATUS_MASK		BIT(ZYNQMP_PL_STATUS_BIT)
#define ZYNQMP_CSU_VERSION_MASK		~(ZYNQMP_PL_STATUS_MASK)

#if defined(CONFIG_FPGA) && defined(CONFIG_FPGA_ZYNQMPPL) && \
	!defined(CONFIG_SPL_BUILD)
static char *zynqmp_get_silicon_idcode_name(void)
{
	u32 i, id, ver;
	char *buf;
	static char name[ZYNQMP_VERSION_SIZE];

	id = chip_id(IDCODE);
	ver = chip_id(IDCODE2);

	for (i = 0; i < ARRAY_SIZE(zynqmp_devices); i++) {
		if ((zynqmp_devices[i].id == id) &&
		    (zynqmp_devices[i].ver == (ver &
		    ZYNQMP_CSU_VERSION_MASK))) {
			strncat(name, "zu", 2);
			strncat(name, zynqmp_devices[i].name,
				ZYNQMP_VERSION_SIZE - 3);
			break;
		}
	}

	if (i >= ARRAY_SIZE(zynqmp_devices))
		return "unknown";

	if (!zynqmp_devices[i].evexists)
		return name;

	if (ver & ZYNQMP_PL_STATUS_MASK)
		return name;

	if (strstr(name, "eg") || strstr(name, "ev")) {
		buf = strstr(name, "e");
		*buf = '\0';
	}

	return name;
}
#endif

int board_early_init_f(void)
{
	int ret = 0;
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_CLK_ZYNQMP)
	zynqmp_pmufw_version();
#endif

#if defined(CONFIG_ZYNQMP_PSU_INIT_ENABLED)
	ret = psu_init();
#endif

	return ret;
}

int board_init(void)
{
	printf("EL Level:\tEL%d\n", current_el());

#if defined(CONFIG_FPGA) && defined(CONFIG_FPGA_ZYNQMPPL) && \
    !defined(CONFIG_SPL_BUILD) || (defined(CONFIG_SPL_FPGA_SUPPORT) && \
    defined(CONFIG_SPL_BUILD))
	if (current_el() != 3) {
		zynqmppl.name = zynqmp_get_silicon_idcode_name();
		printf("Chip ID:\t%s\n", zynqmppl.name);
		fpga_init();
		fpga_add(fpga_xilinx, &zynqmppl);
	}
#endif

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

int zynq_board_read_rom_ethaddr(unsigned char *ethaddr)
{
#if defined(CONFIG_ZYNQ_GEM_EEPROM_ADDR) && \
    defined(CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET) && \
    defined(CONFIG_ZYNQ_EEPROM_BUS)
	i2c_set_bus_num(CONFIG_ZYNQ_EEPROM_BUS);

	if (eeprom_read(CONFIG_ZYNQ_GEM_EEPROM_ADDR,
			CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET,
			ethaddr, 6))
		printf("I2C EEPROM MAC address read failed\n");
#endif

	return 0;
}

unsigned long do_go_exec(ulong (*entry)(int, char * const []), int argc,
			 char * const argv[])
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
	return fdtdec_setup_memory_banksize();
}

int dram_init(void)
{
	if (fdtdec_setup_memory_size() != 0)
		return -EINVAL;

	return 0;
}
#else
int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}
#endif

void reset_cpu(ulong addr)
{
}

int board_late_init(void)
{
	u32 ver, reg = 0;
	u8 bootmode;
	const char *mode;
	char *new_targets;
	char *env_targets;
	int ret;

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	ver = zynqmp_get_silicon_version();

	switch (ver) {
	case ZYNQMP_CSU_VERSION_VELOCE:
		env_set("setup", "setenv baudrate 4800 && env_set bootcmd run veloce");
	case ZYNQMP_CSU_VERSION_EP108:
	case ZYNQMP_CSU_VERSION_SILICON:
	case ZYNQMP_CSU_VERSION_QEMU:
		env_set("setup", "setenv partid auto");
		break;
	default:
		env_set("setup", "setenv partid 0");
	}

	ret = zynqmp_mmio_read((ulong)&crlapb_base->boot_mode, &reg);
	if (ret)
		return -EINVAL;

	if (reg >> BOOT_MODE_ALT_SHIFT)
		reg >>= BOOT_MODE_ALT_SHIFT;

	bootmode = reg & BOOT_MODES_MASK;

	puts("Bootmode: ");
	switch (bootmode) {
	case USB_MODE:
		puts("USB_MODE\n");
		mode = "usb";
		env_set("modeboot", "usb_dfu_spl");
		break;
	case JTAG_MODE:
		puts("JTAG_MODE\n");
		mode = "pxe dhcp";
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
		mode = "mmc0";
		env_set("modeboot", "emmcboot");
		break;
	case SD_MODE:
		puts("SD_MODE\n");
		mode = "mmc0";
		env_set("modeboot", "sdboot");
		break;
	case SD1_LSHFT_MODE:
		puts("LVL_SHFT_");
		/* fall through */
	case SD_MODE1:
		puts("SD_MODE1\n");
#if defined(CONFIG_ZYNQ_SDHCI0) && defined(CONFIG_ZYNQ_SDHCI1)
		mode = "mmc1";
		env_set("sdbootdev", "1");
#else
		mode = "mmc0";
#endif
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

	/*
	 * One terminating char + one byte for space between mode
	 * and default boot_targets
	 */
	env_targets = env_get("boot_targets");
	if (env_targets) {
		new_targets = calloc(1, strlen(mode) +
				     strlen(env_targets) + 2);
		sprintf(new_targets, "%s %s", mode, env_targets);
	} else {
		new_targets = calloc(1, strlen(mode) + 2);
		sprintf(new_targets, "%s", mode);
	}

	env_set("boot_targets", new_targets);

	return 0;
}

int checkboard(void)
{
	puts("Board: Xilinx ZynqMP\n");
	return 0;
}

#if defined(CONFIG_AES)

#define KEY_LEN				64
#define IV_LEN				24
#define ZYNQMP_SIP_SVC_PM_SECURE_LOAD	0xC2000019
#define ZYNQMP_PM_SECURE_AES		0x1

int aes_decrypt_hw(u8 *key_ptr, u8 *src_ptr, u8 *dst_ptr, u32 len)
{
	int ret;
	u32 src_lo, src_hi, wlen;
	u32 ret_payload[PAYLOAD_ARG_CNT];

	if ((ulong)src_ptr != ALIGN((ulong)src_ptr,
				    CONFIG_SYS_CACHELINE_SIZE)) {
		debug("FAIL: Source address not aligned:%p\n", src_ptr);
		return -EINVAL;
	}

	src_lo = (u32)(ulong)src_ptr;
	src_hi = upper_32_bits((ulong)src_ptr);
	wlen = DIV_ROUND_UP(len, 4);

	memcpy(src_ptr + len, key_ptr, KEY_LEN + IV_LEN);
	len = ROUND(len + KEY_LEN + IV_LEN, CONFIG_SYS_CACHELINE_SIZE);
	flush_dcache_range((ulong)src_ptr, (ulong)(src_ptr + len));

	ret = invoke_smc(ZYNQMP_SIP_SVC_PM_SECURE_LOAD, src_lo, src_hi, wlen,
			 ZYNQMP_PM_SECURE_AES, ret_payload);
	if (ret)
		printf("Fail: %s: %d\n", __func__, ret);

	return ret;
}
#endif
