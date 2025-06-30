// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <config.h>
#include <cpu_func.h>
#include <log.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/bsec.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <linux/bitfield.h>
#include <malloc.h>

/* RCC register */
#define RCC_TZCR		(STM32_RCC_BASE + 0x00)
#define RCC_BDCR		(STM32_RCC_BASE + 0x400)
#define RCC_DBGCFGR		(STM32_RCC_BASE + 0x468)
#define RCC_MP_APB5ENSETR	(STM32_RCC_BASE + 0x740)
#define RCC_MP_AHB6ENSETR	(STM32_RCC_BASE + 0x780)

#define RCC_BDCR_VSWRST		BIT(31)
#define RCC_BDCR_RTCSRC		GENMASK(17, 16)

#define RCC_DBGCFGR_DBGCKEN	BIT(8)

/* DBGMCU register */
#define DBGMCU_APB4FZ1		(STM32_DBGMCU_BASE + 0x2c)
#define DBGMCU_APB4FZ1_IWDG2	BIT(2)

/* Security register */
#define ETZPC_TZMA1_SIZE	(STM32_ETZPC_BASE + 0x04)
#define ETZPC_DECPROT0		(STM32_ETZPC_BASE + 0x10)

#define TZC_ACTION		(STM32_TZC_BASE + 0x004)
#define TZC_GATE_KEEPER		(STM32_TZC_BASE + 0x008)
#define TZC_REGION_BASE(n)	(STM32_TZC_BASE + 0x100 + (0x20 * (n)))
#define TZC_REGION_TOP(n)	(STM32_TZC_BASE + 0x108 + (0x20 * (n)))
#define TZC_REGION_ATTRIBUTE(n)	(STM32_TZC_BASE + 0x110 + (0x20 * (n)))
#define TZC_REGION_ID_ACCESS(n)	(STM32_TZC_BASE + 0x114 + (0x20 * (n)))

#define TAMP_CR1		(STM32_TAMP_BASE + 0x00)

#define PWR_CR1			(STM32_PWR_BASE + 0x00)
#define PWR_CR1_DBP		BIT(8)

/* boot interface from Bootrom
 * - boot instance = bit 31:16
 * - boot device = bit 15:0
 */
#define BOOTROM_MODE_MASK	GENMASK(15, 0)
#define BOOTROM_MODE_SHIFT	0
#define BOOTROM_INSTANCE_MASK	GENMASK(31, 16)
#define BOOTROM_INSTANCE_SHIFT	16

/* SYSCFG register */
#define SYSCFG_IDC_OFFSET	0x380
#define SYSCFG_IDC_DEV_ID_MASK	GENMASK(11, 0)
#define SYSCFG_IDC_DEV_ID_SHIFT	0
#define SYSCFG_IDC_REV_ID_MASK	GENMASK(31, 16)
#define SYSCFG_IDC_REV_ID_SHIFT	16

/* Device Part Number (RPN) = OTP_DATA1 lower 11 bits */
#define RPN_SHIFT	0
#define RPN_MASK	GENMASK(11, 0)

