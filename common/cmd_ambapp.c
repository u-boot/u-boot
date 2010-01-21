/*
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * AMBA Plug&Play information list command
 *
 */
#include <common.h>
#include <command.h>
#include <ambapp.h>

DECLARE_GLOBAL_DATA_PTR;

typedef struct {
	int device_id;
	char *name;
	char *desc;
} ambapp_device_name;

typedef struct {
	unsigned int vendor_id;
	char *name;
	char *desc;
	ambapp_device_name *devices;
} ambapp_vendor_devnames;

/** Vendor GAISLER devices */
static ambapp_device_name GAISLER_devices[] = {
	{GAISLER_LEON2DSU, "LEON2DSU", "Leon2 Debug Support Unit"},
	{GAISLER_LEON3, "LEON3", "Leon3 SPARC V8 Processor"},
	{GAISLER_LEON3DSU, "LEON3DSU", "Leon3 Debug Support Unit"},
	{GAISLER_ETHAHB, "ETHAHB", "OC ethernet AHB interface"},
	{GAISLER_APBMST, "APBMST", "AHB/APB Bridge"},
	{GAISLER_AHBUART, "AHBUART", "AHB Debug UART"},
	{GAISLER_SRCTRL, "SRCTRL", "Simple SRAM Controller"},
	{GAISLER_SDCTRL, "SDCTRL", "PC133 SDRAM Controller"},
	{GAISLER_SSRCTRL, "SSRCTRL", "Synchronous SRAM Controller"},
	{GAISLER_APBUART, "APBUART", "Generic UART"},
	{GAISLER_IRQMP, "IRQMP", "Multi-processor Interrupt Ctrl."},
	{GAISLER_AHBRAM, "AHBRAM", "Single-port AHB SRAM module"},
	{GAISLER_AHBDPRAM, "AHBDPRAM", "Dual-port AHB SRAM module"},
	{GAISLER_GPTIMER, "GPTIMER", "Modular Timer Unit"},
	{GAISLER_PCITRG, "PCITRG", "Simple 32-bit PCI Target"},
	{GAISLER_PCISBRG, "PCISBRG", "Simple 32-bit PCI Bridge"},
	{GAISLER_PCIFBRG, "PCIFBRG", "Fast 32-bit PCI Bridge"},
	{GAISLER_PCITRACE, "PCITRACE", "32-bit PCI Trace Buffer"},
	{GAISLER_DMACTRL, "DMACTRL", "AMBA DMA controller"},
	{GAISLER_AHBTRACE, "AHBTRACE", "AMBA Trace Buffer"},
	{GAISLER_DSUCTRL, "DSUCTRL", "DSU/ETH controller"},
	{GAISLER_CANAHB, "CANAHB", "OC CAN AHB interface"},
	{GAISLER_GPIO, "GPIO", "General Purpose I/O port"},
	{GAISLER_AHBROM, "AHBROM", "Generic AHB ROM"},
	{GAISLER_AHBJTAG, "AHBJTAG", "JTAG Debug Link"},
	{GAISLER_ETHMAC, "ETHMAC", "GR Ethernet MAC"},
	{GAISLER_SWNODE, "SWNODE", "SpaceWire Node Interface"},
	{GAISLER_SPW, "SPW", "SpaceWire Serial Link"},
	{GAISLER_AHB2AHB, "AHB2AHB", "AHB-to-AHB Bridge"},
	{GAISLER_USBDC, "USBDC", "GR USB 2.0 Device Controller"},
	{GAISLER_USB_DCL, "USB_DCL", "USB Debug Communication Link"},
	{GAISLER_DDRMP, "DDRMP", "Multi-port DDR controller"},
	{GAISLER_ATACTRL, "ATACTRL", "ATA controller"},
	{GAISLER_DDRSP, "DDRSP", "Single-port DDR266 controller"},
	{GAISLER_EHCI, "EHCI", "USB Enhanced Host Controller"},
	{GAISLER_UHCI, "UHCI", "USB Universal Host Controller"},
	{GAISLER_I2CMST, "I2CMST", "AMBA Wrapper for OC I2C-master"},
	{GAISLER_SPW2, "SPW2", "GRSPW2 SpaceWire Serial Link"},
	{GAISLER_AHBDMA, "AHBDMA", ""},
	{GAISLER_NUHOSP3, "NUHOSP3", "Nuhorizons Spartan3 IO I/F"},
	{GAISLER_CLKGATE, "CLKGATE", "Clock gating unit"},
	{GAISLER_SPICTRL, "SPICTRL", "SPI Controller"},
	{GAISLER_DDR2SP, "DDR2SP", "Single-port DDR2 controller"},
	{GAISLER_SLINK, "SLINK", "SLINK Master"},
	{GAISLER_GRTM, "GRTM", "CCSDS Telemetry Encoder"},
	{GAISLER_GRTC, "GRTC", "CCSDS Telecommand Decoder"},
	{GAISLER_GRPW, "GRPW", "PacketWire to AMBA AHB I/F"},
	{GAISLER_GRCTM, "GRCTM", "CCSDS Time Manager"},
	{GAISLER_GRHCAN, "GRHCAN", "ESA HurriCANe CAN with DMA"},
	{GAISLER_GRFIFO, "GRFIFO", "FIFO Controller"},
	{GAISLER_GRADCDAC, "GRADCDAC", "ADC / DAC Interface"},
	{GAISLER_GRPULSE, "GRPULSE", "General Purpose I/O with Pulses"},
	{GAISLER_GRTIMER, "GRTIMER", "Timer Unit with Latches"},
	{GAISLER_AHB2PP, "AHB2PP", "AMBA AHB to Packet Parallel I/F"},
	{GAISLER_GRVERSION, "GRVERSION", "Version and Revision Register"},
	{GAISLER_APB2PW, "APB2PW", "PacketWire Transmit Interface"},
	{GAISLER_PW2APB, "PW2APB", "PacketWire Receive Interface"},
	{GAISLER_GRCAN, "GRCAN", "CAN Controller with DMA"},
	{GAISLER_I2CSLV, "I2CSLV", "I2C Slave"},
	{GAISLER_U16550, "U16550", "Simple 16550 UART"},
	{GAISLER_AHBMST_EM, "AHBMST_EM", "AMBA Master Emulator"},
	{GAISLER_AHBSLV_EM, "AHBSLV_EM", "AMBA Slave Emulator"},
	{GAISLER_GRTESTMOD, "GRTESTMOD", "Test report module"},
	{GAISLER_ASCS, "ASCS", "ASCS Master"},
	{GAISLER_IPMVBCTRL, "IPMVBCTRL", "IPM-bus/MVBC memory controller"},
	{GAISLER_SPIMCTRL, "SPIMCTRL", "SPI Memory Controller"},
	{GAISLER_L4STAT, "L4STAT", "Leon4 Statistics Module"},
	{GAISLER_LEON4, "LEON4", "Leon4 SPARC V8 Processor"},
	{GAISLER_LEON4DSU, "LEON4DSU", "Leon4 Debug Support Unit"},
	{GAISLER_PWM, "PWM", "PWM generator"},
	{GAISLER_L2CACHE, "L2CACHE", "L2-Cache Controller"},
	{GAISLER_SDCTRL64, "SDCTRL64", "64-bit PC133 SDRAM Controller"},
	{GAISLER_GR1553B, "GR1553B", "MIL-STD-1553B Interface"},
	{GAISLER_1553TST, "1553TST", "MIL-STD-1553B Test Device"},
	{GAISLER_GRIOMMU, "GRIOMMU", "I/O Memory Management Unit"},
	{GAISLER_FTAHBRAM, "FTAHBRAM", "Generic FT AHB SRAM module"},
	{GAISLER_FTSRCTRL, "FTSRCTRL", "Simple FT SRAM Controller"},
	{GAISLER_AHBSTAT, "AHBSTAT", "AHB Status Register"},
	{GAISLER_LEON3FT, "LEON3FT", "Leon3-FT SPARC V8 Processor"},
	{GAISLER_FTMCTRL, "FTMCTRL", "Memory controller with EDAC"},
	{GAISLER_FTSDCTRL, "FTSDCTRL", "FT PC133 SDRAM Controller"},
	{GAISLER_FTSRCTRL8, "FTSRCTRL8", "FT 8-bit SRAM/16-bit IO Ctrl"},
	{GAISLER_MEMSCRUB, "MEMSCRUB", "AHB Memory Scrubber"},
	{GAISLER_FTSDCTRL64, "FTSDCTRL64", "64-bit FT SDRAM Controller"},
	{GAISLER_APBPS2, "APBPS2", "PS2 interface"},
	{GAISLER_VGACTRL, "VGACTRL", "VGA controller"},
	{GAISLER_LOGAN, "LOGAN", "On chip Logic Analyzer"},
	{GAISLER_SVGACTRL, "SVGACTRL", "SVGA frame buffer"},
	{GAISLER_T1AHB, "T1AHB", "Niagara T1 PCX/AHB bridge"},
	{GAISLER_MP7WRAP, "MP7WRAP", "CoreMP7 wrapper"},
	{GAISLER_GRSYSMON, "GRSYSMON", "AMBA wrapper for System Monitor"},
	{GAISLER_GRACECTRL, "GRACECTRL", "System ACE I/F Controller"},
	{GAISLER_ATAHBSLV, "ATAHBSLV", "AMBA Test Framework AHB Slave"},
	{GAISLER_ATAHBMST, "ATAHBMST", "AMBA Test Framework AHB Master"},
	{GAISLER_ATAPBSLV, "ATAPBSLV", "AMBA Test Framework APB Slave"},
	{GAISLER_B1553BC, "B1553BC", "AMBA Wrapper for Core1553BBC"},
	{GAISLER_B1553RT, "B1553RT", "AMBA Wrapper for Core1553BRT"},
	{GAISLER_B1553BRM, "B1553BRM", "AMBA Wrapper for Core1553BRM"},
	{GAISLER_AES, "AES", "Advanced Encryption Standard"},
	{GAISLER_ECC, "ECC", "Elliptic Curve Cryptography"},
	{GAISLER_PCIF, "PCIF", "AMBA Wrapper for CorePCIF"},
	{GAISLER_CLKMOD, "CLKMOD", "CPU Clock Switching Ctrl module"},
	{GAISLER_HAPSTRAK, "HAPSTRAK", "HAPS HapsTrak I/O Port"},
	{GAISLER_TEST_1X2, "TEST_1X2", "HAPS TEST_1x2 interface"},
	{GAISLER_WILD2AHB, "WILD2AHB", "WildCard CardBus interface"},
	{GAISLER_BIO1, "BIO1", "Basic I/O board BIO1"},
	{GAISLER_AESDMA, "AESDMA", "AES 256 DMA"},
	{GAISLER_SATCAN, "SATCAN", "SatCAN controller"},
	{GAISLER_CANMUX, "CANMUX", "CAN Bus multiplexer"},
	{GAISLER_GRTMRX, "GRTMRX", "CCSDS Telemetry Receiver"},
	{GAISLER_GRTCTX, "GRTCTX", "CCSDS Telecommand Transmitter"},
	{GAISLER_GRTMDESC, "GRTMDESC", "CCSDS Telemetry Descriptor"},
	{GAISLER_GRTMVC, "GRTMVC", "CCSDS Telemetry VC Generator"},
	{GAISLER_GEFFE, "GEFFE", "Geffe Generator"},
	{GAISLER_GPREG, "GPREG", "General Purpose Register"},
	{GAISLER_GRTMPAHB, "GRTMPAHB", "CCSDS Telemetry VC AHB Input"},
	{GAISLER_SPWCUC, "SPWCUC", "CCSDS CUC / SpaceWire I/F"},
	{GAISLER_SPW2_DMA, "SPW2_DMA", "GRSPW Router DMA interface"},
	{GAISLER_SPWROUTER, "SPWROUTER", "GRSPW Router"},
	{0, NULL, NULL}
};


