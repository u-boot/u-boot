/*
 * (C) Copyright 2006-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <ppc4xx.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <spd_sdram.h>
#include <libfdt.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

static inline u32 get_async_pci_freq(void)
{
	if (in_8((void *)(CONFIG_SYS_BCSR_BASE + 5)) &
		CONFIG_SYS_BCSR5_PCI66EN)
		return 66666666;
	else
		return 33333333;
}

int board_early_init_f(void)
{
	register uint reg;

	/*--------------------------------------------------------------------
	 * Setup the external bus controller/chip selects
	 *-------------------------------------------------------------------*/
	mtdcr(EBC0_CFGADDR, EBC0_CFG);
	reg = mfdcr(EBC0_CFGDATA);
	mtdcr(EBC0_CFGDATA, reg | 0x04000000);	/* Set ATC */

	/*--------------------------------------------------------------------
	 * Setup the GPIO pins
	 *-------------------------------------------------------------------*/
	/*CPLD cs */
	/*setup Address lines for flash size 64Meg. */
	out32(GPIO0_OSRL, in32(GPIO0_OSRL) | 0x50010000);
	out32(GPIO0_TSRL, in32(GPIO0_TSRL) | 0x50010000);
	out32(GPIO0_ISR1L, in32(GPIO0_ISR1L) | 0x50000000);

	/*setup emac */
	out32(GPIO0_TCR, in32(GPIO0_TCR) | 0xC080);
	out32(GPIO0_TSRL, in32(GPIO0_TSRL) | 0x40);
	out32(GPIO0_ISR1L, in32(GPIO0_ISR1L) | 0x55);
	out32(GPIO0_OSRH, in32(GPIO0_OSRH) | 0x50004000);
	out32(GPIO0_ISR1H, in32(GPIO0_ISR1H) | 0x00440000);

	/*UART1 */
	out32(GPIO1_TCR, in32(GPIO1_TCR) | 0x02000000);
	out32(GPIO1_OSRL, in32(GPIO1_OSRL) | 0x00080000);
	out32(GPIO1_ISR2L, in32(GPIO1_ISR2L) | 0x00010000);

	/* external interrupts IRQ0...3 */
	out32(GPIO1_TCR, in32(GPIO1_TCR) & ~0x00f00000);
	out32(GPIO1_TSRL, in32(GPIO1_TSRL) & ~0x0000ff00);
	out32(GPIO1_ISR1L, in32(GPIO1_ISR1L) | 0x00005500);

#ifdef CONFIG_440EP
	/*setup USB 2.0 */
	out32(GPIO1_TCR, in32(GPIO1_TCR) | 0xc0000000);
	out32(GPIO1_OSRL, in32(GPIO1_OSRL) | 0x50000000);
	out32(GPIO0_TCR, in32(GPIO0_TCR) | 0xf);
	out32(GPIO0_OSRH, in32(GPIO0_OSRH) | 0xaa);
	out32(GPIO0_ISR2H, in32(GPIO0_ISR2H) | 0x00000500);
