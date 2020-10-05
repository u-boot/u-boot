// SPDX-License-Identifier: GPL-2.0
/*
 * Special driver to handle of-platdata
 *
 * Copyright 2019 Google LLC
 *
 * Some code from coreboot lpss.c
 */

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
#include <malloc.h>
#include <ns16550.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/lpss.h>

/* Low-power Subsystem (LPSS) clock register */
enum {
	LPSS_CLOCK_CTL_REG	= 0x200,
	LPSS_CNT_CLOCK_EN	= 1,
	LPSS_CNT_CLK_UPDATE	= 1U << 31,
	LPSS_CLOCK_DIV_N_SHIFT	= 16,
	LPSS_CLOCK_DIV_N_MASK	= 0x7fff << LPSS_CLOCK_DIV_N_SHIFT,
	LPSS_CLOCK_DIV_M_SHIFT	= 1,
	LPSS_CLOCK_DIV_M_MASK	= 0x7fff << LPSS_CLOCK_DIV_M_SHIFT,

	/* These set the UART input clock speed */
	LPSS_UART_CLK_M_VAL	= 0x25a,
	LPSS_UART_CLK_N_VAL	= 0x7fff,
};

static void lpss_clk_update(void *regs, u32 clk_m_val, u32 clk_n_val)
{
	u32 clk_sel;

	clk_sel = clk_n_val << LPSS_CLOCK_DIV_N_SHIFT |
		 clk_m_val << LPSS_CLOCK_DIV_M_SHIFT;
	clk_sel |= LPSS_CNT_CLK_UPDATE | LPSS_CNT_CLOCK_EN;

	writel(clk_sel, regs + LPSS_CLOCK_CTL_REG);
}

static void uart_lpss_init(void *regs)
{
	/* Take UART out of reset */
	lpss_reset_release(regs);

	/* Set M and N divisor inputs and enable clock */
	lpss_clk_update(regs, LPSS_UART_CLK_M_VAL, LPSS_UART_CLK_N_VAL);
}

void apl_uart_init(pci_dev_t bdf, ulong base)
{
	/* Set UART base address */
	pci_x86_write_config(bdf, PCI_BASE_ADDRESS_0, base, PCI_SIZE_32);

	/* Enable memory access and bus master */
	pci_x86_write_config(bdf, PCI_COMMAND, PCI_COMMAND_MEMORY |
			     PCI_COMMAND_MASTER, PCI_SIZE_32);

	uart_lpss_init((void *)base);
}

/*
 * This driver uses its own compatible string but almost everything else from
 * the standard ns16550 driver. This allows us to provide an of-platdata
 * implementation, since the platdata produced by of-platdata does not match
 * struct ns16550_platdata.
 *
 * When running with of-platdata (generally TPL), the platdata is converted to
 * something that ns16550 expects. When running withoutof-platdata (SPL, U-Boot
 * proper), we use ns16550's ofdata_to_platdata routine.
 */

static int apl_ns16550_probe(struct udevice *dev)
{
	struct ns16550_platdata *plat = dev_get_platdata(dev);

	if (!CONFIG_IS_ENABLED(PCI))
		apl_uart_init(plat->bdf, plat->base);

	return ns16550_serial_probe(dev);
}

static int apl_ns16550_ofdata_to_platdata(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_intel_apl_ns16550 *dtplat = dev_get_platdata(dev);
	struct ns16550_platdata *plat;

	/*
	 * Convert our platdata to the ns16550's platdata, so we can just use
	 * that driver
	 */
	plat = malloc(sizeof(*plat));
	if (!plat)
		return -ENOMEM;
	plat->base = dtplat->early_regs[0];
	plat->reg_width = 1;
	plat->reg_shift = dtplat->reg_shift;
	plat->reg_offset = 0;
	plat->clock = dtplat->clock_frequency;
	plat->fcr = UART_FCR_DEFVAL;
	plat->bdf = pci_ofplat_get_devfn(dtplat->reg[0]);
	dev->platdata = plat;
#else
	int ret;

	ret = ns16550_serial_ofdata_to_platdata(dev);
	if (ret)
		return ret;
#endif /* OF_PLATDATA */

	return 0;
}

static const struct udevice_id apl_ns16550_serial_ids[] = {
	{ .compatible = "intel,apl-ns16550" },
	{ },
};

U_BOOT_DRIVER(intel_apl_ns16550) = {
	.name	= "intel_apl_ns16550",
	.id	= UCLASS_SERIAL,
	.of_match = apl_ns16550_serial_ids,
	.platdata_auto_alloc_size = sizeof(struct ns16550_platdata),
	.priv_auto_alloc_size = sizeof(struct NS16550),
	.ops	= &ns16550_serial_ops,
	.ofdata_to_platdata = apl_ns16550_ofdata_to_platdata,
	.probe = apl_ns16550_probe,
};
