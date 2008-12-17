/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@freescale.com.
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
#include <mpc5xxx.h>
#include <pci.h>
#include <netdev.h>

#include "sdram.h"

#if CONFIG_TOTAL5200_REV==2
#include "mt48lc32m16a2-75.h"
#else
#include "mt48lc16m16a2-75.h"
#endif

phys_size_t initdram (int board_type)
{
	sdram_conf_t sdram_conf;

	sdram_conf.ddr = SDRAM_DDR;
	sdram_conf.mode = SDRAM_MODE;
	sdram_conf.emode = 0;
	sdram_conf.control = SDRAM_CONTROL;
	sdram_conf.config1 = SDRAM_CONFIG1;
	sdram_conf.config2 = SDRAM_CONFIG2;
#if defined(CONFIG_MPC5200)
	sdram_conf.tapdelay = 0;
#endif
#if defined(CONFIG_MGT5100)
	sdram_conf.addrsel = SDRAM_ADDRSEL;
#endif
	return mpc5xxx_sdram_init (&sdram_conf);
}

int checkboard (void)
{
#if defined(CONFIG_MPC5200)
#if CONFIG_TOTAL5200_REV==2
	puts ("Board: Total5200 Rev.2 ");
#else
	puts ("Board: Total5200 ");
#endif
#elif defined(CONFIG_MGT5100)
	puts ("Board: Total5100 ");
#endif

	/*
	 * Retrieve FPGA Revision.
	 */
	printf ("(FPGA %08lX)\n", *(vu_long *) (CONFIG_SYS_FPGA_BASE + 0x400));

	/*
	 * Take all peripherals in power-up mode.
	 */
#if CONFIG_TOTAL5200_REV==2
	*(vu_char *) (CONFIG_SYS_CPLD_BASE + 0x46) = 0x70;
#else
	*(vu_long *) (CONFIG_SYS_CPLD_BASE + 0x400) = 0x70;
#endif

	return 0;
}

#if defined(CONFIG_MGT5100)
int board_early_init_r(void)
{
	/*
	 * Now, when we are in RAM, enable CS0
	 * because CS_BOOT cannot be written.
	 */
	*(vu_long *)MPC5XXX_ADDECR &= ~(1 << 25); /* disable CS_BOOT */
	*(vu_long *)MPC5XXX_ADDECR |= (1 << 16); /* enable CS0 */

	return 0;
}
#endif

#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc5xxx_init(&hose);
}
#endif

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_RESET)

/* IRDA_1 aka PSC6_3 (pin C13) */
#define GPIO_IRDA_1	0x20000000UL

void init_ide_reset (void)
{
	debug ("init_ide_reset\n");

	/* Configure IRDA_1 (PSC6_3) as GPIO output for ATA reset */
	*(vu_long *) MPC5XXX_GPIO_ENABLE |= GPIO_IRDA_1;
	*(vu_long *) MPC5XXX_GPIO_DIR    |= GPIO_IRDA_1;
}

void ide_set_reset (int idereset)
{
	debug ("ide_reset(%d)\n", idereset);

	if (idereset) {
		*(vu_long *) MPC5XXX_GPIO_DATA_O &= ~GPIO_IRDA_1;
	} else {
		*(vu_long *) MPC5XXX_GPIO_DATA_O |=  GPIO_IRDA_1;
	}
}
#endif

#ifdef CONFIG_VIDEO_SED13806
#include <sed13806.h>

#define DISPLAY_WIDTH   640
#define DISPLAY_HEIGHT  480

#ifdef CONFIG_VIDEO_SED13806_8BPP
#error CONFIG_VIDEO_SED13806_8BPP not supported.
#endif /* CONFIG_VIDEO_SED13806_8BPP */

