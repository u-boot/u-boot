/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
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

#include <common.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/ic/sc520.h>
#include <asm/ic/ali512x.h>

/* ------------------------------------------------------------------------- */

static void irq_init(void)
{
	
	/* disable global interrupt mode */
	write_mmcr_byte(SC520_PICICR, 0x40); 
	
	/* set irq0-7 to edge */
	write_mmcr_byte(SC520_MPICMODE, 0x00);
	
	/* set irq9-12 to level, all the other (8, 13-15) are edge */
	write_mmcr_byte(SC520_SL1PICMODE, 0x1e);
	
	/* set irq16-24 (unused slave pic2) to level */
	write_mmcr_byte(SC520_SL2PICMODE, 0xff);
	
	/* active low polarity on PIC interrupt pins, 
	   active high polarity on all other irq pins */
	write_mmcr_word(SC520_INTPINPOL, 0);

	/* set irq number mapping */
	write_mmcr_byte(SC520_GPTMR0MAP,0);            /* disable GP timer 0 INT */       
	write_mmcr_byte(SC520_GPTMR1MAP,0);            /* disable GP timer 1 INT */
	write_mmcr_byte(SC520_GPTMR2MAP,0);            /* disable GP timer 2 INT */
	write_mmcr_byte(SC520_PIT0MAP,0x1);            /* Set PIT timer 0 INT to IRQ0 */ 
	write_mmcr_byte(SC520_PIT1MAP,0);              /* diable PIT timer 1 INT */
	write_mmcr_byte(SC520_PIT2MAP,0);              /* diable PIT timer 2 INT */
	write_mmcr_byte(SC520_PCIINTAMAP,0x4);         /* Set PCI INT A to IRQ9 */
	write_mmcr_byte(SC520_PCIINTBMAP,0x5);         /* Set PCI INT B to IRQ10 */
	write_mmcr_byte(SC520_PCIINTCMAP,0x6);         /* Set PCI INT C to IRQ11 */
	write_mmcr_byte(SC520_PCIINTDMAP,0x7);         /* Set PCI INT D to IRQ12 */
	write_mmcr_byte(SC520_DMABCINTMAP,0);          /* disable DMA INT */ 
	write_mmcr_byte(SC520_SSIMAP,0);               /* disable Synchronius serial INT */
	write_mmcr_byte(SC520_WDTMAP,0);               /* disable Watchdor INT */
	write_mmcr_byte(SC520_RTCMAP,0x3);             /* Set RTC int to 8 */
	write_mmcr_byte(SC520_WPVMAP,0);               /* disable write protect INT */
	write_mmcr_byte(SC520_ICEMAP,0x2);             /* Set ICE Debug Serielport INT to IRQ1 */
	write_mmcr_byte(SC520_FERRMAP,0x8);            /* Set FP error INT to IRQ13 */
	write_mmcr_byte(SC520_GP0IMAP,6);              /* Set GPIRQ0 (ISA IRQ2) to IRQ9 */
	write_mmcr_byte(SC520_GP1IMAP,2);              /* Set GPIRQ1 (SIO IRQ1) to IRQ1 */
	write_mmcr_byte(SC520_GP2IMAP,7);              /* Set GPIRQ2 (ISA IRQ12) to IRQ12 */
	
	if (CFG_USE_SIO_UART) {
		write_mmcr_byte(SC520_UART1MAP,0);     /* disable internal UART1 INT */
		write_mmcr_byte(SC520_UART2MAP,0);     /* disable internal UART2 INT */
		write_mmcr_byte(SC520_GP3IMAP,11);     /* Set GPIRQ3 (ISA IRQ3) to IRQ3 */ 
		write_mmcr_byte(SC520_GP4IMAP,12);     /* Set GPIRQ4 (ISA IRQ4) to IRQ4 */
	} else {
		write_mmcr_byte(SC520_UART1MAP,12);    /* Set internal UART2 INT to IRQ4 */
		write_mmcr_byte(SC520_UART2MAP,11);    /* Set internal UART2 INT to IRQ3 */
		write_mmcr_byte(SC520_GP3IMAP,0);      /* disable GPIRQ3 (ISA IRQ3) */ 
		write_mmcr_byte(SC520_GP4IMAP,0);      /* disable GPIRQ4 (ISA IRQ4) */
	}
	
	write_mmcr_byte(SC520_GP5IMAP,13);             /* Set GPIRQ5 (ISA IRQ5) to IRQ5 */
	write_mmcr_byte(SC520_GP6IMAP,21);             /* Set GPIRQ6 (ISA IRQ6) to IRQ6 */
	write_mmcr_byte(SC520_GP7IMAP,22);             /* Set GPIRQ7 (ISA IRQ7) to IRQ7 */
	write_mmcr_byte(SC520_GP8IMAP,3);              /* Set GPIRQ8 (SIO IRQ8) to IRQ8 */
	write_mmcr_byte(SC520_GP9IMAP,4);              /* Set GPIRQ9 (ISA IRQ9) to IRQ9 */
	write_mmcr_byte(SC520_GP10IMAP,9);             /* Set GPIRQ10 (ISA IRQ10) to IRQ10 */          
	write_mmcr_word(SC520_PCIHOSTMAP,0x11f);       /* Map PCI hostbridge INT to NMI */
	write_mmcr_word(SC520_ECCMAP,0x100);           /* Map SDRAM ECC failure INT to NMI */
 
}

