#include <common.h>
#include <pci.h>
#include <74xx_7xx.h>


#ifdef DEBUG
#undef DEBUG
#endif

#ifdef DEBUG
#define PRINTF(format, args...) _printf(format , ## args)
#else
#define PRINTF(format, argc...)
#endif

static pci_dev_t to_pci(int bus, int devfn)
{
    return PCI_BDF(bus, (devfn>>3), devfn&3);
}

int mypci_find_device(int vendor, int product, int index)
{
    return pci_find_device(vendor, product, index);
}

int mypci_bus(int device)
{
    return PCI_BUS(device);
}

int mypci_devfn(int device)
{
    return (PCI_DEV(device)<<3) | PCI_FUNC(device);
}


#define mypci_read_func(type, size)				\
type mypci_read_cfg_##size##(int bus, int devfn, int offset)	\
{								\
    type c;							\
    pci_read_config_##size##(to_pci(bus, devfn), offset, &c);	\
    return c;							\
}

#define mypci_write_func(type, size)				\
void mypci_write_cfg_##size##(int bus, int devfn, int offset, int value)	\
{								\
    pci_write_config_##size##(to_pci(bus, devfn), offset, value);	\
}

mypci_read_func(u8,byte);
mypci_read_func(u16,word);

mypci_write_func(u8,byte);
mypci_write_func(u16,word);

u32 mypci_read_cfg_long(int bus, int devfn, int offset)
{
    u32 c;
    pci_read_config_dword(to_pci(bus, devfn), offset, &c);
    return c;
}

void mypci_write_cfg_long(int bus, int devfn, int offset, int value)
{
    pci_write_config_dword(to_pci(bus, devfn), offset, value);
}

void _printf(const char *fmt, ...)
{
    va_list args;
    char buf[CFG_PBSIZE];

    va_start(args, fmt);
    (void)vsprintf(buf, fmt, args);
    va_end(args);

    printf(buf);
}

char *_getenv(char *name)
{
    return getenv(name);
}

