/*
 * (C) Copyright 2003
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
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

#define	DEBUG

#include <common.h>
#include <exports.h>
#include <timestamp.h>
#include <asm/arch/s3c24x0_cpu.h>
#include "tsc2000.h"
#include "rs485.h"

/*
 * define, to wait for the touch to be pressed, before reading coordinates in
 * command do_touch. If not defined, an error message is printed, when the
 * command do_touch is invoked and the touch is not pressed within an specific
 * interval.
 */
#undef	CONFIG_TOUCH_WAIT_PRESSED

/* max time to wait for touch is pressed */
#ifndef CONFIG_TOUCH_WAIT_PRESSED
#define TOUCH_TIMEOUT   5
#endif /* !CONFIG_TOUCH_WAIT_PRESSED */

/* assignment of CPU internal ADC channels with TRAB hardware */
#define VCC5V   2
#define VCC12V  3

/* CPLD-Register for controlling TRAB hardware functions */
#define CPLD_BUTTONS            ((volatile unsigned long *)0x04020000)
#define CPLD_FILL_LEVEL         ((volatile unsigned long *)0x04008000)
#define CPLD_ROTARY_SWITCH      ((volatile unsigned long *)0x04018000)
#define CPLD_RS485_RE           ((volatile unsigned long *)0x04028000)

/* timer configuration bits for buzzer and PWM */
#define START2		(1 << 12)
#define UPDATE2         (1 << 13)
#define INVERT2         (1 << 14)
#define RELOAD2         (1 << 15)
#define START3		(1 << 16)
#define UPDATE3         (1 << 17)
#define INVERT3         (1 << 18)
#define RELOAD3         (1 << 19)

#define PCLK		66000000
#define BUZZER_FREQ     1000    /* frequency in Hz */
#define PWM_FREQ        500


/* definitions of I2C EEPROM device address */
#define I2C_EEPROM_DEV_ADDR     0x54

/* definition for touch panel calibration points */
#define CALIB_TL 0              /* calibration point in (T)op (L)eft corner */
#define CALIB_DR 1              /* calibration point in (D)own (R)ight corner */

/* EEPROM address map */
#define SERIAL_NUMBER           8
#define TOUCH_X0                52
#define TOUCH_Y0                54
#define TOUCH_X1                56
#define TOUCH_Y1                58
#define CRC16                   60

/* EEPROM stuff */
#define EEPROM_MAX_CRC_BUF      64

/* RS485 stuff */
#define RS485_MAX_RECEIVE_BUF_LEN  100

/* Bit definitions for ADCCON */
#define ADC_ENABLE_START     0x1
#define ADC_READ_START       0x2
#define ADC_STDBM            0x4
#define ADC_INP_AIN0         (0x0 << 3)
#define ADC_INP_AIN1         (0x1 << 3)
#define ADC_INP_AIN2         (0x2 << 3)
#define ADC_INP_AIN3         (0x3 << 3)
#define ADC_INP_AIN4         (0x4 << 3)
#define ADC_INP_AIN5         (0x5 << 3)
#define ADC_INP_AIN6         (0x6 << 3)
#define ADC_INP_AIN7         (0x7 << 3)
#define ADC_PRSCEN           0x4000
#define ADC_ECFLG            0x8000

/* function test functions */
int do_dip (void);
int do_info (void);
int do_vcc5v (void);
int do_vcc12v (void);
int do_buttons (void);
int do_fill_level (void);
int do_rotary_switch (void);
int do_pressure (void);
int do_v_bat (void);
int do_vfd_id (void);
int do_buzzer (char * const *);
int do_led (char * const *);
int do_full_bridge (char * const *);
int do_dac (char * const *);
int do_motor_contact (void);
int do_motor (char * const *);
int do_pwm (char * const *);
int do_thermo (char * const *);
int do_touch (char * const *);
int do_rs485 (char * const *);
int do_serial_number (char * const *);
int do_crc16 (void);
int do_power_switch (void);
int do_gain (char * const *);
int do_eeprom (char * const *);

/* helper functions */
static void adc_init (void);
static int adc_read (unsigned int channel);
static void print_identifier (void);

#ifdef CONFIG_TOUCH_WAIT_PRESSED
static void touch_wait_pressed (void);
#else
static int touch_check_pressed (void);
#endif /* CONFIG_TOUCH_WAIT_PRESSED */

static void touch_read_x_y (int *x, int *y);
static int touch_write_clibration_values (int calib_point, int x, int y);
static int rs485_send_line (const char *data);
static int rs485_receive_chars (char *data, int timeout);
static unsigned short updcrc(unsigned short icrc, unsigned char *icp,
			     unsigned int icnt);

#if defined(CONFIG_CMD_I2C)
static int trab_eeprom_read (char * const *argv);
static int trab_eeprom_write (char * const *argv);
int i2c_write_multiple (uchar chip, uint addr, int alen, uchar *buffer,
			int len);
int i2c_read_multiple ( uchar chip, uint addr, int alen, uchar *buffer,
			int len);
