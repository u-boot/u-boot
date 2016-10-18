/*
 * (C) Copyright 2003-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2004-2006
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <console.h>
#include <mpc5xxx.h>
#include <pci.h>
#include <asm/processor.h>
#include <libfdt.h>
#include <netdev.h>
#include <video.h>

#ifdef CONFIG_VIDEO_SM501
#include <sm501.h>
#endif

#if defined(CONFIG_MPC5200_DDR)
#include "mt46v16m16-75.h"
#else
#include "mt48lc16m16a2-75.h"
#endif

#ifdef CONFIG_OF_LIBFDT
#include <fdt_support.h>
#endif /* CONFIG_OF_LIBFDT */

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_PS2MULT
void ps2mult_early_init(void);
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) && \
    defined(CONFIG_VIDEO)
/*
 * EDID block has been generated using Phoenix EDID Designer 1.3.
 * This tool creates a text file containing:
 *
 * EDID BYTES:
 *
 * 0x   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
 *     ------------------------------------------------
 *     00 | 00 FF FF FF FF FF FF 00 04 21 00 00 00 00 00 00
 *     10 | 01 00 01 03 00 00 00 00 00 00 00 00 00 00 00 00
 *     20 | 00 00 00 21 00 00 01 01 01 01 01 01 01 01 01 01
 *     30 | 01 01 01 01 01 01 64 00 00 00 00 00 00 00 00 00
 *     40 | 00 00 00 00 00 00 00 00 00 00 00 10 00 00 00 00
 *     50 | 00 00 00 00 00 00 00 00 00 00 00 00 00 10 00 00
 *     60 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10
 *     70 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 17
 *
 * Then this data has been manually converted to the char
 * array below.
 */
static unsigned char edid_buf[128] = {
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
	0x04, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x64, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17,
};
#endif

#ifndef CONFIG_SYS_RAMBOOT
static void sdram_start (int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000000 |
		hi_addr_bit;
	__asm__ volatile ("sync");

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000002 |
		hi_addr_bit;
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set mode register: extended mode */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_EMODE;
	__asm__ volatile ("sync");

	/* set mode register: reset DLL */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_MODE | 0x04000000;
	__asm__ volatile ("sync");
#endif

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000002 |
		hi_addr_bit;
	__asm__ volatile ("sync");

	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000004 |
		hi_addr_bit;
	__asm__ volatile ("sync");

	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_MODE;
	__asm__ volatile ("sync");

	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | hi_addr_bit;
	__asm__ volatile ("sync");
}
#endif

/*
 * ATTENTION: Although partially referenced initdram does NOT make real use
 *	      use of CONFIG_SYS_SDRAM_BASE. The code does not work if CONFIG_SYS_SDRAM_BASE
 *	      is something else than 0x00000000.
 */

phys_size_t initdram (int board_type)
{
	ulong dramsize = 0;
	ulong dramsize2 = 0;
	uint svr, pvr;

#ifndef CONFIG_SYS_RAMBOOT
	ulong test1, test2;

	/* setup SDRAM chip selects */
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x0000001c; /* 512MB at 0x0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = 0x40000000; /* disabled */
	__asm__ volatile ("sync");

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = SDRAM_CONFIG1;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = SDRAM_CONFIG2;
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set tap delay */
	*(vu_long *)MPC5XXX_CDM_PORCFG = SDRAM_TAPDELAY;
	__asm__ volatile ("sync");
#endif

	/* find RAM size using SDRAM CS0 only */
	sdram_start(0);
	test1 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x20000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x20000000);
	if (test1 > test2) {
		sdram_start(0);
		dramsize = test1;
	} else {
		dramsize = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize < (1 << 20)) {
		dramsize = 0;
	}

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0) {
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x13 +
			__builtin_ffs(dramsize >> 20) - 1;
	} else {
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0; /* disabled */
	}

	/* let SDRAM CS1 start right after CS0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize + 0x0000001c; /* 512MB */

	/* find RAM size using SDRAM CS1 only */
	if (!dramsize)
		sdram_start(0);
	test2 = test1 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize), 0x20000000);
	if (!dramsize) {
		sdram_start(1);
		test2 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize), 0x20000000);
	}
	if (test1 > test2) {
		sdram_start(0);
		dramsize2 = test1;
	} else {
		dramsize2 = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize2 < (1 << 20)) {
		dramsize2 = 0;
	}

	/* set SDRAM CS1 size according to the amount of RAM found */
	if (dramsize2 > 0) {
		*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize
			| (0x13 + __builtin_ffs(dramsize2 >> 20) - 1);
	} else {
		*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize; /* disabled */
	}

