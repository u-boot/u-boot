/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
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

#ifdef CONFIG_PCI
#include <pci.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

void flash__init (void);
void ether__init (void);
void peripheral_power_enable (void);

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

#define COMP_MODE_ENABLE ((unsigned int)0x0000EAEF)

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
		"subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	/* arch number of Integrator Board */
	gd->bd->bi_arch_number = MACH_TYPE_INTEGRATOR;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	gd->flags = 0;

#ifdef CONFIG_CM_REMAP
extern void cm_remap(void);
	cm_remap();	/* remaps writeable memory to 0x00000000 */
#endif

	icache_enable ();

	flash__init ();
	return 0;
}


int misc_init_r (void)
{
#ifdef CONFIG_PCI
	pci_init();
#endif
	setenv("verify", "n");
	return (0);
}

/*
 * Initialize PCI Devices, report devices found.
 */
#ifdef CONFIG_PCI

#ifndef CONFIG_PCI_PNP

static struct pci_config_table pci_integrator_config_table[] = {
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x0f, PCI_ANY_ID,
	  pci_cfgfunc_config_device, { PCI_ENET0_IOADDR,
				       PCI_ENET0_MEMADDR,
				       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER }},
	{ }
};
#endif

/* V3 access routines */
#define _V3Write16(o,v) (*(volatile unsigned short *)(PCI_V3_BASE + (unsigned int)(o)) = (unsigned short)(v))
#define _V3Read16(o)	(*(volatile unsigned short *)(PCI_V3_BASE + (unsigned int)(o)))

#define _V3Write32(o,v) (*(volatile unsigned int *)(PCI_V3_BASE + (unsigned int)(o)) = (unsigned int)(v))
#define _V3Read32(o)	(*(volatile unsigned int *)(PCI_V3_BASE + (unsigned int)(o)))

/* Compute address necessary to access PCI config space for the given */
/* bus and device. */
#define PCI_CONFIG_ADDRESS( __bus, __devfn, __offset ) ({				\
	unsigned int __address, __devicebit;						\
	unsigned short __mapaddress;							\
	unsigned int __dev = PCI_DEV (__devfn); /* FIXME to check!! (slot?) */		\
											\
	if (__bus == 0) {								\
		/* local bus segment so need a type 0 config cycle */			\
		/* build the PCI configuration "address" with one-hot in A31-A11 */	\
		__address = PCI_CONFIG_BASE;						\
		__address |= ((__devfn & 0x07) << 8);					\
		__address |= __offset & 0xFF;						\
		__mapaddress = 0x000A;	/* 101=>config cycle, 0=>A1=A0=0 */		\
		__devicebit = (1 << (__dev + 11));					\
											\
		if ((__devicebit & 0xFF000000) != 0) {					\
			/* high order bits are handled by the MAP register */		\
			__mapaddress |= (__devicebit >> 16);				\
		} else {								\
			/* low order bits handled directly in the address */		\
			__address |= __devicebit;					\
		}									\
	} else {		/* bus !=0 */						\
		/* not the local bus segment so need a type 1 config cycle */		\
		/* A31-A24 are don't care (so clear to 0) */				\
		__mapaddress = 0x000B;	/* 101=>config cycle, 1=>A1&A0 from PCI_CFG */	\
		__address = PCI_CONFIG_BASE;						\
		__address |= ((__bus & 0xFF) << 16);	/* bits 23..16 = bus number	*/  \
		__address |= ((__dev & 0x1F) << 11);	/* bits 15..11 = device number	*/  \
		__address |= ((__devfn & 0x07) << 8);	/* bits 10..8  = function number */ \
		__address |= __offset & 0xFF;	/* bits	 7..0  = register number */	\
	}										\
	_V3Write16 (V3_LB_MAP1, __mapaddress);						\
	__address;									\
})

/* _V3OpenConfigWindow - open V3 configuration window */
#define _V3OpenConfigWindow() {								\
	/* Set up base0 to see all 512Mbytes of memory space (not	     */		\
	/* prefetchable), this frees up base1 for re-use by configuration*/		\
	/* memory */									\
											\
	_V3Write32 (V3_LB_BASE0, ((INTEGRATOR_PCI_BASE & 0xFFF00000) |			\
				     0x90 | V3_LB_BASE_M_ENABLE));			\
	/* Set up base1 to point into configuration space, note that MAP1 */		\
	/* register is set up by pciMakeConfigAddress(). */				\
											\
	_V3Write32 (V3_LB_BASE1, ((CPU_PCI_CNFG_ADRS & 0xFFF00000) |			\
				     0x40 | V3_LB_BASE_M_ENABLE));			\
}

