/*
 * (C) Copyright 2009
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd.eu
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
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx-gpio.h>
#include <asm/4xx_pci.h>
#include <command.h>
#include <malloc.h>

/*
 * PMC405-DE cpld registers
 * - all registers are 8 bit
 * - all registers are on 32 bit addesses
 */
struct pmc405de_cpld {
	/* cpld design version */
	u8 version;
	u8 reserved0[3];

	/* misc. status lines */
	u8 status;
	u8 reserved1[3];

	/*
	 * gated control flags
	 * gate bit(s) must be written with '1' to
	 * access control flag
	 */
	u8 control;
	u8 reserved2[3];
};

#define CPLD_VERSION_MASK		0x0f
#define CPLD_CONTROL_POSTLED_N		0x01
#define CPLD_CONTROL_POSTLED_GATE	0x02
#define CPLD_CONTROL_RESETOUT_N		0x40
#define CPLD_CONTROL_RESETOUT_N_GATE	0x80

DECLARE_GLOBAL_DATA_PTR;

extern void __ft_board_setup(void *blob, bd_t *bd);
extern void pll_write(u32 a, u32 b);

static int wait_for_pci_ready_done;

static int is_monarch(void);
static int pci_is_66mhz(void);
static int board_revision(void);
static int cpld_revision(void);
static void upd_plb_pci_div(u32 pllmr0, u32 pllmr1, u32 div);

int board_early_init_f(void)
{
	u32 pllmr0, pllmr1;

	/*
	 * check M66EN and patch PLB:PCI divider for 66MHz PCI
	 *
	 * fCPU==333MHz && fPCI==66MHz (PLBDiv==3 && M66EN==1): PLB/PCI=1
	 * fCPU==333MHz && fPCI==33MHz (PLBDiv==3 && M66EN==0): PLB/PCI=2
	 * fCPU==133|266MHz && fPCI==66MHz (PLBDiv==1|2 && M66EN==1): PLB/PCI=2
	 * fCPU==133|266MHz && fPCI==33MHz (PLBDiv==1|2 && M66EN==0): PLB/PCI=3
	 *
	 * calling upd_plb_pci_div() may end in calling pll_write() which will
	 * do a chip reset and never return.
	 */
	pllmr0 = mfdcr(CPC0_PLLMR0);
	pllmr1 = mfdcr(CPC0_PLLMR1);

	if ((pllmr0 & PLLMR0_CPU_TO_PLB_MASK) == PLLMR0_CPU_PLB_DIV_3) {
		/* fCPU=333MHz, fPLB=111MHz */
		if (pci_is_66mhz())
			upd_plb_pci_div(pllmr0, pllmr1, PLLMR0_PCI_PLB_DIV_1);
		else
			upd_plb_pci_div(pllmr0, pllmr1, PLLMR0_PCI_PLB_DIV_2);
	} else {
		/* fCPU=133|266MHz, fPLB=133MHz */
		if (pci_is_66mhz())
			upd_plb_pci_div(pllmr0, pllmr1, PLLMR0_PCI_PLB_DIV_2);
		else
			upd_plb_pci_div(pllmr0, pllmr1, PLLMR0_PCI_PLB_DIV_3);
	}

	/*
	 * IRQ 25 (EXT IRQ 0) PCI-INTA#; active low; level sensitive
	 * IRQ 26 (EXT IRQ 1) PCI-INTB#; active low; level sensitive
	 * IRQ 27 (EXT IRQ 2) PCI-INTC#; active low; level sensitive
	 * IRQ 28 (EXT IRQ 3) PCI-INTD#; active low; level sensitive
	 * IRQ 29 (EXT IRQ 4) ETH0-PHY-IRQ#; active low; level sensitive
	 * IRQ 30 (EXT IRQ 5) ETH1-PHY-IRQ#; active low; level sensitive
	 * IRQ 31 (EXT IRQ 6) PLD-IRQ#; active low; level sensitive
	 */
	mtdcr(UIC0SR, 0xFFFFFFFF);       /* clear all ints */
	mtdcr(UIC0ER, 0x00000000);       /* disable all ints */
	mtdcr(UIC0CR, 0x00000000);       /* set all to be non-critical*/
	mtdcr(UIC0PR, 0xFFFFFF80);       /* set int polarities */
	mtdcr(UIC0TR, 0x10000000);       /* set int trigger levels */
	mtdcr(UIC0VCR, 0x00000001);      /* set vect base=0, INT0 highest prio */
	mtdcr(UIC0SR, 0xFFFFFFFF);       /* clear all ints */

	/*
	 * EBC Configuration Register:
	 * - set ready timeout to 512 ebc-clks -> ca. 15 us
	 * - EBC lines are always driven
	 */
	mtebc(EBC0_CFG, 0xa8400000);

	return 0;
}

