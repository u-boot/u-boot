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
#include "memio.h"
#include "articiaS.h"

#undef ARTICIA_PCI_DEBUG

#ifdef  ARTICIA_PCI_DEBUG
#define PRINTF(fmt,args...)     printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

struct pci_controller articiaS_hose;

long irq_alloc(long wanted);

static pci_dev_t pci_hose_find_class(struct pci_controller *hose, int bus, short find_class, int index);
static int articiaS_init_vga(void);
static void pci_cfgfunc_dummy(struct pci_controller *host, pci_dev_t dev, struct pci_config_table *table);
unsigned char pci_irq_alloc(void);

extern void via_cfgfunc_via686(struct pci_controller * host, pci_dev_t dev, struct pci_config_table *table);
extern void via_cfgfunc_ide_init(struct pci_controller *host, pci_dev_t dev, struct pci_config_table *table);
extern void via_init_irq_routing(uint8 []);
extern void via_init_afterscan(void);

#define cfgfunc_via686      1
#define cfgfunc_dummy  2
#define cfgfunc_ide_init    3

static struct pci_config_table config_table[] =
{
    {
	0x1106, PCI_ANY_ID, PCI_CLASS_BRIDGE_ISA, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	(void *)cfgfunc_via686, {0, 0, 0}
    },
    {
	0x1106, PCI_ANY_ID, PCI_ANY_ID, 0,7,4,
	(void *)cfgfunc_dummy, {0,0,0}
    },
    {
	0x1106, 0x3068, PCI_ANY_ID, 0, 7, PCI_ANY_ID,
	(void *)cfgfunc_dummy, {0,0,0}
    },
    {
	0x1106, PCI_ANY_ID, PCI_ANY_ID, 0,7,1,
	(void *)cfgfunc_ide_init, {0,0,0}
    },
    {
	0,
    }
};


void pci_cfgfunc_dummy(struct pci_controller *host, pci_dev_t dev, struct pci_config_table *table)
{


}

unsigned long irq_penalties[16] =
{
    1000,    /* 0:timer */
    1000,    /* 1:keyboard */
    1000,    /* 2:cascade */
    50,      /* 3:serial (COM2) */
    50,      /* 4:serial (COM1) */
    4,       /* 5:USB2 */
    100,     /* 6:floppy */
    3,       /* 7:parallel */
    50,      /* 8:AC97/MC97 */
    0,       /* 9: */
    3,       /* 10:: */
    0,       /* 11: */
    3,       /* 12: USB1 */
    0,       /* 13: */
    100,     /* 14: ide0 */
    100,     /* 15: ide1 */
};


/*
 * The following defines a hard-coded interrupt mapping for the
 * know devices on the board.
 * If a device isn't found here, assumed to be a device that's
 * plugged into a PCI or AGP slot
 * NOTE: This table is machine dependant.
 */

struct pci_irq_fixup_table
{
    uint8   bus;             /* Bus number */
    uint8   device;          /* Device number */
    uint8   func;            /* Function number */
    uint8   interrupt;       /* Interrupt to use (0xff to disable) */
};

struct pci_irq_fixup_table fixuptab [] =
{
    { 0, 0, 0, 0xff},        /* Articia S host bridge */
    { 0, 1, 0, 0xff},        /* Articia S AGP bridge */
/*    { 0, 6, 0, 0x05},        /###* 3COM ethernet */
    { 0, 7, 0, 0xff},        /* VIA southbridge */
    { 0, 7, 1, 0x0e},        /* IDE controller in legacy mode */
/*    { 0, 7, 2, 0x05},        /###* First USB controller */
/*    { 0, 7, 3, 0x0c},        /###* Second USB controller (shares interrupt with ethernet) */
    { 0, 7, 4, 0xff},        /* ACPI Power Management */
/*    { 0, 7, 5, 0x08},        /###* AC97 */
/*    { 0, 7, 6, 0x08},        /###* MC97 */
    { 0xff, 0xff, 0xff, 0xff}
};


/*
 * This table maps IRQ's to PCI interrupts
 */

uint8 pci_intmap[4] = {0, 0, 0, 0};

/*
 * Map PCI slots to interrupt routings
 * This table lists the device number assigned to a card inserted
 * into the slot, along with a permutation for the slot's IRQ routing.
 * NOTE: This table is machine dependant.
 */

struct pci_slot_irq_routing
{
    uint8 bus;
    uint8 device;

    uint8 ints[4];
};

