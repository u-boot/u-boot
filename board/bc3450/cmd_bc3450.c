/*
 * (C) Copyright 2005
 * Stefan Strobl, GERSYS GmbH, stefan.strobl@gersys.de
 *
 * (C) Copyright 2005
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de.
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

/*
 * BC3450 specific commands
 */
#if defined(CONFIG_CMD_BSP)

#undef DEBUG
#ifdef DEBUG
# define dprintf(fmt,args...)	printf(fmt, ##args)
#else
# define dprintf(fmt,args...)
#endif

/*
 * Definitions for DS1620 chip
 */
#define THERM_START_CONVERT	0xee
#define THERM_RESET		0xaf
#define THERM_READ_CONFIG	0xac
#define THERM_READ_TEMP		0xaa
#define THERM_READ_TL		0xa2
#define THERM_READ_TH		0xa1
#define THERM_WRITE_CONFIG	0x0c
#define THERM_WRITE_TL		0x02
#define THERM_WRITE_TH		0x01

#define CONFIG_SYS_CPU			2
#define CONFIG_SYS_1SHOT		1
#define CONFIG_SYS_STANDALONE		0

struct therm {
	int hi;
	int lo;
};

/*
 * SM501 Register
 */
#define SM501_GPIO_CTRL_LOW		0x00000008UL	/* gpio pins 0..31  */
#define SM501_GPIO_CTRL_HIGH		0x0000000CUL	/* gpio pins 32..63 */
#define SM501_POWER_MODE0_GATE		0x00000040UL
#define SM501_POWER_MODE1_GATE		0x00000048UL
#define POWER_MODE_GATE_GPIO_PWM_I2C	0x00000040UL
#define SM501_GPIO_DATA_LOW		0x00010000UL
#define SM501_GPIO_DATA_HIGH		0x00010004UL
#define SM501_GPIO_DATA_DIR_LOW		0x00010008UL
#define SM501_GPIO_DATA_DIR_HIGH	0x0001000CUL
#define SM501_PANEL_DISPLAY_CONTROL	0x00080000UL
#define SM501_CRT_DISPLAY_CONTROL	0x00080200UL

/* SM501 CRT Display Control Bits */
#define SM501_CDC_SEL			(1 << 9)
#define SM501_CDC_TE			(1 << 8)
#define SM501_CDC_E			(1 << 2)

/* SM501 Panel Display Control Bits */
#define SM501_PDC_FPEN			(1 << 27)
#define SM501_PDC_BIAS			(1 << 26)
#define SM501_PDC_DATA			(1 << 25)
#define SM501_PDC_VDDEN			(1 << 24)

/* SM501 GPIO Data LOW Bits */
#define SM501_GPIO24			0x01000000
#define SM501_GPIO25			0x02000000
#define SM501_GPIO26			0x04000000
#define SM501_GPIO27			0x08000000
#define SM501_GPIO28			0x10000000
#define SM501_GPIO29			0x20000000
#define SM501_GPIO30			0x40000000
#define SM501_GPIO31			0x80000000

/* SM501 GPIO Data HIGH Bits */
#define SM501_GPIO46			0x00004000
#define SM501_GPIO47			0x00008000
#define SM501_GPIO48			0x00010000
#define SM501_GPIO49			0x00020000
#define SM501_GPIO50			0x00040000
#define SM501_GPIO51			0x00080000

/* BC3450 GPIOs @ SM501 Data LOW */
#define DIP				(SM501_GPIO24 | SM501_GPIO25 | SM501_GPIO26 | SM501_GPIO27)
#define DS1620_DQ			SM501_GPIO29	/* I/O             */
#define DS1620_CLK			SM501_GPIO30	/* High active O/P */
#define DS1620_RES			SM501_GPIO31	/* Low active O/P  */
/* BC3450 GPIOs @ SM501 Data HIGH */
#define BUZZER				SM501_GPIO47	/* Low active O/P  */
#define DS1620_TLOW			SM501_GPIO48	/* High active I/P */
#define PWR_OFF				SM501_GPIO49	/* Low active O/P  */
#define FP_DATA_TRI			SM501_GPIO50	/* High active O/P */


/*
 * Initialise GPIO on SM501
 *
 * This function may be called from several other functions.
 * Yet, the initialisation sequence is executed only the first
 * time the function is called.
 */