static void upd_plb_pci_div(u32 pllmr0, u32 pllmr1, u32 div)
{
	if ((pllmr0 & PLLMR0_PCI_TO_PLB_MASK) != div)
		pll_write((pllmr0 & ~PLLMR0_PCI_TO_PLB_MASK) | div, pllmr1);
}

int misc_init_r(void)
{
	int i;
	struct ppc4xx_gpio *gpio0 = (struct ppc4xx_gpio *)GPIO_BASE;
	struct pmc405de_cpld *cpld =
		(struct pmc405de_cpld *)CONFIG_SYS_CPLD_BASE;

	if (!is_monarch()) {
		/* PCI configuration done: release EREADY */
		setbits_be32(&gpio0->or, CONFIG_SYS_GPIO_EREADY);
		setbits_be32(&gpio0->tcr, CONFIG_SYS_GPIO_EREADY);
	}

	/* turn off POST LED */
	out_8(&cpld->control,
	      CPLD_CONTROL_POSTLED_N | CPLD_CONTROL_POSTLED_GATE);

	/* turn on LEDs: RUN, A, B */
	clrbits_be32(&gpio0->or,
		     CONFIG_SYS_GPIO_LEDRUN_N |
		     CONFIG_SYS_GPIO_LEDA_N |
		     CONFIG_SYS_GPIO_LEDB_N);

	for (i=0; i < 200; i++)
		udelay(1000);

	/* turn off LEDs: A, B */
	setbits_be32(&gpio0->or,
		     CONFIG_SYS_GPIO_LEDA_N |
		     CONFIG_SYS_GPIO_LEDB_N);

	return (0);
}

static int is_monarch(void)
{
	struct ppc4xx_gpio *gpio0 = (struct ppc4xx_gpio *)GPIO_BASE;
	return (in_be32(&gpio0->ir) & CONFIG_SYS_GPIO_MONARCH_N) == 0;
}

static int pci_is_66mhz(void)
{
	struct ppc4xx_gpio *gpio0 = (struct ppc4xx_gpio *)GPIO_BASE;
	return (in_be32(&gpio0->ir) & CONFIG_SYS_GPIO_M66EN);
}

static int board_revision(void)
{
	struct ppc4xx_gpio *gpio0 = (struct ppc4xx_gpio *)GPIO_BASE;
	return ((in_be32(&gpio0->ir) & CONFIG_SYS_GPIO_HWREV_MASK) >>
		CONFIG_SYS_GPIO_HWREV_SHIFT);
}

static int cpld_revision(void)
{
	struct pmc405de_cpld *cpld =
		(struct pmc405de_cpld *)CONFIG_SYS_CPLD_BASE;
	return ((in_8(&cpld->version) & CPLD_VERSION_MASK));
}

/*
 * Check Board Identity
 */
int checkboard(void)
{
	puts("Board: esd GmbH - PMC-CPU/405-DE");

	gd->board_type = board_revision();
	printf(", Rev 1.%ld, ", gd->board_type);

	if (!is_monarch())
		puts("non-");

	printf("monarch, PCI=%s MHz, PLD-Rev 1.%d\n",
	       pci_is_66mhz() ? "66" : "33", cpld_revision());

	return 0;
}