struct pci_slot_irq_routing amigaone_pci_routing[] =
{
    {0,  8,   {0, 1, 2, 3}},       /* Slot 1 (left of riser slot) */
    {0,  9,   {1, 2, 3, 0}},       /* Slot 2 (middle slot) */
    {0, 10,   {2, 3, 0, 1}},       /* Slot 3 (leftmost slot) */
    {1,  0,   {1, 0, 2, 3}},       /* AGP slot (only IRQA and IRQB) */
    {1,  1,   {1, 2, 3, 0}},       /* PCI slot on AGP bus */
    {0,  6,   {3, 3, 3, 3}},       /* On board ethernet */
    {0,  7,   {0, 1, 2, 3}},       /* Southbridge */
    {0xff, 0, {0, 0, 0, 0}}
};

void articiaS_pci_irq_init(void)
{
    char *s;

    s = getenv("pci_irqa");
    if (s)
	pci_intmap[0] = simple_strtoul (s, NULL, 10);
    else
	pci_intmap[0] = pci_irq_alloc();

    s = getenv("pci_irqb");
    if (s)
	pci_intmap[1] = simple_strtoul (s, NULL, 10);
    else
	pci_intmap[1] = pci_irq_alloc();

    s = getenv("pci_irqc");
    if (s)
	pci_intmap[2] = simple_strtoul (s, NULL, 10);
    else
	pci_intmap[2] = pci_irq_alloc();

    s = getenv("pci_irqd");
    if (s)
	pci_intmap[3] = simple_strtoul (s, NULL, 10);
    else
	pci_intmap[3] = pci_irq_alloc();
}


unsigned char pci_irq_alloc(void)
{
    int i;
    int interrupt = 10;
    unsigned long min_penalty = 1000;

    /* Search for the minimal penalty, favoring interrupts at the end */
    for (i = 0; i < 16; i++)
    {
	if (irq_penalties[i] <= min_penalty)
	{
	    interrupt = i;
	    min_penalty = irq_penalties[i];
	}
    }

    PRINTF("pci_irq_alloc: Minimal penalty is %ld for %d\n", min_penalty, interrupt);

    irq_penalties[interrupt]++;

    return interrupt;
}


void articiaS_pci_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
    int8 bus, device, func, pin, line;
    int i;

    bus = PCI_BUS(dev);
    device = PCI_DEV(dev);
    func = PCI_FUNC(dev);

    PRINTF("Fixup irq of %d:%d.%d\n", bus, device, func);

    /* Search for the device in the table */
    for (i = 0; fixuptab[i].bus != 0xff; i++)
    {
	if (bus == fixuptab[i].bus && device == fixuptab[i].device && func == fixuptab[i].func)
	{
	    /* If the device needs an interrupt, write it */
	    if (fixuptab[i].interrupt != 0xff)
	    {
		PRINTF("Assigning IRQ %d (fixed)\n", fixuptab[i].interrupt);
		pci_write_config_byte(dev, PCI_INTERRUPT_LINE, fixuptab[i].interrupt);
	    }
	    else
	    {
		/* Otherwise, see if it wants an interrupt, and disable it if needed */
		pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &pin);
		if (pin)
		{
		    PRINTF("Disabling IRQ\n");
		    pci_write_config_byte(dev, PCI_INTERRUPT_LINE, 0xff);
		}
	    }

	    return;
	}
    }

    /* If we get here, we have another PCI device in a slot... find the appropriate IRQ */

    /* Find matching pin */
    pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &pin);
    pin--;

    /* Search for it's map */
    for (i = 0; amigaone_pci_routing[i].bus != 0xff; i++)
    {
	if (bus == amigaone_pci_routing[i].bus && device == amigaone_pci_routing[i].device)
	{
	    line = pci_intmap[amigaone_pci_routing[i].ints[pin]];
	    PRINTF("Assigning IRQ %d (pin %d)\n", line, pin);
	    pci_write_config_byte(dev, PCI_INTERRUPT_LINE, line);
	    return;
	}
    }

    PRINTF("Unkonwn PCI device found\n");
}