/* PCI stuff */
static void pci_sc520_cdp_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	char pin;
	int irq;
	
	
	pci_hose_read_config_byte(hose, dev, PCI_INTERRUPT_PIN, &pin);
	irq = pin-1;
	
	switch (PCI_DEV(dev)) {
	case 20:
		break;
	case 19:
		irq+=1;
		break;
	case 18:
		irq+=2;
		break;
	case 17:
		irq+=3;
		break;
	default: 
		return;
	}
	
	irq&=3; /* wrap around */
	irq+=9; /* lowest IRQ is 9 */
	
	pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE, irq);
#if 0	
	printf("fixup_irq: device %d pin %c irq %d\n", 
	       PCI_DEV(dev), 'A' + pin -1, irq);
#endif
}
 
static struct pci_controller sc520_cdp_hose = {
	fixup_irq: pci_sc520_cdp_fixup_irq,
};

void pci_init_board(void)
{
	pci_sc520_init(&sc520_cdp_hose);
}


static void silence_uart(int port)
{
	outb(0, port+1);
}

void setup_ali_sio(int uart_primary)
{
	ali512x_init();
	
	ali512x_set_fdc(ALI_ENABLED, 0x3f2, 6, 0);
	ali512x_set_pp(ALI_ENABLED, 0x278, 7, 3);
	ali512x_set_uart(ALI_ENABLED, ALI_UART1, uart_primary?0x3f8:0x3e8, 4);
	ali512x_set_uart(ALI_ENABLED, ALI_UART2, uart_primary?0x2f8:0x2e8, 3);
	ali512x_set_rtc(ALI_DISABLED, 0, 0);
	ali512x_set_kbc(ALI_ENABLED, 1, 12);
	ali512x_set_cio(ALI_ENABLED);
	
	/* IrDa pins */
	ali512x_cio_function(12, 1, 0, 0);
	ali512x_cio_function(13, 1, 0, 0);
	
	/* SSI chip select pins */
	ali512x_cio_function(14, 0, 0, 0);  /* SSI_CS */
	ali512x_cio_function(15, 0, 0, 0);  /* SSI_MV */					     
	ali512x_cio_function(16, 0, 1, 0);  /* SSI_SPI# (inverted) */

	/* Board REV pins */
	ali512x_cio_function(20, 0, 0, 1);
	ali512x_cio_function(21, 0, 0, 1);
	ali512x_cio_function(22, 0, 0, 1);
	ali512x_cio_function(23, 0, 0, 1);      
}


