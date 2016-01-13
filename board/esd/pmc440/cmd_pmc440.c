/*
 * (C) Copyright 2007-2008
 * Matthias Fuchs, esd Gmbh, matthias.fuchs@esd-electronics.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <command.h>
#include <console.h>
#include <asm/io.h>
#include <asm/cache.h>
#include <asm/processor.h>
#if defined(CONFIG_LOGBUFFER)
#include <logbuff.h>
#endif

#include "pmc440.h"

int is_monarch(void);
int bootstrap_eeprom_write(unsigned dev_addr, unsigned offset,
			   uchar *buffer, unsigned cnt);
int eeprom_write_enable(unsigned dev_addr, int state);

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_BSP)

static int got_fifoirq;
static int got_hcirq;

int fpga_interrupt(u32 arg)
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)arg;
	int rc = -1; /* not for us */
	u32 status = FPGA_IN32(&fpga->status);

	/* check for interrupt from fifo module */
	if (status & STATUS_FIFO_ISF) {
		/* disable this int source */
		FPGA_OUT32(&fpga->hostctrl, HOSTCTRL_FIFOIE_GATE);
		rc = 0;
		got_fifoirq = 1; /* trigger backend */
	}

	if (status & STATUS_HOST_ISF) {
		FPGA_OUT32(&fpga->hostctrl, HOSTCTRL_HCINT_GATE);
		rc = 0;
		got_hcirq = 1;
	}

	return rc;
}

int do_waithci(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)FPGA_BA;

	got_hcirq = 0;

	FPGA_CLRBITS(&fpga->ctrla, CTRL_HOST_IE);
	FPGA_OUT32(&fpga->hostctrl, HOSTCTRL_HCINT_GATE);

	irq_install_handler(IRQ0_FPGA,
			    (interrupt_handler_t *)fpga_interrupt,
			    fpga);

	FPGA_SETBITS(&fpga->ctrla, CTRL_HOST_IE);

	while (!got_hcirq) {
		/* Abort if ctrl-c was pressed */
		if (ctrlc()) {
			puts("\nAbort\n");
			break;
		}
	}
	if (got_hcirq)
		printf("Got interrupt!\n");

	FPGA_CLRBITS(&fpga->ctrla, CTRL_HOST_IE);
	irq_free_handler(IRQ0_FPGA);
	return 0;
}
U_BOOT_CMD(
	waithci,	1,	1,	do_waithci,
	"Wait for host control interrupt",
	""
);

void dump_fifo(pmc440_fpga_t *fpga, int f, int *n)
{
	u32 ctrl;

	while (!((ctrl = FPGA_IN32(&fpga->fifo[f].ctrl)) & FIFO_EMPTY)) {
		printf("%5d  %d    %3d  %08x",
		       (*n)++, f, ctrl & (FIFO_LEVEL_MASK | FIFO_FULL),
		       FPGA_IN32(&fpga->fifo[f].data));
		if (ctrl & FIFO_OVERFLOW) {
			printf(" OVERFLOW\n");
			FPGA_CLRBITS(&fpga->fifo[f].ctrl, FIFO_OVERFLOW);
		} else
			printf("\n");
	}
}