#endif

/*
 * TRAB board specific commands. Especially commands for burn-in and function
 * test.
 */

int trab_fkt (int argc, char * const argv[])
{
	int i;

	app_startup(argv);
	if (get_version () != XF_VERSION) {
		printf ("Wrong XF_VERSION. Please re-compile with actual "
			"u-boot sources\n");
		printf ("Example expects ABI version %d\n", XF_VERSION);
		printf ("Actual U-Boot ABI version %d\n", (int)get_version());
		return 1;
	}

	debug ("argc = %d\n", argc);

	for (i=0; i<=argc; ++i) {
		debug ("argv[%d] = \"%s\"\n", i, argv[i] ? argv[i] : "<NULL>");
	}

	adc_init ();

	switch (argc) {

	case 0:
	case 1:
		break;

	case 2:
		if (strcmp (argv[1], "info") == 0) {
			return (do_info ());
		}
		if (strcmp (argv[1], "dip") == 0) {
			return (do_dip ());
		}
		if (strcmp (argv[1], "vcc5v") == 0) {
			return (do_vcc5v ());
		}
		if (strcmp (argv[1], "vcc12v") == 0) {
			return (do_vcc12v ());
		}
		if (strcmp (argv[1], "buttons") == 0) {
			return (do_buttons ());
		}
		if (strcmp (argv[1], "fill_level") == 0) {
			return (do_fill_level ());
		}
		if (strcmp (argv[1], "rotary_switch") == 0) {
			return (do_rotary_switch ());
		}
		if (strcmp (argv[1], "pressure") == 0) {
			return (do_pressure ());
		}
		if (strcmp (argv[1], "v_bat") == 0) {
			return (do_v_bat ());
		}
		if (strcmp (argv[1], "vfd_id") == 0) {
			return (do_vfd_id ());
		}
		if (strcmp (argv[1], "motor_contact") == 0) {
			return (do_motor_contact ());
		}
		if (strcmp (argv[1], "crc16") == 0) {
			return (do_crc16 ());
		}
		if (strcmp (argv[1], "power_switch") == 0) {
			return (do_power_switch ());
		}
		break;

	case 3:
		if (strcmp (argv[1], "full_bridge") == 0) {
			return (do_full_bridge (argv));
		}
		if (strcmp (argv[1], "dac") == 0) {
			return (do_dac (argv));
		}
		if (strcmp (argv[1], "motor") == 0) {
			return (do_motor (argv));
		}
		if (strcmp (argv[1], "pwm") == 0) {
			return (do_pwm (argv));
		}
		if (strcmp (argv[1], "thermo") == 0) {
			return (do_thermo (argv));
		}
		if (strcmp (argv[1], "touch") == 0) {
			return (do_touch (argv));
		}
		if (strcmp (argv[1], "serial_number") == 0) {
			return (do_serial_number (argv));
		}
		if (strcmp (argv[1], "buzzer") == 0) {
			return (do_buzzer (argv));
		}
		if (strcmp (argv[1], "gain") == 0) {
			return (do_gain (argv));
		}
		break;

	case 4:
		if (strcmp (argv[1], "led") == 0) {
			return (do_led (argv));
		}
		if (strcmp (argv[1], "rs485") == 0) {
			return (do_rs485 (argv));
		}
		if (strcmp (argv[1], "serial_number") == 0) {
			return (do_serial_number (argv));
		}
		break;

	case 5:
		if (strcmp (argv[1], "eeprom") == 0) {
			return (do_eeprom (argv));
		}
		break;

	case 6:
		if (strcmp (argv[1], "eeprom") == 0) {
			return (do_eeprom (argv));
		}
		break;

	default:
		break;
	}

	printf ("Usage:\n<command> <parameter1> <parameter2> ...\n");
	return 1;
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}

int do_info (void)
{
	printf ("Stand-alone application for TRAB board function test\n");
	printf ("Built: %s at %s\n", U_BOOT_DATE, U_BOOT_TIME);

	return 0;
}

int do_dip (void)
{
	unsigned int result = 0;
	int adc_val;
	int i;

	/***********************************************************
	 DIP switch connection (according to wa4-cpu.sp.301.pdf, page 3):
	   SW1 - AIN4
	   SW2 - AIN5
	   SW3 - AIN6
	   SW4 - AIN7

	   "On" DIP switch position short-circuits the voltage from
	   the input channel (i.e. '0' conversion result means "on").
	*************************************************************/

	for (i = 7; i > 3; i--) {

		if ((adc_val = adc_read (i)) == -1) {
			printf ("Channel %d could not be read\n", i);
			return 1;
		}

		/*
		 * Input voltage (switch open) is 1.8 V.
		 * (Vin_High/VRef)*adc_res = (1,8V/2,5V)*1023) = 736
		 * Set trigger at halve that value.
		 */
		if (adc_val < 368)
			result |= (1 << (i-4));
	}

	/* print result to console */
	print_identifier ();
	for (i = 0; i < 4; i++) {
		if ((result & (1 << i)) == 0)
			printf("0");
		else
			printf("1");
	}
	printf("\n");

	return 0;
}


