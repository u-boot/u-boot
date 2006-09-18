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

/* stuff specific for the sc520,
 * but idependent of implementation */

#include <config.h>

#ifdef CONFIG_SC520

#include <common.h>
#include <config.h>
#include <pci.h>
#ifdef CONFIG_SC520_SSI
#include <ssi.h>
#endif
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/ic/sc520.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * utility functions for boards based on the AMD sc520
 *
 * void write_mmcr_byte(u16 mmcr, u8 data)
 * void write_mmcr_word(u16 mmcr, u16 data)
 * void write_mmcr_long(u16 mmcr, u32 data)
 *
 * u8   read_mmcr_byte(u16 mmcr)
 * u16  read_mmcr_word(u16 mmcr)
 * u32  read_mmcr_long(u16 mmcr)
 *
 * void init_sc520(void)
 * unsigned long init_sc520_dram(void)
 * void pci_sc520_init(struct pci_controller *hose)
 *
 * void reset_timer(void)
 * ulong get_timer(ulong base)
 * void set_timer(ulong t)
 * void udelay(unsigned long usec)
 *
 */

static u32 mmcr_base= 0xfffef000;

void write_mmcr_byte(u16 mmcr, u8 data)
{
	writeb(data, mmcr+mmcr_base);
}

void write_mmcr_word(u16 mmcr, u16 data)
{
	writew(data, mmcr+mmcr_base);
}

void write_mmcr_long(u16 mmcr, u32 data)
{
	writel(data, mmcr+mmcr_base);
}

u8 read_mmcr_byte(u16 mmcr)
{
	return readb(mmcr+mmcr_base);
}

u16 read_mmcr_word(u16 mmcr)
{
	return readw(mmcr+mmcr_base);
}

u32 read_mmcr_long(u16 mmcr)
{
	return readl(mmcr+mmcr_base);
}


void init_sc520(void)
{
	/* Set the UARTxCTL register at it's slower,
	 * baud clock giving us a 1.8432 MHz reference
	 */
	write_mmcr_byte(SC520_UART1CTL, 7);
	write_mmcr_byte(SC520_UART2CTL, 7);

	/* first set the timer pin mapping */
	write_mmcr_byte(SC520_CLKSEL, 0x72);	/* no clock frequency selected, use 1.1892MHz */

	/* enable PCI bus arbitrer */
	write_mmcr_byte(SC520_SYSARBCTL,0x02);  /* enable concurrent mode */

	write_mmcr_word(SC520_SYSARBMENB,0x1f); /* enable external grants */
	write_mmcr_word(SC520_HBCTL,0x04);      /* enable posted-writes */


	if (CFG_SC520_HIGH_SPEED) {
		write_mmcr_byte(SC520_CPUCTL, 0x2);	/* set it to 133 MHz and write back */
		gd->cpu_clk = 133000000;
		printf("## CPU Speed set to 133MHz\n");
	} else {
		write_mmcr_byte(SC520_CPUCTL, 1);	/* set CPU to 100 MHz and write back cache */
		printf("## CPU Speed set to 100MHz\n");
		gd->cpu_clk = 100000000;
	}


	/* wait at least one millisecond */
	asm("movl	$0x2000,%%ecx\n"
	    "wait_loop:	pushl %%ecx\n"
	    "popl	%%ecx\n"
	    "loop wait_loop\n": : : "ecx");

	/* turn on the SDRAM write buffer */
	write_mmcr_byte(SC520_DBCTL, 0x11);

	/* turn on the cache and disable write through */
	asm("movl	%%cr0, %%eax\n"
	    "andl	$0x9fffffff, %%eax\n"
	    "movl	%%eax, %%cr0\n"  : : : "eax");
}