/** Vendor PENDER devices */
static ambapp_device_name PENDER_devices[] = {
	{0, NULL, NULL}
};


/** Vendor ESA devices */
static ambapp_device_name ESA_devices[] = {
	{ESA_LEON2, "LEON2", "Leon2 SPARC V8 Processor"},
	{ESA_LEON2APB, "LEON2APB", "Leon2 Peripheral Bus"},
	{ESA_IRQ, "IRQ", "Leon2 Interrupt Controller"},
	{ESA_TIMER, "TIMER", "Leon2 Timer"},
	{ESA_UART, "UART", "Leon2 UART"},
	{ESA_CFG, "CFG", "Leon2 Configuration Register"},
	{ESA_IO, "IO", "Leon2 Input/Output"},
	{ESA_MCTRL, "MCTRL", "Leon2 Memory Controller"},
	{ESA_PCIARB, "PCIARB", "PCI Arbiter"},
	{ESA_HURRICANE, "HURRICANE", "HurriCANe/HurryAMBA CAN Ctrl"},
	{ESA_SPW_RMAP, "SPW_RMAP", "UoD/Saab SpaceWire/RMAP link"},
	{ESA_AHBUART, "AHBUART", "Leon2 AHB Debug UART"},
	{ESA_SPWA, "SPWA", "ESA/ASTRIUM SpaceWire link"},
	{ESA_BOSCHCAN, "BOSCHCAN", "SSC/BOSCH CAN Ctrl"},
	{ESA_IRQ2, "IRQ2", "Leon2 Secondary Irq Controller"},
	{ESA_AHBSTAT, "AHBSTAT", "Leon2 AHB Status Register"},
	{ESA_WPROT, "WPROT", "Leon2 Write Protection"},
	{ESA_WPROT2, "WPROT2", "Leon2 Extended Write Protection"},
	{ESA_PDEC3AMBA, "PDEC3AMBA", "ESA CCSDS PDEC3AMBA TC Decoder"},
	{ESA_PTME3AMBA, "PTME3AMBA", "ESA CCSDS PTME3AMBA TM Encoder"},
	{0, NULL, NULL}
};