int do_vcc5v (void)
{
	int result;

	/* VCC5V is connected to channel 2 */

	if ((result = adc_read (VCC5V)) == -1) {
		printf ("VCC5V could not be read\n");
		return 1;
	}

	/*
	 * Calculate voltage value. Split in two parts because there is no
	 * floating point support.  VCC5V is connected over an resistor divider:
	 * VCC5V=ADCval*2,5V/1023*(10K+30K)/10K.
	 */
	print_identifier ();
	printf ("%d", (result & 0x3FF)* 10 / 1023);
	printf (".%d", ((result & 0x3FF)* 10 % 1023)* 10 / 1023);
	printf ("%d V\n", (((result & 0x3FF) * 10 % 1023 ) * 10 % 1023)
		* 10 / 1024);

	return 0;
}


int do_vcc12v (void)
{
	int result;

	if ((result = adc_read (VCC12V)) == -1) {
		printf ("VCC12V could not be read\n");
		return 1;
	}

	/*
	 * Calculate voltage value. Split in two parts because there is no
	 * floating point support.  VCC5V is connected over an resistor divider:
	 * VCC12V=ADCval*2,5V/1023*(30K+270K)/30K.
	 */
	print_identifier ();
	printf ("%d", (result & 0x3FF)* 25 / 1023);
	printf (".%d V\n", ((result & 0x3FF)* 25 % 1023) * 10 / 1023);

	return 0;
}

static int adc_read (unsigned int channel)
{
	int j = 1000; /* timeout value for wait loop in us */
	int result;
	struct s3c2400_adc *padc;

	padc = s3c2400_get_base_adc();
	channel &= 0x7;

	padc->ADCCON &= ~ADC_STDBM; /* select normal mode */
	padc->ADCCON &= ~(0x7 << 3); /* clear the channel bits */
	padc->ADCCON |= ((channel << 3) | ADC_ENABLE_START);

	while (j--) {
		if ((padc->ADCCON & ADC_ENABLE_START) == 0)
			break;
		udelay (1);
	}

	if (j == 0) {
		printf("%s: ADC timeout\n", __FUNCTION__);
		padc->ADCCON |= ADC_STDBM; /* select standby mode */
		return -1;
	}

	result = padc->ADCDAT & 0x3FF;

	padc->ADCCON |= ADC_STDBM; /* select standby mode */

	debug ("%s: channel %d, result[DIGIT]=%d\n", __FUNCTION__,
	       (padc->ADCCON >> 3) & 0x7, result);

	/*
	 * Wait for ADC to be ready for next conversion. This delay value was
	 * estimated, because the datasheet does not specify a value.
	 */
	udelay (1000);

	return (result);
}


static void adc_init (void)
{
	struct s3c2400_adc *padc;

	padc = s3c2400_get_base_adc();

	padc->ADCCON &= ~(0xff << 6); /* clear prescaler bits */
	padc->ADCCON |= ((65 << 6) | ADC_PRSCEN); /* set prescaler */

	/*
	 * Wait some time to avoid problem with very first call of
	 * adc_read(). Without * this delay, sometimes the first read adc
	 * value is 0. Perhaps because the * adjustment of prescaler takes
	 * some clock cycles?
	 */
	udelay (1000);

	return;
}


int do_buttons (void)
{
	int result;
	int i;

	result = *CPLD_BUTTONS; /* read CPLD */
	debug ("%s: cpld_taster (32 bit) %#x\n", __FUNCTION__, result);

	/* print result to console */
	print_identifier ();
	for (i = 16; i <= 19; i++) {
		if ((result & (1 << i)) == 0)
			printf("0");
		else
			printf("1");
	}
	printf("\n");
	return 0;
}


int do_power_switch (void)
{
	int result;

	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* configure GPE7 as input */
	gpio->PECON &= ~(0x3 << (2 * 7));

	/* signal GPE7 from power switch is low active: 0=on , 1=off */
	result = ((gpio->PEDAT & (1 << 7)) == (1 << 7)) ? 0 : 1;

	print_identifier ();
	printf("%d\n", result);
	return 0;
}


int do_fill_level (void)
{
	int result;

	result = *CPLD_FILL_LEVEL; /* read CPLD */
	debug ("%s: cpld_fuellstand (32 bit) %#x\n", __FUNCTION__, result);

	/* print result to console */
	print_identifier ();
	if ((result & (1 << 16)) == 0)
		printf("0\n");
	else
		printf("1\n");
	return 0;
}


