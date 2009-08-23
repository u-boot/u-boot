/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB
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
 *
 ********************************************************************
 *
 * Lots of code copied from:
 *
 * m8xx_pcmcia.c - Linux PCMCIA socket driver for the mpc8xx series.
 * (C) 1999-2000 Magnus Damm <damm@bitsmart.com>
 *
 * "The ExCA standard specifies that socket controllers should provide
 * two IO and five memory windows per socket, which can be independently
 * configured and positioned in the host address space and mapped to
 * arbitrary segments of card address space. " - David A Hinds. 1999
 *
 * This controller does _not_ meet the ExCA standard.
 *
 * m8xx pcmcia controller brief info:
 * + 8 windows (attrib, mem, i/o)
 * + up to two slots (SLOT_A and SLOT_B)
 * + inputpins, outputpins, event and mask registers.
 * - no offset register. sigh.
 *
 * Because of the lacking offset register we must map the whole card.
 * We assign each memory window PCMCIA_MEM_WIN_SIZE address space.
 * Make sure there is (PCMCIA_MEM_WIN_SIZE * PCMCIA_MEM_WIN_NO
 * * PCMCIA_SOCKETS_NO) bytes at PCMCIA_MEM_WIN_BASE.
 * The i/o windows are dynamically allocated at PCMCIA_IO_WIN_BASE.
 * They are maximum 64KByte each...
 */


#undef DEBUG		/**/

/*
 * PCMCIA support
 */
#include <common.h>
#include <command.h>
#include <config.h>
#include <pci.h>
#include <asm/io.h>

#include <pcmcia.h>

#if defined(CONFIG_CMD_PCMCIA)

int pcmcia_on(int ide_base_bus);

static int  hardware_disable(int slot);
static int  hardware_enable(int slot);
static int  voltage_set(int slot, int vcc, int vpp);
static void print_funcid(int func);
static void print_fixed(volatile char *p);
static int  identify(volatile char *p);
static int  check_ide_device(int slot, int ide_base_bus);


/* ------------------------------------------------------------------------- */


const char *indent = "\t   ";

/* ------------------------------------------------------------------------- */


static struct pci_device_id supported[] = {
	{ PCI_VENDOR_ID_TI, 0xac50 }, /* Ti PCI1410A */
	{ PCI_VENDOR_ID_TI, 0xac56 }, /* Ti PCI1510 */
	{ }
};

static pci_dev_t devbusfn;
static u32 socket_base;
static u32 pcmcia_cis_ptr;