int sm501_gpio_init (void)
{
	static int init_done = 0;

	if (init_done) {
/*	dprintf("sm501_gpio_init: nothing to be done.\n"); */
		return 1;
	}

	/* enable SM501 GPIO control (in both power modes) */
	*(vu_long *) (SM501_MMIO_BASE + SM501_POWER_MODE0_GATE) |=
		POWER_MODE_GATE_GPIO_PWM_I2C;
	*(vu_long *) (SM501_MMIO_BASE + SM501_POWER_MODE1_GATE) |=
		POWER_MODE_GATE_GPIO_PWM_I2C;

	/* set up default O/Ps */
	*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_LOW) &=
		~(DS1620_RES | DS1620_CLK);
	*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_LOW) |= DS1620_DQ;
	*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_HIGH) &=
		~(FP_DATA_TRI);
	*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_HIGH) |=
		(BUZZER | PWR_OFF);

	/* configure directions for SM501 GPIO pins */
	*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_CTRL_LOW) &= ~(0xFF << 24);
	*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_CTRL_HIGH) &=
		~(0x3F << 14);
	*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_DIR_LOW) &=
		~(DIP | DS1620_DQ);
	*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_DIR_LOW) |=
		(DS1620_RES | DS1620_CLK);
	*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_DIR_HIGH) &=
		~DS1620_TLOW;
	*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_DIR_HIGH) |=
		(PWR_OFF | BUZZER | FP_DATA_TRI);

	init_done = 1;
/*  dprintf("sm501_gpio_init: done.\n"); */
	return 0;
}


/*
 * dip - read Config Inputs
 *
 * read and prints the dip switch
 * and/or external config inputs (4bits) 0...0x0F
 */
int cmd_dip (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	vu_long rc = 0;

	sm501_gpio_init ();

	/* read dip switch */
	rc = *(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_LOW);
	rc = ~rc;
	rc &= DIP;
	rc = (int) (rc >> 24);

	/* plausibility check */
	if (rc > 0x0F)
		return -1;

	printf ("0x%lx\n", rc);
	return 0;
}

U_BOOT_CMD (dip, 1, 1, cmd_dip,
	    "read dip switch and config inputs",
	    "\n"
	    "     - prints the state of the dip switch and/or\n"
	    "       external configuration inputs as hex value.\n"
	    "     - \"Config 1\" is the LSB");


/*
 * buz - turns Buzzer on/off
 */
#ifdef CONFIG_BC3450_BUZZER
static int cmd_buz (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	if (argc != 2) {
		printf ("Usage:\nspecify one argument: \"on\" or \"off\"\n");
		return 1;
	}

	sm501_gpio_init ();

	if (strncmp (argv[1], "on", 2) == 0) {
		*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_HIGH) &=
			~(BUZZER);
		return 0;
	} else if (strncmp (argv[1], "off", 3) == 0) {
		*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_HIGH) |=
			BUZZER;
		return 0;
	}
	printf ("Usage:\nspecify one argument: \"on\" or \"off\"\n");
	return 1;
}

U_BOOT_CMD (buz, 2, 1, cmd_buz,
	    "turns buzzer on/off",
	    "\n" "buz <on/off>\n" "     - turns the buzzer on or off");
#endif /* CONFIG_BC3450_BUZZER */


/*
 * fp - front panel commands
 */