/* _V3CloseConfigWindow - close V3 configuration window */
#define _V3CloseConfigWindow() {							\
    /* Reassign base1 for use by prefetchable PCI memory */				\
	_V3Write32 (V3_LB_BASE1, (((INTEGRATOR_PCI_BASE + 0x10000000) & 0xFFF00000)	\
					| 0x84 | V3_LB_BASE_M_ENABLE));			\
	_V3Write16 (V3_LB_MAP1,								\
	    (((INTEGRATOR_PCI_BASE + 0x10000000) & 0xFFF00000) >> 16) | 0x0006);	\
											\
	/* And shrink base0 back to a 256M window (NOTE: MAP0 already correct) */	\
											\
	_V3Write32 (V3_LB_BASE0, ((INTEGRATOR_PCI_BASE & 0xFFF00000) |			\
			     0x80 | V3_LB_BASE_M_ENABLE));				\
}

static int pci_integrator_read_byte (struct pci_controller *hose, pci_dev_t dev,
				     int offset, unsigned char *val)
{
	_V3OpenConfigWindow ();
	*val = *(volatile unsigned char *) PCI_CONFIG_ADDRESS (PCI_BUS (dev),
							       PCI_FUNC (dev),
							       offset);
	_V3CloseConfigWindow ();

	return 0;
}

static int pci_integrator_read__word (struct pci_controller *hose,
				      pci_dev_t dev, int offset,
				      unsigned short *val)
{
	_V3OpenConfigWindow ();
	*val = *(volatile unsigned short *) PCI_CONFIG_ADDRESS (PCI_BUS (dev),
								PCI_FUNC (dev),
								offset);
	_V3CloseConfigWindow ();

	return 0;
}

static int pci_integrator_read_dword (struct pci_controller *hose,
				      pci_dev_t dev, int offset,
				      unsigned int *val)
{
	_V3OpenConfigWindow ();
	*val = *(volatile unsigned short *) PCI_CONFIG_ADDRESS (PCI_BUS (dev),
								PCI_FUNC (dev),
								offset);
	*val |= (*(volatile unsigned int *)
		 PCI_CONFIG_ADDRESS (PCI_BUS (dev), PCI_FUNC (dev),
				     (offset + 2))) << 16;
	_V3CloseConfigWindow ();

	return 0;
}

static int pci_integrator_write_byte (struct pci_controller *hose,
				      pci_dev_t dev, int offset,
				      unsigned char val)
{
	_V3OpenConfigWindow ();
	*(volatile unsigned char *) PCI_CONFIG_ADDRESS (PCI_BUS (dev),
							PCI_FUNC (dev),
							offset) = val;
	_V3CloseConfigWindow ();

	return 0;
}

static int pci_integrator_write_word (struct pci_controller *hose,
				      pci_dev_t dev, int offset,
				      unsigned short val)
{
	_V3OpenConfigWindow ();
	*(volatile unsigned short *) PCI_CONFIG_ADDRESS (PCI_BUS (dev),
							 PCI_FUNC (dev),
							 offset) = val;
	_V3CloseConfigWindow ();

	return 0;
}

static int pci_integrator_write_dword (struct pci_controller *hose,
				       pci_dev_t dev, int offset,
				       unsigned int val)
{
	_V3OpenConfigWindow ();
	*(volatile unsigned short *) PCI_CONFIG_ADDRESS (PCI_BUS (dev),
							 PCI_FUNC (dev),
							 offset) = (val & 0xFFFF);
	*(volatile unsigned short *) PCI_CONFIG_ADDRESS (PCI_BUS (dev),
							 PCI_FUNC (dev),
							 (offset + 2)) = ((val >> 16) & 0xFFFF);
	_V3CloseConfigWindow ();

	return 0;
}
/******************************
 * PCI initialisation
 ******************************/

struct pci_controller integrator_hose = {
#ifndef CONFIG_PCI_PNP
	config_table: pci_integrator_config_table,
#endif
};