int pcmcia_on(int ide_base_bus)
{
	u16 dev_id;
	u32 socket_status;
	int slot = 0;
	int cis_len;
	u16 io_base;
	u16 io_len;

	/*
	 * Find the CardBus PCI device(s).
	 */
	if ((devbusfn = pci_find_devices(supported, 0)) < 0) {
		printf("Ti CardBus: not found\n");
		return 1;
	}

	pci_read_config_word(devbusfn, PCI_DEVICE_ID, &dev_id);

	if (dev_id == 0xac56) {
		debug("Enable PCMCIA Ti PCI1510\n");
	} else {
		debug("Enable PCMCIA Ti PCI1410A\n");
	}

	pcmcia_cis_ptr = CONFIG_SYS_PCMCIA_CIS_WIN;
	cis_len = CONFIG_SYS_PCMCIA_CIS_WIN_SIZE;

	io_base = CONFIG_SYS_PCMCIA_IO_WIN;
	io_len = CONFIG_SYS_PCMCIA_IO_WIN_SIZE;

	/*
	 * Setup the PCI device.
	 */
	pci_read_config_dword(devbusfn, PCI_BASE_ADDRESS_0, &socket_base);
	socket_base &= ~0xf;

	socket_status = readl(socket_base+8);
	if ((socket_status & 6) == 0) {
		printf("Card Present: ");

		switch (socket_status & 0x3c00) {

		case 0x400:
			printf("5V ");
			break;
		case 0x800:
			printf("3.3V ");
			break;
		case 0xc00:
			printf("3.3/5V ");
			break;
		default:
			printf("unsupported Vcc ");
			break;
		}
		switch (socket_status & 0x30) {
		case 0x10:
			printf("16bit PC-Card\n");
			break;
		case 0x20:
			printf("32bit CardBus Card\n");
			break;
		default:
			printf("8bit PC-Card\n");
			break;
		}
	}


	writeb(0x41, socket_base + 0x806); /* Enable I/O window 0 and memory window 0 */
	writeb(0x0e, socket_base + 0x807); /* Reset I/O window options */

	/* Careful: the linux yenta driver do not seem to reset the offset
	 * in the i/o windows, so leaving them non-zero is a problem */

	writeb(io_base & 0xff, socket_base + 0x808); /* I/O window 0 base address */
	writeb(io_base>>8, socket_base + 0x809);
	writeb((io_base + io_len - 1) & 0xff, socket_base + 0x80a); /* I/O window 0 end address */
	writeb((io_base + io_len - 1)>>8, socket_base + 0x80b);
	writeb(0x00, socket_base + 0x836);      /* I/O window 0 offset address 0x000 */
	writeb(0x00, socket_base + 0x837);


	writeb((pcmcia_cis_ptr&0x000ff000) >> 12,
	       socket_base + 0x810); /* Memory window 0 start address bits 19-12 */
	writeb((pcmcia_cis_ptr&0x00f00000) >> 20,
	       socket_base + 0x811);  /* Memory window 0 start address bits 23-20 */
	writeb(((pcmcia_cis_ptr+cis_len-1) & 0x000ff000) >> 12,
		socket_base + 0x812); /* Memory window 0 end address bits 19-12*/
	writeb(((pcmcia_cis_ptr+cis_len-1) & 0x00f00000) >> 20,
		socket_base + 0x813); /* Memory window 0 end address bits 23-20*/
	writeb(0x00, socket_base + 0x814); /* Memory window 0 offset bits 19-12 */
	writeb(0x40, socket_base + 0x815); /* Memory window 0 offset bits 23-20 and
					    * options (read/write, attribute access) */
	writeb(0x00, socket_base + 0x816); /* ExCA card-detect and general control  */
	writeb(0x00, socket_base + 0x81e); /* ExCA global control (interrupt modes) */

	writeb((pcmcia_cis_ptr & 0xff000000) >> 24,
	       socket_base + 0x840); /* Memory window address bits 31-24 */


	/* turn off voltage */
	if (voltage_set(slot, 0, 0)) {
		return 1;
	}

	/* Enable external hardware */
	if (hardware_enable(slot)) {
		return 1;
	}

	if (check_ide_device(slot, ide_base_bus)) {
		return 1;
	}

	return 0;
}

/* ------------------------------------------------------------------------- */


#if defined(CONFIG_CMD_PCMCIA)
int pcmcia_off (void)
{
	int slot = 0;

	writeb(0x00, socket_base + 0x806); /* disable all I/O and memory windows */

	writeb(0x00, socket_base + 0x808); /* I/O window 0 base address */
	writeb(0x00, socket_base + 0x809);
	writeb(0x00, socket_base + 0x80a); /* I/O window 0 end address */
	writeb(0x00, socket_base + 0x80b);
	writeb(0x00, socket_base + 0x836); /* I/O window 0 offset address  */
	writeb(0x00, socket_base + 0x837);

	writeb(0x00, socket_base + 0x80c); /* I/O window 1 base address  */
	writeb(0x00, socket_base + 0x80d);
	writeb(0x00, socket_base + 0x80e); /* I/O window 1 end address  */
	writeb(0x00, socket_base + 0x80f);
	writeb(0x00, socket_base + 0x838); /* I/O window 1 offset address  */
	writeb(0x00, socket_base + 0x839);

	writeb(0x00, socket_base + 0x810); /* Memory window 0 start address */
	writeb(0x00, socket_base + 0x811);
	writeb(0x00, socket_base + 0x812); /* Memory window 0 end address  */
	writeb(0x00, socket_base + 0x813);
	writeb(0x00, socket_base + 0x814); /* Memory window 0 offset */
	writeb(0x00, socket_base + 0x815);

	writeb(0xc0, socket_base + 0x840); /* Memory window 0 page address */


	/* turn off voltage */
	voltage_set(slot, 0, 0);

	/* disable external hardware */
	printf ("Shutdown and Poweroff Ti PCI1410A\n");
	hardware_disable(slot);

	return 0;
}

#endif

/* ------------------------------------------------------------------------- */


