/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Wolfgang Grandegger, DENX Software Engineering, wg@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * PCI routines
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>

extern int cmd_get_data_size(char* arg, int default_size);

unsigned char	ShortPCIListing = 1;

/*
 * Follows routines for the output of infos about devices on PCI bus.
 */

void pci_header_show(pci_dev_t dev);
void pci_header_show_brief(pci_dev_t dev);

/*
 * Subroutine:  pciinfo
 *
 * Description: Show information about devices on PCI bus.
 *				Depending on the define CFG_SHORT_PCI_LISTING
 *				the output will be more or less exhaustive.
 *
 * Inputs:	bus_no		the number of the bus to be scanned.
 *
 * Return:      None
 *
 */
void pciinfo(int BusNum, int ShortPCIListing)
{
	int Device;
	int Function;
	unsigned char HeaderType;
	unsigned short VendorID;
	pci_dev_t dev;

	printf("Scanning PCI devices on bus %d\n", BusNum);

	if (ShortPCIListing) {
		printf("BusDevFun  VendorId   DeviceId   Device Class       Sub-Class\n");
		printf("_____________________________________________________________\n");
	}

	for (Device = 0; Device < PCI_MAX_PCI_DEVICES; Device++) {
		HeaderType = 0;
		VendorID = 0;
		for (Function = 0; Function < PCI_MAX_PCI_FUNCTIONS; Function++) {
			/*
			 * If this is not a multi-function device, we skip the rest.
			 */
			if (Function && !(HeaderType & 0x80))
				break;

			dev = PCI_BDF(BusNum, Device, Function);

			pci_read_config_word(dev, PCI_VENDOR_ID, &VendorID);
			if ((VendorID == 0xFFFF) || (VendorID == 0x0000))
				continue;

			if (!Function) pci_read_config_byte(dev, PCI_HEADER_TYPE, &HeaderType);

			if (ShortPCIListing)
			{
				printf("%02x.%02x.%02x   ", BusNum, Device, Function);
				pci_header_show_brief(dev);
			}
			else
			{
				printf("\nFound PCI device %02x.%02x.%02x:\n",
				       BusNum, Device, Function);
				pci_header_show(dev);
			}
	    }
    }
}

static char *pci_classes_str(u8 class)
{
	switch (class) {
	case PCI_CLASS_NOT_DEFINED:
		return "Build before PCI Rev2.0";
		break;
	case PCI_BASE_CLASS_STORAGE:
		return "Mass storage controller";
		break;
	case PCI_BASE_CLASS_NETWORK:
		return "Network controller";
		break;
	case PCI_BASE_CLASS_DISPLAY:
		return "Display controller";
		break;
	case PCI_BASE_CLASS_MULTIMEDIA:
		return "Multimedia device";
		break;
	case PCI_BASE_CLASS_MEMORY:
		return "Memory controller";
		break;
	case PCI_BASE_CLASS_BRIDGE:
		return "Bridge device";
		break;
	case PCI_BASE_CLASS_COMMUNICATION:
		return "Simple comm. controller";
		break;
	case PCI_BASE_CLASS_SYSTEM:
		return "Base system peripheral";
		break;
	case PCI_BASE_CLASS_INPUT:
		return "Input device";
		break;
	case PCI_BASE_CLASS_DOCKING:
		return "Docking station";
		break;
	case PCI_BASE_CLASS_PROCESSOR:
		return "Processor";
		break;
	case PCI_BASE_CLASS_SERIAL:
		return "Serial bus controller";
		break;
	case PCI_BASE_CLASS_INTELLIGENT:
		return "Intelligent controller";
		break;
	case PCI_BASE_CLASS_SATELLITE:
		return "Satellite controller";
		break;
	case PCI_BASE_CLASS_CRYPT:
		return "Cryptographic device";
		break;
	case PCI_BASE_CLASS_SIGNAL_PROCESSING:
		return "DSP";
		break;
	case PCI_CLASS_OTHERS:
		return "Does not fit any class";
		break;
	default:
	return  "???";
		break;
	};
}

/*
 * Subroutine:  pci_header_show_brief
 *
 * Description: Reads and prints the header of the
 *		specified PCI device in short form.
 *
 * Inputs:	dev      Bus+Device+Function number
 *
 * Return:      None
 *
 */