/** Vendor ASTRIUM devices */
static ambapp_device_name ASTRIUM_devices[] = {
	{0, NULL, NULL}
};


/** Vendor OPENCHIP devices */
static ambapp_device_name OPENCHIP_devices[] = {
	{OPENCHIP_APBGPIO, "APBGPIO", "APB General Purpose IO"},
	{OPENCHIP_APBI2C, "APBI2C", "APB I2C Interface"},
	{OPENCHIP_APBSPI, "APBSPI", "APB SPI Interface"},
	{OPENCHIP_APBCHARLCD, "APBCHARLCD", "APB Character LCD"},
	{OPENCHIP_APBPWM, "APBPWM", "APB PWM"},
	{OPENCHIP_APBPS2, "APBPS2", "APB PS/2 Interface"},
	{OPENCHIP_APBMMCSD, "APBMMCSD", "APB MMC/SD Card Interface"},
	{OPENCHIP_APBNAND, "APBNAND", "APB NAND(SmartMedia) Interface"},
	{OPENCHIP_APBLPC, "APBLPC", "APB LPC Interface"},
	{OPENCHIP_APBCF, "APBCF", "APB CompactFlash (IDE)"},
	{OPENCHIP_APBSYSACE, "APBSYSACE", "APB SystemACE Interface"},
	{OPENCHIP_APB1WIRE, "APB1WIRE", "APB 1-Wire Interface"},
	{OPENCHIP_APBJTAG, "APBJTAG", "APB JTAG TAP Master"},
	{OPENCHIP_APBSUI, "APBSUI", "APB Simple User Interface"},
	{0, NULL, NULL}
};