#define	MAX_TUPEL_SZ	512
#define MAX_FEATURES	4
int ide_devices_found;
static int check_ide_device(int slot, int ide_base_bus)
{
	volatile char *ident = NULL;
	volatile char *feature_p[MAX_FEATURES];
	volatile char *p, *start;
	int n_features = 0;
	uchar func_id = ~0;
	uchar code, len;
	ushort config_base = 0;
	int found = 0;
	int i;
	u32 socket_status;

	debug ("PCMCIA MEM: %08X\n", pcmcia_cis_ptr);

	socket_status = readl(socket_base+8);

	if ((socket_status & 6) != 0 || (socket_status & 0x20) != 0) {
		printf("no card or CardBus card\n");
		return 1;
	}

	start = p = (volatile char *) pcmcia_cis_ptr;

	while ((p - start) < MAX_TUPEL_SZ) {

		code = *p; p += 2;

		if (code == 0xFF) { /* End of chain */
			break;
		}

		len = *p; p += 2;
#if defined(DEBUG) && (DEBUG > 1)
		{
			volatile uchar *q = p;
			printf ("\nTuple code %02x  length %d\n\tData:",
				code, len);

			for (i = 0; i < len; ++i) {
				printf (" %02x", *q);
				q+= 2;
			}
		}
#endif	/* DEBUG */
		switch (code) {
		case CISTPL_VERS_1:
			ident = p + 4;
			break;
		case CISTPL_FUNCID:
			/* Fix for broken SanDisk which may have 0x80 bit set */
			func_id = *p & 0x7F;
			break;
		case CISTPL_FUNCE:
			if (n_features < MAX_FEATURES)
				feature_p[n_features++] = p;
			break;
		case CISTPL_CONFIG:
			config_base = (*(p+6) << 8) + (*(p+4));
			debug ("\n## Config_base = %04x ###\n", config_base);
		default:
			break;
		}
		p += 2 * len;
	}

	found = identify(ident);

	if (func_id != ((uchar)~0)) {
		print_funcid (func_id);

		if (func_id == CISTPL_FUNCID_FIXED)
			found = 1;
		else
			return 1;	/* no disk drive */
	}

	for (i=0; i<n_features; ++i) {
		print_fixed(feature_p[i]);
	}

	if (!found) {
		printf("unknown card type\n");
		return 1;
	}

	/* select config index 1 */
	writeb(1, pcmcia_cis_ptr + config_base);

#if 0
	printf("Confiuration Option Register: %02x\n", readb(pcmcia_cis_ptr + config_base));
	printf("Card Confiuration and Status Register: %02x\n", readb(pcmcia_cis_ptr + config_base + 2));
	printf("Pin Replacement Register Register: %02x\n", readb(pcmcia_cis_ptr + config_base + 4));
	printf("Socket and Copy Register: %02x\n", readb(pcmcia_cis_ptr + config_base + 6));
#endif
	ide_devices_found |= (1 << (slot+ide_base_bus));

	return 0;
}


static int voltage_set(int slot, int vcc, int vpp)
{
	u32 socket_control;
	int reg=0;

	switch (slot) {
	case 0:
		reg = socket_base + 0x10;
		break;
	default:
		return 1;
	}

	socket_control = 0;


	switch (vcc) {
	case 50:
		socket_control |= 0x20;
		break;
	case 33:
		socket_control |= 0x30;
		break;
	case 0:
	default: ;
	}

	switch (vpp) {
	case 120:
		socket_control |= 0x1;
		break;
	case 50:
		socket_control |= 0x2;
		break;
	case 33:
		socket_control |= 0x3;
		break;
	case 0:
	default: ;
	}

	writel(socket_control, reg);

	debug ("voltage_set: Ti PCI1410A Slot %d, Vcc=%d.%d, Vpp=%d.%d\n",
		slot, vcc/10, vcc%10, vpp/10, vpp%10);

	udelay(500);
	return 0;
}


static int hardware_enable(int slot)
{
	u32 socket_status;
	u16 brg_ctrl;
	int is_82365sl;

	socket_status = readl(socket_base+8);

	if ((socket_status & 6) == 0) {

		switch (socket_status & 0x3c00) {

		case 0x400:
			printf("5V ");
			voltage_set(slot, 50, 0);
			break;
		case 0x800:
			voltage_set(slot, 33, 0);
			break;
		case 0xc00:
			voltage_set(slot, 33, 0);
			break;
		default:
			voltage_set(slot, 0, 0);
			break;
		}
	} else {
		voltage_set(slot, 0, 0);
	}

	pci_read_config_word(devbusfn, PCI_BRIDGE_CONTROL, &brg_ctrl);
	brg_ctrl &= ~PCI_BRIDGE_CTL_BUS_RESET;
	pci_write_config_word(devbusfn, PCI_BRIDGE_CONTROL, brg_ctrl);
	is_82365sl = ((readb(socket_base+0x800) & 0x0f) == 2);
	writeb(is_82365sl?0x90:0x98, socket_base+0x802);
	writeb(0x67, socket_base+0x803);
	udelay(100000);
#if 0
	printf("ExCA Id %02x, Card Status %02x, Power config %02x, Interrupt Config %02x, bridge control %04x %d\n",
	       readb(socket_base+0x800), readb(socket_base+0x801),
	       readb(socket_base+0x802), readb(socket_base+0x803), brg_ctrl, is_82365sl);
#endif

	return ((readb(socket_base+0x801)&0x6c)==0x6c)?0:1;
}