void pci_init_board (void)
{
	volatile int i, j;
	struct pci_controller *hose = &integrator_hose;

	/* setting this register will take the V3 out of reset */

	*(volatile unsigned int *) (INTEGRATOR_SC_PCIENABLE) = 1;

	/* wait a few usecs to settle the device and the PCI bus */

	for (i = 0; i < 100; i++)
		j = i + 1;

	/* Now write the Base I/O Address Word to V3_BASE + 0x6C */

	*(volatile unsigned short *) (V3_BASE + V3_LB_IO_BASE) =
		(unsigned short) (V3_BASE >> 16);

	do {
		*(volatile unsigned char *) (V3_BASE + V3_MAIL_DATA) = 0xAA;
		*(volatile unsigned char *) (V3_BASE + V3_MAIL_DATA + 4) =
			0x55;
	} while (*(volatile unsigned char *) (V3_BASE + V3_MAIL_DATA) != 0xAA
		 || *(volatile unsigned char *) (V3_BASE + V3_MAIL_DATA +
						 4) != 0x55);

	/* Make sure that V3 register access is not locked, if it is, unlock it */

	if ((*(volatile unsigned short *) (V3_BASE + V3_SYSTEM) &
	     V3_SYSTEM_M_LOCK)
	    == V3_SYSTEM_M_LOCK)
		*(volatile unsigned short *) (V3_BASE + V3_SYSTEM) = 0xA05F;

	/* Ensure that the slave accesses from PCI are disabled while we */
	/* setup windows */

	*(volatile unsigned short *) (V3_BASE + V3_PCI_CMD) &=
		~(V3_COMMAND_M_MEM_EN | V3_COMMAND_M_IO_EN);

	/* Clear RST_OUT to 0; keep the PCI bus in reset until we've finished */

	*(volatile unsigned short *) (V3_BASE + V3_SYSTEM) &=
		~V3_SYSTEM_M_RST_OUT;

	/* Make all accesses from PCI space retry until we're ready for them */

	*(volatile unsigned short *) (V3_BASE + V3_PCI_CFG) |=
		V3_PCI_CFG_M_RETRY_EN;

	/* Set up any V3 PCI Configuration Registers that we absolutely have to */
	/* LB_CFG controls Local Bus protocol. */
	/* Enable LocalBus byte strobes for READ accesses too. */
	/* set bit 7 BE_IMODE and bit 6 BE_OMODE */

	*(volatile unsigned short *) (V3_BASE + V3_LB_CFG) |= 0x0C0;

	/* PCI_CMD controls overall PCI operation. */
	/* Enable PCI bus master. */

	*(volatile unsigned short *) (V3_BASE + V3_PCI_CMD) |= 0x04;

	/* PCI_MAP0 controls where the PCI to CPU memory window is on Local Bus */

	*(volatile unsigned int *) (V3_BASE + V3_PCI_MAP0) =
		(INTEGRATOR_BOOT_ROM_BASE) | (V3_PCI_MAP_M_ADR_SIZE_512M |
					      V3_PCI_MAP_M_REG_EN |
					      V3_PCI_MAP_M_ENABLE);

	/* PCI_BASE0 is the PCI address of the start of the window */

	*(volatile unsigned int *) (V3_BASE + V3_PCI_BASE0) =
		INTEGRATOR_BOOT_ROM_BASE;

	/* PCI_MAP1 is LOCAL address of the start of the window */

	*(volatile unsigned int *) (V3_BASE + V3_PCI_MAP1) =
		(INTEGRATOR_HDR0_SDRAM_BASE) | (V3_PCI_MAP_M_ADR_SIZE_1024M |
						V3_PCI_MAP_M_REG_EN |
						V3_PCI_MAP_M_ENABLE);

	/* PCI_BASE1 is the PCI address of the start of the window */

	*(volatile unsigned int *) (V3_BASE + V3_PCI_BASE1) =
		INTEGRATOR_HDR0_SDRAM_BASE;

	/* Set up the windows from local bus memory into PCI configuration, */
	/* I/O and Memory. */
	/* PCI I/O, LB_BASE2 and LB_MAP2 are used exclusively for this. */

	*(volatile unsigned short *) (V3_BASE + V3_LB_BASE2) =
		((CPU_PCI_IO_ADRS >> 24) << 8) | V3_LB_BASE_M_ENABLE;
	*(volatile unsigned short *) (V3_BASE + V3_LB_MAP2) = 0;

	/* PCI Configuration, use LB_BASE1/LB_MAP1. */

	/* PCI Memory use LB_BASE0/LB_MAP0 and LB_BASE1/LB_MAP1 */
	/* Map first 256Mbytes as non-prefetchable via BASE0/MAP0 */
	/* (INTEGRATOR_PCI_BASE == PCI_MEM_BASE) */

	*(volatile unsigned int *) (V3_BASE + V3_LB_BASE0) =
		INTEGRATOR_PCI_BASE | (0x80 | V3_LB_BASE_M_ENABLE);

	*(volatile unsigned short *) (V3_BASE + V3_LB_MAP0) =
		((INTEGRATOR_PCI_BASE >> 20) << 0x4) | 0x0006;

	/* Map second 256 Mbytes as prefetchable via BASE1/MAP1 */

	*(volatile unsigned int *) (V3_BASE + V3_LB_BASE1) =
		INTEGRATOR_PCI_BASE | (0x84 | V3_LB_BASE_M_ENABLE);

	*(volatile unsigned short *) (V3_BASE + V3_LB_MAP1) =
		(((INTEGRATOR_PCI_BASE + 0x10000000) >> 20) << 4) | 0x0006;

	/* Allow accesses to PCI Configuration space */
	/* and set up A1, A0 for type 1 config cycles */

	*(volatile unsigned short *) (V3_BASE + V3_PCI_CFG) =
		((*(volatile unsigned short *) (V3_BASE + V3_PCI_CFG)) &
		 ~(V3_PCI_CFG_M_RETRY_EN | V3_PCI_CFG_M_AD_LOW1)) |
		V3_PCI_CFG_M_AD_LOW0;

	/* now we can allow in PCI MEMORY accesses */

	*(volatile unsigned short *) (V3_BASE + V3_PCI_CMD) =
		(*(volatile unsigned short *) (V3_BASE + V3_PCI_CMD)) |
		V3_COMMAND_M_MEM_EN;

	/* Set RST_OUT to take the PCI bus is out of reset, PCI devices can */
	/* initialise and lock the V3 system register so that no one else */
	/* can play with it */

	*(volatile unsigned short *) (V3_BASE + V3_SYSTEM) =
		(*(volatile unsigned short *) (V3_BASE + V3_SYSTEM)) |
		V3_SYSTEM_M_RST_OUT;

	*(volatile unsigned short *) (V3_BASE + V3_SYSTEM) =
		(*(volatile unsigned short *) (V3_BASE + V3_SYSTEM)) |
		V3_SYSTEM_M_LOCK;

	/*
	 * Register the hose
	 */
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	/* System memory space */
	pci_set_region (hose->regions + 0,
			0x00000000, 0x40000000, 0x01000000,
			PCI_REGION_MEM | PCI_REGION_MEMORY);

	/* PCI Memory - config space */
	pci_set_region (hose->regions + 1,
			0x00000000, 0x62000000, 0x01000000, PCI_REGION_MEM);

	/* PCI V3 regs */
	pci_set_region (hose->regions + 2,
			0x00000000, 0x61000000, 0x00080000, PCI_REGION_MEM);

	/* PCI I/O space */
	pci_set_region (hose->regions + 3,
			0x00000000, 0x60000000, 0x00010000, PCI_REGION_IO);

	pci_set_ops (hose,
		     pci_integrator_read_byte,
		     pci_integrator_read__word,
		     pci_integrator_read_dword,
		     pci_integrator_write_byte,
		     pci_integrator_write_word, pci_integrator_write_dword);

	hose->region_count = 4;

	pci_register_hose (hose);

	pciauto_config_init (hose);
	pciauto_config_device (hose, 0);

	hose->last_busno = pci_hose_scan (hose);
}
#endif

