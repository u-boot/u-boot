/*
 * (C) Copyright 2002
 * Hyperion Entertainment, Hans-JoergF@hyperion-entertainment.com
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <pci.h>
#include <ata.h>
#include "memio.h"
#include "articiaS.h"
#include "via686.h"
#include "i8259.h"

DECLARE_GLOBAL_DATA_PTR;

#undef VIA_DEBUG

#ifdef  VIA_DEBUG
#define PRINTF(fmt,args...)     printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif


/*  Setup the ISA-to-PCI host bridge */
void via_isa_init(pci_dev_t dev, struct pci_config_table *table)
{
    char regval;
    if (PCI_FUNC(dev) == 0)
    {
	PRINTF("... PCI-to-ISA bridge, dev=0x%X\n", dev);

	/*  Enable I/O Recovery time */
	pci_write_config_byte(dev, 0x40, 0x08);

	/*  Enable ISA refresh */
	pci_write_config_byte(dev, 0x41, 0x41); /*  was 01 */

	/*  Enable ISA line buffer */
	pci_write_config_byte(dev, 0x45, 0x80);

	/*  Gate INTR, and flush line buffer */
	pci_write_config_byte(dev, 0x46, 0x60);

	/*  Enable EISA ports 4D0/4D1. Do we need this ? */
	pci_write_config_byte(dev, 0x47, 0xe6); /*  was 20 */

	/*  512 K PCI Decode */
	pci_write_config_byte(dev, 0x48, 0x01);

	/*  Wait for PGNT before grant to ISA Master/DMA */
	/*  ports 0-FF to SDBus */
	/*  IRQ 14 and 15 for ide 0/1 */
	pci_write_config_byte(dev, 0x4a, 0x04); /*  Was c4 */

	/*  Plug'n'Play */
	/*  Parallel DRQ 3, Floppy DRQ 2 (default) */
	pci_write_config_byte(dev, 0x50, 0x0e);

	/*  IRQ Routing for Floppy and Parallel port */
	/*  IRQ 6 for floppy, IRQ 7 for parallel port */
	pci_write_config_byte(dev, 0x51, 0x76);

	/*  IRQ Routing for serial ports (take IRQ 3 and 4) */
	pci_write_config_byte(dev, 0x52, 0x34);

	/*  All IRQ's level triggered. */
	pci_write_config_byte(dev, 0x54, 0x00);

	/*  PCI IRQ's all at IRQ 9 */
	pci_write_config_byte(dev, 0x55, 0x90);
	pci_write_config_byte(dev, 0x56, 0x99);
	pci_write_config_byte(dev, 0x57, 0x90);

	/*  Enable Keyboard */
	pci_read_config_byte(dev, 0x5A, &regval);
	regval |= 0x01;
	pci_write_config_byte(dev, 0x5A, regval);

	pci_write_config_byte(dev, 0x80, 0);
	pci_write_config_byte(dev, 0x85, 0x01);

/* 	pci_write_config_byte(dev, 0x77, 0x00); */
    }
}

/*
 * Initialize PNP irq routing
 */

void via_init_irq_routing(uint8 irq_map[])
{
    char *s;
    uint8 level_edge_bits = 0xf;

    /* Set irq routings */
    pci_write_cfg_byte(0, 7<<3, 0x55, irq_map[0]<<4);
    pci_write_cfg_byte(0, 7<<3, 0x56, irq_map[1] | irq_map[2]<<4);
    pci_write_cfg_byte(0, 7<<3, 0x57, irq_map[3]<<4);

    /*
     * Gather level/edge bits
     * Default is to assume level triggered
     */

    s = getenv("pci_irqa_select");
    if (s && strcmp(s, "level") == 0)
	level_edge_bits &= ~0x01;

    s = getenv("pci_irqb_select");
    if (s && strcmp(s, "level") == 0)
	level_edge_bits &= ~0x02;

    s = getenv("pci_irqc_select");
    if (s && strcmp(s, "level") == 0)
	level_edge_bits &= ~0x04;

    s = getenv("pci_irqd_select");
    if (s && strcmp(s, "level") == 0)
	level_edge_bits &= ~0x08;

    PRINTF("IRQ map\n");
    PRINTF("%d: %s\n", irq_map[0], level_edge_bits&0x1 ? "edge" : "level");
    PRINTF("%d: %s\n", irq_map[1], level_edge_bits&0x2 ? "edge" : "level");
    PRINTF("%d: %s\n", irq_map[2], level_edge_bits&0x4 ? "edge" : "level");
    PRINTF("%d: %s\n", irq_map[3], level_edge_bits&0x8 ? "edge" : "level");
    pci_write_cfg_byte(0, 7<<3, 0x54, level_edge_bits);

    PRINTF("%02x %02x %02x %02x\n", pci_read_cfg_byte(0, 7<<3, 0x54),
	   pci_read_cfg_byte(0, 7<<3, 0x55), pci_read_cfg_byte(0, 7<<3, 0x56),
	   pci_read_cfg_byte(0, 7<<3, 0x57));
}


/*  Setup the IDE controller. This doesn't seem to work yet. I/O to an IDE controller port */
/*  always return the last character output on the serial port (!) */
/*  This function is called by the pnp-library when it encounters 0:7:1 */
void via_cfgfunc_ide_init(struct pci_controller *host, pci_dev_t dev, struct pci_config_table *table)
{
    PRINTF("... IDE controller, dev=0x%X\n", dev);

    /*  Enable both IDE channels. */
    pci_write_config_byte(dev, 0x40, 0x03);
    /*  udelay(10000); */
    /*  udelay(10000); */

    /*  Enable IO Space */
    pci_write_config_word(dev, 0x04, 0x03);

    /*  Set to compatibility mode */
    pci_write_config_byte(dev, 0x09, 0x8A); /*  WAS: 0x8f); */

    /*  Set to legacy interrupt mode */
    pci_write_config_byte(dev, 0x3d, 0x00); /* WAS: 0x01); */

}


