// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */
#include <common.h>
#include <clk.h>
#include <debug_uart.h>
#include <environment.h>
#include <misc.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/uclass.h>

/* RCC register */
#define RCC_TZCR		(STM32_RCC_BASE + 0x00)
#define RCC_DBGCFGR		(STM32_RCC_BASE + 0x080C)
#define RCC_BDCR		(STM32_RCC_BASE + 0x0140)
#define RCC_MP_APB5ENSETR	(STM32_RCC_BASE + 0x0208)
#define RCC_BDCR_VSWRST		BIT(31)
#define RCC_BDCR_RTCSRC		GENMASK(17, 16)
#define RCC_DBGCFGR_DBGCKEN	BIT(8)

/* Security register */
#define ETZPC_TZMA1_SIZE	(STM32_ETZPC_BASE + 0x04)
#define ETZPC_DECPROT0		(STM32_ETZPC_BASE + 0x10)

#define TZC_GATE_KEEPER		(STM32_TZC_BASE + 0x008)
#define TZC_REGION_ATTRIBUTE0	(STM32_TZC_BASE + 0x110)
#define TZC_REGION_ID_ACCESS0	(STM32_TZC_BASE + 0x114)

#define TAMP_CR1		(STM32_TAMP_BASE + 0x00)

#define PWR_CR1			(STM32_PWR_BASE + 0x00)
#define PWR_CR1_DBP		BIT(8)

/* DBGMCU register */
#define DBGMCU_IDC		(STM32_DBGMCU_BASE + 0x00)
#define DBGMCU_APB4FZ1		(STM32_DBGMCU_BASE + 0x2C)
#define DBGMCU_APB4FZ1_IWDG2	BIT(2)
#define DBGMCU_IDC_DEV_ID_MASK	GENMASK(11, 0)
#define DBGMCU_IDC_DEV_ID_SHIFT	0
#define DBGMCU_IDC_REV_ID_MASK	GENMASK(31, 16)
#define DBGMCU_IDC_REV_ID_SHIFT	16

/* boot interface from Bootrom
 * - boot instance = bit 31:16
 * - boot device = bit 15:0
 */
#define BOOTROM_PARAM_ADDR	0x2FFC0078
#define BOOTROM_MODE_MASK	GENMASK(15, 0)
#define BOOTROM_MODE_SHIFT	0
#define BOOTROM_INSTANCE_MASK	 GENMASK(31, 16)
#define BOOTROM_INSTANCE_SHIFT	16

/* BSEC OTP index */
#define BSEC_OTP_SERIAL	13
#define BSEC_OTP_MAC	57

#if !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD)
#ifndef CONFIG_STM32MP1_TRUSTED
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
	writel(GENMASK(25, 0), ETZPC_DECPROT0);

	/* Open SYSRAM for no secure access */
	writel(0x0, ETZPC_TZMA1_SIZE);

	/* enable TZC1 TZC2 clock */
	writel(BIT(11) | BIT(12), RCC_MP_APB5ENSETR);

	/* Region 0 set to no access by default */
	/* bit 0 / 16 => nsaid0 read/write Enable
	 * bit 1 / 17 => nsaid1 read/write Enable
	 * ...
	 * bit 15 / 31 => nsaid15 read/write Enable
	 */
	writel(0xFFFFFFFF, TZC_REGION_ID_ACCESS0);
	/* bit 30 / 31 => Secure Global Enable : write/read */
	/* bit 0 / 1 => Region Enable for filter 0/1 */
	writel(BIT(0) | BIT(1) | BIT(30) | BIT(31), TZC_REGION_ATTRIBUTE0);

	/* Enable Filter 0 and 1 */
	setbits_le32(TZC_GATE_KEEPER, BIT(0) | BIT(1));

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
#endif /* CONFIG_STM32MP1_TRUSTED */

/*
 * Debug init
 */
static void dbgmcu_init(void)
{
	setbits_le32(RCC_DBGCFGR, RCC_DBGCFGR_DBGCKEN);

	/* Freeze IWDG2 if Cortex-A7 is in debug mode */
	setbits_le32(DBGMCU_APB4FZ1, DBGMCU_APB4FZ1_IWDG2);
}
#endif /* !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD) */

static u32 get_bootmode(void)
{
	u32 boot_mode;
#if !defined(CONFIG_STM32MP1_TRUSTED) && \
	(!defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD))
	u32 bootrom_itf = readl(BOOTROM_PARAM_ADDR);
	u32 bootrom_device, bootrom_instance;

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
#else
	/* read TAMP backup register */
	boot_mode = (readl(TAMP_BOOT_CONTEXT) & TAMP_BOOT_MODE_MASK) >>
		    TAMP_BOOT_MODE_SHIFT;
#endif
	return boot_mode;
}

/*
 * Early system init
 */