int do_rotary_switch (void)
{
	int result;
	/*
	 * Please note, that the default values of the direction bits are
	 * undefined after reset. So it is a good idea, to make first a dummy
	 * call to this function, to clear the direction bits and set so to
	 * proper values.
	 */

	result = *CPLD_ROTARY_SWITCH; /* read CPLD */
	debug ("%s: cpld_inc (32 bit) %#x\n", __FUNCTION__, result);

	*CPLD_ROTARY_SWITCH |= (3 << 16); /* clear direction bits in CPLD */

	/* print result to console */
	print_identifier ();
	if ((result & (1 << 16)) == (1 << 16))
		printf("R");
	if ((result & (1 << 17)) == (1 << 17))
		printf("L");
	if (((result & (1 << 16)) == 0) && ((result & (1 << 17)) == 0))
		printf("0");
	if ((result & (1 << 18)) == 0)
		printf("0\n");
	else
		printf("1\n");
	return 0;
}


int do_vfd_id (void)
{
	int i;
	long int pcup_old, pccon_old;
	int vfd_board_id;
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* try to red vfd board id from the value defined by pull-ups */

	pcup_old = gpio->PCUP;
	pccon_old = gpio->PCCON;

	gpio->PCUP = (gpio->PCUP & 0xFFF0); /* activate  GPC0...GPC3 pull-ups */
	gpio->PCCON = (gpio->PCCON & 0xFFFFFF00); /* configure GPC0...GPC3 as
						   * inputs */
	udelay (10);            /* allow signals to settle */
	vfd_board_id = (~gpio->PCDAT) & 0x000F;	/* read GPC0...GPC3 port pins */

	gpio->PCCON = pccon_old;
	gpio->PCUP = pcup_old;

	/* print vfd_board_id to console */
	print_identifier ();
	for (i = 0; i < 4; i++) {
		if ((vfd_board_id & (1 << i)) == 0)
			printf("0");
		else
			printf("1");
	}
	printf("\n");
	return 0;
}

int do_buzzer (char * const *argv)
{
	int counter;

	struct s3c24x0_timers * const timers = s3c24x0_get_base_timers();
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* set prescaler for timer 2, 3 and 4 */
	timers->TCFG0 &= ~0xFF00;
	timers->TCFG0 |=  0x0F00;

	/* set divider for timer 2 */
	timers->TCFG1 &= ~0xF00;
	timers->TCFG1 |=  0x300;

	/* set frequency */
	counter = (PCLK / BUZZER_FREQ) >> 9;
	timers->ch[2].TCNTB = counter;
	timers->ch[2].TCMPB = counter / 2;

	if (strcmp (argv[2], "on") == 0) {
		debug ("%s: frequency: %d\n", __FUNCTION__,
		       BUZZER_FREQ);

		/* configure pin GPD7 as TOUT2 */
		gpio->PDCON &= ~0xC000;
		gpio->PDCON |= 0x8000;

		/* start */
		timers->TCON = (timers->TCON | UPDATE2 | RELOAD2) &
				~INVERT2;
		timers->TCON = (timers->TCON | START2) & ~UPDATE2;
		return (0);
	}
	else if (strcmp (argv[2], "off") == 0) {
		/* stop */
		timers->TCON &= ~(START2 | RELOAD2);

		/* configure GPD7 as output and set to low */
		gpio->PDCON &= ~0xC000;
		gpio->PDCON |= 0x4000;
		gpio->PDDAT &= ~0x80;
		return (0);
	}

	printf ("%s: invalid parameter %s\n", __FUNCTION__, argv[2]);
	return 1;
}


int do_led (char * const *argv)
{
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* configure PC14 and PC15 as output */
	gpio->PCCON &= ~(0xF << 28);
	gpio->PCCON |= (0x5 << 28);

	/* configure PD0 and PD4 as output */
	gpio->PDCON &= ~((0x3 << 8) | 0x3);
	gpio->PDCON |= ((0x1 << 8) | 0x1);

	switch (simple_strtoul(argv[2], NULL, 10)) {

	case 0:
	case 1:
		break;

	case 2:
		if (strcmp (argv[3], "on") == 0)
			gpio->PCDAT |= (1 << 14);
		else
			gpio->PCDAT &= ~(1 << 14);
		return 0;

	case 3:
		if (strcmp (argv[3], "on") == 0)
			gpio->PCDAT |= (1 << 15);
		else
			gpio->PCDAT &= ~(1 << 15);
		return 0;

	case 4:
		if (strcmp (argv[3], "on") == 0)
			gpio->PDDAT |= (1 << 0);
		else
			gpio->PDDAT &= ~(1 << 0);
		return 0;

	case 5:
		if (strcmp (argv[3], "on") == 0)
			gpio->PDDAT |= (1 << 4);
		else
			gpio->PDDAT &= ~(1 << 4);
		return 0;

	default:
		break;

	}
	printf ("%s: invalid parameter %s\n", __FUNCTION__, argv[2]);
	return 1;
}


int do_full_bridge (char * const *argv)
{
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* configure PD5 and PD6 as output */
	gpio->PDCON &= ~((0x3 << 5*2) | (0x3 << 6*2));
	gpio->PDCON |= ((0x1 << 5*2) | (0x1 << 6*2));

	if (strcmp (argv[2], "+") == 0) {
	      gpio->PDDAT |= (1 << 5);
	      gpio->PDDAT |= (1 << 6);
	      return 0;
	}
	else if (strcmp (argv[2], "-") == 0) {
		gpio->PDDAT &= ~(1 << 5);
		gpio->PDDAT |= (1 << 6);
		return 0;
	}
	else if (strcmp (argv[2], "off") == 0) {
		gpio->PDDAT &= ~(1 << 5);
		gpio->PDDAT &= ~(1 << 6);
		return 0;
	}
	printf ("%s: invalid parameter %s\n", __FUNCTION__, argv[2]);
	return 1;
}