unsigned long get_bar_size(pci_dev_t dev, int offset)
{
    u32 bar_back, bar_value;

    /*  Save old BAR value */
    pci_read_config_dword(dev, offset, &bar_back);

    /*  Write all 1's. */
    pci_write_config_dword(dev, offset, ~0);

    /*  Now read back the relevant bits */
    pci_read_config_dword(dev, offset, &bar_value);

    /*  Restore original value */
    pci_write_config_dword(dev, offset, bar_back);

    if (bar_value == 0) return 0xFFFFFFFF; /*  This BAR is disabled */

    if ((bar_value & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY)
    {
	/*  This is a memory space BAR. Mask it out so we get the size of it */
	return ~(bar_value & PCI_BASE_ADDRESS_MEM_MASK) + 1;
    }

    /*  Not suitable */
    return 0xFFFFFFFF;
}

void enable_compatibility_hole(void)
{
    u8 cfg;
    pci_dev_t art = PCI_BDF(0,0,0);

    pci_read_config_byte(art, 0x54, &cfg);
    /* cfg |= 0x08; */
    cfg |= 0x20;
    pci_write_config_byte(art, 0x54, cfg);
}

void disable_compatibility_hole(void)
{
    u8 cfg;
    pci_dev_t art = PCI_BDF(0,0,0);

    pci_read_config_byte(art, 0x54, &cfg);
    /* cfg &= ~0x08; */
    cfg &= ~0x20;
    pci_write_config_byte(art, 0x54, cfg);
}

void map_rom(pci_dev_t dev, u32 address)
{
    pci_write_config_dword(dev, PCI_ROM_ADDRESS, address|PCI_ROM_ADDRESS_ENABLE);
}

void unmap_rom(pci_dev_t dev)
{
    pci_write_config_dword(dev, PCI_ROM_ADDRESS, 0);
}

void bat_map(u8 batnum, u32 address, u32 length)
{
    u32 temp = address;
    address &= 0xFFFE0000;
    temp    &= 0x0001FFFF;
    length = (length - 1 ) >> 17;
    length <<= 2;

    switch (batnum)
    {
    case 0:
	__asm volatile ("mtdbatu 0, %0" : : "r" (address | length | 3));
	__asm volatile ("mtdbatl 0, %0" : : "r" (address | 0x22));
	break;
    case 1:
	__asm volatile ("mtdbatu 1, %0" : : "r" (address | length | 3));
	__asm volatile ("mtdbatl 1, %0" : : "r" (address | 0x22));
	break;
    case 2:
	__asm volatile ("mtdbatu 2, %0" : : "r" (address | length | 3));
	__asm volatile ("mtdbatl 2, %0" : : "r" (address | 0x22));
	break;
    case 3:
	__asm volatile ("mtdbatu 3, %0" : : "r" (address | length | 3));
	__asm volatile ("mtdbatl 3, %0" : : "r" (address | 0x22));
	break;
    }
}

int find_image(u32 rom_address, u32 rom_size, void **image, u32 *image_size);

int attempt_map_rom(pci_dev_t dev, void *copy_address)
{
    u32 rom_size      = 0;
    u32 rom_address   = 0;
    u32 bar_size      = 0;
    u32 bar_backup    = 0;
    int i,j;
    void *image       = 0;
    u32 image_size    = 0;
    int did_correct   = 0;
    u32 prefetch_addr = 0;
    u32 prefetch_size = 0;
    u32 prefetch_idx  = 0;

    /*  Get the size of the expansion rom */
    pci_write_config_dword(dev, PCI_ROM_ADDRESS, 0xFFFFFFFF);
    pci_read_config_dword(dev, PCI_ROM_ADDRESS, &rom_size);
    if ((rom_size & 0x01) == 0)
    {
	PRINTF("No ROM\n");
	return 0;
    }

    rom_size &= 0xFFFFF800;
    rom_size = (~rom_size)+1;

    PRINTF("ROM Size is %dK\n", rom_size/1024);

    /*
     * Try to find a place for the ROM. We always attempt to use
     * one of the card's bases for this, as this will be in any
     * bridge's resource range as well as being free of conflicts
     * with other cards. In a graphics card it is very unlikely
     * that there won't be any base address that is large enough to
     * hold the rom.
     *
     * FIXME: To work around this, theoretically the largest base
     * could be used if none is found in the loop below.
     */

    for (i = PCI_BASE_ADDRESS_0; i <= PCI_BASE_ADDRESS_5; i += 4)
    {
	bar_size = get_bar_size(dev, i);
	PRINTF("PCI_BASE_ADDRESS_%d is %dK large\n",
	       (i - PCI_BASE_ADDRESS_0)/4,
	       bar_size/1024);
	if (bar_size != 0xFFFFFFFF && bar_size >= rom_size)
	{
	    PRINTF("Found a match for rom size\n");
	    pci_read_config_dword(dev, i, &rom_address);
	    rom_address &= 0xFFFFFFF0;
	    if (rom_address != 0 && rom_address != 0xFFFFFFF0) break;
	}
    }

    if (rom_address == 0 || rom_address == 0xFFFFFFF0)
    {
	PRINTF("No suitable rom address found\n");
	return 0;
    }

    /*  Disable the BAR */
    pci_read_config_dword(dev, i, &bar_backup);
    pci_write_config_dword(dev, i, 0);

    /*  Map ROM */
    pci_write_config_dword(dev, PCI_ROM_ADDRESS, rom_address | PCI_ROM_ADDRESS_ENABLE);

    /*  Copy the rom to a place in the emulator space */
    PRINTF("Claiming BAT 2\n");
    bat_map(2, rom_address, rom_size);
    /* show_bat_mapping(); */

    if (0 == find_image(rom_address, rom_size, &image, &image_size))
    {
	PRINTF("No x86 BIOS image found\n");
	return 0;
    }

    PRINTF("Copying %ld bytes from 0x%lx to 0x%lx\n", (long)image_size, (long)image, (long)copy_address);

    /* memcpy(copy_address, rom_address, rom_size); */
    {
	unsigned char *from = (unsigned char *)image; /* rom_address; */
	unsigned char *to = (unsigned char *)copy_address;
	for (j=0; j<image_size /*rom_size*/; j++)
	{
	    *to++ = *from++;
	}
    }

    PRINTF("Copy is done\n");

    /*  Unmap the ROM and restore the BAR */
    pci_write_config_dword(dev, PCI_ROM_ADDRESS, 0);
    pci_write_config_dword(dev, i, bar_backup);

    /*  FIXME: Shouldn't be needed anymore*/
    /* bat_map(2, 0x80000000, 256*1024*1024);
       show_bat_mapping(); */

    /*
     * Since most cards can probably only do 16 bit IO addressing, we
     * correct their IO base into an appropriate value.
     * This should do for most.
     */
    for (i = PCI_BASE_ADDRESS_0; i <= PCI_BASE_ADDRESS_5; i += 4)
    {
	unsigned long value;
	pci_read_config_dword(dev, i, &value);
	if (value & 0x01) /*  IO */
	{
	    did_correct = 1;
	    pci_write_config_dword(dev, i, 0x1001);
	    break;
	}

	if (value & PCI_BASE_ADDRESS_MEM_PREFETCH)
	{
	    prefetch_idx = i;
	    prefetch_addr = value & PCI_BASE_ADDRESS_MEM_MASK;
	    prefetch_size = get_bar_size(dev, i);
	}
    }

    if (1) /* did_correct) */
    {
	extern pci_dev_t pci_find_bridge_for_bus(struct pci_controller *hose, int busnr);
	int busnr = PCI_BUS(dev);
	if (busnr)
	{
	    pci_dev_t bridge;
	    PRINTF("Need to correct bridge device for IO range change\n");
	    bridge = pci_find_bridge_for_bus(NULL, busnr);
	    if (bridge == PCI_ANY_ID)
	    {
		PRINTF("Didn't find bridge. Hope that's OK\n");
	    }
	    else
	    {
		/*
		 * Set upper I/O base/limit to 0
		 */
		pci_write_config_byte(bridge, 0x30, 0x00);
		pci_write_config_byte(bridge, 0x31, 0x00);
		pci_write_config_byte(bridge, 0x32, 0x00);
		pci_write_config_byte(bridge, 0x33, 0x00);
		if (did_correct)
		{
		    /*
		     * set lower I/O base to 1000
		     * That is, bits 0:3 are set to 0001 by default.
		     * bits 7:4 contain I/O address bits 15:12
		     * all others are assumed 0.
		     */
		    pci_write_config_byte(bridge, 0x1C, 0x11);
		    /*
		     * Set lower I/O limit to 1FFF
		     * That is, bits 0:3 are reserved and always 0000
		     * Bits 7:4 contain I/O address bits 15:12
		     * All others are assumed F.
		     */
		    pci_write_config_byte(bridge, 0x1D, 0x10);
		    pci_write_config_byte(bridge, 0x0D, 0x20);
		    PRINTF("Corrected bridge resource range of bridge at %02x:%02x:%02x\n",
			   PCI_BUS(bridge), PCI_DEV(bridge), PCI_FUNC(bridge));

		}
		else
		{
		    /*
		     * This card doesn't have I/O, we disable I/O forwarding
		     */
		    pci_write_config_byte(bridge, 0x1C, 0x11);
		    pci_write_config_byte(bridge, 0x1D, 0x00);
		    pci_write_config_byte(bridge, PCI_INTERRUPT_LINE, 0);
		    pci_write_config_byte(bridge, PCI_INTERRUPT_PIN, 0);
		    pci_write_config_dword(bridge, PCI_COMMAND, PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_IO);
		    PRINTF("Disabled bridge resource range of bridge at %02x:%02x:%02x\n",
			   PCI_BUS(bridge), PCI_DEV(bridge), PCI_FUNC(bridge));

		}
	    }
	    /*
	     * Correct the prefetchable memory base, which is not set correctly by
	     * the U-Boot autoconfig stuff
	     */
	    if (prefetch_idx)
	    {
/* 		PRINTF("Setting prefetchable range to %x, %x (%x and %x)\n", */
/* 		       prefetch_addr, prefetch_addr+prefetch_size, */
/* 		       prefetch_addr>>16, (prefetch_addr+prefetch_size)>>16); */
/* 		pci_write_config_word(bridge, PCI_PREF_MEMORY_BASE, (prefetch_addr>>16)); */
/* 		pci_write_config_word(bridge, PCI_PREF_MEMORY_LIMIT, (prefetch_addr+prefetch_size)>>16); */
	    }

	    pci_write_config_word(bridge, PCI_PREF_MEMORY_BASE, 0x1000);
	    pci_write_config_word(bridge, PCI_PREF_MEMORY_LIMIT, 0x0000);

	    pci_write_config_byte(bridge, 0xD0, 0x0A);
	    pci_write_config_byte(bridge, 0xD3, 0x04);

	    /*
	     * Set the interrupt pin to 0
	     */
#if 0
	    pci_write_config_byte(dev, PCI_INTERRUPT_LINE, 0);
	    pci_write_config_byte(dev, PCI_INTERRUPT_PIN, 0);
#endif
	    pci_write_config_byte(bridge, PCI_INTERRUPT_LINE, 0);
	    pci_write_config_byte(bridge, PCI_INTERRUPT_PIN, 0);

	}
    }

    /* Finally, enable the card's IO and memory response */
    pci_write_config_dword(dev, PCI_COMMAND, PCI_COMMAND_MEMORY | PCI_COMMAND_IO);
    pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 0);
    pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0);

    return 1;
}

