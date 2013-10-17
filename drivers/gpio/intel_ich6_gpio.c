/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
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
 *
 *
 * Danger Will Robinson! Bank 0 (GPIOs 0-31) seems to be fairly stable. Most
 * ICH versions have more, but the decoding the matrix that describes them is
 * absurdly complex and constantly changing. We'll provide Bank 1 and Bank 2,
 * but they will ONLY work for certain unspecified chipsets because the offset
 * from GPIOBASE changes randomly. Even then, many GPIOs are unimplemented or
 * reserved or subject to arcane restrictions.
 */

#include <common.h>
#include <pci.h>
#include <asm/gpio.h>
#include <asm/io.h>

/* Where in config space is the register that points to the GPIO registers? */
#define PCI_CFG_GPIOBASE 0x48

#define NUM_BANKS 3

/* Within the I/O space, where are the registers to control the GPIOs? */
static struct {
	u8 use_sel;
	u8 io_sel;
	u8 lvl;
} gpio_bank[NUM_BANKS] = {
	{ 0x00, 0x04, 0x0c },		/* Bank 0 */
	{ 0x30, 0x34, 0x38 },		/* Bank 1 */
	{ 0x40, 0x44, 0x48 }		/* Bank 2 */
};

static pci_dev_t dev;			/* handle for 0:1f:0 */
static u32 gpiobase;			/* offset into I/O space */
static int found_it_once;		/* valid GPIO device? */
static u32 lock[NUM_BANKS];		/* "lock" for access to pins */

static int bad_arg(int num, int *bank, int *bitnum)
{
	int i = num / 32;
	int j = num % 32;

	if (num < 0 || i > NUM_BANKS) {
		debug("%s: bogus gpio num: %d\n", __func__, num);
		return -1;
	}
	*bank = i;
	*bitnum = j;
	return 0;
}

static int mark_gpio(int bank, int bitnum)
{
	if (lock[bank] & (1UL << bitnum)) {
		debug("%s: %d.%d already marked\n", __func__, bank, bitnum);
		return -1;
	}
	lock[bank] |= (1 << bitnum);
	return 0;
}

static void clear_gpio(int bank, int bitnum)
{
	lock[bank] &= ~(1 << bitnum);
}

static int notmine(int num, int *bank, int *bitnum)
{
	if (bad_arg(num, bank, bitnum))
		return -1;
	return !(lock[*bank] & (1UL << *bitnum));
}

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

	pci_read_config_word(dev, PCI_DEVICE_ID, &tmpword);
	debug("Found %04x:%04x\n", PCI_VENDOR_ID_INTEL, tmpword);
	/*
	 * We'd like to validate the Device ID too, but pretty much any
	 * value is either a) correct with slight differences, or b)
	 * correct but undocumented. We'll have to check a bunch of other
	 * things instead...
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

int gpio_request(unsigned num, const char *label /* UNUSED */)
{
	u32 tmplong;
	int i = 0, j = 0;

	/* Is the hardware ready? */
	if (gpio_init())
		return -1;

	if (bad_arg(num, &i, &j))
		return -1;

	/*
	 * Make sure that the GPIO pin we want isn't already in use for some
	 * built-in hardware function. We have to check this for every
	 * requested pin.
	 */
	tmplong = inl(gpiobase + gpio_bank[i].use_sel);
	if (!(tmplong & (1UL << j))) {
		debug("%s: gpio %d is reserved for internal use\n", __func__,
		      num);
		return -1;
	}

	return mark_gpio(i, j);
}

int gpio_free(unsigned num)
{
	int i = 0, j = 0;

	if (notmine(num, &i, &j))
		return -1;

	clear_gpio(i, j);
	return 0;
}

int gpio_direction_input(unsigned num)
{
	u32 tmplong;
	int i = 0, j = 0;

	if (notmine(num, &i, &j))
		return -1;

	tmplong = inl(gpiobase + gpio_bank[i].io_sel);
	tmplong |= (1UL << j);
	outl(gpiobase + gpio_bank[i].io_sel, tmplong);
	return 0;
}

int gpio_direction_output(unsigned num, int value)
{
	u32 tmplong;
	int i = 0, j = 0;

	if (notmine(num, &i, &j))
		return -1;

	tmplong = inl(gpiobase + gpio_bank[i].io_sel);
	tmplong &= ~(1UL << j);
	outl(gpiobase + gpio_bank[i].io_sel, tmplong);
	return 0;
}

int gpio_get_value(unsigned num)
{
	u32 tmplong;
	int i = 0, j = 0;
	int r;

	if (notmine(num, &i, &j))
		return -1;

	tmplong = inl(gpiobase + gpio_bank[i].lvl);
	r = (tmplong & (1UL << j)) ? 1 : 0;
	return r;
}

int gpio_set_value(unsigned num, int value)
{
	u32 tmplong;
	int i = 0, j = 0;

	if (notmine(num, &i, &j))
		return -1;

	tmplong = inl(gpiobase + gpio_bank[i].lvl);
	if (value)
		tmplong |= (1UL << j);
	else
		tmplong &= ~(1UL << j);
	outl(gpiobase + gpio_bank[i].lvl, tmplong);
	return 0;
}