static void security_init(void)
{
	/* Disable the backup domain write protection */
	/* the protection is enable at each reset by hardware */
	/* And must be disable by software */
	setbits_le32(PWR_CR1, PWR_CR1_DBP);

	while (!(readl(PWR_CR1) & PWR_CR1_DBP))
		;

	/* If RTC clock isn't enable so this is a cold boot then we need
	 * to reset the backup domain
	 */
	if (!(readl(RCC_BDCR) & RCC_BDCR_RTCSRC)) {
		setbits_le32(RCC_BDCR, RCC_BDCR_VSWRST);
		while (!(readl(RCC_BDCR) & RCC_BDCR_VSWRST))
			;
		clrbits_le32(RCC_BDCR, RCC_BDCR_VSWRST);
	}

	/* allow non secure access in Write/Read for all peripheral */
	writel(0, ETZPC_DECPROT0);

	/* Open SYSRAM for no secure access */
	writel(0x0, ETZPC_TZMA1_SIZE);

	/* enable MCE clock */
	writel(BIT(1), RCC_MP_AHB6ENSETR);

	/* enable TZC clock */
	writel(BIT(11), RCC_MP_APB5ENSETR);

	/* Disable Filter 0 */
	writel(0, TZC_GATE_KEEPER);

	/* Region 0 set to no access by default */
	/* bit 0 / 16 => nsaid0 read/write Enable
	 * bit 1 / 17 => nsaid1 read/write Enable
	 * ...
	 * bit 15 / 31 => nsaid15 read/write Enable
	 */
	writel(0xFFFFFFFF, TZC_REGION_ID_ACCESS(0));

	/* bit 30 / 31 => Secure Global Enable : write/read */
	writel(BIT(0) | BIT(30) | BIT(31), TZC_REGION_ATTRIBUTE(0));

	writel(0xFFFFFFFF, TZC_REGION_ID_ACCESS(1));
	writel(0xC0000000, TZC_REGION_BASE(1));
	writel(0xDDFFFFFF, TZC_REGION_TOP(1));
	writel(BIT(0) | BIT(30) | BIT(31), TZC_REGION_ATTRIBUTE(1));

	writel(0x00000000, TZC_REGION_ID_ACCESS(2));
	writel(0xDE000000, TZC_REGION_BASE(2));
	writel(0xDFFFFFFF, TZC_REGION_TOP(2));
	writel(BIT(0) | BIT(30) | BIT(31), TZC_REGION_ATTRIBUTE(2));

	writel(0xFFFFFFFF, TZC_REGION_ID_ACCESS(3));
	writel(0x00000000, TZC_REGION_BASE(3));
	writel(0xBFFFFFFF, TZC_REGION_TOP(3));
	writel(BIT(0) | BIT(30) | BIT(31), TZC_REGION_ATTRIBUTE(3));

	/* Set Action */
	writel(BIT(0), TZC_ACTION);

	/* Enable Filter 0 */
	writel(BIT(0), TZC_GATE_KEEPER);

	/* RCC trust zone deactivated */
	writel(0x0, RCC_TZCR);

	/* TAMP: deactivate the internal tamper
	 * Bit 23 ITAMP8E: monotonic counter overflow
	 * Bit 20 ITAMP5E: RTC calendar overflow
	 * Bit 19 ITAMP4E: HSE monitoring
	 * Bit 18 ITAMP3E: LSE monitoring
	 * Bit 16 ITAMP1E: RTC power domain supply monitoring
	 */
	writel(0x0, TAMP_CR1);
}

/*
 * Debug init
 */
void dbgmcu_init(void)
{
	/*
	 * Freeze IWDG2 if Cortex-A7 is in debug mode
	 * done in TF-A for TRUSTED boot and
	 * DBGMCU access is controlled by BSEC_DENABLE.DBGSWENABLE
	 */
	if (bsec_dbgswenable()) {
		setbits_le32(RCC_DBGCFGR, RCC_DBGCFGR_DBGCKEN);
		setbits_le32(DBGMCU_APB4FZ1, DBGMCU_APB4FZ1_IWDG2);
	}
}

void spl_board_init(void)
{
	struct udevice *dev;
	u8 *tlb;
	int ret;

	dbgmcu_init();

	/* force probe of BSEC driver to shadow the upper OTP */
	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(stm32mp_bsec), &dev);
	if (ret)
		log_warning("BSEC probe failed: %d\n", ret);

	/* Enable Dcache here, now that DRAM is available */
	if (IS_ENABLED(CONFIG_XPL_BUILD) && IS_ENABLED(CONFIG_STM32MP13X)) {
		tlb = memalign(0x4000, PGTABLE_SIZE);
		if (!tlb)
			return;

		gd->arch.tlb_size = PGTABLE_SIZE;
		gd->arch.tlb_addr = (unsigned long)tlb;
		dcache_enable();
	}
}