#ifdef CONFIG_VIDEO_SED13806_16BPP
static const S1D_REGS init_regs [] =
{
    {0x0001,0x00},   /* Miscellaneous Register */
    {0x01FC,0x00},   /* Display Mode Register */
    {0x0004,0x00},   /* General IO Pins Configuration Register 0 */
    {0x0005,0x00},   /* General IO Pins Configuration Register 1 */
    {0x0008,0x00},   /* General IO Pins Control Register 0 */
    {0x0009,0x00},   /* General IO Pins Control Register 1 */
    {0x0010,0x02},   /* Memory Clock Configuration Register */
    {0x0014,0x02},   /* LCD Pixel Clock Configuration Register */
    {0x0018,0x02},   /* CRT/TV Pixel Clock Configuration Register */
    {0x001C,0x02},   /* MediaPlug Clock Configuration Register */
    {0x001E,0x01},   /* CPU To Memory Wait State Select Register */
    {0x0021,0x03},   /* DRAM Refresh Rate Register */
    {0x002A,0x00},   /* DRAM Timings Control Register 0 */
    {0x002B,0x01},   /* DRAM Timings Control Register 1 */
    {0x0020,0x80},   /* Memory Configuration Register */
    {0x0030,0x25},   /* Panel Type Register */
    {0x0031,0x00},   /* MOD Rate Register */
    {0x0032,0x4F},   /* LCD Horizontal Display Width Register */
    {0x0034,0x13},   /* LCD Horizontal Non-Display Period Register */
    {0x0035,0x01},   /* TFT FPLINE Start Position Register */
    {0x0036,0x0B},   /* TFT FPLINE Pulse Width Register */
    {0x0038,0xDF},   /* LCD Vertical Display Height Register 0 */
    {0x0039,0x01},   /* LCD Vertical Display Height Register 1 */
    {0x003A,0x2C},   /* LCD Vertical Non-Display Period Register */
    {0x003B,0x0A},   /* TFT FPFRAME Start Position Register */
    {0x003C,0x01},   /* TFT FPFRAME Pulse Width Register */
    {0x0040,0x05},   /* LCD Display Mode Register */
    {0x0041,0x00},   /* LCD Miscellaneous Register */
    {0x0042,0x00},   /* LCD Display Start Address Register 0 */
    {0x0043,0x00},   /* LCD Display Start Address Register 1 */
    {0x0044,0x00},   /* LCD Display Start Address Register 2 */
    {0x0046,0x80},   /* LCD Memory Address Offset Register 0 */
    {0x0047,0x02},   /* LCD Memory Address Offset Register 1 */
    {0x0048,0x00},   /* LCD Pixel Panning Register */
    {0x004A,0x00},   /* LCD Display FIFO High Threshold Control Register */
    {0x004B,0x00},   /* LCD Display FIFO Low Threshold Control Register */
    {0x0050,0x4F},   /* CRT/TV Horizontal Display Width Register */
    {0x0052,0x13},   /* CRT/TV Horizontal Non-Display Period Register */
    {0x0053,0x01},   /* CRT/TV HRTC Start Position Register */
    {0x0054,0x0B},   /* CRT/TV HRTC Pulse Width Register */
    {0x0056,0xDF},   /* CRT/TV Vertical Display Height Register 0 */
    {0x0057,0x01},   /* CRT/TV Vertical Display Height Register 1 */
    {0x0058,0x2B},   /* CRT/TV Vertical Non-Display Period Register */
    {0x0059,0x09},   /* CRT/TV VRTC Start Position Register */
    {0x005A,0x01},   /* CRT/TV VRTC Pulse Width Register */
    {0x005B,0x10},   /* TV Output Control Register */
    {0x0060,0x05},   /* CRT/TV Display Mode Register */
    {0x0062,0x00},   /* CRT/TV Display Start Address Register 0 */
    {0x0063,0x00},   /* CRT/TV Display Start Address Register 1 */
    {0x0064,0x00},   /* CRT/TV Display Start Address Register 2 */
    {0x0066,0x80},   /* CRT/TV Memory Address Offset Register 0 */
    {0x0067,0x02},   /* CRT/TV Memory Address Offset Register 1 */
    {0x0068,0x00},   /* CRT/TV Pixel Panning Register */
    {0x006A,0x00},   /* CRT/TV Display FIFO High Threshold Control Register */
    {0x006B,0x00},   /* CRT/TV Display FIFO Low Threshold Control Register */
    {0x0070,0x00},   /* LCD Ink/Cursor Control Register */
    {0x0071,0x01},   /* LCD Ink/Cursor Start Address Register */
    {0x0072,0x00},   /* LCD Cursor X Position Register 0 */
    {0x0073,0x00},   /* LCD Cursor X Position Register 1 */
    {0x0074,0x00},   /* LCD Cursor Y Position Register 0 */
    {0x0075,0x00},   /* LCD Cursor Y Position Register 1 */
    {0x0076,0x00},   /* LCD Ink/Cursor Blue Color 0 Register */
    {0x0077,0x00},   /* LCD Ink/Cursor Green Color 0 Register */
    {0x0078,0x00},   /* LCD Ink/Cursor Red Color 0 Register */
    {0x007A,0x1F},   /* LCD Ink/Cursor Blue Color 1 Register */
    {0x007B,0x3F},   /* LCD Ink/Cursor Green Color 1 Register */
    {0x007C,0x1F},   /* LCD Ink/Cursor Red Color 1 Register */
    {0x007E,0x00},   /* LCD Ink/Cursor FIFO Threshold Register */
    {0x0080,0x00},   /* CRT/TV Ink/Cursor Control Register */
    {0x0081,0x01},   /* CRT/TV Ink/Cursor Start Address Register */
    {0x0082,0x00},   /* CRT/TV Cursor X Position Register 0 */
    {0x0083,0x00},   /* CRT/TV Cursor X Position Register 1 */
    {0x0084,0x00},   /* CRT/TV Cursor Y Position Register 0 */
    {0x0085,0x00},   /* CRT/TV Cursor Y Position Register 1 */
    {0x0086,0x00},   /* CRT/TV Ink/Cursor Blue Color 0 Register */
    {0x0087,0x00},   /* CRT/TV Ink/Cursor Green Color 0 Register */
    {0x0088,0x00},   /* CRT/TV Ink/Cursor Red Color 0 Register */
    {0x008A,0x1F},   /* CRT/TV Ink/Cursor Blue Color 1 Register */
    {0x008B,0x3F},   /* CRT/TV Ink/Cursor Green Color 1 Register */
    {0x008C,0x1F},   /* CRT/TV Ink/Cursor Red Color 1 Register */
    {0x008E,0x00},   /* CRT/TV Ink/Cursor FIFO Threshold Register */
    {0x0100,0x00},   /* BitBlt Control Register 0 */
    {0x0101,0x00},   /* BitBlt Control Register 1 */
    {0x0102,0x00},   /* BitBlt ROP Code/Color Expansion Register */
    {0x0103,0x00},   /* BitBlt Operation Register */
    {0x0104,0x00},   /* BitBlt Source Start Address Register 0 */
    {0x0105,0x00},   /* BitBlt Source Start Address Register 1 */
    {0x0106,0x00},   /* BitBlt Source Start Address Register 2 */
    {0x0108,0x00},   /* BitBlt Destination Start Address Register 0 */
    {0x0109,0x00},   /* BitBlt Destination Start Address Register 1 */
    {0x010A,0x00},   /* BitBlt Destination Start Address Register 2 */
    {0x010C,0x00},   /* BitBlt Memory Address Offset Register 0 */
    {0x010D,0x00},   /* BitBlt Memory Address Offset Register 1 */
    {0x0110,0x00},   /* BitBlt Width Register 0 */
    {0x0111,0x00},   /* BitBlt Width Register 1 */
    {0x0112,0x00},   /* BitBlt Height Register 0 */
    {0x0113,0x00},   /* BitBlt Height Register 1 */
    {0x0114,0x00},   /* BitBlt Background Color Register 0 */
    {0x0115,0x00},   /* BitBlt Background Color Register 1 */
    {0x0118,0x00},   /* BitBlt Foreground Color Register 0 */
    {0x0119,0x00},   /* BitBlt Foreground Color Register 1 */
    {0x01E0,0x00},   /* Look-Up Table Mode Register */
    {0x01E2,0x00},   /* Look-Up Table Address Register */
    {0x01E4,0x00},   /* Look-Up Table Data Register */
    {0x01F0,0x00},   /* Power Save Configuration Register */
    {0x01F1,0x00},   /* Power Save Status Register */
    {0x01F4,0x00},   /* CPU-to-Memory Access Watchdog Timer Register */
    {0x01FC,0x01},   /* Display Mode Register */
    {0, 0}
};
#endif /* CONFIG_VIDEO_SED13806_16BPP */

