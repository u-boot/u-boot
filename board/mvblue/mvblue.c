/*
 * GNU General Public License for more details.
 *
 * MATRIX Vision GmbH / June 2002-Nov 2003
 * Andre Schwarz
 */

#include <common.h>
#include <mpc824x.h>
#include <asm/io.h>
#include <ns16550.h>

#ifdef CONFIG_PCI
#include <pci.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

u32 get_BoardType (void);

#define PCI_CONFIG(b,d,f,r)    cpu_to_le32(0x80000000 | ((b&0xff)<<16) \
						      | ((d&0x1f)<<11) \
						      | ((f&0x7)<<7)   \
						      | (r&0xfc) )

int mv_pci_read (int bus, int dev, int func, int reg)
{
	*(u32 *) (0xfec00cf8) = PCI_CONFIG (bus, dev, func, reg);
	asm ("sync");
	return cpu_to_le32 (*(u32 *) (0xfee00cfc));
}

u32 get_BoardType ()
{
	return (mv_pci_read (0, 0xe, 0, 0) == 0x06801095 ? 0 : 1);
}

void init_2nd_DUART (void)
{
	NS16550_t console = (NS16550_t) CFG_NS16550_COM2;
	int clock_divisor = CFG_NS16550_CLK / 16 / CONFIG_BAUDRATE;

	*(u8 *) (0xfc004511) = 0x1;
	NS16550_init (console, clock_divisor);
}
void hw_watchdog_reset (void)
{
	if (get_BoardType () == 0) {
		*(u32 *) (0xff000005) = 0;
		asm ("sync");
	}
}
int checkboard (void)
{
	ulong busfreq = get_bus_freq (0);
	char buf[32];
	u32 BoardType = get_BoardType ();
	char *BoardName[2] = { "mvBlueBOX", "mvBlueLYNX" };
	char *p;
	bd_t *bd = gd->bd;

	hw_watchdog_reset ();

	printf ("U-Boot (%s) running on mvBLUE device.\n", MV_VERSION);
	printf ("       Found %s running at %s MHz memory clock.\n",
		BoardName[BoardType], strmhz (buf, busfreq));

	init_2nd_DUART ();

	if ((p = getenv ("console_nr")) != NULL) {
		unsigned long con_nr = simple_strtoul (p, NULL, 10) & 3;

		bd->bi_baudrate &= ~3;
		bd->bi_baudrate |= con_nr & 3;
	}
	return 0;
}