/* get bootmode from ROM code boot context: saved in TAMP register */
static void update_bootmode(void)
{
	u32 boot_mode;
	u32 bootrom_itf = readl(get_stm32mp_rom_api_table());
	u32 bootrom_device, bootrom_instance;

	/* enable TAMP clock = RTCAPBEN */
	writel(BIT(8), RCC_MP_APB5ENSETR);

	/* read bootrom context */
	bootrom_device =
		(bootrom_itf & BOOTROM_MODE_MASK) >> BOOTROM_MODE_SHIFT;
	bootrom_instance =
		(bootrom_itf & BOOTROM_INSTANCE_MASK) >> BOOTROM_INSTANCE_SHIFT;
	boot_mode =
		((bootrom_device << BOOT_TYPE_SHIFT) & BOOT_TYPE_MASK) |
		((bootrom_instance << BOOT_INSTANCE_SHIFT) &
		 BOOT_INSTANCE_MASK);

	/* save the boot mode in TAMP backup register */
	clrsetbits_le32(TAMP_BOOT_CONTEXT,
			TAMP_BOOT_MODE_MASK,
			boot_mode << TAMP_BOOT_MODE_SHIFT);
}

/* weak function: STM32MP15x mach init for boot without TFA */
void stm32mp_cpu_init(void)
{
	if (IS_ENABLED(CONFIG_XPL_BUILD)) {
		security_init();
		update_bootmode();
	}
}

static u32 read_idc(void)
{
	void *syscfg = syscon_get_first_range(STM32MP_SYSCON_SYSCFG);

	return readl(syscfg + SYSCFG_IDC_OFFSET);
}

u32 get_cpu_dev(void)
{
	return (read_idc() & SYSCFG_IDC_DEV_ID_MASK) >> SYSCFG_IDC_DEV_ID_SHIFT;
}

u32 get_cpu_rev(void)
{
	return (read_idc() & SYSCFG_IDC_REV_ID_MASK) >> SYSCFG_IDC_REV_ID_SHIFT;
}

/* Get Device Part Number (RPN) from OTP */
static u32 get_cpu_rpn(void)
{
	return get_otp(BSEC_OTP_RPN, RPN_SHIFT, RPN_MASK);
}

u32 get_cpu_type(void)
{
	return (get_cpu_dev() << 16) | get_cpu_rpn();
}

int get_eth_nb(void)
{
	int nb_eth = 2;

	switch (get_cpu_type()) {
	case CPU_STM32MP131Dxx:
		fallthrough;
	case CPU_STM32MP131Cxx:
		fallthrough;
	case CPU_STM32MP131Axx:
		nb_eth = 1;
		break;
	default:
		nb_eth = 2;
		break;
	}

	return nb_eth;
}

void get_soc_name(char name[SOC_NAME_SIZE])
{
	char *cpu_s, *cpu_r;

	/* MPUs Part Numbers */
	switch (get_cpu_type()) {
	case CPU_STM32MP135Fxx:
		cpu_s = "135F";
		break;
	case CPU_STM32MP135Dxx:
		cpu_s = "135D";
		break;
	case CPU_STM32MP135Cxx:
		cpu_s = "135C";
		break;
	case CPU_STM32MP135Axx:
		cpu_s = "135A";
		break;
	case CPU_STM32MP133Fxx:
		cpu_s = "133F";
		break;
	case CPU_STM32MP133Dxx:
		cpu_s = "133D";
		break;
	case CPU_STM32MP133Cxx:
		cpu_s = "133C";
		break;
	case CPU_STM32MP133Axx:
		cpu_s = "133A";
		break;
	case CPU_STM32MP131Fxx:
		cpu_s = "131F";
		break;
	case CPU_STM32MP131Dxx:
		cpu_s = "131D";
		break;
	case CPU_STM32MP131Cxx:
		cpu_s = "131C";
		break;
	case CPU_STM32MP131Axx:
		cpu_s = "131A";
		break;
	default:
		cpu_s = "????";
		break;
	}

	/* REVISION */
	switch (get_cpu_rev()) {
	case CPU_REV1:
		cpu_r = "A";
		break;
	case CPU_REV1_1:
		cpu_r = "Z";
		break;
	case CPU_REV1_2:
		cpu_r = "Y";
		break;
	default:
		cpu_r = "?";
		break;
	}

	snprintf(name, SOC_NAME_SIZE, "STM32MP%s Rev.%s", cpu_s, cpu_r);
}