/* val must be in [0, 4095] */
static inline unsigned long tsc2000_to_uv (u16 val)
{
	return ((250000 * val) / 4096) * 10;
}


int do_dac (char * const *argv)
{
	int brightness;

	/* initialize SPI */
	tsc2000_spi_init ();

	if  (((brightness = simple_strtoul (argv[2], NULL, 10)) < 0) ||
	     (brightness > 255)) {
		printf ("%s: invalid parameter %s\n", __FUNCTION__, argv[2]);
		return 1;
	}
	tsc2000_write(TSC2000_REG_DACCTL, 0x0); /* Power up DAC */
	tsc2000_write(TSC2000_REG_DAC, brightness & 0xff);

	return 0;
}


int do_v_bat (void)
{
	unsigned long ret, res;

	/* initialize SPI */
	spi_init ();

	tsc2000_write(TSC2000_REG_ADC, 0x1836);

	/* now wait for data available */
	adc_wait_conversion_done();

	ret = tsc2000_read(TSC2000_REG_BAT1);
	res = (tsc2000_to_uv(ret) + 1250) / 2500;
	res += (ERROR_BATTERY * res) / 1000;

	print_identifier ();
	printf ("%ld", (res / 100));
	printf (".%ld", ((res % 100) / 10));
	printf ("%ld V\n", (res % 10));
	return 0;
}


int do_pressure (void)
{
	/* initialize SPI */
	spi_init ();

	tsc2000_write(TSC2000_REG_ADC, 0x2436);

	/* now wait for data available */
	adc_wait_conversion_done();

	print_identifier ();
	printf ("%d\n", tsc2000_read(TSC2000_REG_AUX2));
	return 0;
}


int do_motor_contact (void)
{
	int result;

	result = *CPLD_FILL_LEVEL; /* read CPLD */
	debug ("%s: cpld_fuellstand (32 bit) %#x\n", __FUNCTION__, result);

	/* print result to console */
	print_identifier ();
	if ((result & (1 << 17)) == 0)
		printf("0\n");
	else
		printf("1\n");
	return 0;
}

int do_motor (char * const *argv)
{
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* Configure I/O port */
	gpio->PGCON &= ~(0x3 << 0);
	gpio->PGCON |= (0x1 << 0);

	if (strcmp (argv[2], "on") == 0) {
		gpio->PGDAT &= ~(1 << 0);
		return 0;
	}
	if (strcmp (argv[2], "off") == 0) {
		gpio->PGDAT |= (1 << 0);
		return 0;
	}
	printf ("%s: invalid parameter %s\n", __FUNCTION__, argv[2]);
	return 1;
}

static void print_identifier (void)
{
	printf ("## FKT: ");
}

int do_pwm (char * const *argv)
{
	int counter;
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();
	struct s3c24x0_timers * const timers = s3c24x0_get_base_timers();

	if (strcmp (argv[2], "on") == 0) {
		/* configure pin GPD8 as TOUT3 */
		gpio->PDCON &= ~(0x3 << 8*2);
		gpio->PDCON |= (0x2 << 8*2);

		/* set prescaler for timer 2, 3 and 4 */
		timers->TCFG0 &= ~0xFF00;
		timers->TCFG0 |= 0x0F00;

		/* set divider for timer 3 */
		timers->TCFG1 &= ~(0xf << 12);
		timers->TCFG1 |= (0x3 << 12);

		/* set frequency */
		counter = (PCLK / PWM_FREQ) >> 9;
		timers->ch[3].TCNTB = counter;
		timers->ch[3].TCMPB = counter / 2;

		/* start timer */
		timers->TCON = (timers->TCON | UPDATE3 | RELOAD3) & ~INVERT3;
		timers->TCON = (timers->TCON | START3) & ~UPDATE3;
		return 0;
	}
	if (strcmp (argv[2], "off") == 0) {

		/* stop timer */
		timers->TCON &= ~(START2 | RELOAD2);

		/* configure pin GPD8 as output and set to 0 */
		gpio->PDCON &= ~(0x3 << 8*2);
		gpio->PDCON |= (0x1 << 8*2);
		gpio->PDDAT &= ~(1 << 8);
		return 0;
	}
	printf ("%s: invalid parameter %s\n", __FUNCTION__, argv[2]);
	return 1;
}