long int initdram (int board_type)
{
	long size;
	long new_bank0_end;
	long mear1;
	long emear1;

	size = get_ram_size(CFG_SDRAM_BASE, CFG_MAX_RAM_SIZE);

	new_bank0_end = size - 1;
	mear1 = mpc824x_mpc107_getreg(MEAR1);
	emear1 = mpc824x_mpc107_getreg(EMEAR1);
	mear1 = (mear1  & 0xFFFFFF00) |
		((new_bank0_end & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT);
	emear1 = (emear1 & 0xFFFFFF00) |
		((new_bank0_end & MICR_ADDR_MASK) >> MICR_EADDR_SHIFT);
	mpc824x_mpc107_setreg(MEAR1,  mear1);
	mpc824x_mpc107_setreg(EMEAR1, emear1);

	return (size);
}

/* ------------------------------------------------------------------------- */
u8 *dhcp_vendorex_prep (u8 * e)
{
	char *ptr;

	/* DHCP vendor-class-identifier = 60 */
	if ((ptr = getenv ("dhcp_vendor-class-identifier"))) {
		*e++ = 60;
		*e++ = strlen (ptr);
		while (*ptr)
			*e++ = *ptr++;
	}
	/* my DHCP_CLIENT_IDENTIFIER = 61 */
	if ((ptr = getenv ("dhcp_client_id"))) {
		*e++ = 61;
		*e++ = strlen (ptr);
		while (*ptr)
			*e++ = *ptr++;
	}
	return e;
}

u8 *dhcp_vendorex_proc (u8 * popt)
{
	return NULL;
}

/* ------------------------------------------------------------------------- */

/*
 * Initialize PCI Devices
 */
#ifdef CONFIG_PCI
void pci_mvblue_clear_base (struct pci_controller *hose, pci_dev_t dev)
{
	u32 cnt;

	printf ("clear base @ dev/func 0x%02x/0x%02x ... ", PCI_DEV (dev),
		PCI_FUNC (dev));
	for (cnt = 0; cnt < 6; cnt++)
		pci_hose_write_config_dword (hose, dev, 0x10 + (4 * cnt),
					     0x0);
	printf ("done\n");
}

void duart_setup (u32 base, u16 divisor)
{
	printf ("duart setup ...");
	out_8 ((u8 *) (CFG_ISA_IO + base + 3), 0x80);
	out_8 ((u8 *) (CFG_ISA_IO + base + 0), divisor & 0xff);
	out_8 ((u8 *) (CFG_ISA_IO + base + 1), divisor >> 8);
	out_8 ((u8 *) (CFG_ISA_IO + base + 3), 0x03);
	out_8 ((u8 *) (CFG_ISA_IO + base + 4), 0x03);
	out_8 ((u8 *) (CFG_ISA_IO + base + 2), 0x07);
	printf ("done\n");
}

void pci_mvblue_fixup_irq_behind_bridge (struct pci_controller *hose,
					 pci_dev_t bridge, unsigned char irq)
{
	pci_dev_t d;
	unsigned char bus;
	unsigned short vendor, class;

	pci_hose_read_config_byte (hose, bridge, PCI_SECONDARY_BUS, &bus);
	for (d = PCI_BDF (bus, 0, 0);
	     d < PCI_BDF (bus, PCI_MAX_PCI_DEVICES - 1,
			  PCI_MAX_PCI_FUNCTIONS - 1);
	     d += PCI_BDF (0, 0, 1)) {
		pci_hose_read_config_word (hose, d, PCI_VENDOR_ID, &vendor);
		if (vendor != 0xffff && vendor != 0x0000) {
			pci_hose_read_config_word (hose, d, PCI_CLASS_DEVICE,
						   &class);
			if (class == PCI_CLASS_BRIDGE_PCI)
				pci_mvblue_fixup_irq_behind_bridge (hose, d,
								    irq);
			else
				pci_hose_write_config_byte (hose, d,
							    PCI_INTERRUPT_LINE,
							    irq);
		}
	}
}

#define MV_MAX_PCI_BUSSES	3
#define SLOT0_IRQ	3
#define SLOT1_IRQ	4
void pci_mvblue_fixup_irq (struct pci_controller *hose, pci_dev_t dev)
{
	unsigned char line = 0xff;
	unsigned short class;

	if (PCI_BUS (dev) == 0) {
		switch (PCI_DEV (dev)) {
		case 0xd:
			if (get_BoardType () == 0) {
				line = 1;
			} else
				/* mvBL */
				line = 2;
			break;
		case 0xe:
			/* mvBB: IDE */
			line = 2;
			pci_hose_write_config_byte (hose, dev, 0x8a, 0x20);
			break;
		case 0xf:
			/* mvBB: Slot0 (Grabber) */
			pci_hose_read_config_word (hose, dev,
						   PCI_CLASS_DEVICE, &class);
			if (class == PCI_CLASS_BRIDGE_PCI) {
				pci_mvblue_fixup_irq_behind_bridge (hose, dev,
								    SLOT0_IRQ);
				line = 0xff;
			} else
				line = SLOT0_IRQ;
			break;
		case 0x10:
			/* mvBB: Slot1 */
			pci_hose_read_config_word (hose, dev,
						   PCI_CLASS_DEVICE, &class);
			if (class == PCI_CLASS_BRIDGE_PCI) {
				pci_mvblue_fixup_irq_behind_bridge (hose, dev,
								    SLOT1_IRQ);
				line = 0xff;
			} else
				line = SLOT1_IRQ;
			break;
		default:
			printf ("***pci_scan: illegal dev = 0x%08x\n",
				PCI_DEV (dev));
			line = 0xff;
			break;
		}
		pci_hose_write_config_byte (hose, dev, PCI_INTERRUPT_LINE,
					    line);
	}
}

struct pci_controller hose = {
	fixup_irq:pci_mvblue_fixup_irq
};

void pci_init_board (void)
{
	pci_mpc824x_init (&hose);
}
#endif