void pci_header_show_brief(pci_dev_t dev)
{
	u16 vendor, device;
	u8 class, subclass;

	pci_read_config_word(dev, PCI_VENDOR_ID, &vendor);
	pci_read_config_word(dev, PCI_DEVICE_ID, &device);
	pci_read_config_byte(dev, PCI_CLASS_CODE, &class);
	pci_read_config_byte(dev, PCI_CLASS_SUB_CODE, &subclass);

	printf("0x%.4x     0x%.4x     %-23s 0x%.2x\n",
	       vendor, device,
	       pci_classes_str(class), subclass);
}

/*
 * Subroutine:  PCI_Header_Show
 *
 * Description: Reads the header of the specified PCI device.
 *
 * Inputs:		BusDevFunc      Bus+Device+Function number
 *
 * Return:      None
 *
 */
void pci_header_show(pci_dev_t dev)
{
	u8 _byte, header_type;
	u16 _word;
	u32 _dword;

#define PRINT(msg, type, reg) \
	pci_read_config_##type(dev, reg, &_##type); \
	printf(msg, _##type)

#define PRINT2(msg, type, reg, func) \
	pci_read_config_##type(dev, reg, &_##type); \
	printf(msg, _##type, func(_##type))

	pci_read_config_byte(dev, PCI_HEADER_TYPE, &header_type);

	PRINT ("  vendor ID =                   0x%.4x\n", word, PCI_VENDOR_ID);
	PRINT ("  device ID =                   0x%.4x\n", word, PCI_DEVICE_ID);
	PRINT ("  command register =            0x%.4x\n", word, PCI_COMMAND);
	PRINT ("  status register =             0x%.4x\n", word, PCI_STATUS);
	PRINT ("  revision ID =                 0x%.2x\n", byte, PCI_REVISION_ID);
	PRINT2("  class code =                  0x%.2x (%s)\n", byte, PCI_CLASS_CODE,
								pci_classes_str);
	PRINT ("  sub class code =              0x%.2x\n", byte, PCI_CLASS_SUB_CODE);
	PRINT ("  programming interface =       0x%.2x\n", byte, PCI_CLASS_PROG);
	PRINT ("  cache line =                  0x%.2x\n", byte, PCI_CACHE_LINE_SIZE);
	PRINT ("  latency time =                0x%.2x\n", byte, PCI_LATENCY_TIMER);
	PRINT ("  header type =                 0x%.2x\n", byte, PCI_HEADER_TYPE);
	PRINT ("  BIST =                        0x%.2x\n", byte, PCI_BIST);
	PRINT ("  base address 0 =              0x%.8x\n", dword, PCI_BASE_ADDRESS_0);

	switch (header_type & 0x03) {
	case PCI_HEADER_TYPE_NORMAL:	/* "normal" PCI device */
		PRINT ("  base address 1 =              0x%.8x\n", dword, PCI_BASE_ADDRESS_1);
		PRINT ("  base address 2 =              0x%.8x\n", dword, PCI_BASE_ADDRESS_2);
		PRINT ("  base address 3 =              0x%.8x\n", dword, PCI_BASE_ADDRESS_3);
		PRINT ("  base address 4 =              0x%.8x\n", dword, PCI_BASE_ADDRESS_4);
		PRINT ("  base address 5 =              0x%.8x\n", dword, PCI_BASE_ADDRESS_5);
		PRINT ("  cardBus CIS pointer =         0x%.8x\n", dword, PCI_CARDBUS_CIS);
		PRINT ("  sub system vendor ID =        0x%.4x\n", word, PCI_SUBSYSTEM_VENDOR_ID);
		PRINT ("  sub system ID =               0x%.4x\n", word, PCI_SUBSYSTEM_ID);
		PRINT ("  expansion ROM base address =  0x%.8x\n", dword, PCI_ROM_ADDRESS);
		PRINT ("  interrupt line =              0x%.2x\n", byte, PCI_INTERRUPT_LINE);
		PRINT ("  interrupt pin =               0x%.2x\n", byte, PCI_INTERRUPT_PIN);
		PRINT ("  min Grant =                   0x%.2x\n", byte, PCI_MIN_GNT);
		PRINT ("  max Latency =                 0x%.2x\n", byte, PCI_MAX_LAT);
		break;

	case PCI_HEADER_TYPE_BRIDGE:	/* PCI-to-PCI bridge */

		PRINT ("  base address 1 =              0x%.8x\n", dword, PCI_BASE_ADDRESS_1);
		PRINT ("  primary bus number =          0x%.2x\n", byte, PCI_PRIMARY_BUS);
		PRINT ("  secondary bus number =        0x%.2x\n", byte, PCI_SECONDARY_BUS);
		PRINT ("  subordinate bus number =      0x%.2x\n", byte, PCI_SUBORDINATE_BUS);
		PRINT ("  secondary latency timer =     0x%.2x\n", byte, PCI_SEC_LATENCY_TIMER);
		PRINT ("  IO base =                     0x%.2x\n", byte, PCI_IO_BASE);
		PRINT ("  IO limit =                    0x%.2x\n", byte, PCI_IO_LIMIT);
		PRINT ("  secondary status =            0x%.4x\n", word, PCI_SEC_STATUS);
		PRINT ("  memory base =                 0x%.4x\n", word, PCI_MEMORY_BASE);
		PRINT ("  memory limit =                0x%.4x\n", word, PCI_MEMORY_LIMIT);
		PRINT ("  prefetch memory base =        0x%.4x\n", word, PCI_PREF_MEMORY_BASE);
		PRINT ("  prefetch memory limit =       0x%.4x\n", word, PCI_PREF_MEMORY_LIMIT);
		PRINT ("  prefetch memory base upper =  0x%.8x\n", dword, PCI_PREF_BASE_UPPER32);
		PRINT ("  prefetch memory limit upper = 0x%.8x\n", dword, PCI_PREF_LIMIT_UPPER32);
		PRINT ("  IO base upper 16 bits =       0x%.4x\n", word, PCI_IO_BASE_UPPER16);
		PRINT ("  IO limit upper 16 bits =      0x%.4x\n", word, PCI_IO_LIMIT_UPPER16);
		PRINT ("  expansion ROM base address =  0x%.8x\n", dword, PCI_ROM_ADDRESS1);
		PRINT ("  interrupt line =              0x%.2x\n", byte, PCI_INTERRUPT_LINE);
		PRINT ("  interrupt pin =               0x%.2x\n", byte, PCI_INTERRUPT_PIN);
		PRINT ("  bridge control =              0x%.4x\n", word, PCI_BRIDGE_CONTROL);
		break;

	case PCI_HEADER_TYPE_CARDBUS:	/* PCI-to-CardBus bridge */

		PRINT ("  capabilities =                0x%.2x\n", byte, PCI_CB_CAPABILITY_LIST);
		PRINT ("  secondary status =            0x%.4x\n", word, PCI_CB_SEC_STATUS);
		PRINT ("  primary bus number =          0x%.2x\n", byte, PCI_CB_PRIMARY_BUS);
		PRINT ("  CardBus number =              0x%.2x\n", byte, PCI_CB_CARD_BUS);
		PRINT ("  subordinate bus number =      0x%.2x\n", byte, PCI_CB_SUBORDINATE_BUS);
		PRINT ("  CardBus latency timer =       0x%.2x\n", byte, PCI_CB_LATENCY_TIMER);
		PRINT ("  CardBus memory base 0 =       0x%.8x\n", dword, PCI_CB_MEMORY_BASE_0);
		PRINT ("  CardBus memory limit 0 =      0x%.8x\n", dword, PCI_CB_MEMORY_LIMIT_0);
		PRINT ("  CardBus memory base 1 =       0x%.8x\n", dword, PCI_CB_MEMORY_BASE_1);
		PRINT ("  CardBus memory limit 1 =      0x%.8x\n", dword, PCI_CB_MEMORY_LIMIT_1);
		PRINT ("  CardBus IO base 0 =           0x%.4x\n", word, PCI_CB_IO_BASE_0);
		PRINT ("  CardBus IO base high 0 =      0x%.4x\n", word, PCI_CB_IO_BASE_0_HI);
		PRINT ("  CardBus IO limit 0 =          0x%.4x\n", word, PCI_CB_IO_LIMIT_0);
		PRINT ("  CardBus IO limit high 0 =     0x%.4x\n", word, PCI_CB_IO_LIMIT_0_HI);
		PRINT ("  CardBus IO base 1 =           0x%.4x\n", word, PCI_CB_IO_BASE_1);
		PRINT ("  CardBus IO base high 1 =      0x%.4x\n", word, PCI_CB_IO_BASE_1_HI);
		PRINT ("  CardBus IO limit 1 =          0x%.4x\n", word, PCI_CB_IO_LIMIT_1);
		PRINT ("  CardBus IO limit high 1 =     0x%.4x\n", word, PCI_CB_IO_LIMIT_1_HI);
		PRINT ("  interrupt line =              0x%.2x\n", byte, PCI_INTERRUPT_LINE);
		PRINT ("  interrupt pin =               0x%.2x\n", byte, PCI_INTERRUPT_PIN);
		PRINT ("  bridge control =              0x%.4x\n", word, PCI_CB_BRIDGE_CONTROL);
		PRINT ("  subvendor ID =                0x%.4x\n", word, PCI_CB_SUBSYSTEM_VENDOR_ID);
		PRINT ("  subdevice ID =                0x%.4x\n", word, PCI_CB_SUBSYSTEM_ID);
		PRINT ("  PC Card 16bit base address =  0x%.8x\n", dword, PCI_CB_LEGACY_MODE_BASE);
		break;

	default:
		printf("unknown header\n");
		break;
    }

#undef PRINT
#undef PRINT2
}

/* Convert the "bus.device.function" identifier into a number.
 */
static pci_dev_t get_pci_dev(char* name)
{
	char cnum[12];
	int len, i, iold, n;
	int bdfs[3] = {0,0,0};

	len = strlen(name);
	if (len > 8)
		return -1;
	for (i = 0, iold = 0, n = 0; i < len; i++) {
		if (name[i] == '.') {
			memcpy(cnum, &name[iold], i - iold);
			cnum[i - iold] = '\0';
			bdfs[n++] = simple_strtoul(cnum, NULL, 16);
			iold = i + 1;
		}
	}
	strcpy(cnum, &name[iold]);
	if (n == 0)
		n = 1;
	bdfs[n] = simple_strtoul(cnum, NULL, 16);
	return PCI_BDF(bdfs[0], bdfs[1], bdfs[2]);
}

static int pci_cfg_display(pci_dev_t bdf, ulong addr, ulong size, ulong length)
{
#define DISP_LINE_LEN	16
	ulong i, nbytes, linebytes;
	int rc = 0;

	if (length == 0)
		length = 0x40 / size; /* Standard PCI configuration space */

	/* Print the lines.
	 * once, and all accesses are with the specified bus width.
	 */
	nbytes = length * size;
	do {
		uint	val4;
		ushort  val2;
		u_char	val1;

		printf("%08lx:", addr);
		linebytes = (nbytes>DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;
		for (i=0; i<linebytes; i+= size) {
			if (size == 4) {
				pci_read_config_dword(bdf, addr, &val4);
				printf(" %08x", val4);
			} else if (size == 2) {
				pci_read_config_word(bdf, addr, &val2);
				printf(" %04x", val2);
			} else {
				pci_read_config_byte(bdf, addr, &val1);
				printf(" %02x", val1);
			}
			addr += size;
		}
		printf("\n");
		nbytes -= linebytes;
		if (ctrlc()) {
			rc = 1;
			break;
		}
	} while (nbytes > 0);

	return (rc);
}

static int pci_cfg_write (pci_dev_t bdf, ulong addr, ulong size, ulong value)
{
	if (size == 4) {
		pci_write_config_dword(bdf, addr, value);
	}
	else if (size == 2) {
		ushort val = value & 0xffff;
		pci_write_config_word(bdf, addr, val);
	}
	else {
		u_char val = value & 0xff;
		pci_write_config_byte(bdf, addr, val);
	}
	return 0;
}

static int
pci_cfg_modify (pci_dev_t bdf, ulong addr, ulong size, ulong value, int incrflag)
{
	ulong	i;
	int	nbytes;
	extern char console_buffer[];
	uint	val4;
	ushort  val2;
	u_char	val1;

	/* Print the address, followed by value.  Then accept input for
	 * the next value.  A non-converted value exits.
	 */
	do {
		printf("%08lx:", addr);
		if (size == 4) {
			pci_read_config_dword(bdf, addr, &val4);
			printf(" %08x", val4);
		}
		else if (size == 2) {
			pci_read_config_word(bdf, addr, &val2);
			printf(" %04x", val2);
		}
		else {
			pci_read_config_byte(bdf, addr, &val1);
			printf(" %02x", val1);
		}

		nbytes = readline (" ? ");
		if (nbytes == 0 || (nbytes == 1 && console_buffer[0] == '-')) {
			/* <CR> pressed as only input, don't modify current
			 * location and move to next. "-" pressed will go back.
			 */
			if (incrflag)
				addr += nbytes ? -size : size;
			nbytes = 1;
#ifdef CONFIG_BOOT_RETRY_TIME
			reset_cmd_timeout(); /* good enough to not time out */
#endif
		}
#ifdef CONFIG_BOOT_RETRY_TIME
		else if (nbytes == -2) {
			break;	/* timed out, exit the command	*/
		}
#endif
		else {
			char *endp;
			i = simple_strtoul(console_buffer, &endp, 16);
			nbytes = endp - console_buffer;
			if (nbytes) {
#ifdef CONFIG_BOOT_RETRY_TIME
				/* good enough to not time out
				 */
				reset_cmd_timeout();
#endif
				pci_cfg_write (bdf, addr, size, i);
				if (incrflag)
					addr += size;
			}
		}
	} while (nbytes);

	return 0;
}

/* PCI Configuration Space access commands
 *
 * Syntax:
 *	pci display[.b, .w, .l] bus.device.function} [addr] [len]
 *	pci next[.b, .w, .l] bus.device.function [addr]
 *      pci modify[.b, .w, .l] bus.device.function [addr]
 *      pci write[.b, .w, .l] bus.device.function addr value
 */
int do_pci (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong addr = 0, value = 0, size = 0;
	pci_dev_t bdf = 0;
	char cmd = 's';

	if (argc > 1)
		cmd = argv[1][0];

	switch (cmd) {
	case 'd':		/* display */
	case 'n':		/* next */
	case 'm':		/* modify */
	case 'w':		/* write */
		/* Check for a size specification. */
		size = cmd_get_data_size(argv[1], 4);
		if (argc > 3)
			addr = simple_strtoul(argv[3], NULL, 16);
		if (argc > 4)
			value = simple_strtoul(argv[4], NULL, 16);
	case 'h':		/* header */
		if (argc < 3)
			goto usage;
		if ((bdf = get_pci_dev(argv[2])) == -1)
			return 1;
		break;
	default:		/* scan bus */
		value = 1; /* short listing */
		bdf = 0;   /* bus number  */
		if (argc > 1) {
			if (argv[argc-1][0] == 'l') {
				value = 0;
				argc--;
			}
			if (argc > 1)
				bdf = simple_strtoul(argv[1], NULL, 16);
		}
		pciinfo(bdf, value);
		return 0;
	}

	switch (argv[1][0]) {
	case 'h':		/* header */
		pci_header_show(bdf);
		return 0;
	case 'd':		/* display */
		return pci_cfg_display(bdf, addr, size, value);
	case 'n':		/* next */
		if (argc < 4)
			goto usage;
		return pci_cfg_modify(bdf, addr, size, value, 0);
	case 'm':		/* modify */
		if (argc < 4)
			goto usage;
		return pci_cfg_modify(bdf, addr, size, value, 1);
	case 'w':		/* write */
		if (argc < 5)
			goto usage;
		return pci_cfg_write(bdf, addr, size, value);
	}

	return 1;
 usage:
	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

/***************************************************/


U_BOOT_CMD(
	pci,	5,	1,	do_pci,
	"pci     - list and access PCI Configuration Space\n",
	"[bus] [long]\n"
	"    - short or long list of PCI devices on bus 'bus'\n"
	"pci header b.d.f\n"
	"    - show header of PCI device 'bus.device.function'\n"
	"pci display[.b, .w, .l] b.d.f [address] [# of objects]\n"
	"    - display PCI configuration space (CFG)\n"
	"pci next[.b, .w, .l] b.d.f address\n"
	"    - modify, read and keep CFG address\n"
	"pci modify[.b, .w, .l] b.d.f address\n"
	"    -  modify, auto increment CFG address\n"
	"pci write[.b, .w, .l] b.d.f address value\n"
	"    - write to CFG address\n"
);