#endif

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */
	mtdcr(UIC0ER, 0x00000000);	/* disable all */
	mtdcr(UIC0CR, 0x00000009);	/* ATI & UIC1 crit are critical */
	mtdcr(UIC0PR, 0xfffffe13);	/* per ref-board manual */
	mtdcr(UIC0TR, 0x01c00008);	/* per ref-board manual */
	mtdcr(UIC0VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */

	mtdcr(UIC1SR, 0xffffffff);	/* clear all */
	mtdcr(UIC1ER, 0x00000000);	/* disable all */
	mtdcr(UIC1CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC1PR, 0xffffe0ff);	/* per ref-board manual */
	mtdcr(UIC1TR, 0x00ffc000);	/* per ref-board manual */
	mtdcr(UIC1VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all */

	/*--------------------------------------------------------------------
	 * Setup other serial configuration
	 *-------------------------------------------------------------------*/
	mfsdr(SDR0_PCI0, reg);
	mtsdr(SDR0_PCI0, 0x80000000 | reg);	/* PCI arbiter enabled */
	mtsdr(SDR0_PFC0, 0x00003e00);	/* Pin function */
	mtsdr(SDR0_PFC1, 0x00048000);	/* Pin function: UART0 has 4 pins */

	/* Check and reconfigure the PCI sync clock if necessary */
	ppc4xx_pci_sync_clock_config(get_async_pci_freq());

	/*clear tmrclk divisor */
	*(unsigned char *)(CONFIG_SYS_BCSR_BASE | 0x04) = 0x00;

	/*enable ethernet */
	*(unsigned char *)(CONFIG_SYS_BCSR_BASE | 0x08) = 0xf0;

#ifdef CONFIG_440EP
	/*enable usb 1.1 fs device and remove usb 2.0 reset */
	*(unsigned char *)(CONFIG_SYS_BCSR_BASE | 0x09) = 0x00;
#endif

	/*get rid of flash write protect */
	*(unsigned char *)(CONFIG_SYS_BCSR_BASE | 0x07) = 0x00;

	return 0;
}

int misc_init_r (void)
{
	uint pbcr;
	int size_val = 0;

	/* Re-do sizing to get full correct info */
	mtdcr(EBC0_CFGADDR, PB0CR);
	pbcr = mfdcr(EBC0_CFGDATA);
	switch (gd->bd->bi_flashsize) {
	case 1 << 20:
		size_val = 0;
		break;
	case 2 << 20:
		size_val = 1;
		break;
	case 4 << 20:
		size_val = 2;
		break;
	case 8 << 20:
		size_val = 3;
		break;
	case 16 << 20:
		size_val = 4;
		break;
	case 32 << 20:
		size_val = 5;
		break;
	case 64 << 20:
		size_val = 6;
		break;
	case 128 << 20:
		size_val = 7;
		break;
	}
	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
	mtdcr(EBC0_CFGADDR, PB0CR);
	mtdcr(EBC0_CFGDATA, pbcr);

	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/* Monitor protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    -CONFIG_SYS_MONITOR_LEN,
			    0xffffffff,
			    &flash_info[0]);

	return 0;
}

int checkboard(void)
{
	char *s = getenv("serial#");
	u8 rev;
	u32 clock = get_async_pci_freq();

#ifdef CONFIG_440EP
	printf("Board: Yosemite - AMCC PPC440EP Evaluation Board");
#else
	printf("Board: Yellowstone - AMCC PPC440GR Evaluation Board");
#endif

	rev = in_8((void *)(CONFIG_SYS_BCSR_BASE + 0));
	printf(", Rev. %X, PCI-Async=%d MHz", rev, clock / 1000000);

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	/*
	 * Reconfiguration of the PCI sync clock is already done,
	 * now check again if everything is in range:
	 */
	if (ppc4xx_pci_sync_clock_config(clock)) {
		printf("ERROR: PCI clocking incorrect (async=%d "
		       "sync=%ld)!\n", clock, get_PCI_freq());
	}

	return (0);
}

/*************************************************************************
 *  initdram -- doesn't use serial presence detect.
 *
 *  Assumes:    256 MB, ECC, non-registered
 *              PLB @ 133 MHz
 *
 ************************************************************************/
#define NUM_TRIES 64
#define NUM_READS 10

void sdram_tr1_set(int ram_address, int* tr1_value)
{
	int i;
	int j, k;
	volatile unsigned int* ram_pointer =  (unsigned int*)ram_address;
	int first_good = -1, last_bad = 0x1ff;

	unsigned long test[NUM_TRIES] = {
		0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
		0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
		0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
		0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555,
		0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555,
		0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA,
		0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA,
		0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A,
		0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A,
		0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5,
		0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5,
		0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA,
		0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA,
		0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55,
		0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55 };

	/* go through all possible SDRAM0_TR1[RDCT] values */
	for (i=0; i<=0x1ff; i++) {
		/* set the current value for TR1 */
		mtsdram(SDRAM0_TR1, (0x80800800 | i));

		/* write values */
		for (j=0; j<NUM_TRIES; j++) {
			ram_pointer[j] = test[j];

			/* clear any cache at ram location */
			__asm__("dcbf 0,%0": :"r" (&ram_pointer[j]));
		}

		/* read values back */
		for (j=0; j<NUM_TRIES; j++) {
			for (k=0; k<NUM_READS; k++) {
				/* clear any cache at ram location */
				__asm__("dcbf 0,%0": :"r" (&ram_pointer[j]));

				if (ram_pointer[j] != test[j])
					break;
			}

			/* read error */
			if (k != NUM_READS) {
				break;
			}
		}

		/* we have a SDRAM0_TR1[RDCT] that is part of the window */
		if (j == NUM_TRIES) {
			if (first_good == -1)
				first_good = i;		/* found beginning of window */
		} else { /* bad read */
			/* if we have not had a good read then don't care */
			if(first_good != -1) {
				/* first failure after a good read */
				last_bad = i-1;
				break;
			}
		}
	}

	/* return the current value for TR1 */
	*tr1_value = (first_good + last_bad) / 2;
}

phys_size_t initdram(int board)
{
	register uint reg;
	int tr1_bank1, tr1_bank2;

	/*--------------------------------------------------------------------
	 * Setup some default
	 *------------------------------------------------------------------*/
	mtsdram(SDRAM0_UABBA, 0x00000000);	/* ubba=0 (default)             */
	mtsdram(SDRAM0_SLIO, 0x00000000);	/* rdre=0 wrre=0 rarw=0         */
	mtsdram(SDRAM0_DEVOPT, 0x00000000);	/* dll=0 ds=0 (normal)          */
	mtsdram(SDRAM0_CLKTR, 0x40000000);	/* ?? */
	mtsdram(SDRAM0_WDDCTR, 0x40000000);	/* ?? */

	/*clear this first, if the DDR is enabled by a debugger
	  then you can not make changes. */
	mtsdram(SDRAM0_CFG0, 0x00000000);	/* Disable EEC */

	/*--------------------------------------------------------------------
	 * Setup for board-specific specific mem
	 *------------------------------------------------------------------*/
	/*
	 * Following for CAS Latency = 2.5 @ 133 MHz PLB
	 */
	mtsdram(SDRAM0_B0CR, 0x000a4001);	/* SDBA=0x000 128MB, Mode 3, enabled */
	mtsdram(SDRAM0_B1CR, 0x080a4001);	/* SDBA=0x080 128MB, Mode 3, enabled */

	mtsdram(SDRAM0_TR0, 0x410a4012);	/* ?? */
	mtsdram(SDRAM0_RTR, 0x04080000);	/* ?? */
	mtsdram(SDRAM0_CFG1, 0x00000000);	/* Self-refresh exit, disable PM    */
	mtsdram(SDRAM0_CFG0, 0x30000000);	/* Disable EEC */
	udelay(400);		/* Delay 200 usecs (min)            */

	/*--------------------------------------------------------------------
	 * Enable the controller, then wait for DCEN to complete
	 *------------------------------------------------------------------*/
	mtsdram(SDRAM0_CFG0, 0x80000000);	/* Enable */

	for (;;) {
		mfsdram(SDRAM0_MCSTS, reg);
		if (reg & 0x80000000)
			break;
	}

	sdram_tr1_set(0x00000000, &tr1_bank1);
	sdram_tr1_set(0x08000000, &tr1_bank2);
	mtsdram(SDRAM0_TR1, (((tr1_bank1+tr1_bank2)/2) | 0x80800800));

	return CONFIG_SYS_SDRAM_BANKS * (CONFIG_SYS_KBYTES_SDRAM * 1024);	/* return bytes */
}

/*************************************************************************
 *  pci_pre_init
 *
 *  This routine is called just prior to registering the hose and gives
 *  the board the opportunity to check things. Returning a value of zero
 *  indicates that things are bad & PCI initialization should be aborted.
 *
 *	Different boards may wish to customize the pci controller structure
 *	(add regions, override default access routines, etc) or perform
 *	certain pre-initialization actions.
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int pci_pre_init(struct pci_controller *hose)
{
	unsigned long addr;

	/*-------------------------------------------------------------------------+
	  | Set priority for all PLB3 devices to 0.
	  | Set PLB3 arbiter to fair mode.
	  +-------------------------------------------------------------------------*/
	mfsdr(SD0_AMP1, addr);
	mtsdr(SD0_AMP1, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(PLB3_ACR);
	mtdcr(PLB3_ACR, addr | 0x80000000);

	/*-------------------------------------------------------------------------+
	  | Set priority for all PLB4 devices to 0.
	  +-------------------------------------------------------------------------*/
	mfsdr(SD0_AMP0, addr);
	mtsdr(SD0_AMP0, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(PLB4_ACR) | 0xa0000000;	/* Was 0x8---- */
	mtdcr(PLB4_ACR, addr);

	/*-------------------------------------------------------------------------+
	  | Set Nebula PLB4 arbiter to fair mode.
	  +-------------------------------------------------------------------------*/
	/* Segment0 */
	addr = (mfdcr(PLB0_ACR) & ~PLB0_ACR_PPM_MASK) | PLB0_ACR_PPM_FAIR;
	addr = (addr & ~PLB0_ACR_HBU_MASK) | PLB0_ACR_HBU_ENABLED;
	addr = (addr & ~PLB0_ACR_RDP_MASK) | PLB0_ACR_RDP_4DEEP;
	addr = (addr & ~PLB0_ACR_WRP_MASK) | PLB0_ACR_WRP_2DEEP;
	mtdcr(PLB0_ACR, addr);

	/* Segment1 */
	addr = (mfdcr(PLB1_ACR) & ~PLB1_ACR_PPM_MASK) | PLB1_ACR_PPM_FAIR;
	addr = (addr & ~PLB1_ACR_HBU_MASK) | PLB1_ACR_HBU_ENABLED;
	addr = (addr & ~PLB1_ACR_RDP_MASK) | PLB1_ACR_RDP_4DEEP;
	addr = (addr & ~PLB1_ACR_WRP_MASK) | PLB1_ACR_WRP_2DEEP;
	mtdcr(PLB1_ACR, addr);

	return 1;
}
#endif	/* defined(CONFIG_PCI) */

/*************************************************************************
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller *hose)
{
	/*--------------------------------------------------------------------------+
	 * Set up Direct MMIO registers
	 *--------------------------------------------------------------------------*/
	/*--------------------------------------------------------------------------+
	  | PowerPC440 EP PCI Master configuration.
	  | Map one 1Gig range of PLB/processor addresses to PCI memory space.
	  |   PLB address 0xA0000000-0xDFFFFFFF ==> PCI address 0xA0000000-0xDFFFFFFF
	  |   Use byte reversed out routines to handle endianess.
	  | Make this region non-prefetchable.
	  +--------------------------------------------------------------------------*/
	out32r(PCIL0_PMM0MA, 0x00000000);	/* PMM0 Mask/Attribute - disabled b4 setting */
	out32r(PCIL0_PMM0LA, CONFIG_SYS_PCI_MEMBASE);	/* PMM0 Local Address */
	out32r(PCIL0_PMM0PCILA, CONFIG_SYS_PCI_MEMBASE);	/* PMM0 PCI Low Address */
	out32r(PCIL0_PMM0PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIL0_PMM0MA, 0xE0000001);	/* 512M + No prefetching, and enable region */

	out32r(PCIL0_PMM1MA, 0x00000000);	/* PMM0 Mask/Attribute - disabled b4 setting */
	out32r(PCIL0_PMM1LA, CONFIG_SYS_PCI_MEMBASE2);	/* PMM0 Local Address */
	out32r(PCIL0_PMM1PCILA, CONFIG_SYS_PCI_MEMBASE2);	/* PMM0 PCI Low Address */
	out32r(PCIL0_PMM1PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIL0_PMM1MA, 0xE0000001);	/* 512M + No prefetching, and enable region */

	out32r(PCIL0_PTM1MS, 0x00000001);	/* Memory Size/Attribute */
	out32r(PCIL0_PTM1LA, 0);	/* Local Addr. Reg */
	out32r(PCIL0_PTM2MS, 0);	/* Memory Size/Attribute */
	out32r(PCIL0_PTM2LA, 0);	/* Local Addr. Reg */

	/*--------------------------------------------------------------------------+
	 * Set up Configuration registers
	 *--------------------------------------------------------------------------*/

	/* Program the board's subsystem id/vendor id */
	pci_write_config_word(0, PCI_SUBSYSTEM_VENDOR_ID,
			      CONFIG_SYS_PCI_SUBSYS_VENDORID);
	pci_write_config_word(0, PCI_SUBSYSTEM_ID, CONFIG_SYS_PCI_SUBSYS_ID);

	/* Configure command register as bus master */
	pci_write_config_word(0, PCI_COMMAND, PCI_COMMAND_MASTER);

	/* 240nS PCI clock */
	pci_write_config_word(0, PCI_LATENCY_TIMER, 1);

	/* No error reporting */
	pci_write_config_word(0, PCI_ERREN, 0);

	pci_write_config_dword(0, PCI_BRDGOPT2, 0x00000101);

}
#endif				/* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT) */

/*************************************************************************
 *  pci_master_init
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_MASTER_INIT)
void pci_master_init(struct pci_controller *hose)
{
	unsigned short temp_short;

	/*--------------------------------------------------------------------------+
	  | Write the PowerPC440 EP PCI Configuration regs.
	  |   Enable PowerPC440 EP to be a master on the PCI bus (PMM).
	  |   Enable PowerPC440 EP to act as a PCI memory target (PTM).
	  +--------------------------------------------------------------------------*/
	pci_read_config_word(0, PCI_COMMAND, &temp_short);
	pci_write_config_word(0, PCI_COMMAND,
			      temp_short | PCI_COMMAND_MASTER |
			      PCI_COMMAND_MEMORY);
}
#endif				/* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_MASTER_INIT) */

/*************************************************************************
 *  is_pci_host
 *
 *	This routine is called to determine if a pci scan should be
 *	performed. With various hardware environments (especially cPCI and
 *	PPMC) it's insufficient to depend on the state of the arbiter enable
 *	bit in the strap register, or generic host/adapter assumptions.
 *
 *	Rather than hard-code a bad assumption in the general 440 code, the
 *	440 pci code requires the board to decide at runtime.
 *
 *	Return 0 for adapter mode, non-zero for host (monarch) mode.
 *
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int is_pci_host(struct pci_controller *hose)
{
	/* Bamboo is always configured as host. */
	return (1);
}
#endif				/* defined(CONFIG_PCI) */

/*************************************************************************
 *  hw_watchdog_reset
 *
 *	This routine is called to reset (keep alive) the watchdog timer
 *
 ************************************************************************/
#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{

}
#endif

void board_reset(void)
{
	/* give reset to BCSR */
	*(unsigned char *)(CONFIG_SYS_BCSR_BASE | 0x06) = 0x09;
}