static int cmd_fp (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	sm501_gpio_init ();

	if (strncmp (argv[1], "on", 2) == 0) {
		/* turn on VDD first */
		*(vu_long *) (SM501_MMIO_BASE +
			      SM501_PANEL_DISPLAY_CONTROL) |= SM501_PDC_VDDEN;
		udelay (1000);
		/* then put data on */
		*(vu_long *) (SM501_MMIO_BASE +
			      SM501_PANEL_DISPLAY_CONTROL) |= SM501_PDC_DATA;
		/* wait some time and enable backlight */
		udelay (1000);
		*(vu_long *) (SM501_MMIO_BASE +
			      SM501_PANEL_DISPLAY_CONTROL) |= SM501_PDC_BIAS;
		udelay (1000);
		*(vu_long *) (SM501_MMIO_BASE +
			      SM501_PANEL_DISPLAY_CONTROL) |= SM501_PDC_FPEN;
		return 0;
	} else if (strncmp (argv[1], "off", 3) == 0) {
		/* turn off the backlight first */
		*(vu_long *) (SM501_MMIO_BASE +
			      SM501_PANEL_DISPLAY_CONTROL) &= ~SM501_PDC_FPEN;
		udelay (1000);
		*(vu_long *) (SM501_MMIO_BASE +
			      SM501_PANEL_DISPLAY_CONTROL) &= ~SM501_PDC_BIAS;
		udelay (200000);
		/* wait some time, then remove data */
		*(vu_long *) (SM501_MMIO_BASE +
			      SM501_PANEL_DISPLAY_CONTROL) &= ~SM501_PDC_DATA;
		udelay (1000);
		/* and remove VDD last */
		*(vu_long *) (SM501_MMIO_BASE +
			      SM501_PANEL_DISPLAY_CONTROL) &=
			~SM501_PDC_VDDEN;
		return 0;
	} else if (strncmp (argv[1], "bl", 2) == 0) {
		/* turn on/off backlight only */
		if (strncmp (argv[2], "on", 2) == 0) {
			*(vu_long *) (SM501_MMIO_BASE +
				      SM501_PANEL_DISPLAY_CONTROL) |=
				SM501_PDC_BIAS;
			udelay (1000);
			*(vu_long *) (SM501_MMIO_BASE +
				      SM501_PANEL_DISPLAY_CONTROL) |=
				SM501_PDC_FPEN;
			return 0;
		} else if (strncmp (argv[2], "off", 3) == 0) {
			*(vu_long *) (SM501_MMIO_BASE +
				      SM501_PANEL_DISPLAY_CONTROL) &=
				~SM501_PDC_FPEN;
			udelay (1000);
			*(vu_long *) (SM501_MMIO_BASE +
				      SM501_PANEL_DISPLAY_CONTROL) &=
				~SM501_PDC_BIAS;
			return 0;
		}
	}
#ifdef CONFIG_BC3450_CRT
	else if (strncmp (argv[1], "crt", 3) == 0) {
		/* enables/disables the crt output (debug only) */
		if (strncmp (argv[2], "on", 2) == 0) {
			*(vu_long *) (SM501_MMIO_BASE +
				      SM501_CRT_DISPLAY_CONTROL) |=
				(SM501_CDC_TE | SM501_CDC_E);
			*(vu_long *) (SM501_MMIO_BASE +
				      SM501_CRT_DISPLAY_CONTROL) &=
				~SM501_CDC_SEL;
			return 0;
		} else if (strncmp (argv[2], "off", 3) == 0) {
			*(vu_long *) (SM501_MMIO_BASE +
				      SM501_CRT_DISPLAY_CONTROL) &=
				~(SM501_CDC_TE | SM501_CDC_E);
			*(vu_long *) (SM501_MMIO_BASE +
				      SM501_CRT_DISPLAY_CONTROL) |=
				SM501_CDC_SEL;
			return 0;
		}
	}
#endif /* CONFIG_BC3450_CRT */
	printf ("Usage:%s\n", cmdtp->help);
	return 1;
}

U_BOOT_CMD (fp, 3, 1, cmd_fp,
	    "front panes access functions",
	    "\n"
	    "fp bl <on/off>\n"
	    "     - turns the CCFL backlight of the display on/off\n"
	    "fp <on/off>\n" "     - turns the whole display on/off"
#ifdef CONFIG_BC3450_CRT
	    "\n"
	    "fp crt <on/off>\n"
	    "     - enables/disables the crt output (debug only)"
#endif /* CONFIG_BC3450_CRT */
	);

/*
 * temp - DS1620 thermometer
 */
/* GERSYS BC3450 specific functions */
static inline void bc_ds1620_set_clk (int clk)
{
	if (clk)
		*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_LOW) |=
			DS1620_CLK;
	else
		*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_LOW) &=
			~DS1620_CLK;
}

static inline void bc_ds1620_set_data (int dat)
{
	if (dat)
		*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_LOW) |=
			DS1620_DQ;
	else
		*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_LOW) &=
			~DS1620_DQ;
}

static inline int bc_ds1620_get_data (void)
{
	vu_long rc;

	rc = *(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_LOW);
	rc &= DS1620_DQ;
	if (rc != 0)
		rc = 1;
	return (int) rc;
}

static inline void bc_ds1620_set_data_dir (int dir)
{
	if (dir)		/* in */
		*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_DIR_LOW) &= ~DS1620_DQ;
	else			/* out */
		*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_DIR_LOW) |= DS1620_DQ;
}