int do_thermo (char * const *argv)
{
	int     channel, res;

	tsc2000_reg_init ();

	if (strcmp (argv[2], "all") == 0) {
		int i;
		for (i=0; i <= 15; i++) {
			res = tsc2000_read_channel(i);
			print_identifier ();
			printf ("c%d: %d\n", i, res);
		}
		return 0;
	}
	channel = simple_strtoul (argv[2], NULL, 10);
	res = tsc2000_read_channel(channel);
	print_identifier ();
	printf ("%d\n", res);
	return 0;                 /* return OK */
}


int do_touch (char * const *argv)
{
	int     x, y;

	if (strcmp (argv[2], "tl") == 0) {
#ifdef CONFIG_TOUCH_WAIT_PRESSED
		touch_wait_pressed();
#else
		{
			int i;
			for (i = 0; i < (TOUCH_TIMEOUT * 1000); i++) {
				if (touch_check_pressed ()) {
					break;
				}
				udelay (1000);  /* pause 1 ms */
			}
		}
		if (!touch_check_pressed()) {
			print_identifier ();
			printf ("error: touch not pressed\n");
			return 1;
		}
#endif /* CONFIG_TOUCH_WAIT_PRESSED */
		touch_read_x_y (&x, &y);

		print_identifier ();
		printf ("x=%d y=%d\n", x, y);
		return touch_write_clibration_values (CALIB_TL, x, y);
	}
	else if (strcmp (argv[2], "dr") == 0) {
#ifdef CONFIG_TOUCH_WAIT_PRESSED
		touch_wait_pressed();
#else
		{
			int i;
			for (i = 0; i < (TOUCH_TIMEOUT * 1000); i++) {
				if (touch_check_pressed ()) {
					break;
				}
				udelay (1000);  /* pause 1 ms */
			}
		}
		if (!touch_check_pressed()) {
			print_identifier ();
			printf ("error: touch not pressed\n");
			return 1;
		}
#endif /* CONFIG_TOUCH_WAIT_PRESSED */
		touch_read_x_y (&x, &y);

		print_identifier ();
		printf ("x=%d y=%d\n", x, y);

		return touch_write_clibration_values (CALIB_DR, x, y);
	}
	return 1;                 /* not "tl", nor "dr", so return error */
}


#ifdef CONFIG_TOUCH_WAIT_PRESSED
static void touch_wait_pressed (void)
{
	while (!(tsc2000_read(TSC2000_REG_ADC) & TC_PSM));
}

#else
static int touch_check_pressed (void)
{
	return (tsc2000_read(TSC2000_REG_ADC) & TC_PSM);
}
#endif /* CONFIG_TOUCH_WAIT_PRESSED */

static int touch_write_clibration_values (int calib_point, int x, int y)
{
#if defined(CONFIG_CMD_I2C)
	int x_verify = 0;
	int y_verify = 0;

	tsc2000_reg_init ();

	if (calib_point == CALIB_TL) {
		if (i2c_write_multiple (I2C_EEPROM_DEV_ADDR, TOUCH_X0, 1,
			       (unsigned char *)&x, 2)) {
			return 1;
		}
		if (i2c_write_multiple (I2C_EEPROM_DEV_ADDR, TOUCH_Y0, 1,
			       (unsigned char *)&y, 2)) {
			return 1;
		}

		/* verify written values */
		if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, TOUCH_X0, 1,
			      (unsigned char *)&x_verify, 2)) {
			return 1;
		}
		if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, TOUCH_Y0, 1,
			       (unsigned char *)&y_verify, 2)) {
			return 1;
		}
		if ((y != y_verify) || (x != x_verify)) {
			print_identifier ();
			printf ("error: verify error\n");
			return 1;
		}
		return 0;       /* no error */
	}
	else if (calib_point == CALIB_DR) {
		  if (i2c_write_multiple (I2C_EEPROM_DEV_ADDR, TOUCH_X1, 1,
			       (unsigned char *)&x, 2)) {
			return 1;
		  }
		if (i2c_write_multiple (I2C_EEPROM_DEV_ADDR, TOUCH_Y1, 1,
			       (unsigned char *)&y, 2)) {
			return 1;
		}

		/* verify written values */
		if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, TOUCH_X1, 1,
				       (unsigned char *)&x_verify, 2)) {
			return 1;
		}
		if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, TOUCH_Y1, 1,
			       (unsigned char *)&y_verify, 2)) {
			return 1;
		}
		if ((y != y_verify) || (x != x_verify)) {
			print_identifier ();
			printf ("error: verify error\n");
			return 1;
		}
		return 0;
	}
	return 1;
#else
	printf ("No I2C support enabled (CONFIG_CMD_I2C), could not write "
		"to EEPROM\n");
	return (1);
#endif
}


static void touch_read_x_y (int *px, int *py)
{
	tsc2000_write(TSC2000_REG_ADC, DEFAULT_ADC | TC_AD0 | TC_AD1);
	adc_wait_conversion_done();
	*px = tsc2000_read(TSC2000_REG_X);

	tsc2000_write(TSC2000_REG_ADC, DEFAULT_ADC | TC_AD2);
	adc_wait_conversion_done();
	*py = tsc2000_read(TSC2000_REG_Y);
}