#ifdef CONFIG_CONSOLE_EXTRA_INFO
/* Return text to be printed besides the logo. */
void video_get_info_str (int line_number, char *info)
{
	if (line_number == 1) {
#ifdef CONFIG_MGT5100
		strcpy (info, " Total5100");
#elif CONFIG_TOTAL5200_REV==1
		strcpy (info, " Total5200");
#elif CONFIG_TOTAL5200_REV==2
		strcpy (info, " Total5200 Rev.2");
#else
#error CONFIG_TOTAL5200_REV must be 1 or 2.
#endif
	} else {
		info [0] = '\0';
	}
}
#endif

/* Returns  SED13806 base address. First thing called in the driver. */
unsigned int board_video_init (void)
{
	return CONFIG_SYS_LCD_BASE;
}

/* Called after initializing the SED13806 and before clearing the screen. */
void board_validate_screen (unsigned int base)
{
}

/* Return a pointer to the initialization sequence. */
const S1D_REGS *board_get_regs (void)
{
	return init_regs;
}

int board_get_width (void)
{
	return DISPLAY_WIDTH;
}

int board_get_height (void)
{
	return DISPLAY_HEIGHT;
}

#endif /* CONFIG_VIDEO_SED13806 */

int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis); /* Built in FEC comes first */
	return pci_eth_init(bis);
}
