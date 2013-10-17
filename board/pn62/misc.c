/*
 * (C) Copyright 2002 Wolfgang Grandegger <wg@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc824x.h>
#include <asm/io.h>
#include <pci.h>

#include "pn62.h"

typedef struct {
    pci_dev_t    devno;
    volatile u32 *csr;

} i2155x_t;

static i2155x_t i2155x = { 0, NULL };

static struct pci_device_id i2155x_ids[] = {
    { 0x1011, 0x0046 },		/* i21554 */
    { 0x8086, 0xb555 }		/* i21555 */
};

int i2155x_init(void)
{
    pci_dev_t devno;
    u32 val;
    int i;

    /*
     * Find the Intel bridge.
     */
    if ((devno = pci_find_devices(i2155x_ids, 0)) < 0) {
	printf("Error: Intel bridge 2155x not found!\n");
	return -1;
    }
    i2155x.devno = devno;

    /*
     * Get auto-configured base address for CSR access.
     */
    pci_read_config_dword(devno, PCI_BASE_ADDRESS_1, &val);
    if (val & PCI_BASE_ADDRESS_SPACE_IO) {
	val &= PCI_BASE_ADDRESS_IO_MASK;
	i2155x.csr = (volatile u32 *)(_IO_BASE + val);
    } else {
	val &= PCI_BASE_ADDRESS_MEM_MASK;
	i2155x.csr =  (volatile u32 *)val;
    }

    /*
     * Translate downstream memory 2 (bar3) to base of shared memory.
     */
    i2155x_set_bar_base(3, PN62_SMEM_DEFAULT);

    /*
     * Enable memory space, I/O space and bus master bits
     * in both Primary and Secondary command registers.
     */
    val = PCI_COMMAND_MEMORY|PCI_COMMAND_MASTER|PCI_COMMAND_IO;
    pci_write_config_word(devno, 0x44, val);
    pci_write_config_word(devno, 0x04, val);

    /*
     * Clear scratchpad registers.
     */
    for (i = 0; i < (I2155X_SCRAPAD_MAX - 1); i++) {
	i2155x_write_scrapad(i, 0x0);
    }

    /*
     * Set interrupt line for Linux.
     */
    pci_write_config_byte(devno, PCI_INTERRUPT_LINE, 3);

    return 0;
}

/*
 * Access the Scratchpad registers 0..7 of the Intel bridge.
 */
void i2155x_write_scrapad(int idx, u32 val)
{
    if (idx >= 0 && idx < I2155X_SCRAPAD_MAX)
	out_le32(i2155x.csr + (I2155X_SCRAPAD_ADDR/4) + idx, val);
    else
	printf("i2155x_write_scrapad: invalid index\n");
}

u32 i2155x_read_scrapad(int idx)
{
    if (idx >= 0 && idx < I2155X_SCRAPAD_MAX)
	return in_le32(i2155x.csr + (I2155X_SCRAPAD_ADDR/4) + idx);
    else
	printf("i2155x_read_scrapad: invalid index\n");
    return -1;
}

void i2155x_set_bar_base(int bar, u32 base)
{
    if (bar >= 2 && bar <= 4) {
	pci_write_config_dword(i2155x.devno,
			       I2155X_BAR2_BASE + (bar - 2) * 4,
			       base);
    }
}

/*
 * Read Vital Product Data (VPD) from the Serial EPROM attached
 * to the Intel bridge.
 */
int i2155x_read_vpd(int offset, int size, unsigned char *data)
{
    int i, n;
    u16 val16;

    for (i = 0; i < size; i++) {
	pci_write_config_word(i2155x.devno, I2155X_VPD_ADDR,
			      offset + i - I2155X_VPD_START);
	for (n = 10000; n > 0; n--) {
	    pci_read_config_word(i2155x.devno, I2155X_VPD_ADDR, &val16);
	    if ((val16 & 0x8000) != 0) /* wait for completion */
		break;
	    udelay(100);
	}
	if (n == 0) {
	    printf("i2155x_read_vpd: TIMEOUT\n");
	    return -1;
	}

	pci_read_config_byte(i2155x.devno, I2155X_VPD_DATA, &data[i]);
    }

    return i;
}

static struct pci_device_id am79c95x_ids [] = {
	{ PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_LANCE },
	{ }
};


/*
 * Initialize the AMD ethernet controllers.
 */
int am79c95x_init(void)
{
    pci_dev_t devno;
    int i;

    /*
     * Set interrupt line for Linux.
     */
    for (i = 0; i < 2; i++) {
	if ((devno = pci_find_devices(am79c95x_ids, i)) < 0)
	    break;
	pci_write_config_byte(devno, PCI_INTERRUPT_LINE, 2+i);
    }
    if (i < 2)
	printf("Error: Only %d AMD Ethernet Controller found!\n", i);

    return 0;
}


void set_led(unsigned int number, unsigned int function)
{
    volatile u8 *addr;

    if ((number >= 0) && (number < PN62_LED_MAX) &&
	(function >= 0) && (function <= LED_LAST_FUNCTION)) {
	addr = (volatile u8 *)(PN62_LED_BASE + number * 8);
	out_8(addr, function&0xff);
    }
}

/*
 * Show fatal error indicated by Kinght Rider(tm) effect
 * in LEDS 0-7. LEDS 8-11 contain 4 bit error code.
 * Note: this function will not terminate.
 */
void fatal_error(unsigned int error_code)
{
    int i, d;

    for (i = 0; i < 12; i++) {
	set_led(i, LED_0);
    }

    /*
     * Write error code.
     */
    set_led(8,  (error_code & 0x01) ? LED_1 : LED_0);
    set_led(9,  (error_code & 0x02) ? LED_1 : LED_0);
    set_led(10, (error_code & 0x04) ? LED_1 : LED_0);
    set_led(11, (error_code & 0x08) ? LED_1 : LED_0);

    /*
     * Yay - Knight Rider effect!
     */
    while(1) {
	unsigned int delay = 2000;

	for (i = 0; i < 8; i++) {
	    set_led(i, LED_1);
	    for (d = 0; d < delay; d++);
	    set_led(i, LED_0);
	}

	for (i = 7; i > 0; i--) {
	    set_led(i, LED_1);
	    for (d = 0; d < delay; d++);
	    set_led(i, LED_0);
	}
    }
}