/** Vendor OPENCORES devices */
static ambapp_device_name OPENCORES_devices[] = {
	{OPENCORES_PCIBR, "PCIBR", "PCI Bridge"},
	{OPENCORES_ETHMAC, "ETHMAC", "Ethernet MAC"},
	{0, NULL}
};


/** Vendor CONTRIB devices */
static ambapp_device_name CONTRIB_devices[] = {
	{CONTRIB_CORE1, "CORE1", "Contributed core 1"},
	{CONTRIB_CORE2, "CORE2", "Contributed core 2"},
	{0, NULL, NULL}
};


/** Vendor EONIC devices */
static ambapp_device_name EONIC_devices[] = {
	{0, NULL, NULL}
};


/** Vendor RADIONOR devices */
static ambapp_device_name RADIONOR_devices[] = {
	{0, NULL, NULL}
};


/** Vendor GLEICHMANN devices */
static ambapp_device_name GLEICHMANN_devices[] = {
	{GLEICHMANN_CUSTOM, "CUSTOM", "Custom device"},
	{GLEICHMANN_GEOLCD01, "GEOLCD01", "GEOLCD01 graphics system"},
	{GLEICHMANN_DAC, "DAC", "Sigma delta DAC"},
	{GLEICHMANN_HPI, "HPI", "AHB-to-HPI bridge"},
	{GLEICHMANN_SPI, "SPI", "SPI master"},
	{GLEICHMANN_HIFC, "HIFC", "Human interface controller"},
	{GLEICHMANN_ADCDAC, "ADCDAC", "Sigma delta ADC/DAC"},
	{GLEICHMANN_SPIOC, "SPIOC", ""},
	{GLEICHMANN_AC97, "AC97", ""},
	{0, NULL, NULL}
};