unsigned long init_sc520_dram(void)
{
	bd_t *bd = gd->bd;

	u32 dram_present=0;
	u32 dram_ctrl;
#ifdef CFG_SDRAM_DRCTMCTL
	/* these memory control registers are set up in the assember part,
	 * in sc520_asm.S, during 'mem_init'.  If we muck with them here,
	 * after we are running a stack in RAM, we have troubles.  Besides,
	 * these refresh and delay values are better ? simply specified
	 * outright in the include/configs/{cfg} file since the HW designer
	 * simply dictates it.
	 */
#else
	int val;

	int cas_precharge_delay = CFG_SDRAM_PRECHARGE_DELAY;
	int refresh_rate        = CFG_SDRAM_REFRESH_RATE;
	int ras_cas_delay       = CFG_SDRAM_RAS_CAS_DELAY;

	/* set SDRAM speed here */

	refresh_rate/=78;
	if (refresh_rate<=1) {
		val = 0;  /* 7.8us */
	} else if (refresh_rate==2) {
		val = 1;  /* 15.6us */
	} else if (refresh_rate==3 || refresh_rate==4) {
		val = 2;  /* 31.2us */
	} else {
		val = 3;  /* 62.4us */
	}

	write_mmcr_byte(SC520_DRCCTL, (read_mmcr_byte(SC520_DRCCTL) & 0xcf) | (val<<4));

	val = read_mmcr_byte(SC520_DRCTMCTL);
	val &= 0xf0;

	if (cas_precharge_delay==3) {
		val |= 0x04;   /* 3T */
	} else if (cas_precharge_delay==4) {
		val |= 0x08;   /* 4T */
	} else if (cas_precharge_delay>4) {
		val |= 0x0c;
	}

	if (ras_cas_delay > 3) {
		val |= 2;
	} else {
		val |= 1;
	}
	write_mmcr_byte(SC520_DRCTMCTL, val);
#endif

	/* We read-back the configuration of the dram
	 * controller that the assembly code wrote */
	dram_ctrl = read_mmcr_long(SC520_DRCBENDADR);

	bd->bi_dram[0].start = 0;
	if (dram_ctrl & 0x80) {
		/* bank 0 enabled */
		dram_present = bd->bi_dram[1].start = (dram_ctrl & 0x7f) << 22;
		bd->bi_dram[0].size = bd->bi_dram[1].start;

	} else {
		bd->bi_dram[0].size = 0;
		bd->bi_dram[1].start = bd->bi_dram[0].start;
	}

	if (dram_ctrl & 0x8000) {
		/* bank 1 enabled */
		dram_present = bd->bi_dram[2].start = (dram_ctrl & 0x7f00) << 14;
		bd->bi_dram[1].size = bd->bi_dram[2].start -  bd->bi_dram[1].start;
	} else {
		bd->bi_dram[1].size = 0;
		bd->bi_dram[2].start = bd->bi_dram[1].start;
	}

	if (dram_ctrl & 0x800000) {
		/* bank 2 enabled */
		dram_present = bd->bi_dram[3].start = (dram_ctrl & 0x7f0000) << 6;
		bd->bi_dram[2].size = bd->bi_dram[3].start -  bd->bi_dram[2].start;
	} else {
		bd->bi_dram[2].size = 0;
		bd->bi_dram[3].start = bd->bi_dram[2].start;
	}

	if (dram_ctrl & 0x80000000) {
		/* bank 3 enabled */
		dram_present  = (dram_ctrl & 0x7f000000) >> 2;
		bd->bi_dram[3].size = dram_present -  bd->bi_dram[3].start;
	} else {
		bd->bi_dram[3].size = 0;
	}


#if 0
	printf("Configured %d bytes of dram\n", dram_present);
#endif
	gd->ram_size = dram_present;

	return dram_present;
}


#ifdef CONFIG_PCI


