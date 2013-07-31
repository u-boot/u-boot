/*
 * (C) Copyright 2009 Wolfgang Denk <wd@denx.de>
 * (C) Copyright 2009 Dave Srl www.dave.eu
 * (C) Copyright 2010 ifm ecomatic GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/bitops.h>
#include <command.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/mpc512x.h>
#include <fdt_support.h>
#ifdef CONFIG_MISC_INIT_R
#include <i2c.h>
#endif

static int eeprom_diag;
static int mac_diag;
static int gpio_diag;

DECLARE_GLOBAL_DATA_PTR;

static void gpio_configure(void)
{
	immap_t *im;
	gpio512x_t *gpioregs;

	im = (immap_t *) CONFIG_SYS_IMMR;
	gpioregs = &im->gpio;
	out_be32(&gpioregs->gpodr, 0x00290000); /* open drain */
	out_be32(&gpioregs->gpdat, 0x80001040); /* data (when output) */

	/*
	 * out_be32(&gpioregs->gpdir, 0xC2293020);
	 * workaround for a hardware effect: configure direction in pieces,
	 * setting all outputs at once drops the reset line too low and
	 * makes us lose the MII connection (breaks ethernet for us)
	 */
	out_be32(&gpioregs->gpdir, 0x02003060); /* direction */
	setbits_be32(&gpioregs->gpdir, 0x00200000); /* += reset asi */
	udelay(10);
	setbits_be32(&gpioregs->gpdir, 0x00080000); /* += reset safety */
	udelay(10);
	setbits_be32(&gpioregs->gpdir, 0x00010000); /* += reset comm */
	udelay(10);
	setbits_be32(&gpioregs->gpdir, 0xC0000000); /* += backlight, KB sel */

	/* to turn from red to yellow when U-Boot runs */
	setbits_be32(&gpioregs->gpdat, 0x00002020);
	out_be32(&gpioregs->gpimr, 0x00000000); /* interrupt mask */
	out_be32(&gpioregs->gpicr1, 0x00000004); /* interrupt sense part 1 */
	out_be32(&gpioregs->gpicr2, 0x00A80000); /* interrupt sense part 2 */
	out_be32(&gpioregs->gpier, 0xFFFFFFFF); /* interrupt events, clear */
}

/* the physical location of the pins */
#define GPIOKEY_ROW_BITMASK	0x40000000
#define GPIOKEY_ROW_UPPER	0
#define GPIOKEY_ROW_LOWER	1

#define GPIOKEY_COL0_BITMASK	0x20000000
#define GPIOKEY_COL1_BITMASK	0x10000000
#define GPIOKEY_COL2_BITMASK	0x08000000

/* the logical presentation of pressed keys */
#define GPIOKEY_BIT_FNLEFT	(1 << 5)
#define GPIOKEY_BIT_FNRIGHT	(1 << 4)
#define GPIOKEY_BIT_DIRUP	(1 << 3)
#define GPIOKEY_BIT_DIRLEFT	(1 << 2)
#define GPIOKEY_BIT_DIRRIGHT	(1 << 1)
#define GPIOKEY_BIT_DIRDOWN	(1 << 0)

/* the hotkey combination which starts recovery */
#define GPIOKEY_BITS_RECOVERY	(GPIOKEY_BIT_FNLEFT | GPIOKEY_BIT_DIRUP | \
				 GPIOKEY_BIT_DIRDOWN)

static void gpio_selectrow(gpio512x_t *gpioregs, u32 row)
{

	if (row)
		setbits_be32(&gpioregs->gpdat, GPIOKEY_ROW_BITMASK);
	else
		clrbits_be32(&gpioregs->gpdat, GPIOKEY_ROW_BITMASK);
	udelay(10);
}