int do_fifo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)FPGA_BA;
	int i;
	int n = 0;
	u32 ctrl, data, f;
	char str[] = "\\|/-";
	int abort = 0;
	int count = 0;
	int count2 = 0;

	switch (argc) {
	case 1:
		/* print all fifos status information */
		printf("fifo level status\n");
		printf("______________________________\n");
		for (i=0; i<FIFO_COUNT; i++) {
			ctrl = FPGA_IN32(&fpga->fifo[i].ctrl);
			printf(" %d    %3d  %s%s%s %s\n",
			       i, ctrl & (FIFO_LEVEL_MASK | FIFO_FULL),
			       ctrl & FIFO_FULL ? "FULL     " : "",
			       ctrl & FIFO_EMPTY ? "EMPTY    " : "",
			       ctrl & (FIFO_FULL|FIFO_EMPTY) ? "" : "NOT EMPTY",
			       ctrl & FIFO_OVERFLOW ? "OVERFLOW" : "");
		}
		break;

	case 2:
		/* completely read out fifo 'n' */
		if (!strcmp(argv[1],"read")) {
			printf("  #   fifo level data\n");
			printf("______________________________\n");

			for (i=0; i<FIFO_COUNT; i++)
				dump_fifo(fpga, i, &n);

		} else if (!strcmp(argv[1],"wait")) {
			got_fifoirq = 0;

			irq_install_handler(IRQ0_FPGA,
					    (interrupt_handler_t *)fpga_interrupt,
					    fpga);

			printf("  #   fifo level data\n");
			printf("______________________________\n");

			/* enable all fifo interrupts */
			FPGA_OUT32(&fpga->hostctrl,
				   HOSTCTRL_FIFOIE_GATE | HOSTCTRL_FIFOIE_FLAG);
			for (i=0; i<FIFO_COUNT; i++) {
				/* enable interrupts from all fifos */
				FPGA_SETBITS(&fpga->fifo[i].ctrl, FIFO_IE);
			}

			while (1) {
				/* wait loop */
				while (!got_fifoirq) {
					count++;
					if (!(count % 100)) {
						count2++;
						putc(0x08); /* backspace */
						putc(str[count2 % 4]);
					}

					/* Abort if ctrl-c was pressed */
					if ((abort = ctrlc())) {
						puts("\nAbort\n");
						break;
					}
					udelay(1000);
				}
				if (abort)
					break;

				/* simple fifo backend */
				if (got_fifoirq) {
					for (i=0; i<FIFO_COUNT; i++)
						dump_fifo(fpga, i, &n);

					got_fifoirq = 0;
					/* unmask global fifo irq */
					FPGA_OUT32(&fpga->hostctrl,
						   HOSTCTRL_FIFOIE_GATE |
						   HOSTCTRL_FIFOIE_FLAG);
				}
			}

			/* disable all fifo interrupts */
			FPGA_OUT32(&fpga->hostctrl, HOSTCTRL_FIFOIE_GATE);
			for (i=0; i<FIFO_COUNT; i++)
				FPGA_CLRBITS(&fpga->fifo[i].ctrl, FIFO_IE);

			irq_free_handler(IRQ0_FPGA);

		} else {
			printf("Usage:\nfifo %s\n", cmdtp->help);
			return 1;
		}
		break;

	case 4:
	case 5:
		if (!strcmp(argv[1],"write")) {
			/* get fifo number or fifo address */
			f = simple_strtoul(argv[2], NULL, 16);

			/* data paramter */
			data = simple_strtoul(argv[3], NULL, 16);

			/* get optional count parameter */
			n = 1;
			if (argc >= 5)
				n = (int)simple_strtoul(argv[4], NULL, 10);

			if (f < FIFO_COUNT) {
				printf("writing %d x %08x to fifo %d\n",
				       n, data, f);
				for (i=0; i<n; i++)
					FPGA_OUT32(&fpga->fifo[f].data, data);
			} else {
				printf("writing %d x %08x to fifo port at "
				       "address %08x\n",
				       n, data, f);
				for (i=0; i<n; i++)
					out_be32((void *)f, data);
			}
		} else {
			printf("Usage:\nfifo %s\n", cmdtp->help);
			return 1;
		}
		break;

	default:
		printf("Usage:\nfifo %s\n", cmdtp->help);
		return 1;
	}
	return 0;
}
U_BOOT_CMD(
	fifo,	5,	1,	do_fifo,
	"Fifo module operations",
	"wait\nfifo read\n"
	"fifo write fifo(0..3) data [cnt=1]\n"
	"fifo write address(>=4) data [cnt=1]\n"
	"  - without arguments: print all fifo's status\n"
	"  - with 'wait' argument: interrupt driven read from all fifos\n"
	"  - with 'read' argument: read current contents from all fifos\n"
	"  - with 'write' argument: write 'data' 'cnt' times to "
	"'fifo' or 'address'"
);