/*  Set the base address of the floppy controller to 0x3F0 */
void via_fdc_init(pci_dev_t dev)
{
    unsigned char c;
    /*  Enable Configuration mode */
    pci_read_config_byte(dev, 0x85, &c);
    c |= 0x02;
    pci_write_config_byte(dev, 0x85, c);

    /*  Set floppy controller port to 0x3F0. */
    SIO_WRITE_CONFIG(0xE3, (0x3F<<2));

    /*  Enable floppy controller */
    SIO_READ_CONFIG(0xE2, c);
    c |= 0x10;
    SIO_WRITE_CONFIG(0xE2, c);

    /*  Switch of configuration mode */
    pci_read_config_byte(dev, 0x85, &c);
    c &= ~0x02;
    pci_write_config_byte(dev, 0x85, c);
}

/*  Init function 0 of the via southbridge. Called by the pnp-library */
void via_cfgfunc_via686(struct pci_controller *host, pci_dev_t dev, struct pci_config_table *table)
{
    if (PCI_FUNC(dev) == 0)
    {
	/* FIXME: Try to generate a PCI reset */
	/* unsigned char c; */
	/* pci_read_config_byte(dev, 0x47, &c); */
	/* pci_write_config_byte(dev, 0x47, c | 0x01); */

	via_isa_init(dev, table);
	via_fdc_init(dev);
    }
}

__asm         ("    .globl via_calibrate_time_base \n"
	       "via_calibrate_time_base: 	   \n"
	       "   lis     9, 0xfe00		   \n"
	       "   li      0, 0x00		   \n"
	       "   mttbu   0			   \n"
	       "   mttbl   0			   \n"
	       "ctb_loop:			   \n"
	       "   lbz     0, 0x61(9)		   \n"
	       "   eieio			   \n"
	       "   andi.   0, 0, 0x20		   \n"
	       "   beq     ctb_loop		   \n"
	       "ctb_done:			   \n"
	       "   mftb    3			   \n"
	       "   blr");

extern unsigned long via_calibrate_time_base(void);

void via_calibrate_bus_freq (void)
{
	unsigned long tb;

	/* This is 20 microseconds */
#define CALIBRATE_TIME 28636

	/* Enable the timer (and disable speaker) */
	unsigned char c;

	c = in_byte (0x61);
	out_byte (0x61, ((c & ~0x02) | 0x01));

	/* Set timer 2 to low/high writing */
	out_byte (0x43, 0xb0);
	out_byte (0x42, CALIBRATE_TIME & 0xff);
	out_byte (0x42, CALIBRATE_TIME >> 8);

	/* Read the time base */
	tb = via_calibrate_time_base ();

	if (tb >= 700000)
		gd->bus_clk = 133333333;
	else
		gd->bus_clk = 100000000;

}


void ide_led(uchar led, uchar status)
{
/*     unsigned char c = in_byte(0x92); */

/*     if (!status) */
/* 	out_byte(0x92, c | 0xC0); */
/*     else */
/* 	out_byte(0x92, c & ~0xC0); */
}


void via_init_afterscan(void)
{
    /* Modify IDE controller setup */
    pci_write_cfg_byte(0, 7<<3|1, PCI_LATENCY_TIMER, 0x20);
    pci_write_cfg_byte(0, 7<<3|1, PCI_COMMAND, PCI_COMMAND_IO|PCI_COMMAND_MEMORY|PCI_COMMAND_MASTER);
    pci_write_cfg_byte(0, 7<<3|1, PCI_INTERRUPT_LINE, 0xff);
    pci_write_cfg_byte(0, 7<<3|1, 0x40, 0x0b);   /* FIXME: Might depend on drives connected */
    pci_write_cfg_byte(0, 7<<3|1, 0x41, 0x42);   /* FIXME: Might depend on drives connected */
    pci_write_cfg_byte(0, 7<<3|1, 0x43, 0x05);
    pci_write_cfg_byte(0, 7<<3|1, 0x44, 0x18);
    pci_write_cfg_byte(0, 7<<3|1, 0x45, 0x10);
    pci_write_cfg_byte(0, 7<<3|1, 0x4e, 0x22);   /* FIXME: Not documented, but set in PC bios */
    pci_write_cfg_byte(0, 7<<3|1, 0x4f, 0x20);   /* FIXME: Not documented */

    /* Modify some values in the USB controller */
    pci_write_cfg_byte(0, 7<<3|2, 0x05, 0x17);
    pci_write_cfg_byte(0, 7<<3|2, 0x06, 0x01);
    pci_write_cfg_byte(0, 7<<3|2, 0x41, 0x12);
    pci_write_cfg_byte(0, 7<<3|2, 0x42, 0x03);
    pci_write_cfg_byte(0, 7<<3|2, PCI_LATENCY_TIMER, 0x40);

    pci_write_cfg_byte(0, 7<<3|3, 0x05, 0x17);
    pci_write_cfg_byte(0, 7<<3|3, 0x06, 0x01);
    pci_write_cfg_byte(0, 7<<3|3, 0x41, 0x12);
    pci_write_cfg_byte(0, 7<<3|3, 0x42, 0x03);
    pci_write_cfg_byte(0, 7<<3|3, PCI_LATENCY_TIMER, 0x40);


}
