/*
 * (C) Copyright 2003
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
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
#include <asm/processor.h>
#include <command.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */

/* Prototypes */
int gunzip(void *, int, unsigned char *, unsigned long *);

int board_early_init_f (void)
{
	out32(GPIO0_OR, CONFIG_SYS_NAND0_CE);                 /* set initial outputs     */
	out32(GPIO0_OR, CONFIG_SYS_NAND1_CE);                 /* set initial outputs     */

	/*
	 * IRQ 0-15  405GP internally generated; active high; level sensitive
	 * IRQ 16    405GP internally generated; active low; level sensitive
	 * IRQ 17-24 RESERVED
	 * IRQ 25 (EXT IRQ 0)
	 * IRQ 26 (EXT IRQ 1)
	 * IRQ 27 (EXT IRQ 2)
	 * IRQ 28 (EXT IRQ 3)
	 * IRQ 29 (EXT IRQ 4)
	 * IRQ 30 (EXT IRQ 5)
	 * IRQ 31 (EXT IRQ 6)
	 */
	mtdcr(uicsr, 0xFFFFFFFF);       /* clear all ints */
	mtdcr(uicer, 0x00000000);       /* disable all ints */
	mtdcr(uiccr, 0x00000000);       /* set all to be non-critical*/
	mtdcr(uicpr, 0xFFFFFF80);       /* set int polarities */
	mtdcr(uictr, 0x10000000);       /* set int trigger levels */
	mtdcr(uicvcr, 0x00000001);      /* set vect base=0,INT0 highest priority*/
	mtdcr(uicsr, 0xFFFFFFFF);       /* clear all ints */

	/*
	 * EBC Configuration Register: set ready timeout to 512 ebc-clks -> ca. 15 us
	 */
#if 1 /* test-only */
	mtebc (epcr, 0xa8400000); /* ebc always driven */
#else
	mtebc (epcr, 0x28400000); /* ebc in high-z */
#endif
	return 0;
}

/* ------------------------------------------------------------------------- */

int misc_init_f (void)
{
	return 0;  /* dummy implementation */
}

extern flash_info_t flash_info[];	/* info for FLASH chips */

int misc_init_r (void)
{
	/* adjust flash start and size as well as the offset */
	gd->bd->bi_flashstart = 0 - flash_info[0].size;
	gd->bd->bi_flashoffset= flash_info[0].size - CONFIG_SYS_MONITOR_LEN;
#if 0
	volatile unsigned short *fpga_mode =
		(unsigned short *)((ulong)CONFIG_SYS_FPGA_BASE_ADDR + CONFIG_SYS_FPGA_CTRL);
	volatile unsigned char *duart0_mcr =
		(unsigned char *)((ulong)DUART0_BA + 4);
	volatile unsigned char *duart1_mcr =
		(unsigned char *)((ulong)DUART1_BA + 4);

	bd_t *bd = gd->bd;
	char *	tmp;                    /* Temporary char pointer      */
	unsigned char *dst;
	ulong len = sizeof(fpgadata);
	int status;
	int index;
	int i;
	unsigned long cntrl0Reg;

	dst = malloc(CONFIG_SYS_FPGA_MAX_SIZE);
	if (gunzip (dst, CONFIG_SYS_FPGA_MAX_SIZE, (uchar *)fpgadata, &len) != 0) {
		printf ("GUNZIP ERROR - must RESET board to recover\n");
		do_reset (NULL, 0, 0, NULL);
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
		for (i=20; i>0; i--) {
			printf("Rebooting in %2d seconds \r",i);
			for (index=0;index<1000;index++)
				udelay(1000);
		}
		putc ('\n');
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
	putc ('\n');

	free(dst);

	/*
	 * Reset FPGA via FPGA_DATA pin
	 */
	SET_FPGA(FPGA_PRG | FPGA_CLK);
	udelay(1000); /* wait 1ms */
	SET_FPGA(FPGA_PRG | FPGA_CLK | FPGA_DATA);
	udelay(1000); /* wait 1ms */
#endif

#if 0
	/*
	 * Enable power on PS/2 interface
	 */
	*fpga_mode |= CONFIG_SYS_FPGA_CTRL_PS2_RESET;

	/*
	 * Enable interrupts in exar duart mcr[3]
	 */
	*duart0_mcr = 0x08;
	*duart1_mcr = 0x08;
#endif
	return (0);
}

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	char str[64];
	int i = getenv_r ("serial#", str, sizeof(str));

	puts ("Board: ");

	if (i == -1) {
		puts ("### No HW ID - assuming PPChameleonEVB");
	} else {
		puts(str);
	}

	putc ('\n');

	return 0;
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
	/* TODO: XXX XXX XXX */
	printf ("test: 16 MB - ok\n");

	return (0);
}

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_CFB_CONSOLE
# ifdef CONFIG_CONSOLE_EXTRA_INFO
# include <video_fb.h>
extern GraphicDevice smi;

void video_get_info_str (int line_number, char *info)
{
	uint pvr = get_pvr ();

	/* init video info strings for graphic console */
	switch (line_number) {
	case 1:
		switch (pvr) {
		case PVR_405EP_RB:
			sprintf (info, " AMCC PowerPC 405EP Rev. B");
			break;
		default:
			sprintf (info, " AMCC PowerPC 405EP Rev. <unknown>");
			break;
		}
		return;
	case 2:
		sprintf (info, " DAVE Srl PPChameleonEVB - www.dave-tech.it");
		return;
	case 3:
		sprintf (info, " %s", smi.modeIdent);
		return;
	}

	/* no more info lines */
	*info = 0;
	return;
}
# endif	/* CONFIG_CONSOLE_EXTRA_INFO */
#endif	/* CONFIG_CFB_CONSOLE */