int do_setup_bootstrap_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong sdsdp[5];
	ulong delay;
	int count=16;

	if (argc < 2) {
		printf("Usage:\nsbe %s\n", cmdtp->help);
		return -1;
	}

	if (argc > 1) {
		if (!strcmp(argv[1], "400")) {
			/* PLB=133MHz, PLB/PCI=3 */
			printf("Bootstrapping for 400MHz\n");
			sdsdp[0]=0x8678624e;
			sdsdp[1]=0x095fa030;
			sdsdp[2]=0x40082350;
			sdsdp[3]=0x0d050000;
		} else if (!strcmp(argv[1], "533")) {
			/* PLB=133MHz, PLB/PCI=3 */
			printf("Bootstrapping for 533MHz\n");
			sdsdp[0]=0x87788252;
			sdsdp[1]=0x095fa030;
			sdsdp[2]=0x40082350;
			sdsdp[3]=0x0d050000;
		} else if (!strcmp(argv[1], "667")) {
			/* PLB=133MHz, PLB/PCI=3 */
			printf("Bootstrapping for 667MHz\n");
			sdsdp[0]=0x8778a256;
			sdsdp[1]=0x095fa030;
			sdsdp[2]=0x40082350;
			sdsdp[3]=0x0d050000;
		} else {
			printf("Usage:\nsbe %s\n", cmdtp->help);
			return -1;
		}
	}

	if (argc > 2) {
		sdsdp[4] = 0;
		if (argv[2][0]=='1')
			sdsdp[4]=0x19750100;
		else if (argv[2][0]=='0')
			sdsdp[4]=0x19750000;
		if (sdsdp[4])
			count += 4;
	}

	if (argc > 3) {
		delay = simple_strtoul(argv[3], NULL, 10);
		if (delay > 20)
			delay = 20;
		sdsdp[4] |= delay;
	}

	printf("Writing boot EEPROM ...\n");
	if (bootstrap_eeprom_write(CONFIG_SYS_I2C_BOOT_EEPROM_ADDR,
				   0, (uchar*)sdsdp, count) != 0)
		printf("bootstrap_eeprom_write failed\n");
	else
		printf("done (dump via 'i2c md 52 0.1 14')\n");

	return 0;
}
U_BOOT_CMD(
	sbe, 4, 0, do_setup_bootstrap_eeprom,
	"setup bootstrap eeprom",
	"<cpufreq:400|533|667> [<console-uart:0|1> [<bringup delay (0..20s)>]]"
);

#if defined(CONFIG_PRAM)
#include <environment.h>
#include <search.h>
#include <errno.h>

int do_painit(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 pram, nextbase, base;
	char *v;
	u32 param;
	ulong *lptr;

	env_t *envp;
	char *res;
	int len;

	v = getenv("pram");
	if (v)
		pram = simple_strtoul(v, NULL, 10);
	else {
		printf("Error: pram undefined. Please define pram in KiB\n");
		return 1;
	}

	base = (u32)gd->ram_size;
#if defined(CONFIG_LOGBUFFER)
	base -= LOGBUFF_LEN + LOGBUFF_OVERHEAD;
#endif
	/*
	 * gd->ram_size == physical ram size - CONFIG_SYS_MEM_TOP_HIDE
	 */
	param = base - (pram << 10);
	printf("PARAM: @%08x\n", param);
	debug("memsize=0x%08x, base=0x%08x\n", (u32)gd->ram_size, base);

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
	envp = (env_t *)nextbase;
	res = (char *)envp->data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	envp->crc = crc32(0, envp->data, ENV_SIZE);

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
	in_be32((void*)CONFIG_SYS_RESET_BASE);
	return 0;
}
U_BOOT_CMD(
	selfreset,	1,	1,	do_selfreset,
	"assert self-reset# signal",
	""
);