static struct {
	u8 priority;
	u16 level_reg;
	u8 level_bit;
} sc520_irq[] = {
	{ SC520_IRQ0,  SC520_MPICMODE,  0x01 },
	{ SC520_IRQ1,  SC520_MPICMODE,  0x02 },
	{ SC520_IRQ2,  SC520_SL1PICMODE, 0x02 },
	{ SC520_IRQ3,  SC520_MPICMODE,  0x08 },
	{ SC520_IRQ4,  SC520_MPICMODE,  0x10 },
	{ SC520_IRQ5,  SC520_MPICMODE,  0x20 },
	{ SC520_IRQ6,  SC520_MPICMODE,  0x40 },
	{ SC520_IRQ7,  SC520_MPICMODE,  0x80 },

	{ SC520_IRQ8,  SC520_SL1PICMODE, 0x01 },
	{ SC520_IRQ9,  SC520_SL1PICMODE, 0x02 },
	{ SC520_IRQ10, SC520_SL1PICMODE, 0x04 },
	{ SC520_IRQ11, SC520_SL1PICMODE, 0x08 },
	{ SC520_IRQ12, SC520_SL1PICMODE, 0x10 },
	{ SC520_IRQ13, SC520_SL1PICMODE, 0x20 },
	{ SC520_IRQ14, SC520_SL1PICMODE, 0x40 },
	{ SC520_IRQ15, SC520_SL1PICMODE, 0x80 }
};


/* The interrupt used for PCI INTA-INTD  */
int sc520_pci_ints[15] = {
	-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1
};

/* utility function to configure a pci interrupt */
int pci_sc520_set_irq(int pci_pin, int irq)
{
	int i;

# if 1
	printf("set_irq(): map INT%c to IRQ%d\n", pci_pin + 'A', irq);
#endif
	if (irq < 0 || irq > 15) {
		return -1; /* illegal irq */
	}

	if (pci_pin < 0 || pci_pin > 15) {
		return -1; /* illegal pci int pin */
	}

	/* first disable any non-pci interrupt source that use
	 * this level */
	for (i=SC520_GPTMR0MAP;i<=SC520_GP10IMAP;i++) {
		if (i>=SC520_PCIINTAMAP&&i<=SC520_PCIINTDMAP) {
			continue;
		}
		if (read_mmcr_byte(i) == sc520_irq[irq].priority) {
			write_mmcr_byte(i, SC520_IRQ_DISABLED);
		}
	}

	/* Set the trigger to level */
	write_mmcr_byte(sc520_irq[irq].level_reg,
			read_mmcr_byte(sc520_irq[irq].level_reg) | sc520_irq[irq].level_bit);


	if (pci_pin < 4) {
		/* PCI INTA-INTD */
		/* route the interrupt */
		write_mmcr_byte(SC520_PCIINTAMAP + pci_pin, sc520_irq[irq].priority);


	} else {
		/* GPIRQ0-GPIRQ10 used for additional PCI INTS */
		write_mmcr_byte(SC520_GP0IMAP + pci_pin - 4, sc520_irq[irq].priority);

		/* also set the polarity in this case */
		write_mmcr_word(SC520_INTPINPOL,
				read_mmcr_word(SC520_INTPINPOL) | (1 << (pci_pin-4)));

	}

	/* register the pin */
	sc520_pci_ints[pci_pin] = irq;


	return 0; /* OK */
}

