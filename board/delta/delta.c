/*
 * (C) Copyright 2006
 * DENX Software Engineering
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
#include <i2c.h>
#include <da9030.h>
#include <malloc.h>
#include <command.h>
#include <asm/arch/pxa-regs.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */

static void init_DA9030(void);
static void keys_init(void);
static void get_pressed_keys(uchar *s);
static uchar *key_match(uchar *kbd_data);

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */

	/* arch number of Lubbock-Board mk@tbd: fix this! */
	gd->bd->bi_arch_number = MACH_TYPE_LUBBOCK;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	return 0;
}

int board_late_init(void)
{
#ifdef DELTA_CHECK_KEYBD
	uchar kbd_data[KEYBD_DATALEN];
	char keybd_env[2 * KEYBD_DATALEN + 1];
	char *str;
	int i;
#endif /* DELTA_CHECK_KEYBD */

	setenv("stdout", "serial");
	setenv("stderr", "serial");

#ifdef DELTA_CHECK_KEYBD
	keys_init();

	memset(kbd_data, '\0', KEYBD_DATALEN);

	/* check for pressed keys and setup keybd_env */
	get_pressed_keys(kbd_data);

	for (i = 0; i < KEYBD_DATALEN; ++i) {
		sprintf (keybd_env + i + i, "%02X", kbd_data[i]);
	}
	setenv ("keybd", keybd_env);

	str = strdup ((char *)key_match (kbd_data));	/* decode keys */

# ifdef CONFIG_PREBOOT	/* automatically configure "preboot" command on key match */
	setenv ("preboot", str);	/* set or delete definition */
# endif /* CONFIG_PREBOOT */
	if (str != NULL) {
		free (str);
	}
#endif /* DELTA_CHECK_KEYBD */

	init_DA9030();
	return 0;
}

/*
 * Magic Key Handling, mainly copied from board/lwmon/lwmon.c
 */
#ifdef DELTA_CHECK_KEYBD

static uchar kbd_magic_prefix[] = "key_magic";
static uchar kbd_command_prefix[] = "key_cmd";

/*
 * Get pressed keys
 * s is a buffer of size KEYBD_DATALEN-1
 */
static void get_pressed_keys(uchar *s)
{
	unsigned long val;
	val = GPLR3;

	if(val & (1<<31))
		*s++ = KEYBD_KP_DKIN0;
	if(val & (1<<18))
		*s++ = KEYBD_KP_DKIN1;
	if(val & (1<<29))
		*s++ = KEYBD_KP_DKIN2;
	if(val & (1<<22))
		*s++ = KEYBD_KP_DKIN5;
}

static void keys_init()
{
	CKENB |= CKENB_7_GPIO;
	udelay(100);

	/* Configure GPIOs */
	GPIO127 = 0xa840;	/* KP_DKIN0 */
	GPIO114 = 0xa840;	/* KP_DKIN1 */
	GPIO125 = 0xa840;	/* KP_DKIN2 */
	GPIO118 = 0xa840;	/* KP_DKIN5 */

	/* Configure GPIOs as inputs */
	GPDR3 &= ~(1<<31 | 1<<18 | 1<<29 | 1<<22);
	GCDR3 = (1<<31 | 1<<18 | 1<<29 | 1<<22);

	udelay(100);
}