static int hardware_disable(int slot)
{
	voltage_set(slot, 0, 0);
	return 0;
}

static void print_funcid(int func)
{
	puts(indent);
	switch (func) {
	case CISTPL_FUNCID_MULTI:
		puts(" Multi-Function");
		break;
	case CISTPL_FUNCID_MEMORY:
		puts(" Memory");
		break;
	case CISTPL_FUNCID_SERIAL:
		puts(" Serial Port");
		break;
	case CISTPL_FUNCID_PARALLEL:
		puts(" Parallel Port");
		break;
	case CISTPL_FUNCID_FIXED:
		puts(" Fixed Disk");
		break;
	case CISTPL_FUNCID_VIDEO:
		puts(" Video Adapter");
		break;
	case CISTPL_FUNCID_NETWORK:
		puts(" Network Adapter");
		break;
	case CISTPL_FUNCID_AIMS:
		puts(" AIMS Card");
		break;
	case CISTPL_FUNCID_SCSI:
		puts(" SCSI Adapter");
		break;
	default:
		puts(" Unknown");
		break;
	}
	puts(" Card\n");
}

/* ------------------------------------------------------------------------- */

static void print_fixed(volatile char *p)
{
	if (p == NULL)
		return;

	puts(indent);

	switch (*p) {
	case CISTPL_FUNCE_IDE_IFACE:
		{   uchar iface = *(p+2);

			puts ((iface == CISTPL_IDE_INTERFACE) ? " IDE" : " unknown");
			puts (" interface ");
			break;
		}
	case CISTPL_FUNCE_IDE_MASTER:
	case CISTPL_FUNCE_IDE_SLAVE:
		{
			uchar f1 = *(p+2);
			uchar f2 = *(p+4);

			puts((f1 & CISTPL_IDE_SILICON) ? " [silicon]" : " [rotating]");

			if (f1 & CISTPL_IDE_UNIQUE) {
				puts(" [unique]");
			}

			puts((f1 & CISTPL_IDE_DUAL) ? " [dual]" : " [single]");

			if (f2 & CISTPL_IDE_HAS_SLEEP) {
				puts(" [sleep]");
			}

			if (f2 & CISTPL_IDE_HAS_STANDBY) {
				puts(" [standby]");
			}

			if (f2 & CISTPL_IDE_HAS_IDLE) {
				puts(" [idle]");
			}

			if (f2 & CISTPL_IDE_LOW_POWER) {
				puts(" [low power]");
			}

			if (f2 & CISTPL_IDE_REG_INHIBIT) {
				puts(" [reg inhibit]");
			}

			if (f2 & CISTPL_IDE_HAS_INDEX) {
				puts(" [index]");
			}

			if (f2 & CISTPL_IDE_IOIS16) {
				puts(" [IOis16]");
			}

			break;
		}
	}
	putc('\n');
}

/* ------------------------------------------------------------------------- */

#define MAX_IDENT_CHARS		64
#define	MAX_IDENT_FIELDS	4

static char *known_cards[] = {
	"ARGOSY PnPIDE D5",
	NULL
};

static int identify(volatile char *p)
{
	char id_str[MAX_IDENT_CHARS];
	char data;
	char *t;
	char **card;
	int i, done;

	if (p == NULL)
		return (0);	/* Don't know */

	t = id_str;
	done =0;

	for (i=0; i<=4 && !done; ++i, p+=2) {
		while ((data = *p) != '\0') {
			if (data == 0xFF) {
				done = 1;
				break;
			}
			*t++ = data;
			if (t == &id_str[MAX_IDENT_CHARS-1]) {
				done = 1;
				break;
			}
			p += 2;
		}
		if (!done)
			*t++ = ' ';
	}
	*t = '\0';
	while (--t > id_str) {
		if (*t == ' ') {
			*t = '\0';
		} else {
			break;
		}
	}
	puts(id_str);
	putc('\n');

	for (card=known_cards; *card; ++card) {
		debug ("## Compare against \"%s\"\n", *card);
		if (strcmp(*card, id_str) == 0) {	/* found! */
			debug ("## CARD FOUND ##\n");
			return 1;
		}
	}

	return 0;	/* don't know */
}

#endif /* CONFIG_CMD_PCMCIA */