static inline void bc_ds1620_set_reset (int res)
{
	if (res)
		*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_LOW) |= DS1620_RES;
	else
		*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_LOW) &= ~DS1620_RES;
}

/* hardware independent functions */
static void ds1620_send_bits (int nr, int value)
{
	int i;

	for (i = 0; i < nr; i++) {
		bc_ds1620_set_data (value & 1);
		bc_ds1620_set_clk (0);
		udelay (1);
		bc_ds1620_set_clk (1);
		udelay (1);

		value >>= 1;
	}
}

static unsigned int ds1620_recv_bits (int nr)
{
	unsigned int value = 0, mask = 1;
	int i;

	bc_ds1620_set_data (0);

	for (i = 0; i < nr; i++) {
		bc_ds1620_set_clk (0);
		udelay (1);

		if (bc_ds1620_get_data ())
			value |= mask;

		mask <<= 1;

		bc_ds1620_set_clk (1);
		udelay (1);
	}

	return value;
}

static void ds1620_out (int cmd, int bits, int value)
{
	bc_ds1620_set_clk (1);
	bc_ds1620_set_data_dir (0);

	bc_ds1620_set_reset (0);
	udelay (1);
	bc_ds1620_set_reset (1);

	udelay (1);

	ds1620_send_bits (8, cmd);
	if (bits)
		ds1620_send_bits (bits, value);

	udelay (1);

	/* go stand alone */
	bc_ds1620_set_data_dir (1);
	bc_ds1620_set_reset (0);
	bc_ds1620_set_clk (0);

	udelay (10000);
}

static unsigned int ds1620_in (int cmd, int bits)
{
	unsigned int value;

	bc_ds1620_set_clk (1);
	bc_ds1620_set_data_dir (0);

	bc_ds1620_set_reset (0);
	udelay (1);
	bc_ds1620_set_reset (1);

	udelay (1);

	ds1620_send_bits (8, cmd);

	bc_ds1620_set_data_dir (1);
	value = ds1620_recv_bits (bits);

	/* go stand alone */
	bc_ds1620_set_data_dir (1);
	bc_ds1620_set_reset (0);
	bc_ds1620_set_clk (0);

	return value;
}

static int cvt_9_to_int (unsigned int val)
{
	if (val & 0x100)
		val |= 0xfffffe00;

	return val;
}

/* set thermostate thresholds */
static void ds1620_write_state (struct therm *therm)
{
	ds1620_out (THERM_WRITE_TL, 9, therm->lo);
	ds1620_out (THERM_WRITE_TH, 9, therm->hi);
	ds1620_out (THERM_START_CONVERT, 0, 0);
}

static int cmd_temp (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i;
	struct therm therm;

	sm501_gpio_init ();

	/* print temperature */
	if (argc == 1) {
		i = cvt_9_to_int (ds1620_in (THERM_READ_TEMP, 9));
		printf ("%d.%d C\n", i >> 1, i & 1 ? 5 : 0);
		return 0;
	}

	/* set to default operation */
	if (strncmp (argv[1], "set", 3) == 0) {
		if (strncmp (argv[2], "default", 3) == 0) {
			therm.hi = +88;
			therm.lo = -20;
			therm.hi <<= 1;
			therm.lo <<= 1;
			ds1620_write_state (&therm);
			ds1620_out (THERM_WRITE_CONFIG, 8, CONFIG_SYS_STANDALONE);
			return 0;
		}
	}

	printf ("Usage:%s\n", cmdtp->help);
	return 1;
}

U_BOOT_CMD (temp, 3, 1, cmd_temp,
	    "print current temperature",
	    "\n" "temp\n" "     - print current temperature");

#ifdef CONFIG_BC3450_CAN
/*
 * Initialise CAN interface
 *
 * return 1 on CAN initialization failure
 * return 0 if no failure
 */