void pci_sc520_init(struct pci_controller *hose)
{
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	/* System memory space */
	pci_set_region(hose->regions + 0,
		       SC520_PCI_MEMORY_BUS,
		       SC520_PCI_MEMORY_PHYS,
		       SC520_PCI_MEMORY_SIZE,
		       PCI_REGION_MEM | PCI_REGION_MEMORY);

	/* PCI memory space */
	pci_set_region(hose->regions + 1,
		       SC520_PCI_MEM_BUS,
		       SC520_PCI_MEM_PHYS,
		       SC520_PCI_MEM_SIZE,
		       PCI_REGION_MEM);

	/* ISA/PCI memory space */
	pci_set_region(hose->regions + 2,
		       SC520_ISA_MEM_BUS,
		       SC520_ISA_MEM_PHYS,
		       SC520_ISA_MEM_SIZE,
		       PCI_REGION_MEM);

	/* PCI I/O space */
	pci_set_region(hose->regions + 3,
		       SC520_PCI_IO_BUS,
		       SC520_PCI_IO_PHYS,
		       SC520_PCI_IO_SIZE,
		       PCI_REGION_IO);

	/* ISA/PCI I/O space */
	pci_set_region(hose->regions + 4,
		       SC520_ISA_IO_BUS,
		       SC520_ISA_IO_PHYS,
		       SC520_ISA_IO_SIZE,
		       PCI_REGION_IO);

	hose->region_count = 5;

	pci_setup_type1(hose,
			SC520_REG_ADDR,
			SC520_REG_DATA);

	pci_register_hose(hose);

	hose->last_busno = pci_hose_scan(hose);

	/* enable target memory acceses on host brige */
	pci_write_config_word(0, PCI_COMMAND,
			      PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

}


#endif

#ifdef CFG_TIMER_SC520


void reset_timer(void)
{
	write_mmcr_word(SC520_GPTMR0CNT, 0);
	write_mmcr_word(SC520_GPTMR0CTL, 0x6001);

}

ulong get_timer(ulong base)
{
	/* fixme: 30 or 33 */
	return 	read_mmcr_word(SC520_GPTMR0CNT) / 33;
}

void set_timer(ulong t)
{
	/* FixMe: use two cascade coupled timers */
	write_mmcr_word(SC520_GPTMR0CTL, 0x4001);
	write_mmcr_word(SC520_GPTMR0CNT, t*33);
	write_mmcr_word(SC520_GPTMR0CTL, 0x6001);
}


void udelay(unsigned long usec)
{
	int m=0;
	long u;

	read_mmcr_word(SC520_SWTMRMILLI);
	read_mmcr_word(SC520_SWTMRMICRO);

#if 0
	/* do not enable this line, udelay is used in the serial driver -> recursion */
	printf("udelay: %ld m.u %d.%d  tm.tu %d.%d\n", usec, m, u, tm, tu);
#endif
	while (1) {

		m += read_mmcr_word(SC520_SWTMRMILLI);
		u = read_mmcr_word(SC520_SWTMRMICRO) + (m * 1000);

		if (usec <= u) {
			break;
		}
	}
}

#endif

int ssi_set_interface(int freq, int lsb_first, int inv_clock, int inv_phase)
{
	u8 temp=0;

	if (freq >= 8192) {
		temp |= CTL_CLK_SEL_4;
	} else if (freq >= 4096) {
		temp |= CTL_CLK_SEL_8;
	} else if (freq >= 2048) {
		temp |= CTL_CLK_SEL_16;
	} else if (freq >= 1024) {
		temp |= CTL_CLK_SEL_32;
	} else if (freq >= 512) {
		temp |= CTL_CLK_SEL_64;
	} else if (freq >= 256) {
		temp |= CTL_CLK_SEL_128;
	} else if (freq >= 128) {
		temp |= CTL_CLK_SEL_256;
	} else {
		temp |= CTL_CLK_SEL_512;
	}

	if (!lsb_first) {
		temp |= MSBF_ENB;
	}

	if (inv_clock) {
		temp |= CLK_INV_ENB;
	}

	if (inv_phase) {
		temp |= PHS_INV_ENB;
	}

	write_mmcr_byte(SC520_SSICTL, temp);

	return 0;
}

u8 ssi_txrx_byte(u8 data)
{
	write_mmcr_byte(SC520_SSIXMIT, data);
	while ((read_mmcr_byte(SC520_SSISTA)) & SSISTA_BSY);
	write_mmcr_byte(SC520_SSICMD, SSICMD_CMD_SEL_XMITRCV);
	while ((read_mmcr_byte(SC520_SSISTA)) & SSISTA_BSY);
	return read_mmcr_byte(SC520_SSIRCV);
}


void ssi_tx_byte(u8 data)
{
	write_mmcr_byte(SC520_SSIXMIT, data);
	while ((read_mmcr_byte(SC520_SSISTA)) & SSISTA_BSY);
	write_mmcr_byte(SC520_SSICMD, SSICMD_CMD_SEL_XMIT);
}

u8 ssi_rx_byte(void)
{
	while ((read_mmcr_byte(SC520_SSISTA)) & SSISTA_BSY);
	write_mmcr_byte(SC520_SSICMD, SSICMD_CMD_SEL_RCV);
	while ((read_mmcr_byte(SC520_SSISTA)) & SSISTA_BSY);
	return read_mmcr_byte(SC520_SSIRCV);
}

#endif /* CONFIG_SC520 */