int find_image(u32 rom_address, u32 rom_size, void **image, u32 *image_size)
{
    int i = 0;
    unsigned char *rom = (unsigned char *)rom_address;
    /* if (*rom != 0x55 || *(rom+1) != 0xAA) return 0; /* No bios rom this is, yes. */ */

    for (;;)
    {
	unsigned short pci_data_offset = *(rom+0x18) + 256 * *(rom+0x19);
	unsigned short pci_image_length = (*(rom+pci_data_offset+0x10) + 256 * *(rom+pci_data_offset+0x11)) * 512;
	unsigned char pci_image_type = *(rom+pci_data_offset+0x14);
	if (*rom != 0x55 || *(rom+1) != 0xAA)
	{
	    PRINTF("Invalid header this is\n");
	    return 0;
	}
	PRINTF("Image %i: Type %d (%s)\n", i++, pci_image_type,
	       pci_image_type==0 ? "x86" :
	       pci_image_type==1 ? "OpenFirmware" :
	       "Unknown");
	if (pci_image_type == 0)
	{
	    *image = rom;
	    *image_size = pci_image_length;
	    return 1;
	}

	if (*(rom+pci_data_offset+0x15) & 0x80)
	{
	    PRINTF("LAST image encountered, no image found\n");
	    return 0;
	}

	rom += pci_image_length;
    }
}