static u32 gpio_querykbd(void)
{
	immap_t *im;
	gpio512x_t *gpioregs;
	u32 keybits;
	u32 input;

	im = (immap_t *)CONFIG_SYS_IMMR;
	gpioregs = &im->gpio;
	keybits = 0;

	/* query upper row */
	gpio_selectrow(gpioregs, GPIOKEY_ROW_UPPER);
	input = in_be32(&gpioregs->gpdat);
	if ((input & GPIOKEY_COL0_BITMASK) == 0)
		keybits |= GPIOKEY_BIT_FNLEFT;
	if ((input & GPIOKEY_COL1_BITMASK) == 0)
		keybits |= GPIOKEY_BIT_DIRUP;
	if ((input & GPIOKEY_COL2_BITMASK) == 0)
		keybits |= GPIOKEY_BIT_FNRIGHT;

	/* query lower row */
	gpio_selectrow(gpioregs, GPIOKEY_ROW_LOWER);
	input = in_be32(&gpioregs->gpdat);
	if ((input & GPIOKEY_COL0_BITMASK) == 0)
		keybits |= GPIOKEY_BIT_DIRLEFT;
	if ((input & GPIOKEY_COL1_BITMASK) == 0)
		keybits |= GPIOKEY_BIT_DIRRIGHT;
	if ((input & GPIOKEY_COL2_BITMASK) == 0)
		keybits |= GPIOKEY_BIT_DIRDOWN;

	/* return bit pattern for keys */
	return keybits;
}

/* excerpt from the recovery's hw_info.h */

struct __attribute__ ((__packed__)) eeprom_layout {
	char	magic[3];	/** 'ifm' */
	u8	len[2];		/** content length without magic/len fields */
	u8	version[3];	/** structure version */
	u8	type;		/** type of PCB */
	u8	reserved[0x37];	/** padding up to offset 0x40 */
	u8	macaddress[6];	/** ethernet MAC (for the mainboard) @0x40 */
};

#define HW_COMP_MAINCPU 2

static struct eeprom_layout eeprom_content;
static int eeprom_was_read;	/* has_been_read */
static int eeprom_is_valid;
static int eeprom_version;

#define get_eeprom_field_int(name) ({ \
	int value; \
	int idx; \
	value = 0; \
	for (idx = 0; idx < sizeof(name); idx++) { \
		value <<= 8; \
		value |= name[idx]; \
	} \
	value; \
})

static int read_eeprom(void)
{
	int eeprom_datalen;
	int ret;

	if (eeprom_was_read)
		return 0;

	eeprom_is_valid = 0;
	ret = i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0,
			CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
			(uchar *)&eeprom_content, sizeof(eeprom_content));
	if (eeprom_diag) {
		printf("DIAG: %s() read rc[%d], size[%d]\n",
			__func__, ret, sizeof(eeprom_content));
	}

	if (ret != 0)
		return -1;

	eeprom_was_read = 1;

	/*
	 * check validity of EEPROM content
	 * (check version, length, optionally checksum)
	 */
	eeprom_is_valid = 1;
	eeprom_datalen = get_eeprom_field_int(eeprom_content.len);
	eeprom_version = get_eeprom_field_int(eeprom_content.version);

	if (eeprom_diag) {
		printf("DIAG: %s() magic[%c%c%c] len[%d] ver[%d] type[%d]\n",
			__func__, eeprom_content.magic[0],
			eeprom_content.magic[1], eeprom_content.magic[2],
			eeprom_datalen, eeprom_version, eeprom_content.type);
	}
	if (strncmp(eeprom_content.magic, "ifm", strlen("ifm")) != 0)
		eeprom_is_valid = 0;
	if (eeprom_datalen < sizeof(struct eeprom_layout) - 5)
		eeprom_is_valid = 0;
	if ((eeprom_version != 1) && (eeprom_version != 2))
		eeprom_is_valid = 0;
	if (eeprom_content.type != HW_COMP_MAINCPU)
		eeprom_is_valid = 0;

	if (eeprom_diag)
		printf("DIAG: %s() valid[%d]\n", __func__, eeprom_is_valid);

	return ret;
}