#else /* CONFIG_SYS_RAMBOOT */

	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = *(vu_long *)MPC5XXX_SDRAM_CS0CFG & 0xFF;
	if (dramsize >= 0x13) {
		dramsize = (1 << (dramsize - 0x13)) << 20;
	} else {
		dramsize = 0;
	}

	/* retrieve size of memory connected to SDRAM CS1 */
	dramsize2 = *(vu_long *)MPC5XXX_SDRAM_CS1CFG & 0xFF;
	if (dramsize2 >= 0x13) {
		dramsize2 = (1 << (dramsize2 - 0x13)) << 20;
	} else {
		dramsize2 = 0;
	}
#endif /* CONFIG_SYS_RAMBOOT */

	/*
	 * On MPC5200B we need to set the special configuration delay in the
	 * DDR controller. Please refer to Freescale's AN3221 "MPC5200B SDRAM
	 * Initialization and Configuration", 3.3.1 SDelay--MBAR + 0x0190:
	 *
	 * "The SDelay should be written to a value of 0x00000004. It is
	 * required to account for changes caused by normal wafer processing
	 * parameters."
	 */
	svr = get_svr();
	pvr = get_pvr();
	if ((SVR_MJREV(svr) >= 2) &&
	    (PVR_MAJ(pvr) == 1) && (PVR_MIN(pvr) == 4)) {

		*(vu_long *)MPC5XXX_SDRAM_SDELAY = 0x04;
		__asm__ volatile ("sync");
	}

#if defined(CONFIG_TQM5200_B)
	return dramsize + dramsize2;
#else
	return dramsize;
#endif /* CONFIG_TQM5200_B */
}

int checkboard (void)
{
#if defined(CONFIG_TQM5200S)
# define MODULE_NAME	"TQM5200S"
#else
# define MODULE_NAME	"TQM5200"
#endif

#if defined(CONFIG_STK52XX)
# define CARRIER_NAME	"STK52xx"
#elif defined(CONFIG_CAM5200)
# define CARRIER_NAME	"CAM5200"
#elif defined(CONFIG_FO300)
# define CARRIER_NAME	"FO300"
#elif defined(CONFIG_CHARON)
# define CARRIER_NAME	"CHARON"
#else
# error "UNKNOWN"
#endif

	puts (	"Board: " MODULE_NAME " (TQ-Components GmbH)\n"
		"       on a " CARRIER_NAME " carrier board\n");

	return 0;
}

#undef MODULE_NAME
#undef CARRIER_NAME

void flash_preinit(void)
{
	/*
	 * Now, when we are in RAM, enable flash write
	 * access for detection process.
	 * Note that CS_BOOT cannot be cleared when
	 * executing in flash.
	 */
	*(vu_long *)MPC5XXX_BOOTCS_CFG &= ~0x1; /* clear RO */
}


#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc5xxx_init(&hose);
}
#endif

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_RESET)

#if defined (CONFIG_MINIFAP)
#define SM501_POWER_MODE0_GATE		0x00000040UL
#define SM501_POWER_MODE1_GATE		0x00000048UL
#define POWER_MODE_GATE_GPIO_PWM_I2C	0x00000040UL
#define SM501_GPIO_DATA_DIR_HIGH	0x0001000CUL
#define SM501_GPIO_DATA_HIGH		0x00010004UL
#define SM501_GPIO_51			0x00080000UL
#endif /* CONFIG MINIFAP */

void init_ide_reset (void)
{
	debug ("init_ide_reset\n");

#if defined (CONFIG_MINIFAP)
	/* Configure GPIO_51 of the SM501 grafic controller as ATA reset */

	/* enable GPIO control (in both power modes) */
	*(vu_long *) (SM501_MMIO_BASE+SM501_POWER_MODE0_GATE) |=
		POWER_MODE_GATE_GPIO_PWM_I2C;
	*(vu_long *) (SM501_MMIO_BASE+SM501_POWER_MODE1_GATE) |=
		POWER_MODE_GATE_GPIO_PWM_I2C;
	/* configure GPIO51 as output */
	*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_DIR_HIGH) |=
		SM501_GPIO_51;