/******************************
 Routine:
 Description:
******************************/
void flash__init (void)
{
}
/*************************************************************
 Routine:ether__init
 Description: take the Ethernet controller out of reset and wait
			   for the EEPROM load to complete.
*************************************************************/
void ether__init (void)
{
}

/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size	 = PHYS_SDRAM_1_SIZE;

#ifdef CONFIG_CM_SPD_DETECT
	{
extern void dram_query(void);
	unsigned long cm_reg_sdram;
	unsigned long sdram_shift;

	dram_query();	/* Assembler accesses to CM registers */
			/* Queries the SPD values	      */

	/* Obtain the SDRAM size from the CM SDRAM register */

	cm_reg_sdram = *(volatile ulong *)(CM_BASE + OS_SDRAM);
	/*   Register	      SDRAM size
	 *
	 *   0xXXXXXXbbb000bb	 16 MB
	 *   0xXXXXXXbbb001bb	 32 MB
	 *   0xXXXXXXbbb010bb	 64 MB
	 *   0xXXXXXXbbb011bb	128 MB
	 *   0xXXXXXXbbb100bb	256 MB
	 *
	 */
	sdram_shift		 = ((cm_reg_sdram & 0x0000001C)/4)%4;
	gd->bd->bi_dram[0].size	 = 0x01000000 << sdram_shift;

	}