void articiaS_pci_init (void)
{
    int i;
    char *s;

    PRINTF("atriciaS_pci_init\n");

    /* Why aren't these relocated?? */
    for (i=0; config_table[i].config_device; i++)
    {
	switch((int)config_table[i].config_device)
	{
	case cfgfunc_via686:     config_table[i].config_device = via_cfgfunc_via686;      break;
	case cfgfunc_dummy:      config_table[i].config_device = pci_cfgfunc_dummy;       break;
	case cfgfunc_ide_init:   config_table[i].config_device = via_cfgfunc_ide_init;    break;
	default: PRINTF("Error: Unknown constant\n");
	}
    }

    articiaS_hose.first_busno = 0;
    articiaS_hose.last_busno = 0xff;
    articiaS_hose.config_table = config_table;
    articiaS_hose.fixup_irq = articiaS_pci_fixup_irq;

    articiaS_pci_irq_init();

    /* System memory */
    pci_set_region(articiaS_hose.regions + 0,
		   ARTICIAS_SYS_BUS,
		   ARTICIAS_SYS_PHYS,
		   ARTICIAS_SYS_MAXSIZE,
		   PCI_REGION_MEM | PCI_REGION_MEMORY);

    /* PCI memory space */
    pci_set_region(articiaS_hose.regions + 1,
		   ARTICIAS_PCI_BUS,
		   ARTICIAS_PCI_PHYS,
		   ARTICIAS_PCI_MAXSIZE,
		   PCI_REGION_MEM);

    /* PCI io space */
    pci_set_region(articiaS_hose.regions + 2,
		   ARTICIAS_PCIIO_BUS,
		   ARTICIAS_PCIIO_PHYS,
		   ARTICIAS_PCIIO_MAXSIZE,
		   PCI_REGION_IO);

    /* PCI/ISA io space */
    pci_set_region(articiaS_hose.regions + 3,
		   ARTICIAS_ISAIO_BUS,
		   ARTICIAS_ISAIO_PHYS,
		   ARTICIAS_ISAIO_MAXSIZE,
		   PCI_REGION_IO);


    articiaS_hose.region_count = 4;

    pci_setup_indirect(&articiaS_hose, ARTICIAS_PCI_CFGADDR, ARTICIAS_PCI_CFGDATA);
    PRINTF("Registering articia hose...\n");
    pci_register_hose(&articiaS_hose);
    PRINTF("Enabling AGP...\n");
    pci_write_config_byte(PCI_BDF(0,0,0), 0x58, 0x01);
    PRINTF("Scanning bus...\n");
    articiaS_hose.last_busno = pci_hose_scan(&articiaS_hose);

    via_init_irq_routing(pci_intmap);

    PRINTF("After-Scan results:\n");
    PRINTF("Bus range: %d - %d\n", articiaS_hose.first_busno , articiaS_hose.last_busno);

    via_init_afterscan();

    pci_write_config_byte(PCI_BDF(0,1,0), PCI_INTERRUPT_LINE, 0xFF);

    s = getenv("as_irq");
    if (s)
    {
	pci_write_config_byte(PCI_BDF(0,0,0), PCI_INTERRUPT_LINE, simple_strtoul (s, NULL, 10));
    }

    s = getenv("x86_run_bios");
    if (!s || (s && strcmp(s, "on")==0))
    {
	if (articiaS_init_vga() == -1)
	{
	    /* If the VGA didn't init and we have stdout set to VGA, reset to serial */
/* 	    s = getenv("stdout"); */
/* 	    if (s && strcmp(s, "vga") == 0) */
/* 	    { */
/* 		setenv("stdout", "serial"); */
/* 	    } */
	}
    }
    pci_write_config_byte(PCI_BDF(0,1,0), PCI_INTERRUPT_LINE, 0xFF);

}

pci_dev_t pci_hose_find_class(struct pci_controller *hose, int bus, short find_class, int index)
{
    unsigned int sub_bus, found_multi=0;
    unsigned short vendor, class;
    unsigned char header_type;
    pci_dev_t dev;
    u8 c1, c2;

    sub_bus = bus;

    for (dev =  PCI_BDF(bus,0,0);
	 dev <  PCI_BDF(bus,PCI_MAX_PCI_DEVICES-1,PCI_MAX_PCI_FUNCTIONS-1);
	 dev += PCI_BDF(0,0,1))
    {
	if ( dev == PCI_BDF(hose->first_busno,0,0) )
	    continue;

	if (PCI_FUNC(dev) && !found_multi)
	    continue;

	pci_hose_read_config_byte(hose, dev, PCI_HEADER_TYPE, &header_type);

	pci_hose_read_config_word(hose, dev, PCI_VENDOR_ID, &vendor);

	if (vendor != 0xffff && vendor != 0x0000)
	{

	    if (!PCI_FUNC(dev))
		found_multi = header_type & 0x80;
	    pci_hose_read_config_byte(hose, dev, 0x0B, &c1);
	    pci_hose_read_config_byte(hose, dev, 0x0A, &c2);
	    class = c1<<8 | c2;
	    /*printf("At %02x:%02x:%02x: class %x\n", */
	    /*	   PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev), class); */
	    if (class == find_class)
	    {
		if (index == 0)
		    return dev;
		else index--;
	    }
	}
    }

    return ~0;
}