int mac_read_from_eeprom(void)
{
	const u8 *mac;
	const char *mac_txt;

	if (read_eeprom()) {
		printf("I2C EEPROM read failed.\n");
		return -1;
	}

	if (!eeprom_is_valid) {
		printf("I2C EEPROM content not valid\n");
		return -1;
	}

	mac = NULL;
	switch (eeprom_version) {
	case 1:
	case 2:
		mac = (const u8 *)&eeprom_content.macaddress;
		break;
	}

	if (mac && is_valid_ether_addr(mac)) {
		eth_setenv_enetaddr("ethaddr", mac);
		if (mac_diag) {
			mac_txt = getenv("ethaddr");
			if (mac_txt)
				printf("DIAG: MAC value [%s]\n", mac_txt);
			else
				printf("DIAG: failed to setup MAC env\n");
		}
	}

	return 0;
}

/*
 * BEWARE!
 * this board uses DDR1(!) Micron SDRAM, *NOT* the DDR2
 * which the ADS, Aria or PDM360NG boards are using
 * (the steps outlined here refer to the Micron datasheet)
 */
u32 sdram_init_seq[] = {
	/* item 6, at least one NOP after CKE went high */
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	/* item 7, precharge all; item 8, tRP (20ns) */
	CONFIG_SYS_DDRCMD_PCHG_ALL,
	CONFIG_SYS_DDRCMD_NOP,
	/* item 9, extended mode register; item 10, tMRD 10ns) */
	CONFIG_SYS_MICRON_EMODE | CONFIG_SYS_MICRON_EMODE_PARAM,
	CONFIG_SYS_DDRCMD_NOP,
	/*
	 * item 11, (base) mode register _with_ reset DLL;
	 * item 12, tMRD (10ns)
	 */
	CONFIG_SYS_MICRON_BMODE | CONFIG_SYS_MICRON_BMODE_RSTDLL |
	CONFIG_SYS_MICRON_BMODE_PARAM,
	CONFIG_SYS_DDRCMD_NOP,
	/* item 13, precharge all; item 14, tRP (20ns) */
	CONFIG_SYS_DDRCMD_PCHG_ALL,
	CONFIG_SYS_DDRCMD_NOP,
	/*
	 * item 15, auto refresh (i.e. refresh with CKE held high);
	 * item 16, tRFC (70ns)
	 */
	CONFIG_SYS_DDRCMD_RFSH,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	/*
	 * item 17, auto refresh (i.e. refresh with CKE held high);
	 * item 18, tRFC (70ns)
	 */
	CONFIG_SYS_DDRCMD_RFSH,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	/* item 19, optional, unassert DLL reset; item 20, tMRD (20ns) */
	CONFIG_SYS_MICRON_BMODE | CONFIG_SYS_MICRON_BMODE_PARAM,
	CONFIG_SYS_DDRCMD_NOP,
	/*
	 * item 21, "actually done", but make sure 200 DRAM clock cycles
	 * have passed after DLL reset before READ requests are issued
	 * (200 cycles at 160MHz -> 1.25 usec)
	 */
	/* EMPTY, optional, we don't do it */
};

phys_size_t initdram(int board_type)
{
	return fixed_sdram(NULL, sdram_init_seq, ARRAY_SIZE(sdram_init_seq));
}

int misc_init_r(void)
{
	u32 keys;
	char *s;
	int want_recovery;

	/* we use bus I2C-0 for the on-board eeprom */
	i2c_set_bus_num(0);

	/* setup GPIO directions and initial values */
	gpio_configure();

	/*
	 * enforce the start of the recovery system when
	 * - the appropriate keys were pressed
	 * - "some" external software told us to
	 * - a previous installation was aborted or has failed
	 */
	want_recovery = 0;
	keys = gpio_querykbd();
	if (gpio_diag)
		printf("GPIO keyboard status [0x%02X]\n", keys);
	if ((keys & GPIOKEY_BITS_RECOVERY) == GPIOKEY_BITS_RECOVERY) {
		printf("detected recovery request (keyboard)\n");
		want_recovery = 1;
	}
	s = getenv("want_recovery");
	if ((s != NULL) && (*s != '\0')) {
		printf("detected recovery request (environment)\n");
		want_recovery = 1;
	}
	s = getenv("install_in_progress");
	if ((s != NULL) && (*s != '\0')) {
		printf("previous installation has not completed\n");
		want_recovery = 1;
	}
	s = getenv("install_failed");
	if ((s != NULL) && (*s != '\0')) {
		printf("previous installation has failed\n");
		want_recovery = 1;
	}
	if (want_recovery) {
		printf("enforced start of the recovery system\n");
		setenv("bootcmd", "run recovery");
	}

	/*
	 * boot the recovery system without waiting; boot the
	 * production system without waiting by default, only
	 * insert a pause (to provide a chance to get a prompt)
	 * when GPIO keys were pressed during power on
	 */
	if (want_recovery)
		setenv("bootdelay", "0");
	else if (!keys)
		setenv("bootdelay", "0");
	else
		setenv("bootdelay", "2");

	/* get the ethernet MAC from I2C EEPROM */
	mac_read_from_eeprom();

	return 0;
}