#else
	/* Configure PSC1_4 as GPIO output for ATA reset */
	*(vu_long *) MPC5XXX_WU_GPIO_ENABLE |= GPIO_PSC1_4;
	*(vu_long *) MPC5XXX_WU_GPIO_DIR    |= GPIO_PSC1_4;

	/* by default the ATA reset is de-asserted */
	*(vu_long *) MPC5XXX_WU_GPIO_DATA_O |=  GPIO_PSC1_4;
#endif
}

void ide_set_reset (int idereset)
{
	debug ("ide_reset(%d)\n", idereset);

#if defined (CONFIG_MINIFAP)
	if (idereset) {
		*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_HIGH) &=
			~SM501_GPIO_51;
	} else {
		*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_HIGH) |=
			SM501_GPIO_51;
	}
#else
	if (idereset) {
		*(vu_long *) MPC5XXX_WU_GPIO_DATA_O &= ~GPIO_PSC1_4;
	} else {
		*(vu_long *) MPC5XXX_WU_GPIO_DATA_O |=  GPIO_PSC1_4;
	}
#endif
}
#endif

#ifdef CONFIG_POST
/*
 * Reads GPIO pin PSC6_3. A keypress is reported, if PSC6_3 is low. If PSC6_3
 * is left open, no keypress is detected.
 */