/*
 * For a given bus number, find the bridge on this hose that provides this
 * bus number. The function scans for bridges and peeks config space offset
 * 0x19 (PCI_SECONDARY_BUS).
 */
pci_dev_t pci_find_bridge_for_bus(struct pci_controller *hose, int busnr)
{
    pci_dev_t dev;
    int bus;
    unsigned int found_multi=0;
    unsigned char header_type;
    unsigned short vendor;
    unsigned char secondary_bus;

    if (hose == NULL) hose = &articiaS_hose;

    if (busnr < hose->first_busno || busnr > hose->last_busno) return PCI_ANY_ID; /* Not in range */

    /*
     * The bridge must be on a lower bus number
     */
    for (bus = hose->first_busno; bus < busnr; bus++)
    {
	for (dev =  PCI_BDF(bus,0,0);
	     dev <  PCI_BDF(bus,PCI_MAX_PCI_DEVICES-1,PCI_MAX_PCI_FUNCTIONS-1);
	     dev += PCI_BDF(0,0,1))
	{
	    if ( dev == PCI_BDF(hose->first_busno,0,0) )
		continue;

	    if (PCI_FUNC(dev) && !found_multi)
		continue;

	    pci_hose_read_config_byte(hose, dev, PCI_HEADER_TYPE, &header_type);

	    pci_hose_read_config_word(hose, dev, PCI_VENDOR_ID, &vendor);

	    if (vendor != 0xffff && vendor != 0x0000)
	    {

		if (!PCI_FUNC(dev))
		    found_multi = header_type & 0x80;
		if (header_type == 1) /* Bridge device header */
		{
		    pci_hose_read_config_byte(hose, dev, PCI_SECONDARY_BUS, &secondary_bus);
		    if ((int)secondary_bus == busnr) return dev;
		}

	    }
	}
    }
    return PCI_ANY_ID;
}

static short classes[] =
{
    PCI_CLASS_DISPLAY_VGA,
    PCI_CLASS_DISPLAY_XGA,
    PCI_CLASS_DISPLAY_3D,
    PCI_CLASS_DISPLAY_OTHER,
    ~0
};

extern int execute_bios(pci_dev_t gr_dev, void *);

pci_dev_t video_dev;

int articiaS_init_vga (void)
{
	DECLARE_GLOBAL_DATA_PTR;

    extern void shutdown_bios(void);
    pci_dev_t dev = ~0;
    int busnr = 0;
    int classnr = 0;

    video_dev = PCI_ANY_ID;

    printf("VGA:   ");

    PRINTF("Trying to initialize x86 VGA Card(s)\n");

    while (dev == ~0)
    {
	PRINTF("Searching for class 0x%x on bus %d\n", classes[classnr], busnr);
	/* Find the first of this class on this bus */
	dev = pci_hose_find_class(&articiaS_hose, busnr, classes[classnr], 0);
	if (dev != ~0)
	{
	    PRINTF("Found VGA Card at %02x:%02x:%02x\n", PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));
	    break;
	}
	busnr++;
	if (busnr > articiaS_hose.last_busno)
	{
	    busnr = 0;
	    classnr ++;
	    if (classes[classnr] == ~0)
	    {
		printf("NOT PRESENT\n");
		return -1;
	    }
	}
    }

    /*
     * If we get here we have found the first graphics card.
     * If the bus number is not 0, then it is probably behind a bridge, and the
     * bridge needs to be told to forward VGA access.
     */

    if (PCI_BUS(dev) != 0)
    {
	pci_dev_t bridge;
	PRINTF("Behind bridge, looking for bridge\n");
	bridge = pci_find_bridge_for_bus(&articiaS_hose, PCI_BUS(dev));
	if (dev != PCI_ANY_ID)
	{
	    unsigned char agp_control_0;
	    PRINTF("Got the bridge at %02x:%02x:%02x\n",
		   PCI_BUS(bridge), PCI_DEV(bridge), PCI_FUNC(bridge));
	    pci_hose_read_config_byte(&articiaS_hose, bridge, 0x3E, &agp_control_0);
	    agp_control_0 |= 0x18;
	    pci_hose_write_config_byte(&articiaS_hose, bridge, 0x3E, agp_control_0);
	    PRINTF("Configured for VGA forwarding\n");
	}
    }

    /*
     * Now try to run the bios
     */
    PRINTF("Trying to run bios now\n");
    if (execute_bios(dev, gd->relocaddr))
    {
	printf("OK\n");
	video_dev = dev;
    }
    else
    {
	printf("ERROR\n");
    }

    PRINTF("Done scanning.\n");

    shutdown_bios();

    if (dev == PCI_ANY_ID) return -1;
    else return 0;

}