static void wait_for_pci_ready(void)
{
	struct ppc4xx_gpio *gpio0 = (struct ppc4xx_gpio *)GPIO_BASE;
	int i;
	char *s = getenv("pcidelay");

	/* only wait once */
	if (wait_for_pci_ready_done)
		return;

	/*
	 * We have our own handling of the pcidelay variable.
	 * Using CONFIG_PCI_BOOTDELAY enables pausing for host
	 * and adapter devices. For adapter devices we do not
	 * want this.
	 */
	if (s) {
		int ms = simple_strtoul(s, NULL, 10);
		printf("PCI:   Waiting for %d ms\n", ms);
		for (i=0; i<ms; i++)
			udelay(1000);
	}

	if (!(in_be32(&gpio0->ir) & CONFIG_SYS_GPIO_EREADY)) {
		printf("PCI:   Waiting for EREADY (CTRL-C to skip) ... ");
		while (1) {
			if (ctrlc()) {
				puts("abort\n");
				break;
			}
			if (in_be32(&gpio0->ir) & CONFIG_SYS_GPIO_EREADY) {
				printf("done\n");
				break;
			}
		}
	}

	wait_for_pci_ready_done = 1;
}

/*
 * Overwrite weak is_pci_host()
 *
 * This routine is called to determine if a pci scan should be
 * performed. With various hardware environments (especially cPCI and
 * PPMC) it's insufficient to depend on the state of the arbiter enable
 * bit in the strap register, or generic host/adapter assumptions.
 *
 * Return 0 for adapter mode, non-zero for host (monarch) mode.
 */
int is_pci_host(struct pci_controller *hose)
{
	char *s;

	if (!is_monarch()) {
		/*
		 * Overwrite PCI identification when running in
		 * non-monarch mode
		 * This should be moved into pci_target_init()
		 * when it is sometimes available for 405 CPUs
		 */
		pci_write_config_word(PCIDEVID_405GP,
				      PCI_SUBSYSTEM_ID,
				      CONFIG_SYS_PCI_SUBSYS_ID_NONMONARCH);
		pci_write_config_word(PCIDEVID_405GP,
				      PCI_CLASS_SUB_CODE,
				      CONFIG_SYS_PCI_CLASSCODE_NONMONARCH);
	}

	s = getenv("pciscan");
	if (s == NULL) {
		if (is_monarch()) {
			wait_for_pci_ready();
			return 1;
		} else {
			return 0;
		}
	} else {
		if (!strcmp(s, "yes"))
			return 1;
	}

	return 0;
}

/*
 * Overwrite weak pci_pre_init()
 *
 * The default implementation enables the 405EP
 * internal PCI arbiter. We do not want that
 * on a PMC module.
 */