/** Vendor MENTA devices */
static ambapp_device_name MENTA_devices[] = {
	{0, NULL, NULL}
};


/** Vendor SUN devices */
static ambapp_device_name SUN_devices[] = {
	{SUN_T1, "T1", "Niagara T1 SPARC V9 Processor"},
	{SUN_S1, "S1", "Niagara S1 SPARC V9 Processor"},
	{0, NULL, NULL}
};


/** Vendor MOVIDIA devices */
static ambapp_device_name MOVIDIA_devices[] = {
	{0, NULL, NULL}
};


/** Vendor ORBITA devices */
static ambapp_device_name ORBITA_devices[] = {
	{ORBITA_1553B, "1553B", "MIL-STD-1553B Controller"},
	{ORBITA_429, "429", "429 Interface"},
	{ORBITA_SPI, "SPI", "SPI Interface"},
	{ORBITA_I2C, "I2C", "I2C Interface"},
	{ORBITA_SMARTCARD, "SMARTCARD", "Smart Card Reader"},
	{ORBITA_SDCARD, "SDCARD", "SD Card Reader"},
	{ORBITA_UART16550, "UART16550", "16550 UART"},
	{ORBITA_CRYPTO, "CRYPTO", "Crypto Engine"},
	{ORBITA_SYSIF, "SYSIF", "System Interface"},
	{ORBITA_PIO, "PIO", "Programmable IO module"},
	{ORBITA_RTC, "RTC", "Real-Time Clock"},
	{ORBITA_COLORLCD, "COLORLCD", "Color LCD Controller"},
	{ORBITA_PCI, "PCI", "PCI Module"},
	{ORBITA_DSP, "DSP", "DPS Co-Processor"},
	{ORBITA_USBHOST, "USBHOST", "USB Host"},
	{ORBITA_USBDEV, "USBDEV", "USB Device"},
	{0, NULL, NULL}
};


/** Vendor SYNOPSYS devices */
static ambapp_device_name SYNOPSYS_devices[] = {
	{0, NULL, NULL}
};


/** Vendor NASA devices */
static ambapp_device_name NASA_devices[] = {
	{NASA_EP32, "EP32", "EP32 Forth processor"},
	{0, NULL, NULL}
};


/** Vendor CAL devices */
static ambapp_device_name CAL_devices[] = {
	{CAL_DDRCTRL, "DDRCTRL", ""},
	{0, NULL, NULL}
};


/** Vendor EMBEDDIT devices */
static ambapp_device_name EMBEDDIT_devices[] = {
	{0, NULL, NULL}
};


/** Vendor CETON devices */
static ambapp_device_name CETON_devices[] = {
	{0, NULL, NULL}
};


/** Vendor S3 devices */
static ambapp_device_name S3_devices[] = {
	{0, NULL, NULL}
};


/** Vendor ACTEL devices */
static ambapp_device_name ACTEL_devices[] = {
	{ACTEL_COREMP7, "COREMP7", "CoreMP7 Processor"},
	{0, NULL, NULL}
};


/** Vendor APPLECORE devices */
static ambapp_device_name APPLECORE_devices[] = {
	{APPLECORE_UTLEON3, "UTLEON3", "AppleCore uT-LEON3 Processor"},
	{APPLECORE_UTLEON3DSU, "UTLEON3DSU", "AppleCore uT-LEON3 DSU"},
	{0, NULL, NULL}
};