int do_resetout(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	pmc440_fpga_t *fpga = (pmc440_fpga_t *)FPGA_BA;

	/* requiers bootet FPGA and PLD_IOEN_N active */
	if (in_be32((void*)GPIO1_OR) & GPIO1_IOEN_N) {
		printf("Error: resetout requires a bootet FPGA\n");
		return -1;
	}

	if (argc > 1) {
		if (argv[1][0] == '0') {
			/* assert */
			printf("PMC-RESETOUT# asserted\n");
			FPGA_OUT32(&fpga->hostctrl,
				   HOSTCTRL_PMCRSTOUT_GATE);
		} else {
			/* deassert */
			printf("PMC-RESETOUT# deasserted\n");
			FPGA_OUT32(&fpga->hostctrl,
				   HOSTCTRL_PMCRSTOUT_GATE |
				   HOSTCTRL_PMCRSTOUT_FLAG);
		}
	} else {
		printf("PMC-RESETOUT# is %s\n",
		       FPGA_IN32(&fpga->hostctrl) & HOSTCTRL_PMCRSTOUT_FLAG ?
		       "inactive" : "active");
	}

	return 0;
}
U_BOOT_CMD(
	resetout,	2,	1,	do_resetout,
	"assert PMC-RESETOUT# signal",
	""
);

int do_inta(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (is_monarch()) {
		printf("This command is only supported in non-monarch mode\n");
		return -1;
	}

	if (argc > 1) {
		if (argv[1][0] == '0') {
			/* assert */
			printf("inta# asserted\n");
			out_be32((void*)GPIO1_TCR,
				 in_be32((void*)GPIO1_TCR) | GPIO1_INTA_FAKE);
		} else {
			/* deassert */
			printf("inta# deasserted\n");
			out_be32((void*)GPIO1_TCR,
				 in_be32((void*)GPIO1_TCR) & ~GPIO1_INTA_FAKE);
		}
	} else {
		printf("inta# is %s\n",
		       in_be32((void*)GPIO1_TCR) & GPIO1_INTA_FAKE ?
		       "active" : "inactive");
	}
	return 0;
}
U_BOOT_CMD(
	inta,	2,	1,	do_inta,
	"Assert/Deassert or query INTA# state in non-monarch mode",
	""
);

/* test-only */
int do_pmm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong pciaddr;

	if (argc > 1) {
		pciaddr = simple_strtoul(argv[1], NULL, 16);

		pciaddr &= 0xf0000000;

		/* map PCI address at 0xc0000000 in PLB space */

		/* PMM1 Mask/Attribute - disabled b4 setting */
		out32r(PCIL0_PMM1MA, 0x00000000);
		/* PMM1 Local Address */
		out32r(PCIL0_PMM1LA, 0xc0000000);
		/* PMM1 PCI Low Address */
		out32r(PCIL0_PMM1PCILA, pciaddr);
		/* PMM1 PCI High Address */
		out32r(PCIL0_PMM1PCIHA, 0x00000000);
		/* 256MB + No prefetching, and enable region */
		out32r(PCIL0_PMM1MA, 0xf0000001);
	} else {
		printf("Usage:\npmm %s\n", cmdtp->help);
	}
	return 0;
}
U_BOOT_CMD(
	pmm,	2,	1,	do_pmm,
	"Setup pmm[1] registers",
	"<pciaddr> (pciaddr will be aligned to 256MB)"
);

#if defined(CONFIG_SYS_EEPROM_WREN)
int do_eep_wren(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int query = argc == 1;
	int state = 0;

	if (query) {
		/* Query write access state. */
		state = eeprom_write_enable(CONFIG_SYS_I2C_EEPROM_ADDR, -1);
		if (state < 0) {
			puts("Query of write access state failed.\n");
		} else {
			printf("Write access for device 0x%0x is %sabled.\n",
			       CONFIG_SYS_I2C_EEPROM_ADDR, state ? "en" : "dis");
			state = 0;
		}
	} else {
		if ('0' == argv[1][0]) {
			/* Disable write access. */
			state = eeprom_write_enable(CONFIG_SYS_I2C_EEPROM_ADDR, 0);
		} else {
			/* Enable write access. */
			state = eeprom_write_enable(CONFIG_SYS_I2C_EEPROM_ADDR, 1);
		}
		if (state < 0) {
			puts("Setup of write access state failed.\n");
		}
	}

	return state;
}
U_BOOT_CMD(eepwren, 2, 0, do_eep_wren,
	"Enable / disable / query EEPROM write access",
	""
);
#endif /* #if defined(CONFIG_SYS_EEPROM_WREN) */

#endif /* CONFIG_CMD_BSP */