#endif /* CM_SPD_DETECT */

	return 0;
}

/* The Integrator/AP timer1 is clocked at 24MHz
 * can be divided by 16 or 256
 * and is a 16-bit counter
 */
/* U-Boot expects a 32 bit timer running at CFG_HZ*/
static ulong timestamp;		/* U-Boot ticks since startup	      */
static ulong total_count = 0;	/* Total timer count		      */
static ulong lastdec;		/* Timer reading at last call	      */
static ulong div_clock	 = 256; /* Divisor applied to the timer clock */
static ulong div_timer	 = 1;	/* Divisor to convert timer reading
				 * change to U-Boot ticks
				 */
/* CFG_HZ = CFG_HZ_CLOCK/(div_clock * div_timer) */

#define TIMER_LOAD_VAL 0x0000FFFFL
#define READ_TIMER ((*(volatile ulong *)(CFG_TIMERBASE+4)) & 0x0000FFFFL)

/* all function return values in U-Boot ticks i.e. (1/CFG_HZ) sec
 *  - unless otherwise stated
 */

/* starts a counter
 * - the Integrator/AP timer issues an interrupt
 *   each time it reaches zero
 */
int interrupt_init (void)
{
	/* Load timer with initial value */
	*(volatile ulong *)(CFG_TIMERBASE + 0) = TIMER_LOAD_VAL;
	/* Set timer to be
	 *	enabled		  1
	 *	free-running	  0
	 *	XX		 00
	 *	divider 256	 10
	 *	XX		 00
	 */
	*(volatile ulong *)(CFG_TIMERBASE + 8) = 0x00000088;
	total_count = 0;
	/* init the timestamp and lastdec value */
	reset_timer_masked();

	div_timer  = CFG_HZ_CLOCK / CFG_HZ;
	div_timer /= div_clock;

	return (0);
}

/*
 * timer without interrupts
 */
void reset_timer (void)
{
	reset_timer_masked ();
}

ulong get_timer (ulong base_ticks)
{
	return get_timer_masked () - base_ticks;
}

void set_timer (ulong ticks)
{
	timestamp = ticks;
	total_count = ticks * div_timer;
	reset_timer_masked();
}

/* delay x useconds */
void udelay (unsigned long usec)
{
	ulong tmo, tmp;

	/* Convert to U-Boot ticks */
	tmo  = usec * CFG_HZ;
	tmo /= (1000000L);

	tmp  = get_timer_masked();	/* get current timestamp */
	tmo += tmp;			/* wake up timestamp	 */

	while (get_timer_masked () < tmo) { /* loop till event */
		/*NOP*/;
	}
}

void reset_timer_masked (void)
{
	/* reset time */
	lastdec	  = READ_TIMER; /* capture current decrementer value   */
	timestamp = 0;		/* start "advancing" time stamp from 0 */
}

/* converts the timer reading to U-Boot ticks	       */
/* the timestamp is the number of ticks since reset    */
/* This routine does not detect wraps unless called regularly
   ASSUMES a call at least every 16 seconds to detect every reload */
ulong get_timer_masked (void)
{
	ulong now = READ_TIMER;		/* current count */

	if (now > lastdec) {
		/* Must have wrapped */
		total_count += lastdec + TIMER_LOAD_VAL + 1 - now;
	} else {
		total_count += lastdec - now;
	}
	lastdec	  = now;
	timestamp = total_count/div_timer;

	return timestamp;
}

/* waits specified delay value and resets timestamp */
void udelay_masked (unsigned long usec)
{
	udelay(usec);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * Return the timebase clock frequency
 * i.e. how often the timer decrements
 */
ulong get_tbclk (void)
{
	return CFG_HZ_CLOCK/div_clock;
}
