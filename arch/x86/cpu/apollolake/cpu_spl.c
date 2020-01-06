// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 *
 * Portions taken from coreboot
 */

#include <common.h>
#include <acpi_s3.h>
#include <dm.h>
#include <ec_commands.h>
#include <log.h>
#include <spi_flash.h>
#include <spl.h>
#include <syscon.h>
#include <asm/cpu.h>
#include <asm/cpu_common.h>
#include <asm/cpu_x86.h>
#include <asm/fast_spi.h>
#include <asm/intel_pinctrl.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/pci.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/iomap.h>
#include <asm/arch/lpc.h>
#include <asm/arch/pch.h>
#include <asm/arch/systemagent.h>
#include <asm/arch/uart.h>
#include <asm/fsp2/fsp_api.h>
#include <linux/sizes.h>
#include <power/acpi_pmc.h>

/* Define this here to avoid referencing any drivers for the debug UART 1 */
#define PCH_DEV_P2SB	PCI_BDF(0, 0x0d, 0)

static void pch_uart_init(void)
{
	/*
	 * Set up the pinmux so that the UART rx/tx signals are connected
	 * outside the SoC.
	 *
	 * There are about 500 lines of code required to program the GPIO
	 * configuration for the UARTs. But it boils down to four writes, and
	 * for the debug UART we want the minimum possible amount of code before
	 * the UART is running. So just add the magic writes here. See
	 * apl_hostbridge_early_init_pinctrl() for the full horror.
	 */
	if (PCI_FUNC(PCH_DEV_UART) == 1) {
		writel(0x40000402, 0xd0c50650);
		writel(0x3c47, 0xd0c50654);
		writel(0x40000400, 0xd0c50658);
		writel(0x3c48, 0xd0c5065c);
	} else { /* UART2 */
		writel(0x40000402, 0xd0c50670);
		writel(0x3c4b, 0xd0c50674);
		writel(0x40000400, 0xd0c50678);
		writel(0x3c4c, 0xd0c5067c);
	}

#ifdef CONFIG_DEBUG_UART
	apl_uart_init(PCH_DEV_UART, CONFIG_DEBUG_UART_BASE);
#endif
}

static void p2sb_enable_bar(ulong bar)
{
	/* Enable PCR Base address in PCH */
	pci_x86_write_config(PCH_DEV_P2SB, PCI_BASE_ADDRESS_0, bar,
			     PCI_SIZE_32);
	pci_x86_write_config(PCH_DEV_P2SB, PCI_BASE_ADDRESS_1, 0, PCI_SIZE_32);

	/* Enable P2SB MSE */
	pci_x86_write_config(PCH_DEV_P2SB, PCI_COMMAND,
			     PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY,
			     PCI_SIZE_8);
}

/*
 * board_debug_uart_init() - Init the debug UART ready for use
 *
 * This is the minimum init needed to get the UART running. It avoids any
 * drivers or complex code, so that the UART is running as soon as possible.
 */
void board_debug_uart_init(void)
{
	p2sb_enable_bar(IOMAP_P2SB_BAR);
	pch_uart_init();
}

static int fast_spi_cache_bios_region(void)
{
	uint map_size, offset;
	ulong map_base, base;
	int ret;

	ret = fast_spi_early_init(PCH_DEV_SPI, IOMAP_SPI_BASE);
	if (ret)
		return log_msg_ret("early_init", ret);

	ret = fast_spi_get_bios_mmap(PCH_DEV_SPI, &map_base, &map_size,
				     &offset);
	if (ret)
		return log_msg_ret("get_mmap", ret);

	base = SZ_4G - map_size;
	mtrr_set_next_var(MTRR_TYPE_WRPROT, base, map_size);
	log_debug("BIOS cache base=%lx, size=%x\n", base, (uint)map_size);

	return 0;
}

static void enable_pm_timer_emulation(struct udevice *pmc)
{
	struct acpi_pmc_upriv *upriv = dev_get_uclass_priv(pmc);
	msr_t msr;

	/*
	 * The derived frequency is calculated as follows:
	 *    (CTC_FREQ * msr[63:32]) >> 32 = target frequency.
	 *
	 * Back-solve the multiplier so the 3.579545MHz ACPI timer frequency is
	 * used.
	 */
	msr.hi = (3579545ULL << 32) / CTC_FREQ;

	/* Set PM1 timer IO port and enable */
	msr.lo = EMULATE_PM_TMR_EN | (upriv->acpi_base + R_ACPI_PM1_TMR);
	debug("PM timer %x %x\n", msr.hi, msr.lo);
	msr_write(MSR_EMULATE_PM_TIMER, msr);
}

