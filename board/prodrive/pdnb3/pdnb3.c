/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/arch/ixp425.h>

DECLARE_GLOBAL_DATA_PTR;

/* predefine these here for FPGA programming (before including fpga.c) */
#define SET_FPGA(data)	*IXP425_GPIO_GPOUTR = (data)
#define FPGA_DONE_STATE (*IXP425_GPIO_GPINR & CONFIG_SYS_FPGA_DONE)
#define FPGA_INIT_STATE (*IXP425_GPIO_GPINR & CONFIG_SYS_FPGA_INIT)
#define OLD_VAL		old_val

static unsigned long old_val = 0;

/*
 * include common fpga code (for prodrive boards)
 */
#include "../common/fpga.c"

/*
 * Miscelaneous platform dependent initialisations
 */
int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_FPGA_RESET);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_FPGA_RESET);

	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_SYS_RUNNING);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_SYS_RUNNING);

	/*
	 * Setup GPIO's for FPGA programming
	 */
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_PRG);
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_CLK);
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_DATA);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_PRG);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_CLK);
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_DATA);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_INIT);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_DONE);

	/*
	 * Setup GPIO's for interrupts
	 */
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_PCI_INTA);
	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_PCI_INTA);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_PCI_INTB);
	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_PCI_INTB);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_RESTORE_INT);
	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_RESTORE_INT);
	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_RESTART_INT);
	GPIO_INT_ACT_LOW_SET(CONFIG_SYS_GPIO_RESTART_INT);

	/*
	 * Setup GPIO's for 33MHz clock output
	 */
	*IXP425_GPIO_GPCLKR = 0x01FF0000;
	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_CLK_33M);

	/*
	 * Setup other chip select's
	 */
	*IXP425_EXP_CS1 = CONFIG_SYS_EXP_CS1;

	return 0;
}

/*
 * Check Board Identity
 */
int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	puts("Board: PDNB3");

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return (0);
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;

	return (0);
}

int do_fpga_boot(unsigned char *fpgadata)
{
	unsigned char *dst;
	int status;
	int index;
	int i;
	ulong len = CONFIG_SYS_MALLOC_LEN;

	/*
	 * Setup GPIO's for FPGA programming
	 */
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_PRG);
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_CLK);
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_DATA);

	/*
	 * Save value so no readback is required upon programming
	 */
	old_val = *IXP425_GPIO_GPOUTR;

	/*
	 * First try to decompress fpga image (gzip compressed?)
	 */
	dst = malloc(CONFIG_SYS_FPGA_MAX_SIZE);
	if (gunzip(dst, CONFIG_SYS_FPGA_MAX_SIZE, (uchar *)fpgadata, &len) != 0) {
		printf("Error: Image has to be gzipp'ed!\n");
		return -1;
	}

	status = fpga_boot(dst, len);
	if (status != 0) {
		printf("\nFPGA: Booting failed ");
		switch (status) {
		case ERROR_FPGA_PRG_INIT_LOW:
			printf("(Timeout: INIT not low after asserting PROGRAM*)\n ");
			break;
		case ERROR_FPGA_PRG_INIT_HIGH:
			printf("(Timeout: INIT not high after deasserting PROGRAM*)\n ");
			break;
		case ERROR_FPGA_PRG_DONE:
			printf("(Timeout: DONE not high after programming FPGA)\n ");
			break;
		}

		/* display infos on fpgaimage */
		index = 15;
		for (i=0; i<4; i++) {
			len = dst[index];
			printf("FPGA: %s\n", &(dst[index+1]));
			index += len+3;
		}
		putc ('\n');
		/* delayed reboot */
		for (i=5; i>0; i--) {
			printf("Rebooting in %2d seconds \r",i);
			for (index=0;index<1000;index++)
				udelay(1000);
		}
		putc('\n');
		do_reset(NULL, 0, 0, NULL);
	}

	puts("FPGA:  ");

	/* display infos on fpgaimage */
	index = 15;
	for (i=0; i<4; i++) {
		len = dst[index];
		printf("%s ", &(dst[index+1]));
		index += len+3;
	}
	putc('\n');

	free(dst);

	/*
	 * Reset FPGA
	 */
	GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_FPGA_RESET);
	udelay(10);
	GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_FPGA_RESET);

	return (0);
}

int do_fpga(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr;

	if (argc < 2)
		return cmd_usage(cmdtp);

	addr = simple_strtoul(argv[1], NULL, 16);

	return do_fpga_boot((unsigned char *)addr);
}

U_BOOT_CMD(
	fpga,     2,     0,      do_fpga,
	"boot FPGA",
	"address size\n    - boot FPGA with gzipped image at <address>"
);

#if defined(CONFIG_CMD_PCI) || defined(CONFIG_PCI)
extern struct pci_controller hose;
extern void pci_ixp_init(struct pci_controller * hose);

void pci_init_board(void)
{
	extern void pci_ixp_init (struct pci_controller *hose);

	pci_ixp_init(&hose);
}
#endif
