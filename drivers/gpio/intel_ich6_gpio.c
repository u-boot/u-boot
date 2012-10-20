/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * This is a GPIO driver for Intel ICH6 and later. The x86 GPIOs are accessed
 * through the PCI bus. Each PCI device has 256 bytes of configuration space,
 * consisting of a standard header and a device-specific set of registers. PCI
 * bus 0, device 31, function 0 gives us access to the chipset GPIOs (among
 * other things). Within the PCI configuration space, the GPIOBASE register
 * tells us where in the device's I/O region we can find more registers to
 * actually access the GPIOs.
 *
 * PCI bus/device/function 0:1f:0  => PCI config registers
 *   PCI config register "GPIOBASE"
 *     PCI I/O space + [GPIOBASE]  => start of GPIO registers
 *       GPIO registers => gpio pin function, direction, value
 */

#include <common.h>
#include <pci.h>
#include <asm/gpio.h>
#include <asm/io.h>

/* Where in config space is the register that points to the GPIO registers? */
#define PCI_CFG_GPIOBASE 0x48

/*
 * There are often more than 32 GPIOs, depending on the ICH version.
 * For now, we just support bank 0 because it's the same for all.
 */
#define GPIO_MAX 31

/* Within the I/O space, where are the registers to control the GPIOs? */
#define OFS_GPIO_USE_SEL 0x00
#define OFS_GPIO_IO_SEL  0x04
#define OFS_GP_LVL       0x0C

static pci_dev_t dev;				/* handle for 0:1f:0 */
static u32 gpiobase;				/* offset into I/O space */
static int found_it_once;			/* valid GPIO device? */
static int in_use[GPIO_MAX];			/* "lock" for access to pins */

static int gpio_init(void)
{
	u8 tmpbyte;
	u16 tmpword;
	u32 tmplong;

	/* Have we already done this? */
	if (found_it_once)
		return 0;

	/* Where should it be? */
	dev = PCI_BDF(0, 0x1f, 0);

	/* Is the device present? */
	pci_read_config_word(dev, PCI_VENDOR_ID, &tmpword);
	if (tmpword != PCI_VENDOR_ID_INTEL) {
		debug("%s: wrong VendorID\n", __func__);
		return -1;
	}
	/*
	 * We'd like to check the Device ID too, but pretty much any
	 * value is either a) correct with slight differences, or b)
	 * correct but undocumented. We'll have to check other things
	 * instead...
	 */

	/* I/O should already be enabled (it's a RO bit). */
	pci_read_config_word(dev, PCI_COMMAND, &tmpword);
	if (!(tmpword & PCI_COMMAND_IO)) {
		debug("%s: device IO not enabled\n", __func__);
		return -1;
	}

	/* Header Type must be normal (bits 6-0 only; see spec.) */
	pci_read_config_byte(dev, PCI_HEADER_TYPE, &tmpbyte);
	if ((tmpbyte & 0x7f) != PCI_HEADER_TYPE_NORMAL) {
		debug("%s: invalid Header type\n", __func__);
		return -1;
	}

	/* Base Class must be a bridge device */
	pci_read_config_byte(dev, PCI_CLASS_CODE, &tmpbyte);
	if (tmpbyte != PCI_CLASS_CODE_BRIDGE) {
		debug("%s: invalid class\n", __func__);
		return -1;
	}
	/* Sub Class must be ISA */
	pci_read_config_byte(dev, PCI_CLASS_SUB_CODE, &tmpbyte);
	if (tmpbyte != PCI_CLASS_SUB_CODE_BRIDGE_ISA) {
		debug("%s: invalid subclass\n", __func__);
		return -1;
	}

	/* Programming Interface must be 0x00 (no others exist) */
	pci_read_config_byte(dev, PCI_CLASS_PROG, &tmpbyte);
	if (tmpbyte != 0x00) {
		debug("%s: invalid interface type\n", __func__);
		return -1;
	}

	/*
	 * GPIOBASE moved to its current offset with ICH6, but prior to
	 * that it was unused (or undocumented). Check that it looks
	 * okay: not all ones or zeros, and mapped to I/O space (bit 0).
	 */
	pci_read_config_dword(dev, PCI_CFG_GPIOBASE, &tmplong);
	if (tmplong == 0x00000000 || tmplong == 0xffffffff ||
	    !(tmplong & 0x00000001)) {
		debug("%s: unexpected GPIOBASE value\n", __func__);
		return -1;
	}

	/*
	 * Okay, I guess we're looking at the right device. The actual
	 * GPIO registers are in the PCI device's I/O space, starting
	 * at the offset that we just read. Bit 0 indicates that it's
	 * an I/O address, not a memory address, so mask that off.
	 */
	gpiobase = tmplong & 0xfffffffe;

	/* Finally. These are the droids we're looking for. */
	found_it_once = 1;
	return 0;
}

int gpio_request(unsigned gpio, const char *label /* UNUSED */)
{
	u32 tmplong;

	/* Are we doing it wrong? */
	if (gpio > GPIO_MAX || in_use[gpio]) {
		debug("%s: gpio unavailable\n", __func__);
		return -1;
	}

	/* Is the hardware ready? */
	if (gpio_init()) {
		debug("%s: gpio_init failed\n", __func__);
		return -1;
	}

	/*
	 * Make sure that the GPIO pin we want isn't already in use for some
	 * built-in hardware function. We have to check this for every
	 * requested pin.
	 */
	tmplong = inl(gpiobase + OFS_GPIO_USE_SEL);
	if (!(tmplong & (1UL << gpio))) {
		debug("%s: reserved for internal use\n", __func__);
		return -1;
	}

	in_use[gpio] = 1;
	return 0;
}

int gpio_free(unsigned gpio)
{
	if (gpio > GPIO_MAX || !in_use[gpio]) {
		debug("%s: gpio unavailable\n", __func__);
		return -1;
	}
	in_use[gpio] = 0;
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	u32 tmplong;

	if (gpio > GPIO_MAX || !in_use[gpio]) {
		debug("%s: gpio unavailable\n", __func__);
		return -1;
	}
	tmplong = inl(gpiobase + OFS_GPIO_IO_SEL);
	tmplong |= (1UL << gpio);
	outl(gpiobase + OFS_GPIO_IO_SEL, tmplong);
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	u32 tmplong;

	if (gpio > GPIO_MAX || !in_use[gpio]) {
		debug("%s: gpio unavailable\n", __func__);
		return -1;
	}
	tmplong = inl(gpiobase + OFS_GPIO_IO_SEL);
	tmplong &= ~(1UL << gpio);
	outl(gpiobase + OFS_GPIO_IO_SEL, tmplong);
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	u32 tmplong;

	if (gpio > GPIO_MAX || !in_use[gpio]) {
		debug("%s: gpio unavailable\n", __func__);
		return -1;
	}
	tmplong = inl(gpiobase + OFS_GP_LVL);
	return (tmplong & (1UL << gpio)) ? 1 : 0;
}

int gpio_set_value(unsigned gpio, int value)
{
	u32 tmplong;

	if (gpio > GPIO_MAX || !in_use[gpio]) {
		debug("%s: gpio unavailable\n", __func__);
		return -1;
	}
	tmplong = inl(gpiobase + OFS_GP_LVL);
	if (value)
		tmplong |= (1UL << gpio);
	else
		tmplong &= ~(1UL << gpio);
	outl(gpiobase + OFS_GP_LVL, tmplong);
	return 0;
}