/** Vendors and their devices */
static ambapp_vendor_devnames vendors[] = {
	{VENDOR_GAISLER, "GAISLER", "Gaisler Research", GAISLER_devices},
	{VENDOR_PENDER, "PENDER", "", PENDER_devices},
	{VENDOR_ESA, "ESA", "European Space Agency", ESA_devices},
	{VENDOR_ASTRIUM, "ASTRIUM", "", ASTRIUM_devices},
	{VENDOR_OPENCHIP, "OPENCHIP", "OpenChip", OPENCHIP_devices},
	{VENDOR_OPENCORES, "OPENCORES", "OpenCores", OPENCORES_devices},
	{VENDOR_CONTRIB, "CONTRIB", "Various contributions", CONTRIB_devices},
	{VENDOR_EONIC, "EONIC", "Eonic BV", EONIC_devices},
	{VENDOR_RADIONOR, "RADIONOR", "Radionor Communications", RADIONOR_devices},
	{VENDOR_GLEICHMANN, "GLEICHMANN", "Gleichmann Electronics", GLEICHMANN_devices},
	{VENDOR_MENTA, "MENTA", "Menta", MENTA_devices},
	{VENDOR_SUN, "SUN", "Sun Microsystems", SUN_devices},
	{VENDOR_MOVIDIA, "MOVIDIA", "", MOVIDIA_devices},
	{VENDOR_ORBITA, "ORBITA", "Orbita", ORBITA_devices},
	{VENDOR_SYNOPSYS, "SYNOPSYS", "Synopsys Inc.", SYNOPSYS_devices},
	{VENDOR_NASA, "NASA", "NASA", NASA_devices},
	{VENDOR_S3, "S3", "S3 Group", S3_devices},
	{VENDOR_CAL, "CAL", "", CAL_devices},
	{VENDOR_EMBEDDIT, "EMBEDDIT", "Embedd.it", EMBEDDIT_devices},
	{VENDOR_CETON, "CETON", "Ceton Corporation", CETON_devices},
	{VENDOR_ACTEL, "ACTEL", "Actel Corporation", ACTEL_devices},
	{VENDOR_APPLECORE, "APPLECORE", "AppleCore", APPLECORE_devices},
	{0, NULL, NULL, NULL}
};

static ambapp_device_name *ambapp_get_dev(ambapp_device_name *devs, int id)
{
	if (!devs)
		return NULL;

	while (devs->device_id > 0) {
		if (devs->device_id == id)
			return devs;
		devs++;
	}
	return NULL;
}

char *ambapp_device_id2str(int vendor, int id)
{
	ambapp_vendor_devnames *ven = &vendors[0];
	ambapp_device_name *dev;

	while (ven->vendor_id > 0) {
		if (ven->vendor_id == vendor) {
			dev = ambapp_get_dev(ven->devices, id);
			if (!dev)
				return NULL;
			return dev->name;
		}
		ven++;
	}
	return NULL;
}

char *ambapp_device_id2desc(int vendor, int id)
{
	ambapp_vendor_devnames *ven = &vendors[0];
	ambapp_device_name *dev;

	while (ven->vendor_id > 0) {
		if (ven->vendor_id == vendor) {
			dev = ambapp_get_dev(ven->devices, id);
			if (!dev)
				return NULL;
			return dev->desc;
		}
		ven++;
	}
	return NULL;
}

char *ambapp_vendor_id2str(int vendor)
{
	ambapp_vendor_devnames *ven = &vendors[0];

	while (ven->vendor_id > 0) {
		if (ven->vendor_id == vendor) {
			return ven->name;
		}
		ven++;
	}
	return NULL;
}

static char *unknown = "unknown";

char *ambapp_type_names[4] = {
	/* 0 */ "UNUSED",
	/* 1 */ "apb",
	/* 2 */ "ahbmem",
	/* 3 */ "ahbio"
};