/* set up the ISA bus timing and system address mappings */
static void bus_init(void)
{

	/* set up the GP IO pins */
	write_mmcr_word(SC520_PIOPFS31_16, 0xf7ff); 	/* set the GPIO pin function 31-16 reg */		   
	write_mmcr_word(SC520_PIOPFS15_0, 0xffff);  	/* set the GPIO pin function 15-0 reg */
	write_mmcr_byte(SC520_CSPFS, 0xf8);  		/* set the CS pin function  reg */	
	write_mmcr_byte(SC520_CLKSEL, 0x70);

	
	write_mmcr_byte(SC520_GPCSRT, 1);   /* set the GP CS offset */    	
	write_mmcr_byte(SC520_GPCSPW, 3);   /* set the GP CS pulse width */
	write_mmcr_byte(SC520_GPCSOFF, 1);  /* set the GP CS offset */
	write_mmcr_byte(SC520_GPRDW, 3);    /* set the RD pulse width */
	write_mmcr_byte(SC520_GPRDOFF, 1);  /* set the GP RD offset */
        write_mmcr_byte(SC520_GPWRW, 3);    /* set the GP WR pulse width */
	write_mmcr_byte(SC520_GPWROFF, 1);  /* set the GP WR offset */

	write_mmcr_word(SC520_BOOTCSCTL, 0x1823);		/* set up timing of BOOTCS */ 
	write_mmcr_word(SC520_ROMCS1CTL, 0x1823);		/* set up timing of ROMCS1 */
	write_mmcr_word(SC520_ROMCS2CTL, 0x1823);		/* set up timing of ROMCS2 */ 
	
	/* adjust the memory map:
	 * by default the first 256MB (0x00000000 - 0x0fffffff) is mapped to SDRAM
	 * and 256MB to 1G-128k  (0x1000000 - 0x37ffffff) is mapped to PCI mmio
	 * we need to map 1G-128k - 1G (0x38000000 - 0x3fffffff) to CS1 */ 
	
		
	/* SRAM = GPCS3 128k @ d0000-effff*/
	write_mmcr_long(SC520_PAR2,  0x4e00400d);		
	
	/* IDE0 = GPCS6 1f0-1f7 */
	write_mmcr_long(SC520_PAR3,  0x380801f0);		

	/* IDE1 = GPCS7 3f6 */
	write_mmcr_long(SC520_PAR4,  0x3c0003f6);		
	/* bootcs */
	write_mmcr_long(SC520_PAR12, 0x8bffe800);		
	/* romcs2 */
	write_mmcr_long(SC520_PAR13, 0xcbfff000);		
	/* romcs1 */
	write_mmcr_long(SC520_PAR14, 0xabfff800);		
	/* 680 LEDS */
	write_mmcr_long(SC520_PAR15, 0x30000640);		
	
	asm ("wbinvd\n"); /* Flush cache, req. after setting the unchached attribute ona PAR */	

	if (CFG_USE_SIO_UART) {
		write_mmcr_byte(SC520_ADDDECCTL, read_mmcr_byte(SC520_ADDDECCTL) | UART2_DIS|UART1_DIS);	
		setup_ali_sio(1);
	} else {
		write_mmcr_byte(SC520_ADDDECCTL, read_mmcr_byte(SC520_ADDDECCTL) & ~(UART2_DIS|UART1_DIS));	
		setup_ali_sio(0);
		silence_uart(0x3e8);
		silence_uart(0x2e8);
	}

}



/*
 * Miscelaneous platform dependent initialisations
 */

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	
	init_sc520();	
	bus_init();
	irq_init();
		
	/* max drive current on SDRAM */
	write_mmcr_word(SC520_DSCTL, 0x0100);
		
	/* enter debug mode after next reset (only if jumper is also set) */
	write_mmcr_byte(SC520_RESCFG, 0x08);
	
	/* configure the software timer to 33.333MHz */
	write_mmcr_byte(SC520_SWTMRCFG, 0);
	gd->bus_clk = 33333000;
	
	return 0;
}

int dram_init(void)
{
	init_sc520_dram();
	return 0;
}

void show_boot_progress(int val)
{
	outb(val&0xff, 0x80);
	outb((val&0xff00)>>8, 0x680);
}


int last_stage_init(void)
{
	int minor;
	int major;
	
	major = minor = 0;
	major |= ali512x_cio_in(23)?2:0;
	major |= ali512x_cio_in(22)?1:0;
	minor |= ali512x_cio_in(21)?2:0;
	minor |= ali512x_cio_in(20)?1:0;
	
	printf("AMD SC520 CDP revision %d.%d\n", major, minor);
	
	return 0;
}