void show_bat_mapping(void)
{
    u32 dbat0u, dbat0l, ibat0u, ibat0l;
    u32 dbat1u, dbat1l, ibat1u, ibat1l;
    u32 dbat2u, dbat2l, ibat2u, ibat2l;
    u32 dbat3u, dbat3l, ibat3u, ibat3l;
    u32 msr, hid0, l2cr_reg;

    __asm volatile ("mfdbatu %0,0" : "=r" (dbat0u));
    __asm volatile ("mfdbatl %0,0" : "=r" (dbat0l));
    __asm volatile ("mfibatu %0,0" : "=r" (ibat0u));
    __asm volatile ("mfibatl %0,0" : "=r" (ibat0l));

    __asm volatile ("mfdbatu %0,1" : "=r" (dbat1u));
    __asm volatile ("mfdbatl %0,1" : "=r" (dbat1l));
    __asm volatile ("mfibatu %0,1" : "=r" (ibat1u));
    __asm volatile ("mfibatl %0,1" : "=r" (ibat1l));

    __asm volatile ("mfdbatu %0,2" : "=r" (dbat2u));
    __asm volatile ("mfdbatl %0,2" : "=r" (dbat2l));
    __asm volatile ("mfibatu %0,2" : "=r" (ibat2u));
    __asm volatile ("mfibatl %0,2" : "=r" (ibat2l));

    __asm volatile ("mfdbatu %0,3" : "=r" (dbat3u));
    __asm volatile ("mfdbatl %0,3" : "=r" (dbat3l));
    __asm volatile ("mfibatu %0,3" : "=r" (ibat3u));
    __asm volatile ("mfibatl %0,3" : "=r" (ibat3l));

    __asm volatile ("mfmsr %0"     : "=r" (msr));
    __asm volatile ("mfspr %0,1008": "=r" (hid0));
    __asm volatile ("mfspr %0,1017": "=r" (l2cr_reg));

    printf("dbat0u: %08x dbat0l: %08x ibat0u: %08x ibat0l: %08x\n",
	   dbat0u, dbat0l, ibat0u, ibat0l);
    printf("dbat1u: %08x dbat1l: %08x ibat1u: %08x ibat1l: %08x\n",
	   dbat1u, dbat1l, ibat1u, ibat1l);
    printf("dbat2u: %08x dbat2l: %08x ibat2u: %08x ibat2l: %08x\n",
	   dbat2u, dbat2l, ibat2u, ibat2l);
    printf("dbat3u: %08x dbat3l: %08x ibat3u: %08x ibat3l: %08x\n",
	   dbat3u, dbat3l, ibat3u, ibat3l);

    printf("\nMSR: %08x   HID0: %08x   L2CR: %08x \n", msr,hid0, l2cr_reg);
}


void remove_init_data(void)
{
    char *s;

    /*  Invalidate and disable data cache */
    invalidate_l1_data_cache();
    dcache_disable();

    s = getenv("x86_cache");

    if (!s)
    {
	icache_enable();
	dcache_enable();
    }
    else if (s)
    {
	if (strcmp(s, "dcache")==0)
	{
	    dcache_enable();
	}
	else if (strcmp(s, "icache") == 0)
	{
	    icache_enable();
	}
	else if (strcmp(s, "on")== 0 || strcmp(s, "both") == 0)
	{
	    dcache_enable();
	    icache_enable();
	}
    }

    /*   show_bat_mapping();*/
}
