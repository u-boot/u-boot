// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Siemens AG, 2026
 *
 * Based on Linux i6300esb.c
 * (c) Copyright 2004 Google Inc.
 * (c) Copyright 2005 David Härdeman <david@2gen.com>
 */

#include <div64.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <pci.h>
#include <wdt.h>
#include <asm/io.h>

/* PCI configuration registers */
#define ESB_CONFIG_REG  0x60            /* Config register                   */
#define ESB_LOCK_REG    0x68            /* WDT lock register                 */

/* Memory mapped registers */
#define ESB_TIMER1_REG(w) ((w)->base + 0x00)/* Timer1 value after each reset */
#define ESB_TIMER2_REG(w) ((w)->base + 0x04)/* Timer2 value after each reset */
#define ESB_RELOAD_REG(w) ((w)->base + 0x0c)/* Reload register               */

/* Lock register bits */
#define ESB_WDT_ENABLE  (0x01 << 1)   /* Enable WDT                        */
#define ESB_WDT_LOCK    (0x01 << 0)   /* Lock (nowayout)                   */

/* Reload register bits */
#define ESB_WDT_RELOAD  (0x01 << 8)    /* prevent timeout                   */

/* Magic constants */
#define ESB_UNLOCK1     0x80            /* Step 1 to unlock reset registers  */
#define ESB_UNLOCK2     0x86            /* Step 2 to unlock reset registers  */

struct i6300_wdt {
	void __iomem *base;
};

static void unlock_timer_regs(struct i6300_wdt *wdt)
{
	writew(ESB_UNLOCK1, ESB_RELOAD_REG(wdt));
	writew(ESB_UNLOCK2, ESB_RELOAD_REG(wdt));
}

static int i6300esb_wdt_reset(struct udevice *dev)
{
	struct i6300_wdt *wdt = dev_get_priv(dev);

	unlock_timer_regs(wdt);
	writew(ESB_WDT_RELOAD, ESB_RELOAD_REG(wdt));

	return 0;
}

static int i6300esb_wdt_start(struct udevice *dev, u64 ms, ulong flags)
{
	struct i6300_wdt *wdt = dev_get_priv(dev);
	u32 val;

	/*
	 * Scale to ns, then divide by 983040 ns, the period of the timer
	 * (30 ns PCI bus tick * 2^15). This is timer 1 (IRQ). Timer 2
	 * (reboot) is set to immediate expiry.
	 */
	val = lldiv(ms * 1000000ULL, 983040);

	unlock_timer_regs(wdt);
	writel(val, ESB_TIMER1_REG(wdt));

	unlock_timer_regs(wdt);
	writel(0, ESB_TIMER2_REG(wdt));

	i6300esb_wdt_reset(dev);

	dm_pci_write_config8(dev, ESB_LOCK_REG, ESB_WDT_ENABLE);

	return 0;
}

static int i6300esb_wdt_stop(struct udevice *dev)
{
	u8 val;

	/* First, reset timers as suggested by the docs */
	i6300esb_wdt_reset(dev);
	/* Then disable the WDT */
	dm_pci_write_config8(dev, ESB_LOCK_REG, 0x0);
	dm_pci_read_config8(dev, ESB_LOCK_REG, &val);

	/* Returns 0 if the timer was disabled, -EBUSY otherwise */
	return (val & ESB_WDT_ENABLE) ? -EBUSY : 0;
}

static int i6300esb_wdt_probe(struct udevice *dev)
{
	struct wdt_uc_plat *plat = dev_get_uclass_plat(dev);
	struct i6300_wdt *wdt = dev_get_priv(dev);
	u8 val;

	plat->max_timeout_ms = 1030792;

	wdt->base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0,
				   PCI_REGION_TYPE, PCI_REGION_MEM);
	if (!wdt->base)
		return -EINVAL;

	/*
	 * Config register:
	 * Bit    5 : 0 = Enable WDT_OUTPUT
	 * Bit    2 : 0 = set the timer frequency to the PCI clock
	 * divided by 2^15 (approx 1KHz).
	 * Bits 1:0 : 11 = WDT_INT_TYPE Disabled.
	 * The watchdog has two timers, it can be setup so that the
	 * expiry of timer1 results in an interrupt and the expiry of
	 * timer2 results in a reboot. We set it to not generate
	 * any interrupts as there is not much we can do with it
	 * right now.
	 */
	dm_pci_write_config16(dev, ESB_CONFIG_REG, 0x0003);

	/* Check that the WDT isn't already locked */
	dm_pci_read_config8(dev, ESB_LOCK_REG, &val);
	if (val & ESB_WDT_LOCK)
		dev_warn(dev, "nowayout already set\n");

	/* Set the timer to watchdog mode and disable it for now */
	dm_pci_write_config8(dev, ESB_LOCK_REG, 0x00);

	return 0;
}

static const struct wdt_ops i6300esb_wdt_ops = {
	.start	= i6300esb_wdt_start,
	.stop	= i6300esb_wdt_stop,
	.reset	= i6300esb_wdt_reset,
};

U_BOOT_DRIVER(i6300esb_wdt) = {
	.name	= "i6300esb_wdt",
	.id	= UCLASS_WDT,
	.probe	= i6300esb_wdt_probe,
	.priv_auto = sizeof(struct i6300_wdt),
	.ops	= &i6300esb_wdt_ops,
};

static struct pci_device_id i6300esb_wdt_supported[] = {
	{ PCI_VDEVICE(INTEL, PCI_DEVICE_ID_INTEL_ESB_9) },
	{ },
};

U_BOOT_PCI_DEVICE(i6300esb_wdt, i6300esb_wdt_supported);
