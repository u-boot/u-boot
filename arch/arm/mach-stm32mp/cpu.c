/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier:	GPL-2.0+	BSD-3-Clause
 */
#include <common.h>
#include <clk.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>

void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}

#if !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD)
/**********************************************
 * Security init
 *********************************************/
#define ETZPC_TZMA1_SIZE	(STM32_ETZPC_BASE + 0x04)
#define ETZPC_DECPROT0		(STM32_ETZPC_BASE + 0x10)

#define TZC_GATE_KEEPER		(STM32_TZC_BASE + 0x008)
#define TZC_REGION_ATTRIBUTE0	(STM32_TZC_BASE + 0x110)
#define TZC_REGION_ID_ACCESS0	(STM32_TZC_BASE + 0x114)

#define TAMP_CR1		(STM32_TAMP_BASE + 0x00)

#define PWR_CR1			(STM32_PWR_BASE + 0x00)
#define PWR_CR1_DBP		BIT(8)

#define RCC_TZCR		(STM32_RCC_BASE + 0x00)
#define RCC_BDCR		(STM32_RCC_BASE + 0x0140)
#define RCC_MP_APB5ENSETR	(STM32_RCC_BASE + 0x0208)

#define RCC_BDCR_VSWRST		BIT(31)
#define RCC_BDCR_RTCSRC		GENMASK(17, 16)

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

/**********************************************
 * Debug init
 *********************************************/
#define RCC_DBGCFGR (STM32_RCC_BASE + 0x080C)
#define RCC_DBGCFGR_DBGCKEN BIT(8)

#define DBGMCU_APB4FZ1 (STM32_DBGMCU_BASE + 0x2C)
#define DBGMCU_APB4FZ1_IWDG2 BIT(2)

static void dbgmcu_init(void)
{
	setbits_le32(RCC_DBGCFGR, RCC_DBGCFGR_DBGCKEN);

	/* Freeze IWDG2 if Cortex-A7 is in debug mode */
	setbits_le32(DBGMCU_APB4FZ1, DBGMCU_APB4FZ1_IWDG2);
}
#endif /* !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD) */

int arch_cpu_init(void)
{
	/* early armv7 timer init: needed for polling */
	timer_init();

#if !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD)
	dbgmcu_init();

	security_init();
#endif

	return 0;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	printf("CPU: STM32MP15x\n");

	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

void reset_cpu(ulong addr)
{
}
