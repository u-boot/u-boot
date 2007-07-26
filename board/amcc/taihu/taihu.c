/*
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2005-2007
 * Beijing UD Technology Co., Ltd., taihusupport@amcc.com
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
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <spi.h>
#include <asm/gpio.h>

extern int lcd_init(void);

/*
 * board_early_init_f
 */
int board_early_init_f(void)
{
	lcd_init();

	mtdcr(uicsr, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(uicer, 0x00000000);	/* disable all ints */
	mtdcr(uiccr, 0x00000000);
	mtdcr(uicpr, 0xFFFF7F00);	/* set int polarities */
	mtdcr(uictr, 0x00000000);	/* set int trigger levels */
	mtdcr(uicsr, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(uicvcr, 0x00000001);	/* set vect base=0,INT0 highest priority */

	mtebc(pb3ap, CFG_EBC_PB3AP);	/* memory bank 3 (CPLD_LCM) initialization */
	mtebc(pb3cr, CFG_EBC_PB3CR);

	return 0;
}

/*
 * Check Board Identity:
 */
int checkboard(void)
{
	char *s = getenv("serial#");

	puts("Board: Taihu - AMCC PPC405EP Evaluation Board");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return 0;
}

/*************************************************************************
 *  long int initdram
 *
 ************************************************************************/
long int initdram(int board)
{
	return CFG_SDRAM_SIZE_PER_BANK * CFG_SDRAM_BANKS; /* 128Mbytes */
}

static int do_sw_stat(cmd_tbl_t* cmd_tp, int flags, int argc, char *argv[])
{
	char stat;
	int i;

	stat = in_8((u8 *) CPLD_REG0_ADDR);
	printf("SW2 status: ");
	for (i=0; i<4; i++) /* 4-position */
		printf("%d:%s ", i, stat & (0x08 >> i)?"on":"off");
	printf("\n");
	return 0;
}

U_BOOT_CMD (
	sw2_stat, 1, 1, do_sw_stat,
	"sw2_stat - show status of switch 2\n",
	NULL
	);

static int do_led_ctl(cmd_tbl_t* cmd_tp, int flags, int argc, char *argv[])
{
	int led_no;

	if (argc != 3) {
		printf("%s", cmd_tp->usage);
		return -1;
	}

	led_no = simple_strtoul(argv[1], NULL, 16);
	if (led_no != 1 && led_no != 2) {
		printf("%s", cmd_tp->usage);
		return -1;
	}

	if (strcmp(argv[2],"off") == 0x0) {
		if (led_no == 1)
			gpio_write_bit(30, 1);
		else
			gpio_write_bit(31, 1);
	} else if (strcmp(argv[2],"on") == 0x0) {
		if (led_no == 1)
			gpio_write_bit(30, 0);
		else
			gpio_write_bit(31, 0);
	} else {
		printf("%s", cmd_tp->usage);
		return -1;
	}

	return 0;
}

U_BOOT_CMD (
	led_ctl, 3, 1, do_led_ctl,
	"led_ctl	- make led 1 or 2  on or off\n",
	"<led_no> <on/off>	-  make led <led_no> on/off,\n"
	"\tled_no is 1 or 2\t"
	);

#define SPI_CS_GPIO0	0
#define SPI_SCLK_GPIO14	14
#define SPI_DIN_GPIO15	15
#define SPI_DOUT_GPIO16	16

void spi_scl(int bit)
{
	gpio_write_bit(SPI_SCLK_GPIO14, bit);
}

void spi_sda(int bit)
{
	gpio_write_bit(SPI_DOUT_GPIO16, bit);
}

unsigned char spi_read(void)
{
	return (unsigned char)gpio_read_out_bit(SPI_DIN_GPIO15);
}

void taihu_spi_chipsel(int cs)
{
	gpio_write_bit(SPI_CS_GPIO0, cs);
}

spi_chipsel_type spi_chipsel[]= {
	taihu_spi_chipsel
};

int spi_chipsel_cnt = sizeof(spi_chipsel) / sizeof(spi_chipsel[0]);

#ifdef CONFIG_PCI
static unsigned char int_lines[32] = {
	29, 30, 27, 28, 29, 30, 25, 27,
	29, 30, 27, 28, 29, 30, 27, 28,
	29, 30, 27, 28, 29, 30, 27, 28,
	29, 30, 27, 28, 29, 30, 27, 28};

static void taihu_pci_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	unsigned char int_line = int_lines[PCI_DEV(dev) & 31];

	pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE, int_line);
}

int pci_pre_init(struct pci_controller *hose)
{
	hose->fixup_irq = taihu_pci_fixup_irq;
	return 1;
}
#endif /* CONFIG_PCI */

#ifdef CFG_DRAM_TEST
int testdram(void)
{
	unsigned long *mem = (unsigned long *)0;
	const unsigned long kend = (1024 / sizeof(unsigned long));
	unsigned long k, n;
	unsigned long msr;
	unsigned long total_kbytes = CFG_SDRAM_SIZE_PER_BANK * CFG_SDRAM_BANKS / 1024;

	msr = mfmsr();
	mtmsr(msr & ~(MSR_EE));

	for (k = 0; k < total_kbytes ;
	     ++k, mem += (1024 / sizeof(unsigned long))) {
		if ((k & 1023) == 0)
			printf("%3d MB\r", k / 1024);

		memset(mem, 0xaaaaaaaa, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0xaaaaaaaa) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}

		memset(mem, 0x55555555, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0x55555555) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}
	}
	printf("SDRAM test passes\n");
	mtmsr(msr);

	return 0;
}
#endif /* CFG_DRAM_TEST */