int post_hotkeys_pressed(void)
{
#ifdef CONFIG_STK52XX
	struct mpc5xxx_gpio *gpio;

	gpio = (struct mpc5xxx_gpio*) MPC5XXX_GPIO;

	/*
	 * Configure PSC6_0 through PSC6_3 as GPIO.
	 */
	gpio->port_config &= ~(0x00700000);

	/* Enable GPIO for GPIO_IRDA_1 (IR_USB_CLK pin) = PSC6_3 */
	gpio->simple_gpioe |= 0x20000000;

	/* Configure GPIO_IRDA_1 as input */
	gpio->simple_ddr &= ~(0x20000000);

	return ((gpio->simple_ival & 0x20000000) ? 0 : 1);
#else
	return 0;
#endif
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_R
int board_early_init_r (void)
{

	extern int usb_cpu_init(void);

#ifdef CONFIG_PS2MULT
	ps2mult_early_init();
#endif /* CONFIG_PS2MULT */

#if defined(CONFIG_USB_OHCI_NEW) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT)
	/* Low level USB init, required for proper kernel operation */
	usb_cpu_init();
#endif

	return (0);
}
#endif

#ifdef CONFIG_FO300
int silent_boot (void)
{
	vu_long timer3_status;

	/* Configure GPT3 as GPIO input */
	*(vu_long *)MPC5XXX_GPT3_ENABLE = 0x00000004;

	/* Read in TIMER_3 pin status */
	timer3_status = *(vu_long *)MPC5XXX_GPT3_STATUS;

#ifdef FO300_SILENT_CONSOLE_WHEN_S1_CLOSED
	/* Force silent console mode if S1 switch
	 * is in closed position (TIMER_3 pin status is LOW). */
	if (MPC5XXX_GPT_GPIO_PIN(timer3_status) == 0)
		return 1;
#else
	/* Force silent console mode if S1 switch
	 * is in open position (TIMER_3 pin status is HIGH). */
	if (MPC5XXX_GPT_GPIO_PIN(timer3_status) == 1)
		return 1;
#endif

	return 0;
}

int board_early_init_f (void)
{
	if (silent_boot())
		gd->flags |= GD_FLG_SILENT;

	return 0;
}
#endif	/* CONFIG_FO300 */

#if defined(CONFIG_CHARON)
#include <i2c.h>
#include <asm/io.h>

/* The TFP410 registers */
#define TFP410_REG_VEN_ID_L 0x00
#define TFP410_REG_VEN_ID_H 0x01
#define TFP410_REG_DEV_ID_L 0x02
#define TFP410_REG_DEV_ID_H 0x03
#define TFP410_REG_REV_ID 0x04

#define TFP410_REG_CTL_1_MODE 0x08
#define TFP410_REG_CTL_2_MODE 0x09
#define TFP410_REG_CTL_3_MODE 0x0A

#define TFP410_REG_CFG 0x0B

#define TFP410_REG_DE_DLY 0x32
#define TFP410_REG_DE_CTL 0x33
#define TFP410_REG_DE_TOP 0x34
#define TFP410_REG_DE_CNT_L 0x36
#define TFP410_REG_DE_CNT_H 0x37
#define TFP410_REG_DE_LIN_L 0x38
#define TFP410_REG_DE_LIN_H 0x39

#define TFP410_REG_H_RES_L 0x3A
#define TFP410_REG_H_RES_H 0x3B
#define TFP410_REG_V_RES_L 0x3C
#define TFP410_REG_V_RES_H 0x3D

static int tfp410_read_reg(int reg, uchar *buf)
{
	if (i2c_read(CONFIG_SYS_TFP410_ADDR, reg, 1, buf, 1) != 0) {
		puts ("Error reading the chip.\n");
		return 1;
	}
	return 0;
}

static int tfp410_write_reg(int reg, uchar buf)
{
	if (i2c_write(CONFIG_SYS_TFP410_ADDR, reg, 1, &buf, 1) != 0) {
		puts ("Error writing the chip.\n");
		return 1;
	}
	return 0;
}

typedef struct _tfp410_config {
	int	reg;
	uchar	val;
}TFP410_CONFIG;

static TFP410_CONFIG tfp410_configtbl[] = {
	{TFP410_REG_CTL_1_MODE, 0x37},
	{TFP410_REG_CTL_2_MODE, 0x20},
	{TFP410_REG_CTL_3_MODE, 0x80},
	{TFP410_REG_DE_DLY, 0x90},
	{TFP410_REG_DE_CTL, 0x00},
	{TFP410_REG_DE_TOP, 0x23},
	{TFP410_REG_DE_CNT_H, 0x02},
	{TFP410_REG_DE_CNT_L, 0x80},
	{TFP410_REG_DE_LIN_H, 0x01},
	{TFP410_REG_DE_LIN_L, 0xe0},
	{-1, 0},
};

static int charon_last_stage_init(void)
{
	volatile struct mpc5xxx_lpb *lpb =
		(struct mpc5xxx_lpb *) MPC5XXX_LPB;
	int	oldbus = i2c_get_bus_num();
	uchar	buf;
	int	i = 0;

	i2c_set_bus_num(CONFIG_SYS_TFP410_BUS);

	/* check version */
	if (tfp410_read_reg(TFP410_REG_DEV_ID_H, &buf) != 0)
		return -1;
	if (!(buf & 0x04))
		return -1;
	if (tfp410_read_reg(TFP410_REG_DEV_ID_L, &buf) != 0)
		return -1;
	if (!(buf & 0x10))
		return -1;
	/* OK, now init the chip */
	while (tfp410_configtbl[i].reg != -1) {
		int ret;

		ret = tfp410_write_reg(tfp410_configtbl[i].reg,
				tfp410_configtbl[i].val);
		if (ret != 0)
			return -1;
		i++;
	}
	printf("TFP410 initialized.\n");
	i2c_set_bus_num(oldbus);

	/* set deadcycle for cs3 to 0 */
	setbits_be32(&lpb->cs_deadcycle, 0xffffcfff);
	return 0;
}
#endif

int last_stage_init (void)
{
	/*
	 * auto scan for really existing devices and re-set chip select
	 * configuration.
	 */
	u16 save, tmp;
	int restore;

	/*
	 * Check for SRAM and SRAM size
	 */

	/* save original SRAM content  */
	save = *(volatile u16 *)CONFIG_SYS_CS2_START;
	restore = 1;

	/* write test pattern to SRAM */
	*(volatile u16 *)CONFIG_SYS_CS2_START = 0xA5A5;
	__asm__ volatile ("sync");
	/*
	 * Put a different pattern on the data lines: otherwise they may float
	 * long enough to read back what we wrote.
	 */
	tmp = *(volatile u16 *)CONFIG_SYS_FLASH_BASE;
	if (tmp == 0xA5A5)
		puts ("!! possible error in SRAM detection\n");

	if (*(volatile u16 *)CONFIG_SYS_CS2_START != 0xA5A5) {
		/* no SRAM at all, disable cs */
		*(vu_long *)MPC5XXX_ADDECR &= ~(1 << 18);
		*(vu_long *)MPC5XXX_CS2_START = 0x0000FFFF;
		*(vu_long *)MPC5XXX_CS2_STOP = 0x0000FFFF;
		restore = 0;
		__asm__ volatile ("sync");
	} else if (*(volatile u16 *)(CONFIG_SYS_CS2_START + (1<<19)) == 0xA5A5) {
		/* make sure that we access a mirrored address */
		*(volatile u16 *)CONFIG_SYS_CS2_START = 0x1111;
		__asm__ volatile ("sync");
		if (*(volatile u16 *)(CONFIG_SYS_CS2_START + (1<<19)) == 0x1111) {
			/* SRAM size = 512 kByte */
			*(vu_long *)MPC5XXX_CS2_STOP = STOP_REG(CONFIG_SYS_CS2_START,
								0x80000);
			__asm__ volatile ("sync");
			puts ("SRAM:  512 kB\n");
		}
		else
			puts ("!! possible error in SRAM detection\n");
	} else {
		puts ("SRAM:  1 MB\n");
	}
	/* restore origianl SRAM content  */
	if (restore) {
		*(volatile u16 *)CONFIG_SYS_CS2_START = save;
		__asm__ volatile ("sync");
	}

#ifndef CONFIG_TQM5200S	/* The TQM5200S has no SM501 grafic controller */
	/*
	 * Check for Grafic Controller
	 */

	/* save origianl FB content  */
	save = *(volatile u16 *)CONFIG_SYS_CS1_START;
	restore = 1;

	/* write test pattern to FB memory */
	*(volatile u16 *)CONFIG_SYS_CS1_START = 0xA5A5;
	__asm__ volatile ("sync");
	/*
	 * Put a different pattern on the data lines: otherwise they may float
	 * long enough to read back what we wrote.
	 */
	tmp = *(volatile u16 *)CONFIG_SYS_FLASH_BASE;
	if (tmp == 0xA5A5)
		puts ("!! possible error in grafic controller detection\n");

	if (*(volatile u16 *)CONFIG_SYS_CS1_START != 0xA5A5) {
		/* no grafic controller at all, disable cs */
		*(vu_long *)MPC5XXX_ADDECR &= ~(1 << 17);
		*(vu_long *)MPC5XXX_CS1_START = 0x0000FFFF;
		*(vu_long *)MPC5XXX_CS1_STOP = 0x0000FFFF;
		restore = 0;
		__asm__ volatile ("sync");
	} else {
		puts ("VGA:   SMI501 (Voyager) with 8 MB\n");
	}
	/* restore origianl FB content  */
	if (restore) {
		*(volatile u16 *)CONFIG_SYS_CS1_START = save;
		__asm__ volatile ("sync");
	}

#ifdef CONFIG_FO300
	if (silent_boot()) {
		setenv("bootdelay", "0");
		disable_ctrlc(1);
	}
#endif
#endif /* !CONFIG_TQM5200S */

#if defined(CONFIG_CHARON)
	charon_last_stage_init();
#endif
	return 0;
}

#ifdef CONFIG_VIDEO_SM501

#ifdef CONFIG_FO300
#define DISPLAY_WIDTH   800
#else
#define DISPLAY_WIDTH   640
#endif
#define DISPLAY_HEIGHT  480

#ifdef CONFIG_VIDEO_SM501_8BPP
#error CONFIG_VIDEO_SM501_8BPP not supported.
#endif /* CONFIG_VIDEO_SM501_8BPP */

#ifdef CONFIG_VIDEO_SM501_16BPP
#error CONFIG_VIDEO_SM501_16BPP not supported.
#endif /* CONFIG_VIDEO_SM501_16BPP */
#ifdef CONFIG_VIDEO_SM501_32BPP
static const SMI_REGS init_regs [] =
{
#if 0 /* CRT only */
	{0x00004, 0x0},
	{0x00048, 0x00021807},
	{0x0004C, 0x10090a01},
	{0x00054, 0x1},
	{0x00040, 0x00021807},
	{0x00044, 0x10090a01},
	{0x00054, 0x0},
	{0x80200, 0x00010000},
	{0x80204, 0x0},
	{0x80208, 0x0A000A00},
	{0x8020C, 0x02fa027f},
	{0x80210, 0x004a028b},
	{0x80214, 0x020c01df},
	{0x80218, 0x000201e9},
	{0x80200, 0x00013306},
#else  /* panel + CRT */
#ifdef CONFIG_FO300
	{0x00004, 0x0},
	{0x00048, 0x00021807},
	{0x0004C, 0x301a0a01},
	{0x00054, 0x1},
	{0x00040, 0x00021807},
	{0x00044, 0x091a0a01},
	{0x00054, 0x0},
	{0x80000, 0x0f013106},
	{0x80004, 0xc428bb17},
	{0x8000C, 0x00000000},
	{0x80010, 0x0C800C80},
	{0x80014, 0x03200000},
	{0x80018, 0x01e00000},
	{0x8001C, 0x00000000},
	{0x80020, 0x01e00320},
	{0x80024, 0x042a031f},
	{0x80028, 0x0086034a},
	{0x8002C, 0x020c01df},
	{0x80030, 0x000201ea},
	{0x80200, 0x00010000},
#else
	{0x00004, 0x0},
	{0x00048, 0x00021807},
	{0x0004C, 0x091a0a01},
	{0x00054, 0x1},
	{0x00040, 0x00021807},
	{0x00044, 0x091a0a01},
	{0x00054, 0x0},
	{0x80000, 0x0f013106},
	{0x80004, 0xc428bb17},
	{0x8000C, 0x00000000},
	{0x80010, 0x0a000a00},
	{0x80014, 0x02800000},
	{0x80018, 0x01e00000},
	{0x8001C, 0x00000000},
	{0x80020, 0x01e00280},
	{0x80024, 0x02fa027f},
	{0x80028, 0x004a028b},
	{0x8002C, 0x020c01df},
	{0x80030, 0x000201e9},
	{0x80200, 0x00010000},
#endif /* #ifdef CONFIG_FO300 */
#endif
	{0, 0}
};
#endif /* CONFIG_VIDEO_SM501_32BPP */

#ifdef CONFIG_CONSOLE_EXTRA_INFO
/*
 * Return text to be printed besides the logo.
 */
void video_get_info_str (int line_number, char *info)
{
	if (line_number == 1) {
	strcpy (info, " Board: TQM5200 (TQ-Components GmbH)");
#if defined (CONFIG_CHARON) || defined (CONFIG_FO300) || \
	defined(CONFIG_STK52XX)
	} else if (line_number == 2) {
#if defined (CONFIG_CHARON)
		strcpy (info, "        on a CHARON carrier board");
#endif
#if defined (CONFIG_STK52XX)
		strcpy (info, "        on a STK52xx carrier board");
#endif
#if defined (CONFIG_FO300)
		strcpy (info, "        on a FO300 carrier board");
#endif
#endif
	}
	else {
		info [0] = '\0';
	}
}
#endif

/*
 * Returns SM501 register base address. First thing called in the
 * driver. Checks if SM501 is physically present.
 */
unsigned int board_video_init (void)
{
	u16 save, tmp;
	int restore, ret;

	/*
	 * Check for Grafic Controller
	 */

	/* save origianl FB content  */
	save = *(volatile u16 *)CONFIG_SYS_CS1_START;
	restore = 1;

	/* write test pattern to FB memory */
	*(volatile u16 *)CONFIG_SYS_CS1_START = 0xA5A5;
	__asm__ volatile ("sync");
	/*
	 * Put a different pattern on the data lines: otherwise they may float
	 * long enough to read back what we wrote.
	 */
	tmp = *(volatile u16 *)CONFIG_SYS_FLASH_BASE;
	if (tmp == 0xA5A5)
		puts ("!! possible error in grafic controller detection\n");

	if (*(volatile u16 *)CONFIG_SYS_CS1_START != 0xA5A5) {
		/* no grafic controller found */
		restore = 0;
		ret = 0;
	} else {
		ret = SM501_MMIO_BASE;
	}

	if (restore) {
		*(volatile u16 *)CONFIG_SYS_CS1_START = save;
		__asm__ volatile ("sync");
	}
	return ret;
}

/*
 * Returns SM501 framebuffer address
 */
unsigned int board_video_get_fb (void)
{
	return SM501_FB_BASE;
}

/*
 * Called after initializing the SM501 and before clearing the screen.
 */
void board_validate_screen (unsigned int base)
{
}

/*
 * Return a pointer to the initialization sequence.
 */
const SMI_REGS *board_get_regs (void)
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

#endif /* CONFIG_VIDEO_SM501 */

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#if defined(CONFIG_VIDEO)
	fdt_add_edid(blob, "smi,sm501", edid_buf);
#endif

	return 0;
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

#if defined(CONFIG_RESET_PHY_R)
#include <miiphy.h>

void reset_phy(void)
{
	/* init Micrel KSZ8993 PHY */
	miiphy_write("FEC", CONFIG_PHY_ADDR, 0x01, 0x09);
}
#endif

int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis); /* Built in FEC comes first */
	return pci_eth_init(bis);
}