int do_rs485 (char * const *argv)
{
	int timeout;
	char data[RS485_MAX_RECEIVE_BUF_LEN];

	if (strcmp (argv[2], "send") == 0) {
		return (rs485_send_line (argv[3]));
	}
	else if (strcmp (argv[2], "receive") == 0) {
		timeout = simple_strtoul(argv[3], NULL, 10);
		if (rs485_receive_chars (data, timeout) != 0) {
			print_identifier ();
			printf ("## nothing received\n");
			return (1);
		}
		else {
			print_identifier ();
			printf ("%s\n", data);
			return (0);
		}
	}
	printf ("%s: unknown command %s\n", __FUNCTION__, argv[2]);
	return (1);             /* unknown command, return error */
}


static int rs485_send_line (const char *data)
{
	rs485_init ();
	trab_rs485_enable_tx ();
	rs485_puts (data);
	rs485_putc ('\n');

	return (0);
}


static int rs485_receive_chars (char *data, int timeout)
{
	int i;
	int receive_count = 0;

	rs485_init ();
	trab_rs485_enable_rx ();

	/* test every 1 ms for received characters to avoid a receive FIFO
	 * overrun (@ 38.400 Baud) */
	for (i = 0; i < (timeout * 1000); i++) {
		while (rs485_tstc ()) {
			if (receive_count >= RS485_MAX_RECEIVE_BUF_LEN-1)
				break;
			*data++ = rs485_getc ();
			receive_count++;
		}
		udelay (1000);  /* pause 1 ms */
	}
	*data = '\0';           /* terminate string */

	if (receive_count == 0)
		return (1);
	else
		return (0);
}


int do_serial_number (char * const *argv)
{
#if defined(CONFIG_CMD_I2C)
	unsigned int serial_number;

	if (strcmp (argv[2], "read") == 0) {
		if (i2c_read (I2C_EEPROM_DEV_ADDR, SERIAL_NUMBER, 1,
			      (unsigned char *)&serial_number, 4)) {
			printf ("could not read from eeprom\n");
			return (1);
		}
		print_identifier ();
		printf ("%08d\n", serial_number);
		return (0);
	}
	else if (strcmp (argv[2], "write") == 0) {
		serial_number = simple_strtoul(argv[3], NULL, 10);
		if (i2c_write (I2C_EEPROM_DEV_ADDR, SERIAL_NUMBER, 1,
			      (unsigned char *)&serial_number, 4)) {
			printf ("could not write to eeprom\n");
			return (1);
		}
		return (0);
	}
	printf ("%s: unknown command %s\n", __FUNCTION__, argv[2]);
	return (1);             /* unknown command, return error */
#else
	printf ("No I2C support enabled (CONFIG_CMD_I2C), could not write "
		"to EEPROM\n");
	return (1);
#endif
}


int do_crc16 (void)
{
#if defined(CONFIG_CMD_I2C)
	int crc;
	unsigned char buf[EEPROM_MAX_CRC_BUF];

	if (i2c_read (I2C_EEPROM_DEV_ADDR, 0, 1, buf, 60)) {
		printf ("could not read from eeprom\n");
		return (1);
	}
	crc = 0;                /* start value of crc calculation */
	crc = updcrc (crc, buf, 60);

	print_identifier ();
	printf ("crc16=%#04x\n", crc);

	if (i2c_write (I2C_EEPROM_DEV_ADDR, CRC16, 1, (unsigned char *)&crc,
		       sizeof (crc))) {
		printf ("could not read from eeprom\n");
		return (1);
	}
	return (0);
#else
	printf ("No I2C support enabled (CONFIG_CMD_I2C), could not write "
		"to EEPROM\n");
	return (1);
#endif
}


/*
 * Calculate, intelligently, the CRC of a dataset incrementally given a
 * buffer full at a time.
 * Initialize crc to 0 for XMODEM, -1 for CCITT.
 *
 * Usage:
 *   newcrc = updcrc( oldcrc, bufadr, buflen )
 *        unsigned int oldcrc, buflen;
 *        char *bufadr;
 *
 * Compile with -DTEST to generate program that prints CRC of stdin to stdout.
 * Compile with -DMAKETAB to print values for crctab to stdout
 */

    /* the CRC polynomial. This is used by XMODEM (almost CCITT).
     * If you change P, you must change crctab[]'s initial value to what is
     * printed by initcrctab()
     */
#define   P    0x1021

    /* number of bits in CRC: don't change it. */
#define W 16

    /* this the number of bits per char: don't change it. */
#define B 8

static unsigned short crctab[1<<B] = { /* as calculated by initcrctab() */
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
    };

static unsigned short updcrc(unsigned short icrc, unsigned char *icp,
			     unsigned int icnt )
{
	register unsigned short crc = icrc;
	register unsigned char *cp = icp;
	register unsigned int cnt = icnt;

	while (cnt--)
		crc = (crc<<B) ^ crctab[(crc>>(W-B)) ^ *cp++];

	return (crc);
}


int do_gain (char * const *argv)
{
	int range;

	range = simple_strtoul (argv[2], NULL, 10);
	if ((range < 1) || (range > 3))
	{
		printf ("%s: invalid parameter %s\n", __FUNCTION__, argv[2]);
		return 1;
	}

	tsc2000_set_range (range);
	return (0);
}


int do_eeprom (char * const *argv)
{
#if defined(CONFIG_CMD_I2C)
	if (strcmp (argv[2], "read") == 0) {
		return (trab_eeprom_read (argv));
	}

	else if (strcmp (argv[2], "write") == 0) {
		return (trab_eeprom_write (argv));
	}

	printf ("%s: invalid parameter %s\n", __FUNCTION__, argv[2]);
	return (1);
#else
	printf ("No I2C support enabled (CONFIG_CMD_I2C), could not write "
		"to EEPROM\n");
	return (1);
#endif
}

#if defined(CONFIG_CMD_I2C)
static int trab_eeprom_read (char * const *argv)
{
	int i;
	int len;
	unsigned int addr;
	long int value = 0;
	uchar *buffer;

	buffer = (uchar *) &value;
	addr = simple_strtoul (argv[3], NULL, 10);
	addr &= 0xfff;
	len = simple_strtoul (argv[4], NULL, 10);
	if ((len < 1) || (len > 4)) {
		printf ("%s: invalid parameter %s\n", __FUNCTION__,
			argv[4]);
		return (1);
	}
	for (i = 0; i < len; i++) {
		if (i2c_read (I2C_EEPROM_DEV_ADDR, addr+i, 1, buffer+i, 1)) {
			printf ("%s: could not read from i2c device %#x"
				", addr %d\n", __FUNCTION__,
				I2C_EEPROM_DEV_ADDR, addr);
			return (1);
		}
	}
	print_identifier ();
	if (strcmp (argv[5], "-") == 0) {
		if (len == 1)
			printf ("%d\n", (signed char) value);
		else if (len == 2)
			printf ("%d\n", (signed short int) value);
		else
			printf ("%ld\n", value);
	}
	else {
		if (len == 1)
			printf ("%d\n", (unsigned char) value);
		else if (len == 2)
			printf ("%d\n", (unsigned short int) value);
		else
			printf ("%ld\n", (unsigned long int) value);
	}
	return (0);
}

static int trab_eeprom_write (char * const *argv)
{
	int i;
	int len;
	unsigned int addr;
	long int value = 0;
	uchar *buffer;

	buffer = (uchar *) &value;
	addr = simple_strtoul (argv[3], NULL, 10);
	addr &= 0xfff;
	len = simple_strtoul (argv[4], NULL, 10);
	if ((len < 1) || (len > 4)) {
		printf ("%s: invalid parameter %s\n", __FUNCTION__,
			argv[4]);
		return (1);
	}
	value = simple_strtol (argv[5], NULL, 10);
	debug ("value=%ld\n", value);
	for (i = 0; i < len; i++) {
		if (i2c_write (I2C_EEPROM_DEV_ADDR, addr+i, 1, buffer+i, 1)) {
			printf ("%s: could not write to i2c device %d"
				", addr %d\n", __FUNCTION__,
				I2C_EEPROM_DEV_ADDR, addr);
			return (1);
		}
#if 0
		printf ("chip=%#x, addr+i=%#x+%d=%p, alen=%d, *buffer+i="
			"%#x+%d=%p=%#x \n",I2C_EEPROM_DEV_ADDR_DEV_ADDR , addr,
			i, addr+i, 1, buffer, i, buffer+i, *(buffer+i));
#endif
		udelay (30000); /* wait for EEPROM ready */
	}
	return (0);
}

int i2c_write_multiple (uchar chip, uint addr, int alen,
			uchar *buffer, int len)
{
	int i;

	if (alen != 1) {
		printf ("%s: addr len other than 1 not supported\n",
			 __FUNCTION__);
		return (1);
	}

	for (i = 0; i < len; i++) {
		if (i2c_write (chip, addr+i, alen, buffer+i, 1)) {
			printf ("%s: could not write to i2c device %d"
				 ", addr %d\n", __FUNCTION__, chip, addr);
			return (1);
		}
#if 0
		printf ("chip=%#x, addr+i=%#x+%d=%p, alen=%d, *buffer+i="
			"%#x+%d=%p=\"%.1s\"\n", chip, addr, i, addr+i,
			alen, buffer, i, buffer+i, buffer+i);
#endif

		udelay (30000);
	}
	return (0);
}

int i2c_read_multiple ( uchar chip, uint addr, int alen,
			uchar *buffer, int len)
{
	int i;

	if (alen != 1) {
		printf ("%s: addr len other than 1 not supported\n",
			 __FUNCTION__);
		return (1);
	}

	for (i = 0; i < len; i++) {
		if (i2c_read (chip, addr+i, alen, buffer+i, 1)) {
			printf ("%s: could not read from i2c device %#x"
				 ", addr %d\n", __FUNCTION__, chip, addr);
			return (1);
		}
	}
	return (0);
}
#endif