/* Print one APB device */
void ambapp_print_apb(ambapp_apbdev *dev, int index)
{
	char *dev_str, *ven_str;
	unsigned int freq;

	ven_str = ambapp_vendor_id2str(dev->vendor);
	if (!ven_str) {
		ven_str = unknown;
		dev_str = unknown;
	} else {
		dev_str = ambapp_device_id2str(dev->vendor, dev->device);
		if (!dev_str)
			dev_str = unknown;
	}

	/* Get Frequency of Core */
	freq = ambapp_bus_freq(&ambapp_plb, dev->ahb_bus_index);

	printf("0x%02x:0x%02x:0x%02x: %s  %s  (%dkHz)\n"
	       "   apb: 0x%08x - 0x%08x\n"
	       "   irq: %-2d (ver: %-2d)\n",
	       index, dev->vendor, dev->device, ven_str, dev_str, freq / 1000,
	       dev->address, dev->address + (dev->mask-1),
	       dev->irq, dev->ver);
}

void ambapp_print_ahb(ambapp_ahbdev *dev, int index)
{
	char *dev_str, *ven_str, *type_str;
	int i;
	unsigned int freq;

	ven_str = ambapp_vendor_id2str(dev->vendor);
	if (!ven_str) {
		ven_str = unknown;
		dev_str = unknown;
	} else {
		dev_str = ambapp_device_id2str(dev->vendor, dev->device);
		if (!dev_str)
			dev_str = unknown;
	}

	/* Get Frequency of Core */
	freq = ambapp_bus_freq(&ambapp_plb, dev->ahb_bus_index);

	printf("0x%02x:0x%02x:0x%02x: %s  %s  (%dkHz)\n",
	       index, dev->vendor, dev->device, ven_str, dev_str, freq / 1000);

	for (i = 0; i < 4; i++) {
		if (dev->type[i] == 0)
			continue;
		type_str = ambapp_type_names[dev->type[i]];
		printf("   %-7s: 0x%08x - 0x%08x\n", type_str, dev->address[i],
			dev->address[i] + (dev->mask[i]-1));
	}

	printf("   irq: %-2d (ver: %d)\n", dev->irq, dev->ver);
}

int do_ambapp_print(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int index;
	ambapp_apbdev apbdev;
	ambapp_ahbdev ahbdev;

	/* Print AHB Masters */
	puts("\n--------- AHB Masters ---------\n");
	index = 0;
	while (ambapp_ahbmst_find(&ambapp_plb, 0, 0, index, &ahbdev) == 1) {
		/* Found a AHB Master Device */
		ambapp_print_ahb(&ahbdev, index);
		index++;
	}

	/* Print AHB Slaves */
	puts("\n--------- AHB Slaves  ---------\n");
	index = 0;
	while (ambapp_ahbslv_find(&ambapp_plb, 0, 0, index, &ahbdev) == 1) {
		/* Found a AHB Slave Device */
		ambapp_print_ahb(&ahbdev, index);
		index++;
	}

	/* Print APB Slaves */
	puts("\n--------- APB Slaves  ---------\n");
	index = 0;
	while (ambapp_apb_find(&ambapp_plb, 0, 0, index, &apbdev) == 1) {
		/* Found a APB Slave Device */
		ambapp_print_apb(&apbdev, index);
		index++;
	}

	puts("\n");
	return 0;
}

int ambapp_init_reloc(void)
{
	ambapp_vendor_devnames *vend = vendors;
	ambapp_device_name *dev;

	while (vend->vendor_id && vend->name) {
		vend->name = (char *)((unsigned int)vend->name + gd->reloc_off);
		vend->desc = (char *)((unsigned int)vend->desc + gd->reloc_off);
		vend->devices = (ambapp_device_name *)
			((unsigned int)vend->devices + gd->reloc_off);
		dev = vend->devices;
		vend++;
		if (!dev)
			continue;
		while (dev->device_id && dev->name) {
			dev->name =
			    (char *)((unsigned int)dev->name + gd->reloc_off);
			dev->desc =
			    (char *)((unsigned int)dev->desc + gd->reloc_off);
			dev++;
		}
	}
	return 0;
}

U_BOOT_CMD(
	ambapp, 1, 1, do_ambapp_print,
	"list AMBA Plug&Play information",
	"ambapp\n"
	"    - lists AMBA (AHB & APB) Plug&Play devices present on the system"
);