int pci_pre_init(struct pci_controller *hose)
{
	return 1;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	int rc;

	__ft_board_setup(blob, bd);

	/*
	 * Disable PCI in non-monarch mode.
	 */
	if (!is_monarch()) {
		rc = fdt_find_and_setprop(blob, "/plb/pci@ec000000", "status",
					  "disabled", sizeof("disabled"), 1);
		if (rc) {
			printf("Unable to update property status in PCI node, "
			       "err=%s\n",
			       fdt_strerror(rc));
		}
	}
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

#if defined(CONFIG_SYS_EEPROM_WREN)
/* Input: <dev_addr>  I2C address of EEPROM device to enable.
 *         <state>     -1: deliver current state
 *                      0: disable write
 *                      1: enable write
 * Returns:            -1: wrong device address
 *                      0: dis-/en- able done
 *                    0/1: current state if <state> was -1.
 */
int eeprom_write_enable(unsigned dev_addr, int state)
{
	struct ppc4xx_gpio *gpio0 = (struct ppc4xx_gpio *)GPIO_BASE;

	if (CONFIG_SYS_I2C_EEPROM_ADDR != dev_addr) {
		return -1;
	} else {
		switch (state) {
		case 1:
			/* Enable write access, clear bit GPIO0. */
			clrbits_be32(&gpio0->or, CONFIG_SYS_GPIO_EEPROM_WP);
			state = 0;
			break;
		case 0:
			/* Disable write access, set bit GPIO0. */
			setbits_be32(&gpio0->or, CONFIG_SYS_GPIO_EEPROM_WP);
			state = 0;
			break;
		default:
			/* Read current status back. */
			state = (0 == (in_be32(&gpio0->or) &
				       CONFIG_SYS_GPIO_EEPROM_WP));
			break;
		}
	}
	return state;
}

int do_eep_wren(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int query = argc == 1;
	int state = 0;

	if (query) {
		/* Query write access state. */
		state = eeprom_write_enable(CONFIG_SYS_I2C_EEPROM_ADDR, - 1);
		if (state < 0) {
			puts("Query of write access state failed.\n");
		} else {
			printf("Write access for device 0x%0x is %sabled.\n",
				CONFIG_SYS_I2C_EEPROM_ADDR,
				state ? "en" : "dis");
			state = 0;
		}
	} else {
		if ('0' == argv[1][0]) {
			/* Disable write access. */
			state = eeprom_write_enable(
				CONFIG_SYS_I2C_EEPROM_ADDR, 0);
		} else {
			/* Enable write access. */
			state = eeprom_write_enable(
				CONFIG_SYS_I2C_EEPROM_ADDR, 1);
		}
		if (state < 0)
			puts ("Setup of write access state failed.\n");
	}

	return state;
}

U_BOOT_CMD(eepwren, 2, 0, do_eep_wren,
	"Enable / disable / query EEPROM write access",
	""
);
#endif /* #if defined(CONFIG_SYS_EEPROM_WREN) */

#if defined(CONFIG_PRAM)
#include <environment.h>
extern env_t *env_ptr;

int do_painit(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 pram, nextbase, base;
	char *v;
	u32 param;
	ulong *lptr;

	v = getenv("pram");
	if (v)
		pram = simple_strtoul(v, NULL, 10);
	else {
		printf("Error: pram undefined. Please define pram in KiB\n");
		return 1;
	}

	base = gd->bd->bi_memsize;
#if defined(CONFIG_LOGBUFFER)
	base -= LOGBUFF_LEN + LOGBUFF_OVERHEAD;
#endif
	/*
	 * gd->bd->bi_memsize == physical ram size - CONFIG_SYS_MM_TOP_HIDE
	 */
	param = base - (pram << 10);
	printf("PARAM: @%08x\n", param);
	debug("memsize=0x%08x, base=0x%08x\n", gd->bd->bi_memsize, base);

	/* clear entire PA ram */
	memset((void*)param, 0, (pram << 10));

	/* reserve 4k for pointer field */
	nextbase = base - 4096;
	lptr = (ulong*)(base);

	/*
	 * *(--lptr) = item_size;
	 * *(--lptr) = base - item_base = distance from field top;
	 */

	/* env is first (4k aligned) */
	nextbase -= ((CONFIG_ENV_SIZE + 4096 - 1) & ~(4096 - 1));
	memcpy((void*)nextbase, env_ptr, CONFIG_ENV_SIZE);
	*(--lptr) = CONFIG_ENV_SIZE;     /* size */
	*(--lptr) = base - nextbase;  /* offset | type=0 */

	/* free section */
	*(--lptr) = nextbase - param; /* size */
	*(--lptr) = (base - param) | 126; /* offset | type=126 */

	/* terminate pointer field */
	*(--lptr) = crc32(0, (void*)(base - 0x10), 0x10);
	*(--lptr) = 0;                /* offset=0 -> terminator */
	return 0;
}
U_BOOT_CMD(
	painit,	1,	1,	do_painit,
	"prepare PciAccess system",
	""
);
#endif /* CONFIG_PRAM */

int do_selfreset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct ppc4xx_gpio *gpio0 = (struct ppc4xx_gpio *)GPIO_BASE;
	setbits_be32(&gpio0->tcr, CONFIG_SYS_GPIO_SELFRST_N);
	return 0;
}
U_BOOT_CMD(
	selfreset,	1,	1,	do_selfreset,
	"assert self-reset# signal",
	""
);

int do_resetout(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct pmc405de_cpld *cpld =
		(struct pmc405de_cpld *)CONFIG_SYS_CPLD_BASE;

	if (argc > 1) {
		if (argv[1][0] == '0') {
			/* assert */
			printf("PMC-RESETOUT# asserted\n");
			out_8(&cpld->control,
			      CPLD_CONTROL_RESETOUT_N_GATE);
		} else {
			/* deassert */
			printf("PMC-RESETOUT# deasserted\n");
			out_8(&cpld->control,
			      CPLD_CONTROL_RESETOUT_N |
			      CPLD_CONTROL_RESETOUT_N_GATE);
		}
	} else {
		printf("PMC-RESETOUT# is %s\n",
		       (in_8(&cpld->control) & CPLD_CONTROL_RESETOUT_N) ?
		       "inactive" : "active");
	}
	return 0;
}
U_BOOT_CMD(
	resetout,	2,	1,	do_resetout,
	"assert PMC-RESETOUT# signal",
	""
);