int arch_cpu_init(void)
{
	u32 boot_mode;

	/* early armv7 timer init: needed for polling */
	timer_init();

#if !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD)
	dbgmcu_init();
#ifndef CONFIG_STM32MP1_TRUSTED
	security_init();
#endif
#endif

	/* get bootmode from BootRom context: saved in TAMP register */
	boot_mode = get_bootmode();

	if ((boot_mode & TAMP_BOOT_DEVICE_MASK) == BOOT_SERIAL_UART)
		gd->flags |= GD_FLG_SILENT | GD_FLG_DISABLE_CONSOLE;
#if defined(CONFIG_DEBUG_UART) && \
	!defined(CONFIG_STM32MP1_TRUSTED) && \
	(!defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD))
	else
		debug_uart_init();
#endif

	return 0;
}

void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}

static u32 read_idc(void)
{
	setbits_le32(RCC_DBGCFGR, RCC_DBGCFGR_DBGCKEN);

	return readl(DBGMCU_IDC);
}

u32 get_cpu_rev(void)
{
	return (read_idc() & DBGMCU_IDC_REV_ID_MASK) >> DBGMCU_IDC_REV_ID_SHIFT;
}

u32 get_cpu_type(void)
{
	return (read_idc() & DBGMCU_IDC_DEV_ID_MASK) >> DBGMCU_IDC_DEV_ID_SHIFT;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char *cpu_s, *cpu_r;

	switch (get_cpu_type()) {
	case CPU_STMP32MP15x:
		cpu_s = "15x";
		break;
	default:
		cpu_s = "?";
		break;
	}

	switch (get_cpu_rev()) {
	case CPU_REVA:
		cpu_r = "A";
		break;
	case CPU_REVB:
		cpu_r = "B";
		break;
	default:
		cpu_r = "?";
		break;
	}

	printf("CPU: STM32MP%s.%s\n", cpu_s, cpu_r);

	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

static void setup_boot_mode(void)
{
	char cmd[60];
	u32 boot_ctx = readl(TAMP_BOOT_CONTEXT);
	u32 boot_mode =
		(boot_ctx & TAMP_BOOT_MODE_MASK) >> TAMP_BOOT_MODE_SHIFT;
	int instance = (boot_mode & TAMP_BOOT_INSTANCE_MASK) - 1;

	pr_debug("%s: boot_ctx=0x%x => boot_mode=%x, instance=%d\n",
		 __func__, boot_ctx, boot_mode, instance);

	switch (boot_mode & TAMP_BOOT_DEVICE_MASK) {
	case BOOT_SERIAL_UART:
		sprintf(cmd, "%d", instance);
		env_set("boot_device", "uart");
		env_set("boot_instance", cmd);
		break;
	case BOOT_SERIAL_USB:
		env_set("boot_device", "usb");
		env_set("boot_instance", "0");
		break;
	case BOOT_FLASH_SD:
	case BOOT_FLASH_EMMC:
		sprintf(cmd, "%d", instance);
		env_set("boot_device", "mmc");
		env_set("boot_instance", cmd);
		break;
	case BOOT_FLASH_NAND:
		env_set("boot_device", "nand");
		env_set("boot_instance", "0");
		break;
	case BOOT_FLASH_NOR:
		env_set("boot_device", "nor");
		env_set("boot_instance", "0");
		break;
	default:
		pr_debug("unexpected boot mode = %x\n", boot_mode);
		break;
	}
}

/*
 * If there is no MAC address in the environment, then it will be initialized
 * (silently) from the value in the OTP.
 */
static int setup_mac_address(void)
{
#if defined(CONFIG_NET)
	int ret;
	int i;
	u32 otp[2];
	uchar enetaddr[6];
	struct udevice *dev;

	/* MAC already in environment */
	if (eth_env_get_enetaddr("ethaddr", enetaddr))
		return 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_GET_DRIVER(stm32mp_bsec),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, BSEC_OTP_MAC * 4 + STM32_BSEC_OTP_OFFSET,
			otp, sizeof(otp));
	if (ret < 0)
		return ret;

	for (i = 0; i < 6; i++)
		enetaddr[i] = ((uint8_t *)&otp)[i];

	if (!is_valid_ethaddr(enetaddr)) {
		pr_err("invalid MAC address in OTP %pM", enetaddr);
		return -EINVAL;
	}
	pr_debug("OTP MAC address = %pM\n", enetaddr);
	ret = !eth_env_set_enetaddr("ethaddr", enetaddr);
	if (!ret)
		pr_err("Failed to set mac address %pM from OTP: %d\n",
		       enetaddr, ret);
#endif

	return 0;
}

static int setup_serial_number(void)
{
	char serial_string[25];
	u32 otp[3] = {0, 0, 0 };
	struct udevice *dev;
	int ret;

	if (env_get("serial#"))
		return 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_GET_DRIVER(stm32mp_bsec),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, BSEC_OTP_SERIAL * 4 + STM32_BSEC_OTP_OFFSET,
			otp, sizeof(otp));
	if (ret < 0)
		return ret;

	sprintf(serial_string, "%08x%08x%08x", otp[0], otp[1], otp[2]);
	env_set("serial#", serial_string);

	return 0;
}

int arch_misc_init(void)
{
	setup_boot_mode();
	setup_mac_address();
	setup_serial_number();

	return 0;
}