static int compare_magic (uchar *kbd_data, uchar *str)
{
	/* uchar compare[KEYBD_DATALEN-1]; */
	uchar compare[KEYBD_DATALEN];
	char *nxt;
	int i;

	/* Don't include modifier byte */
	/* memcpy (compare, kbd_data+1, KEYBD_DATALEN-1); */
	memcpy (compare, kbd_data, KEYBD_DATALEN);

	for (; str != NULL; str = (*nxt) ? (uchar *)(nxt+1) : (uchar *)nxt) {
		uchar c;
		int k;

		c = (uchar) simple_strtoul ((char *)str, (char **) (&nxt), 16);

		if (str == (uchar *)nxt) {	/* invalid character */
			break;
		}

		/*
		 * Check if this key matches the input.
		 * Set matches to zero, so they match only once
		 * and we can find duplicates or extra keys
		 */
		for (k = 0; k < sizeof(compare); ++k) {
			if (compare[k] == '\0')	/* only non-zero entries */
				continue;
			if (c == compare[k]) {	/* found matching key */
				compare[k] = '\0';
				break;
			}
		}
		if (k == sizeof(compare)) {
			return -1;		/* unmatched key */
		}
	}

	/*
	 * A full match leaves no keys in the `compare' array,
	 */
	for (i = 0; i < sizeof(compare); ++i) {
		if (compare[i])
		{
			return -1;
		}
	}

	return 0;
}


static uchar *key_match (uchar *kbd_data)
{
	char magic[sizeof (kbd_magic_prefix) + 1];
	uchar *suffix;
	char *kbd_magic_keys;

	/*
	 * The following string defines the characters that can pe appended
	 * to "key_magic" to form the names of environment variables that
	 * hold "magic" key codes, i. e. such key codes that can cause
	 * pre-boot actions. If the string is empty (""), then only
	 * "key_magic" is checked (old behaviour); the string "125" causes
	 * checks for "key_magic1", "key_magic2" and "key_magic5", etc.
	 */
	if ((kbd_magic_keys = getenv ("magic_keys")) == NULL)
		kbd_magic_keys = "";

	/* loop over all magic keys;
	 * use '\0' suffix in case of empty string
	 */
	for (suffix=(uchar *)kbd_magic_keys; *suffix || suffix==(uchar *)kbd_magic_keys; ++suffix) {
		sprintf (magic, "%s%c", kbd_magic_prefix, *suffix);
#if 0
		printf ("### Check magic \"%s\"\n", magic);
#endif
		if (compare_magic(kbd_data, (uchar *)getenv(magic)) == 0) {
			char cmd_name[sizeof (kbd_command_prefix) + 1];
			char *cmd;

			sprintf (cmd_name, "%s%c", kbd_command_prefix, *suffix);

			cmd = getenv (cmd_name);
#if 0
			printf ("### Set PREBOOT to $(%s): \"%s\"\n",
				cmd_name, cmd ? cmd : "<<NULL>>");
#endif
			*kbd_data = *suffix;
			return ((uchar *)cmd);
		}
	}
#if 0
	printf ("### Delete PREBOOT\n");
#endif
	*kbd_data = '\0';
	return (NULL);
}

int do_kbd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uchar kbd_data[KEYBD_DATALEN];
	char keybd_env[2 * KEYBD_DATALEN + 1];
	int i;

	/* Read keys */
	get_pressed_keys(kbd_data);
	puts ("Keys:");
	for (i = 0; i < KEYBD_DATALEN; ++i) {
		sprintf (keybd_env + i + i, "%02X", kbd_data[i]);
		printf (" %02x", kbd_data[i]);
	}
	putc ('\n');
	setenv ("keybd", keybd_env);
	return 0;
}

U_BOOT_CMD(
	   kbd,	1,	1,	do_kbd,
	   "kbd     - read keyboard status\n",
	   NULL
);

#endif /* DELTA_CHECK_KEYBD */


int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = PHYS_SDRAM_4_SIZE;

	return 0;
}

void i2c_init_board()
{
	CKENB |= (CKENB_4_I2C);

	/* setup I2C GPIO's */
	GPIO32 = 0x801;		/* SCL = Alt. Fkt. 1 */
	GPIO33 = 0x801;		/* SDA = Alt. Fkt. 1 */
}