static void google_chromeec_ioport_range(uint *out_basep, uint *out_sizep)
{
	uint base;
	uint size;

	if (IS_ENABLED(CONFIG_EC_GOOGLE_CHROMEEC_MEC)) {
		base = MEC_EMI_BASE;
		size = MEC_EMI_SIZE;
	} else {
		base = EC_HOST_CMD_REGION0;
		size = 2 * EC_HOST_CMD_REGION_SIZE;
		/* Make sure MEMMAP region follows host cmd region */
		assert(base + size == EC_LPC_ADDR_MEMMAP);
		size += EC_MEMMAP_SIZE;
	}

	*out_basep = base;
	*out_sizep = size;
}

static void early_ec_init(void)
{
	uint base, size;

	/*
	 * Set up LPC decoding for the Chrome OS EC I/O port ranges:
	 * - Ports 62/66, 60/64, and 200->208
	 * - Chrome OS EC communication I/O ports
	 */
	lpc_enable_fixed_io_ranges(LPC_IOE_EC_62_66 | LPC_IOE_KBC_60_64 |
				   LPC_IOE_LGE_200);
	google_chromeec_ioport_range(&base, &size);
	lpc_open_pmio_window(base, size);
}

static int arch_cpu_init_tpl(void)
{
	struct udevice *pmc, *sa, *p2sb, *serial, *spi, *lpc;
	int ret;

	ret = uclass_first_device_err(UCLASS_ACPI_PMC, &pmc);
	if (ret)
		return log_msg_ret("PMC", ret);

	/* Clear global reset promotion bit */
	ret = pmc_global_reset_set_enable(pmc, false);
	if (ret)
		return log_msg_ret("disable global reset", ret);

	enable_pm_timer_emulation(pmc);

	ret = uclass_first_device_err(UCLASS_P2SB, &p2sb);
	if (ret)
		return log_msg_ret("p2sb", ret);
	ret = uclass_first_device_err(UCLASS_NORTHBRIDGE, &sa);
	if (ret)
		return log_msg_ret("northbridge", ret);
	gd->baudrate = CONFIG_BAUDRATE;
	ret = uclass_first_device_err(UCLASS_SERIAL, &serial);
	if (ret)
		return log_msg_ret("serial", ret);
	if (CONFIG_IS_ENABLED(SPI_FLASH_SUPPORT)) {
		ret = uclass_first_device_err(UCLASS_SPI, &spi);
		if (ret)
			return log_msg_ret("SPI", ret);
	} else {
		/* Alternative code if we don't have SPI in TPL */
		if (IS_ENABLED(CONFIG_APL_BOOT_FROM_FAST_SPI_FLASH))
			printf("Warning: Enable APL_SPI_FLASHBOOT to use SPI-flash driver in TPL");
		ret = fast_spi_cache_bios_region();
		if (ret)
			return log_msg_ret("BIOS cache", ret);
	}
	ret = pmc_disable_tco(pmc);
	if (ret)
		return log_msg_ret("disable TCO", ret);
	ret = pmc_gpe_init(pmc);
	if (ret)
		return log_msg_ret("pmc_gpe", ret);
	ret = uclass_first_device_err(UCLASS_LPC, &lpc);
	if (ret)
		return log_msg_ret("lpc", ret);

	early_ec_init();

	return 0;
}

/*
 * Enables several BARs and devices which are needed for memory init
 * - MCH_BASE_ADDR is needed in order to talk to the memory controller
 * - HPET is enabled because FSP wants to store a pointer to global data in the
 *   HPET comparator register
 */
static int arch_cpu_init_spl(void)
{
	struct udevice *pmc, *p2sb;
	int ret;

	ret = uclass_first_device_err(UCLASS_ACPI_PMC, &pmc);
	if (ret)
		return log_msg_ret("Could not probe PMC", ret);
	ret = uclass_first_device_err(UCLASS_P2SB, &p2sb);
	if (ret)
		return log_msg_ret("Cannot set up p2sb", ret);

	lpc_io_setup_comm_a_b();

	/* TODO(sjg@chromium.org): Enable upper RTC bank here */

	ret = pmc_init(pmc);
	if (ret < 0)
		return log_msg_ret("Could not init PMC", ret);
#ifdef CONFIG_HAVE_ACPI_RESUME
	ret = pmc_prev_sleep_state(pmc);
	if (ret < 0)
		return log_msg_ret("Could not get PMC sleep state", ret);
	gd->arch.prev_sleep_state = ret;
#endif

	return 0;
}

int arch_cpu_init(void)
{
	int ret = 0;

	if (spl_phase() == PHASE_TPL)
		ret = arch_cpu_init_tpl();
	else if (spl_phase() == PHASE_SPL)
		ret = arch_cpu_init_spl();
	if (ret)
		printf("%s: Error %d\n", __func__, ret);

	return ret;
}