int can_init (void)
{
	static int init_done = 0;
	int i;
	struct mpc5xxx_mscan *can1 =
		(struct mpc5xxx_mscan *) (CONFIG_SYS_MBAR + 0x0900);
	struct mpc5xxx_mscan *can2 =
		(struct mpc5xxx_mscan *) (CONFIG_SYS_MBAR + 0x0980);

	/* GPIO configuration of the CAN pins is done in BC3450.h */

	if (!init_done) {
		/* init CAN 1 */
		can1->canctl1 |= 0x80;	/* CAN enable */
		udelay (100);

		i = 0;
		can1->canctl0 |= 0x02;	/* sleep mode */
		/* wait until sleep mode reached */
		while (!(can1->canctl1 & 0x02)) {
			udelay (10);
			i++;
			if (i == 10) {
				printf ("%s: CAN1 initialize error, "
					"can not enter sleep mode!\n",
					__FUNCTION__);
				return 1;
			}
		}
		i = 0;
		can1->canctl0 = 0x01;	/* enter init mode */
		/* wait until init mode reached */
		while (!(can1->canctl1 & 0x01)) {
			udelay (10);
			i++;
			if (i == 10) {
				printf ("%s: CAN1 initialize error, "
					"can not enter init mode!\n",
					__FUNCTION__);
				return 1;
			}
		}
		can1->canctl1 = 0x80;
		can1->canctl1 |= 0x40;
		can1->canbtr0 = 0x0F;
		can1->canbtr1 = 0x7F;
		can1->canidac &= ~(0x30);
		can1->canidar1 = 0x00;
		can1->canidar3 = 0x00;
		can1->canidar5 = 0x00;
		can1->canidar7 = 0x00;
		can1->canidmr0 = 0xFF;
		can1->canidmr1 = 0xFF;
		can1->canidmr2 = 0xFF;
		can1->canidmr3 = 0xFF;
		can1->canidmr4 = 0xFF;
		can1->canidmr5 = 0xFF;
		can1->canidmr6 = 0xFF;
		can1->canidmr7 = 0xFF;

		i = 0;
		can1->canctl0 &= ~(0x01);	/* leave init mode */
		can1->canctl0 &= ~(0x02);
		/* wait until init and sleep mode left */
		while ((can1->canctl1 & 0x01) || (can1->canctl1 & 0x02)) {
			udelay (10);
			i++;
			if (i == 10) {
				printf ("%s: CAN1 initialize error, "
					"can not leave init/sleep mode!\n",
					__FUNCTION__);
				return 1;
			}
		}

		/* init CAN 2 */
		can2->canctl1 |= 0x80;	/* CAN enable */
		udelay (100);

		i = 0;
		can2->canctl0 |= 0x02;	/* sleep mode */
		/* wait until sleep mode reached */
		while (!(can2->canctl1 & 0x02)) {
			udelay (10);
			i++;
			if (i == 10) {
				printf ("%s: CAN2 initialize error, "
					"can not enter sleep mode!\n",
					__FUNCTION__);
				return 1;
			}
		}
		i = 0;
		can2->canctl0 = 0x01;	/* enter init mode */
		/* wait until init mode reached */
		while (!(can2->canctl1 & 0x01)) {
			udelay (10);
			i++;
			if (i == 10) {
				printf ("%s: CAN2 initialize error, "
					"can not enter init mode!\n",
					__FUNCTION__);
				return 1;
			}
		}
		can2->canctl1 = 0x80;
		can2->canctl1 |= 0x40;
		can2->canbtr0 = 0x0F;
		can2->canbtr1 = 0x7F;
		can2->canidac &= ~(0x30);
		can2->canidar1 = 0x00;
		can2->canidar3 = 0x00;
		can2->canidar5 = 0x00;
		can2->canidar7 = 0x00;
		can2->canidmr0 = 0xFF;
		can2->canidmr1 = 0xFF;
		can2->canidmr2 = 0xFF;
		can2->canidmr3 = 0xFF;
		can2->canidmr4 = 0xFF;
		can2->canidmr5 = 0xFF;
		can2->canidmr6 = 0xFF;
		can2->canidmr7 = 0xFF;
		can2->canctl0 &= ~(0x01);	/* leave init mode */
		can2->canctl0 &= ~(0x02);

		i = 0;
		/* wait until init mode left */
		while ((can2->canctl1 & 0x01) || (can2->canctl1 & 0x02)) {
			udelay (10);
			i++;
			if (i == 10) {
				printf ("%s: CAN2 initialize error, "
					"can not leave init/sleep mode!\n",
					__FUNCTION__);
				return 1;
			}
		}
		init_done = 1;
	}
	return 0;
}

/*
 * Do CAN test
 * by sending message between CAN1 and CAN2
 *
 * return 1 on CAN failure
 * return 0 if no failure
 */