/* setup specific IO pad configuration */
static  iopin_t ioregs_init[] = {
	{	/* LPC CS3 */
		offsetof(struct ioctrl512x, io_control_nfc_ce0), 1,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(1) | IO_PIN_DS(2),
	},
	{	/* LPC CS1 */
		offsetof(struct ioctrl512x, io_control_lpc_cs1), 1,
		IO_PIN_OVER_DRVSTR,
		IO_PIN_DS(2),
	},
	{	/* LPC CS2 */
		offsetof(struct ioctrl512x, io_control_lpc_cs2), 1,
		IO_PIN_OVER_DRVSTR,
		IO_PIN_DS(2),
	},
	{	/* LPC CS4, CS5 */
		offsetof(struct ioctrl512x, io_control_pata_ce1), 2,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(1) | IO_PIN_DS(2),
	},
	{	/* SDHC CLK, CMD, D0, D1, D2, D3 */
		offsetof(struct ioctrl512x, io_control_pata_ior), 6,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(1) | IO_PIN_DS(2),
	},
	{	/* GPIO keyboard */
		offsetof(struct ioctrl512x, io_control_pci_ad30), 4,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO DN1 PF, LCD power, DN2 PF */
		offsetof(struct ioctrl512x, io_control_pci_ad26), 3,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO reset AS-i */
		offsetof(struct ioctrl512x, io_control_pci_ad21), 1,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO reset safety */
		offsetof(struct ioctrl512x, io_control_pci_ad19), 1,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO reset netX */
		offsetof(struct ioctrl512x, io_control_pci_ad16), 1,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO ma2 en */
		offsetof(struct ioctrl512x, io_control_pci_ad15), 1,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO SD CD, SD WP */
		offsetof(struct ioctrl512x, io_control_pci_ad08), 2,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* FEC RX DV */
		offsetof(struct ioctrl512x, io_control_pci_ad06), 1,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(2) | IO_PIN_DS(2),
	},
	{	/* GPIO AS-i prog, AS-i done, LCD backlight */
		offsetof(struct ioctrl512x, io_control_pci_ad05), 3,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO AS-i wdg */
		offsetof(struct ioctrl512x, io_control_pci_req2), 1,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO safety wdg */
		offsetof(struct ioctrl512x, io_control_pci_req1), 1,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO netX wdg */
		offsetof(struct ioctrl512x, io_control_pci_req0), 1,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO IRQ powerfail */
		offsetof(struct ioctrl512x, io_control_pci_inta), 1,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO AS-i PWRD */
		offsetof(struct ioctrl512x, io_control_pci_frame), 1,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO LED0, LED1 */
		offsetof(struct ioctrl512x, io_control_pci_idsel), 2,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* GPIO IRQ AS-i 1, IRQ AS-i 2, IRQ safety */
		offsetof(struct ioctrl512x, io_control_pci_irdy), 3,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/* DIU clk */
		offsetof(struct ioctrl512x, io_control_spdif_txclk), 1,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(2) | IO_PIN_DS(2),
	},
	{	/* FEC TX ER, CRS */
		offsetof(struct ioctrl512x, io_control_spdif_tx), 2,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(1) | IO_PIN_DS(2),
	},
	{	/* GPIO/GPT */ /* to *NOT* have the EXT IRQ0 float */
		offsetof(struct ioctrl512x, io_control_irq0), 1,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	{	/*
		 * FEC col, tx en, tx clk, txd 0-3, mdc, rx er,
		 * rdx 3-0, mdio, rx clk
		 */
		offsetof(struct ioctrl512x, io_control_psc0_0), 15,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(1) | IO_PIN_DS(2),
	},
	/* optional: make sure PSC3 remains the serial console */
	{	/* LPC CS6 */
		offsetof(struct ioctrl512x, io_control_psc3_4), 1,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(1) | IO_PIN_DS(2),
	},
	/* make sure PSC4 remains available for SPI,
	    *BUT* PSC4_1 is a GPIO kind of SS! */
	{	/* enforce drive strength on the SPI pin */
		offsetof(struct ioctrl512x, io_control_psc4_0), 5,
		IO_PIN_OVER_DRVSTR,
		IO_PIN_DS(2),
	},
	{
		offsetof(struct ioctrl512x, io_control_psc4_1), 1,
		IO_PIN_OVER_FMUX,
		IO_PIN_FMUX(3),
	},
	/* optional: make sure PSC5 remains available for SPI */
	{	/* enforce drive strength on the SPI pin */
		offsetof(struct ioctrl512x, io_control_psc5_0), 5,
		IO_PIN_OVER_DRVSTR,
		IO_PIN_DS(1),
	},
	{	/* LPC TSIZ1 */
		offsetof(struct ioctrl512x, io_control_psc6_0), 1,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(1) | IO_PIN_DS(2),
	},
	{	/* DIU hsync */
		offsetof(struct ioctrl512x, io_control_psc6_1), 1,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(2) | IO_PIN_DS(1),
	},
	{	/* DIU vsync */
		offsetof(struct ioctrl512x, io_control_psc6_4), 1,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(2) | IO_PIN_DS(1),
	},
	{	/* PSC7, part of DIU RGB */
		offsetof(struct ioctrl512x, io_control_psc7_0), 2,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(2) | IO_PIN_DS(1),
	},
	{	/* PSC7, safety UART */
		offsetof(struct ioctrl512x, io_control_psc7_2), 2,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(0) | IO_PIN_DS(1),
	},
	{	/* DIU (part of) RGB[] */
		offsetof(struct ioctrl512x, io_control_psc8_3), 16,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(2) | IO_PIN_DS(1),
	},
	{	/* DIU data enable */
		offsetof(struct ioctrl512x, io_control_psc11_4), 1,
		IO_PIN_OVER_FMUX | IO_PIN_OVER_DRVSTR,
		IO_PIN_FMUX(2) | IO_PIN_DS(1),
	},
	/* reduce LPB drive strength for improved EMI */
	{	/* LPC OE, LPC RW */
		offsetof(struct ioctrl512x, io_control_lpc_oe), 2,
		IO_PIN_OVER_DRVSTR,
		IO_PIN_DS(2),
	},
	{	/* LPC AX03 through LPC AD00 */
		offsetof(struct ioctrl512x, io_control_lpc_ax03), 36,
		IO_PIN_OVER_DRVSTR,
		IO_PIN_DS(2),
	},
	{	/* LPC CS5 */
		offsetof(struct ioctrl512x, io_control_pata_ce2), 1,
		IO_PIN_OVER_DRVSTR,
		IO_PIN_DS(2),
	},
	{	/* SDHC CLK */
		offsetof(struct ioctrl512x, io_control_nfc_wp), 1,
		IO_PIN_OVER_DRVSTR,
		IO_PIN_DS(2),
	},
	{	/* SDHC DATA */
		offsetof(struct ioctrl512x, io_control_nfc_ale), 4,
		IO_PIN_OVER_DRVSTR,
		IO_PIN_DS(2),
	},
};

int checkboard(void)
{
	puts("Board: ifm AC14xx\n");

	/* initialize function mux & slew rate IO inter alia on IO Pins  */
	iopin_initialize_bits(ioregs_init, ARRAY_SIZE(ioregs_init));

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */
