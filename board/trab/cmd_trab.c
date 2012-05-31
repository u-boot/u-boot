/*
 * (C) Copyright 2003
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

#undef DEBUG

#include <common.h>
#include <command.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <rtc.h>

/*
 * TRAB board specific commands. Especially commands for burn-in and function
 * test.
 */
#if defined(CONFIG_CMD_BSP)

/* limits for valid range of VCC5V in mV  */
#define VCC5V_MIN       4500
#define VCC5V_MAX       5500

/*
 * Test strings for EEPROM test. Length of string 2 must not exceed length of
 * string 1. Otherwise a buffer overrun could occur!
 */
#define EEPROM_TEST_STRING_1    "0987654321 :tset a si siht"
#define EEPROM_TEST_STRING_2    "this is a test: 1234567890"

/*
 * min/max limits for valid contact temperature during burn in test (in
 * degree Centigrade * 100)
 */
#define MIN_CONTACT_TEMP        -1000
#define MAX_CONTACT_TEMP        +9000

/* blinking frequency of status LED */
#define LED_BLINK_FREQ          5

/* delay time between burn in cycles in seconds */
#ifndef BURN_IN_CYCLE_DELAY     /* if not defined in include/configs/trab.h */
#define BURN_IN_CYCLE_DELAY     5
#endif

/* physical SRAM parameters */
#define SRAM_ADDR       0x02000000 /* GCS1 */
#define SRAM_SIZE       0x40000 /* 256 kByte */

/* CPLD-Register for controlling TRAB hardware functions */
#define CPLD_BUTTONS            ((volatile unsigned long *)0x04020000)
#define CPLD_FILL_LEVEL         ((volatile unsigned long *)0x04008000)
#define CPLD_ROTARY_SWITCH      ((volatile unsigned long *)0x04018000)
#define CPLD_RS485_RE           ((volatile unsigned long *)0x04028000)

/* I2C EEPROM device address */
#define I2C_EEPROM_DEV_ADDR     0x54

/* EEPROM address map */
#define EE_ADDR_TEST                    192
#define EE_ADDR_MAX_CYCLES              256
#define EE_ADDR_STATUS                  258
#define EE_ADDR_PASS_CYCLES             259
#define EE_ADDR_FIRST_ERROR_CYCLE       261
#define EE_ADDR_FIRST_ERROR_NUM         263
#define EE_ADDR_FIRST_ERROR_NAME        264
#define EE_ADDR_ACT_CYCLE               280

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
#define ADC_ECFLG            0x800

/* misc */

/* externals */
extern int memory_post_tests (unsigned long start, unsigned long size);
extern int i2c_write (uchar, uint, int , uchar* , int);
extern int i2c_read (uchar, uint, int , uchar* , int);
extern void tsc2000_reg_init (void);
extern s32 tsc2000_contact_temp (void);
extern void tsc2000_spi_init(void);

/* function declarations */
int do_dip (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_vcc5v (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_burn_in (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_contact_temp (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_burn_in_status (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int i2c_write_multiple (uchar chip, uint addr, int alen,
			uchar *buffer, int len);
int i2c_read_multiple (uchar chip, uint addr, int alen,
			uchar *buffer, int len);
int do_temp_log (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

/* helper functions */
static void adc_init (void);
static int adc_read (unsigned int channel);
static int read_dip (void);
static int read_vcc5v (void);
static int test_dip (void);
static int test_vcc5v (void);
static int test_rotary_switch (void);
static int test_sram (void);
static int test_eeprom (void);
static int test_contact_temp (void);
static void led_set (unsigned int);
static void led_blink (void);
static void led_init (void);
static void sdelay (unsigned long seconds); /* delay in seconds */
static int dummy (void);
static int read_max_cycles(void);
static void test_function_table_init (void);
static void global_vars_init (void);
static int global_vars_write_to_eeprom (void);

/* globals */
u16 max_cycles;
u8 status;
u16 pass_cycles;
u16 first_error_cycle;
u8 first_error_num;
char first_error_name[16];
u16 act_cycle;

typedef struct test_function_s {
	char *name;
	int (*pf)(void);
} test_function_t;

/* max number of Burn In Functions */
#define BIF_MAX 6

/* table with burn in functions */
test_function_t test_function[BIF_MAX];


int do_burn_in (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	int cycle_status;

	if (argc > 1)
		return cmd_usage(cmdtp);

	led_init ();
	global_vars_init ();
	test_function_table_init ();
	tsc2000_spi_init ();

	if (global_vars_write_to_eeprom () != 0) {
		printf ("%s: error writing global_vars to eeprom\n",
			__FUNCTION__);
		return (1);
	}

	if (read_max_cycles () != 0) {
		printf ("%s: error reading max_cycles from eeprom\n",
			__FUNCTION__);
		return (1);
	}

	if (max_cycles == 0) {
		printf ("%s: error, burn in max_cycles = 0\n", __FUNCTION__);
		return (1);
	}

	status = 0;
	for (act_cycle = 1; act_cycle <= max_cycles; act_cycle++) {

		cycle_status = 0;

		/*
		 * avoid timestamp overflow problem after about 68 minutes of
		 * udelay() time.
		 */
		reset_timer_masked ();
		for (i = 0; i < BIF_MAX; i++) {

			/* call test function */
			if ((*test_function[i].pf)() != 0) {
				printf ("error in %s test\n",
					test_function[i].name);

				/* is it the first error? */
				if (status == 0) {
					status = 1;
					first_error_cycle = act_cycle;

					/* do not use error_num 0 */
					first_error_num = i+1;
					strncpy (first_error_name,
						 test_function[i].name,
						 sizeof (first_error_name));
					led_set (0);
				}
				cycle_status = 1;
			}
		}
		/* were all tests of actual cycle OK? */
		if (cycle_status == 0)
			pass_cycles++;

		/* set status LED if no error is occoured since yet */
		if (status == 0)
			led_set (1);

		printf ("%s: cycle %d finished\n", __FUNCTION__, act_cycle);

		/* pause between cycles */
		sdelay (BURN_IN_CYCLE_DELAY);
	}

	if (global_vars_write_to_eeprom () != 0) {
		led_set (0);
		printf ("%s: error writing global_vars to eeprom\n",
			__FUNCTION__);
		status = 1;
	}

	if (status == 0) {
		led_blink ();   /* endless loop!! */
		return (0);
	} else {
		led_set (0);
		return (1);
	}
}

U_BOOT_CMD(
	burn_in,	1,	1,	do_burn_in,
	"start burn-in test application on TRAB",
	"\n"
	"    -  start burn-in test application\n"
	"       The burn-in test could took a while to finish!\n"
	"       The content of the onboard EEPROM is modified!"
);


int do_dip (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i, dip;

	if (argc > 1)
		return cmd_usage(cmdtp);

	if ((dip = read_dip ()) == -1)
		return 1;

	for (i = 0; i < 4; i++) {
		if ((dip & (1 << i)) == 0)
			printf("0");
		else
			printf("1");
	}
	printf("\n");

	return 0;
}

U_BOOT_CMD(
	dip,	1,	1,	do_dip,
	"read dip switch on TRAB",
	"\n"
	"    - read state of dip switch (S1) on TRAB board\n"
	"      read sequence: 1-2-3-4; ON=1; OFF=0; e.g.: \"0100\""
);


int do_vcc5v (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int vcc5v;

	if (argc > 1)
		return cmd_usage(cmdtp);

	if ((vcc5v = read_vcc5v ()) == -1)
		return (1);

	printf ("%d", (vcc5v / 1000));
	printf (".%d", (vcc5v % 1000) / 100);
	printf ("%d V\n", (vcc5v % 100) / 10) ;

	return 0;
}

U_BOOT_CMD(
	vcc5v,	1,	1,	do_vcc5v,
	"read VCC5V on TRAB",
	"\n"
	"    - read actual value of voltage VCC5V"
);


int do_contact_temp (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int contact_temp;

	if (argc > 1)
		return cmd_usage(cmdtp);

	tsc2000_spi_init ();

	contact_temp = tsc2000_contact_temp();
	printf ("%d degree C * 100\n", contact_temp) ;

	return 0;
}

U_BOOT_CMD(
	c_temp,	1,	1,	do_contact_temp,
	"read contact temperature on TRAB",
	""
	"    -  reads the onboard temperature (=contact temperature)\n"
);


int do_burn_in_status (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc > 1)
		return cmd_usage(cmdtp);

	if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_STATUS, 1,
				(unsigned char*) &status, 1))
		return (1);

	if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_PASS_CYCLES, 1,
				(unsigned char*) &pass_cycles, 2))
		return (1);

	if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_FIRST_ERROR_CYCLE,
				1, (unsigned char*) &first_error_cycle, 2))
		return (1);

	if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_FIRST_ERROR_NUM,
				1, (unsigned char*) &first_error_num, 1))
		return (1);

	if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_FIRST_ERROR_NAME,
			       1, (unsigned char*)first_error_name,
			       sizeof (first_error_name)))
		return (1);

	if (read_max_cycles () != 0)
		return (1);

	printf ("max_cycles = %d\n", max_cycles);
	printf ("status = %d\n", status);
	printf ("pass_cycles = %d\n", pass_cycles);
	printf ("first_error_cycle = %d\n", first_error_cycle);
	printf ("first_error_num = %d\n", first_error_num);
	printf ("first_error_name = %.*s\n",(int) sizeof(first_error_name),
		first_error_name);

	return 0;
}

U_BOOT_CMD(
	bis,	1,	1,	do_burn_in_status,
	"print burn in status on TRAB",
	"\n"
	"    -  prints the status variables of the last burn in test\n"
	"       stored in the onboard EEPROM on TRAB board"
);

static int read_dip (void)
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
			printf ("%s: Channel %d could not be read\n",
				 __FUNCTION__, i);
			return (-1);
		}

		/*
		 * Input voltage (switch open) is 1.8 V.
		 * (Vin_High/VRef)*adc_res = (1,8V/2,5V)*1023) = 736
		 * Set trigger at halve that value.
		 */
		if (adc_val < 368)
			result |= (1 << (i-4));
	}
	return (result);
}


static int read_vcc5v (void)
{
	s32 result;

	/* VCC5V is connected to channel 2 */

	if ((result = adc_read (2)) == -1) {
		printf ("%s: VCC5V could not be read\n", __FUNCTION__);
		return (-1);
	}
	/*
	 * Calculate voltage value. Split in two parts because there is no
	 * floating point support.  VCC5V is connected over an resistor divider:
	 * VCC5V=ADCval*2,5V/1023*(10K+30K)/10K.
	 */
	result = result * 10 * 1000 / 1023; /* result in mV */

	return (result);
}


static int test_dip (void)
{
	static int first_run = 1;
	static int first_dip;

	if (first_run) {
		if ((first_dip = read_dip ()) == -1) {
			return (1);
		}
		first_run = 0;
		debug ("%s: first_dip=%d\n", __FUNCTION__, first_dip);
	}
	if (first_dip != read_dip ()) {
		return (1);
	} else {
		return (0);
	}
}


static int test_vcc5v (void)
{
	int vcc5v;

	if ((vcc5v = read_vcc5v ()) == -1) {
		return (1);
	}

	if ((vcc5v > VCC5V_MAX) || (vcc5v < VCC5V_MIN)) {
		printf ("%s: vcc5v[V/100]=%d\n", __FUNCTION__, vcc5v);
		return (1);
	} else {
		return (0);
	}
}


static int test_rotary_switch (void)
{
	static int first_run = 1;
	static int first_rs;

	if (first_run) {
		/*
		 * clear bits in CPLD, because they have random values after
		 * power-up or reset.
		 */
		*CPLD_ROTARY_SWITCH |= (1 << 16) | (1 << 17);

		first_rs = ((*CPLD_ROTARY_SWITCH >> 16) & 0x7);
		first_run = 0;
		debug ("%s: first_rs=%d\n", __FUNCTION__, first_rs);
	}

	if (first_rs != ((*CPLD_ROTARY_SWITCH >> 16) & 0x7)) {
		return (1);
	} else {
		return (0);
	}
}


static int test_sram (void)
{
	return (memory_post_tests (SRAM_ADDR, SRAM_SIZE));
}


static int test_eeprom (void)
{
	unsigned char temp[sizeof (EEPROM_TEST_STRING_1)];
	int result = 0;

	/* write test string 1, read back and verify */
	if (i2c_write_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_TEST, 1,
				(unsigned char*)EEPROM_TEST_STRING_1,
				sizeof (EEPROM_TEST_STRING_1))) {
		return (1);
	}

	if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_TEST, 1,
			       temp, sizeof (EEPROM_TEST_STRING_1))) {
		return (1);
	}

	if (strcmp ((char *)temp, EEPROM_TEST_STRING_1) != 0) {
		result = 1;
		printf ("%s: error; read_str = \"%s\"\n", __FUNCTION__, temp);
	}

	/* write test string 2, read back and verify */
	if (result == 0) {
		if (i2c_write_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_TEST, 1,
					(unsigned char*)EEPROM_TEST_STRING_2,
					sizeof (EEPROM_TEST_STRING_2))) {
			return (1);
		}

		if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_TEST, 1,
				       temp, sizeof (EEPROM_TEST_STRING_2))) {
			return (1);
		}

		if (strcmp ((char *)temp, EEPROM_TEST_STRING_2) != 0) {
			result = 1;
			printf ("%s: error; read str = \"%s\"\n",
				__FUNCTION__, temp);
		}
	}
	return (result);
}


static int test_contact_temp (void)
{
	int contact_temp;

	contact_temp = tsc2000_contact_temp ();

	if ((contact_temp < MIN_CONTACT_TEMP)
	    || (contact_temp > MAX_CONTACT_TEMP))
		return (1);
	else
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


static int adc_read (unsigned int channel)
{
	int j = 1000; /* timeout value for wait loop in us */
	int result;
	struct s3c2400_adc *padc;

	padc = s3c2400_get_base_adc();
	channel &= 0x7;

	adc_init ();

	padc->adccon &= ~ADC_STDBM; /* select normal mode */
	padc->adccon &= ~(0x7 << 3); /* clear the channel bits */
	padc->adccon |= ((channel << 3) | ADC_ENABLE_START);

	while (j--) {
		if ((padc->adccon & ADC_ENABLE_START) == 0)
			break;
		udelay (1);
	}

	if (j == 0) {
		printf("%s: ADC timeout\n", __FUNCTION__);
		padc->adccon |= ADC_STDBM; /* select standby mode */
		return -1;
	}

	result = padc->adcdat & 0x3FF;

	padc->adccon |= ADC_STDBM; /* select standby mode */

	debug ("%s: channel %d, result[DIGIT]=%d\n", __FUNCTION__,
	       (padc->adccon >> 3) & 0x7, result);

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

	padc->adccon &= ~(0xff << 6); /* clear prescaler bits */
	padc->adccon |= ((65 << 6) | ADC_PRSCEN); /* set prescaler */

	/*
	 * Wait some time to avoid problem with very first call of
	 * adc_read(). Without this delay, sometimes the first read
	 * adc value is 0. Perhaps because the adjustment of prescaler
	 * takes some clock cycles?
	 */
	udelay (1000);

	return;
}


static void led_set (unsigned int state)
{
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	led_init ();

	switch (state) {
	case 0: /* turn LED off */
		gpio->padat |= (1 << 12);
		break;
	case 1: /* turn LED on */
		gpio->padat &= ~(1 << 12);
		break;
	default:
		break;
	}
}

static void led_blink (void)
{
	led_init ();

	/* blink LED. This function does not return! */
	while (1) {
		reset_timer_masked ();
		led_set (1);
		udelay (1000000 / LED_BLINK_FREQ / 2);
		led_set (0);
		udelay (1000000 / LED_BLINK_FREQ / 2);
	}
}


static void led_init (void)
{
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* configure GPA12 as output and set to High -> LED off */
	gpio->pacon &= ~(1 << 12);
	gpio->padat |= (1 << 12);
}


static void sdelay (unsigned long seconds)
{
	unsigned long i;

	for (i = 0; i < seconds; i++) {
		udelay (1000000);
	}
}


static int global_vars_write_to_eeprom (void)
{
	if (i2c_write_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_STATUS, 1,
				(unsigned char*) &status, 1)) {
		return (1);
	}
	if (i2c_write_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_PASS_CYCLES, 1,
				(unsigned char*) &pass_cycles, 2)) {
		return (1);
	}
	if (i2c_write_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_FIRST_ERROR_CYCLE,
				1, (unsigned char*) &first_error_cycle, 2)) {
		return (1);
	}
	if (i2c_write_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_FIRST_ERROR_NUM,
				1, (unsigned char*) &first_error_num, 1)) {
		return (1);
	}
	if (i2c_write_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_FIRST_ERROR_NAME,
				1, (unsigned char*) first_error_name,
				sizeof(first_error_name))) {
		return (1);
	}
	return (0);
}

static void global_vars_init (void)
{
	status                  = 1; /* error */
	pass_cycles             = 0;
	first_error_cycle       = 0;
	first_error_num         = 0;
	first_error_name[0]     = '\0';
	act_cycle               = 0;
	max_cycles              = 0;
}


static void test_function_table_init (void)
{
	int i;

	for (i = 0; i < BIF_MAX; i++)
		test_function[i].pf = dummy;

	/*
	 * the length of "name" must not exceed 16, including the '\0'
	 * termination. See also the EEPROM address map.
	 */
	test_function[0].pf = test_dip;
	test_function[0].name = "dip";

	test_function[1].pf = test_vcc5v;
	test_function[1].name = "vcc5v";

	test_function[2].pf = test_rotary_switch;
	test_function[2].name = "rotary_switch";

	test_function[3].pf = test_sram;
	test_function[3].name = "sram";

	test_function[4].pf = test_eeprom;
	test_function[4].name = "eeprom";

	test_function[5].pf = test_contact_temp;
	test_function[5].name = "contact_temp";
}


static int read_max_cycles (void)
{
	if (i2c_read_multiple (I2C_EEPROM_DEV_ADDR, EE_ADDR_MAX_CYCLES, 1,
			       (unsigned char *) &max_cycles, 2) != 0) {
		return (1);
	}

	return (0);
}

static int dummy(void)
{
	return (0);
}

int do_temp_log (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int contact_temp;
	int delay = 0;
#if defined(CONFIG_CMD_DATE)
	struct rtc_time tm;
#endif

	if (argc > 2)
		return cmd_usage(cmdtp);

	if (argc > 1)
		delay = simple_strtoul(argv[1], NULL, 10);

	tsc2000_spi_init ();
	while (1) {

#if defined(CONFIG_CMD_DATE)
		rtc_get (&tm);
		printf ("%4d-%02d-%02d %2d:%02d:%02d - ",
			tm.tm_year, tm.tm_mon, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
#endif

		contact_temp = tsc2000_contact_temp();
		printf ("%d\n", contact_temp) ;

		if (delay != 0)
			/*
			 * reset timer to avoid timestamp overflow problem
			 * after about 68 minutes of udelay() time.
			 */
			reset_timer_masked ();
			sdelay (delay);
	}

	return 0;
}

U_BOOT_CMD(
	tlog,	2,	1,	do_temp_log,
	"log contact temperature [1/100 C] to console (endlessly)",
	"delay\n"
	"    - contact temperature [1/100 C] is printed endlessly to console\n"
	"      <delay> specifies the seconds to wait between two measurements\n"
	"      For each measurment a timestamp is printeted"
);

#endif