/* initialize the DA9030 Power Controller */
static void init_DA9030()
{
	uchar addr = (uchar) DA9030_I2C_ADDR, val = 0;

	CKENB |= CKENB_7_GPIO;
	udelay(100);

	/* Rising Edge on EXTON to reset DA9030 */
	GPIO17 = 0x8800;	/* configure GPIO17, no pullup, -down */
	GPDR0 |= (1<<17);	/* GPIO17 is output */
	GSDR0 = (1<<17);
	GPCR0 = (1<<17);	/* drive GPIO17 low */
	GPSR0 = (1<<17);	/* drive GPIO17 high */

#if CFG_DA9030_EXTON_DELAY
	udelay((unsigned long) CFG_DA9030_EXTON_DELAY);	/* wait for DA9030 */
#endif
	GPCR0 = (1<<17);	/* drive GPIO17 low */

	/* reset the watchdog and go active (0xec) */
	val = (SYS_CONTROL_A_HWRES_ENABLE |
	       (0x6<<4) |
	       SYS_CONTROL_A_WDOG_ACTION |
	       SYS_CONTROL_A_WATCHDOG);
	if(i2c_write(addr, SYS_CONTROL_A, 1, &val, 1)) {
		printf("Error accessing DA9030 via i2c.\n");
		return;
	}

	val = 0x80;
	if(i2c_write(addr, IRQ_MASK_B, 1, &val, 1)) {
		printf("Error accessing DA9030 via i2c.\n");
		return;
	}

	i2c_reg_write(addr, REG_CONTROL_1_97, 0xfd); /* disable LDO1, enable LDO6 */
	i2c_reg_write(addr, LDO2_3, 0xd1);	/* LDO2 =1,9V, LDO3=3,1V */
	i2c_reg_write(addr, LDO4_5, 0xcc);	/* LDO2 =1,9V, LDO3=3,1V */
	i2c_reg_write(addr, LDO6_SIMCP, 0x3e);	/* LDO6=3,2V, SIMCP = 5V support */
	i2c_reg_write(addr, LDO7_8, 0xc9);	/* LDO7=2,7V, LDO8=3,0V */
	i2c_reg_write(addr, LDO9_12, 0xec);	/* LDO9=3,0V, LDO12=3,2V */
	i2c_reg_write(addr, BUCK, 0x0c);	/* Buck=1.2V */
	i2c_reg_write(addr, REG_CONTROL_2_98, 0x7f); /* All LDO'S on 8,9,10,11,12,14 */
	i2c_reg_write(addr, LDO_10_11, 0xcc);	/* LDO10=3.0V  LDO11=3.0V */
	i2c_reg_write(addr, LDO_15, 0xae);	/* LDO15=1.8V, dislock first 3bit */
	i2c_reg_write(addr, LDO_14_16, 0x05);	/* LDO14=2.8V, LDO16=NB */
	i2c_reg_write(addr, LDO_18_19, 0x9c);	/* LDO18=3.0V, LDO19=2.7V */
	i2c_reg_write(addr, LDO_17_SIMCP0, 0x2c); /* LDO17=3.0V, SIMCP=3V support */
	i2c_reg_write(addr, BUCK2_DVC1, 0x9a);	/* Buck2=1.5V plus Update support of 520 MHz */
	i2c_reg_write(addr, REG_CONTROL_2_18, 0x43); /* Ball on */
	i2c_reg_write(addr, MISC_CONTROLB, 0x08); /* session valid enable */
	i2c_reg_write(addr, USBPUMP, 0xc1);	/* start pump, ignore HW signals */

	val = i2c_reg_read(addr, STATUS);
	if(val & STATUS_CHDET)
		printf("Charger detected, turning on LED.\n");
	else {
		printf("No charger detetected.\n");
		/* undervoltage? print error and power down */
	}
}


#if 0
/* reset the DA9030 watchdog */
void hw_watchdog_reset(void)
{
	uchar addr = (uchar) DA9030_I2C_ADDR, val = 0;
	val = i2c_reg_read(addr, SYS_CONTROL_A);
	val |= SYS_CONTROL_A_WATCHDOG;
	i2c_reg_write(addr, SYS_CONTROL_A, val);
}
#endif