int do_can (char *argv[])
{
	int i;
	struct mpc5xxx_mscan *can1 =
		(struct mpc5xxx_mscan *) (CONFIG_SYS_MBAR + 0x0900);
	struct mpc5xxx_mscan *can2 =
		(struct mpc5xxx_mscan *) (CONFIG_SYS_MBAR + 0x0980);

	/* send a message on CAN1 */
	can1->cantbsel = 0x01;
	can1->cantxfg.idr[0] = 0x55;
	can1->cantxfg.idr[1] = 0x00;
	can1->cantxfg.idr[1] &= ~0x8;
	can1->cantxfg.idr[1] &= ~0x10;
	can1->cantxfg.dsr[0] = 0xCC;
	can1->cantxfg.dlr = 1;
	can1->cantxfg.tbpr = 0;
	can1->cantflg = 0x01;

	i = 0;
	while ((can1->cantflg & 0x01) == 0) {
		i++;
		if (i == 10) {
			printf ("%s: CAN1 send timeout, "
				"can not send message!\n", __FUNCTION__);
			return 1;
		}
		udelay (1000);
	}
	udelay (1000);

	i = 0;
	while (!(can2->canrflg & 0x01)) {
		i++;
		if (i == 10) {
			printf ("%s: CAN2 receive timeout, "
				"no message received!\n", __FUNCTION__);
			return 1;
		}
		udelay (1000);
	}

	if (can2->canrxfg.dsr[0] != 0xCC) {
		printf ("%s: CAN2 receive error, "
			"data mismatch!\n", __FUNCTION__);
		return 1;
	}

	/* send a message on CAN2 */
	can2->cantbsel = 0x01;
	can2->cantxfg.idr[0] = 0x55;
	can2->cantxfg.idr[1] = 0x00;
	can2->cantxfg.idr[1] &= ~0x8;
	can2->cantxfg.idr[1] &= ~0x10;
	can2->cantxfg.dsr[0] = 0xCC;
	can2->cantxfg.dlr = 1;
	can2->cantxfg.tbpr = 0;
	can2->cantflg = 0x01;

	i = 0;
	while ((can2->cantflg & 0x01) == 0) {
		i++;
		if (i == 10) {
			printf ("%s: CAN2 send error, "
				"can not send message!\n", __FUNCTION__);
			return 1;
		}
		udelay (1000);
	}
	udelay (1000);

	i = 0;
	while (!(can1->canrflg & 0x01)) {
		i++;
		if (i == 10) {
			printf ("%s: CAN1 receive timeout, "
				"no message received!\n", __FUNCTION__);
			return 1;
		}
		udelay (1000);
	}

	if (can1->canrxfg.dsr[0] != 0xCC) {
		printf ("%s: CAN1 receive error 0x%02x\n",
			__FUNCTION__, (can1->canrxfg.dsr[0]));
		return 1;
	}

	return 0;
}
#endif /* CONFIG_BC3450_CAN */

/*
 * test - BC3450 HW test routines
 */
int cmd_test (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
#ifdef CONFIG_BC3450_CAN
	int rcode;

	can_init ();
#endif /* CONFIG_BC3450_CAN */

	sm501_gpio_init ();

	if (argc != 2) {
		printf ("Usage:%s\n", cmdtp->help);
		return 1;
	}

	if (strncmp (argv[1], "unit-off", 8) == 0) {
		printf ("waiting 2 seconds...\n");
		udelay (2000000);
		*(vu_long *) (SM501_MMIO_BASE + SM501_GPIO_DATA_HIGH) &=
			~PWR_OFF;
		return 0;
	}
#ifdef CONFIG_BC3450_CAN
	else if (strncmp (argv[1], "can", 2) == 0) {
		rcode = do_can (argv);
		if (simple_strtoul (argv[2], NULL, 10) == 2) {
			if (rcode == 0)
				printf ("OK\n");
			else
				printf ("Error\n");
		}
		return rcode;
	}
#endif /* CONFIG_BC3450_CAN */

	printf ("Usage:%s\n", cmdtp->help);
	return 1;
}

U_BOOT_CMD (test, 2, 1, cmd_test, "unit test routines", "\n"
#ifdef CONFIG_BC3450_CAN
	"test can\n"
	"     - connect CAN1 (X8) with CAN2 (X9) for this test\n"
#endif /* CONFIG_BC3450_CAN */
	"test unit-off\n"
	"     - turns off the BC3450 unit\n"
	"       WARNING: Unsaved environment variables will be lost!"
);
#endif
