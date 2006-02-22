/*
 * amirix.c: ppcboot platform support for AMIRIX board
 *
 * Copyright 2002 Mind NV
 * Copyright 2003 AMIRIX Systems Inc.
 *
 * http://www.mind.be/
 * http://www.amirix.com/
 *
 * Author : Peter De Schrijver (p2@mind.be)
 *          Frank Smith (smith@amirix.com)
 *
 * Derived from : Other platform support files in this tree, ml2
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License (GPL) version 2, incorporated herein by
 * reference. Drivers based on or derived from this code fall under the GPL
 * and must retain the authorship, copyright and this license notice. This
 * file is not a complete program and may only be used when the entire
 * program is licensed under the GPL.
 *
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>

#include "powerspan.h"
#include "ap1000.h"

int board_pre_init (void)
{
	return 0;
}

/** serial number and platform display at startup */
int checkboard (void)
{
	char *s = getenv ("serial#");
	char *e;

	/* After a loadace command, the SystemAce control register is left in a wonky state. */
	/* this code did not work in board_pre_init */
	unsigned char *p = (unsigned char *) AP1000_SYSACE_REGBASE;

	p[SYSACE_CTRLREG0] = 0x0;

	/* add platform and device to banner */
	switch (get_device ()) {
	case AP1xx_AP107_TARGET:
		puts (AP1xx_AP107_TARGET_STR);
		break;
	case AP1xx_AP120_TARGET:
		puts (AP1xx_AP120_TARGET_STR);
		break;
	case AP1xx_AP130_TARGET:
		puts (AP1xx_AP130_TARGET_STR);
		break;
	case AP1xx_AP1070_TARGET:
		puts (AP1xx_AP1070_TARGET_STR);
		break;
	case AP1xx_AP1100_TARGET:
		puts (AP1xx_AP1100_TARGET_STR);
		break;
	default:
		puts (AP1xx_UNKNOWN_STR);
		break;
	}
	puts (AP1xx_TARGET_STR);
	puts (" with ");

	switch (get_platform ()) {
	case AP100_BASELINE_PLATFORM:
	case AP1000_BASELINE_PLATFORM:
		puts (AP1xx_BASELINE_PLATFORM_STR);
		break;
	case AP1xx_QUADGE_PLATFORM:
		puts (AP1xx_QUADGE_PLATFORM_STR);
		break;
	case AP1xx_MGT_REF_PLATFORM:
		puts (AP1xx_MGT_REF_PLATFORM_STR);
		break;
	case AP1xx_STANDARD_PLATFORM:
		puts (AP1xx_STANDARD_PLATFORM_STR);
		break;
	case AP1xx_DUAL_PLATFORM:
		puts (AP1xx_DUAL_PLATFORM_STR);
		break;
	case AP1xx_BASE_SRAM_PLATFORM:
		puts (AP1xx_BASE_SRAM_PLATFORM_STR);
		break;
	case AP1xx_PCI_PCB_TESTPLATFORM:
	case AP1000_PCI_PCB_TESTPLATFORM:
		puts (AP1xx_PCI_PCB_TESTPLATFORM_STR);
		break;
	case AP1xx_DUAL_GE_MEZZ_TESTPLATFORM:
		puts (AP1xx_DUAL_GE_MEZZ_TESTPLATFORM_STR);
		break;
	case AP1xx_SFP_MEZZ_TESTPLATFORM:
		puts (AP1xx_SFP_MEZZ_TESTPLATFORM_STR);
		break;
	default:
		puts (AP1xx_UNKNOWN_STR);
		break;
	}

	if ((get_platform () & AP1xx_TESTPLATFORM_MASK) != 0) {
		puts (AP1xx_TESTPLATFORM_STR);
	} else {
		puts (AP1xx_PLATFORM_STR);
	}

	putc ('\n');

	puts ("Serial#: ");

	if (!s) {
		printf ("### No HW ID - assuming AMIRIX");
	} else {
		for (e = s; *e; ++e) {
			if (*e == ' ')
				break;
		}

		for (; s < e; ++s) {
			putc (*s);
		}
	}

	putc ('\n');

	return (0);
}


long int initdram (int board_type)
{
	char *s = getenv ("dramsize");

	if (s != NULL) {
		if ((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X'))) {
			s += 2;
		}
		return (long int)simple_strtoul (s, NULL, 16);
	} else {
		/* give all 64 MB */
		return 64 * 1024 * 1024;
	}
}

unsigned int get_platform (void)
{
	unsigned int *revision_reg_ptr = (unsigned int *) AP1xx_FPGA_REV_ADDR;

	return (*revision_reg_ptr & AP1xx_PLATFORM_MASK);
}

unsigned int get_device (void)
{
	unsigned int *revision_reg_ptr = (unsigned int *) AP1xx_FPGA_REV_ADDR;

	return (*revision_reg_ptr & AP1xx_TARGET_MASK);
}

#if 0				/* loadace is not working; it appears to be a hardware issue with the system ace. */
/*
   This function loads FPGA configurations from the SystemACE CompactFlash
*/
int do_loadace (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned char *p = (unsigned char *) AP1000_SYSACE_REGBASE;
	int cfg;

	if ((p[SYSACE_STATREG0] & 0x10) == 0) {
		p[SYSACE_CTRLREG0] = 0x80;
		printf ("\nNo CompactFlash Detected\n\n");
		p[SYSACE_CTRLREG0] = 0x00;
		return 1;
	}

	/* reset configuration controller: |  0x80 */
	/* select cpflash                  & ~0x40 */
	/* cfg start                       |  0x20 */
	/* wait for cfgstart               & ~0x10 */
	/* force cfgmode:                  |  0x08 */
	/* do no force cfgaddr:            & ~0x04 */
	/* clear mpulock:                  & ~0x02 */
	/* do not force lock request       & ~0x01 */

	p[SYSACE_CTRLREG0] = 0x80 | 0x20 | 0x08;
	p[SYSACE_CTRLREG1] = 0x00;

	/* force config address if arg2 exists */
	if (argc == 2) {
		cfg = simple_strtoul (argv[1], NULL, 10);

		if (cfg > 7) {
			printf ("\nInvalid Configuration\n\n");
			p[SYSACE_CTRLREG0] = 0x00;
			return 1;
		}
		/* Set config address */
		p[SYSACE_CTRLREG1] = (cfg << 5);
		/* force cfgaddr */
		p[SYSACE_CTRLREG0] |= 0x04;

	} else {
		cfg = (p[SYSACE_STATREG1] & 0xE0) >> 5;
	}

	/* release configuration controller */
	printf ("\nLoading V2PRO with config %d...\n", cfg);
	p[SYSACE_CTRLREG0] &= ~0x80;


	while ((p[SYSACE_STATREG1] & 0x01) == 0) {

		if (p[SYSACE_ERRREG0] & 0x80) {
			/* attempting to load an invalid configuration makes the cpflash */
			/* appear to be removed. Reset here to avoid that problem */
			p[SYSACE_CTRLREG0] = 0x80;
			printf ("\nConfiguration %d Read Error\n\n", cfg);
			p[SYSACE_CTRLREG0] = 0x00;
			return 1;
		}
	}

	p[SYSACE_CTRLREG0] |= 0x20;

	return 0;
}
#endif

/** Console command to display and set the software reconfigure byte
  * <pre>
  * swconfig        - display the current value of the software reconfigure byte
  * swconfig [#]    - change the software reconfigure byte to #
  * </pre>
  * @param  *cmdtp  [IN] as passed by run_command (ignored)
  * @param  flag    [IN] as passed by run_command (ignored)
  * @param  argc    [IN] as passed by run_command if 1, display, if 2 change
  * @param  *argv[] [IN] contains the parameters to use
  * @return
  * <pre>
  *      0 if passed
  *     -1 if failed
  * </pre>
  */
int do_swconfigbyte (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned char *sector_buffer = NULL;
	unsigned char input_char;
	int write_result;
	unsigned int input_uint;

	/* display value if no argument */
	if (argc < 2) {
		printf ("Software configuration byte is currently: 0x%02x\n",
			*((unsigned char *) (SW_BYTE_SECTOR_ADDR +
					     SW_BYTE_SECTOR_OFFSET)));
		return 0;
	} else if (argc > 3) {
		printf ("Too many arguments\n");
		return -1;
	}

	/* if 3 arguments, 3rd argument is the address to use */
	if (argc == 3) {
		input_uint = simple_strtoul (argv[1], NULL, 16);
		sector_buffer = (unsigned char *) input_uint;
	} else {
		sector_buffer = (unsigned char *) DEFAULT_TEMP_ADDR;
	}

	input_char = simple_strtoul (argv[1], NULL, 0);
	if ((input_char & ~SW_BYTE_MASK) != 0) {
		printf ("Input of 0x%02x will be masked to 0x%02x\n",
			input_char, (input_char & SW_BYTE_MASK));
		input_char = input_char & SW_BYTE_MASK;
	}

	memcpy (sector_buffer, (void *) SW_BYTE_SECTOR_ADDR,
		SW_BYTE_SECTOR_SIZE);
	sector_buffer[SW_BYTE_SECTOR_OFFSET] = input_char;


	printf ("Erasing Flash...");
	if (flash_sect_erase
	    (SW_BYTE_SECTOR_ADDR,
	     (SW_BYTE_SECTOR_ADDR + SW_BYTE_SECTOR_OFFSET))) {
		return -1;
	}

	printf ("Writing to Flash... ");
	write_result =
		flash_write ((char *)sector_buffer, SW_BYTE_SECTOR_ADDR,
			     SW_BYTE_SECTOR_SIZE);
	if (write_result != 0) {
		flash_perror (write_result);
		return -1;
	} else {
		printf ("done\n");
		printf ("Software configuration byte is now: 0x%02x\n",
			*((unsigned char *) (SW_BYTE_SECTOR_ADDR +
					     SW_BYTE_SECTOR_OFFSET)));
	}

	return 0;
}

#define ONE_SECOND 1000000

int do_pause (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int pause_time;
	unsigned int delay_time;
	int break_loop = 0;

	/* display value if no argument */
	if (argc < 2) {
		pause_time = 1;
	}

	else if (argc > 2) {
		printf ("Too many arguments\n");
		return -1;
	} else {
		pause_time = simple_strtoul (argv[1], NULL, 0);
	}

	printf ("Pausing with a poll time of %d, press any key to reactivate\n", pause_time);
	delay_time = pause_time * ONE_SECOND;
	while (break_loop == 0) {
		udelay (delay_time);
		if (serial_tstc () != 0) {
			break_loop = 1;
			/* eat user key presses */
			while (serial_tstc () != 0) {
				serial_getc ();
			}
		}
	}

	return 0;
}

int do_swreconfig (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	printf ("Triggering software reconfigure (software config byte is 0x%02x)...\n",
		*((unsigned char *) (SW_BYTE_SECTOR_ADDR + SW_BYTE_SECTOR_OFFSET)));
	udelay (1000);
	*((unsigned char *) AP1000_CPLD_BASE) = 1;

	return 0;
}

#define GET_DECIMAL(low_byte) ((low_byte >> 5) * 125)
#define TEMP_BUSY_BIT   0x80
#define TEMP_LHIGH_BIT  0x40
#define TEMP_LLOW_BIT   0x20
#define TEMP_EHIGH_BIT  0x10
#define TEMP_ELOW_BIT   0x08
#define TEMP_OPEN_BIT   0x04
#define TEMP_ETHERM_BIT 0x02
#define TEMP_LTHERM_BIT 0x01

int do_temp_sensor (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char cmd;
	int ret_val = 0;
	unsigned char temp_byte;
	int temp;
	int temp_low;
	int low;
	int low_low;
	int high;
	int high_low;
	int therm;
	unsigned char user_data[4] = { 0 };
	int user_data_count = 0;
	int ii;

	if (argc > 1) {
		cmd = argv[1][0];
	} else {
		cmd = 's';	/* default to status */
	}

	user_data_count = argc - 2;
	for (ii = 0; ii < user_data_count; ii++) {
		user_data[ii] = simple_strtoul (argv[2 + ii], NULL, 0);
	}
	switch (cmd) {
	case 's':
		if (I2CAccess
		    (0x2, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		printf ("Status    : 0x%02x  ", temp_byte);
		if (temp_byte & TEMP_BUSY_BIT)
			printf ("BUSY ");

		if (temp_byte & TEMP_LHIGH_BIT)
			printf ("LHIGH ");

		if (temp_byte & TEMP_LLOW_BIT)
			printf ("LLOW ");

		if (temp_byte & TEMP_EHIGH_BIT)
			printf ("EHIGH ");

		if (temp_byte & TEMP_ELOW_BIT)
			printf ("ELOW ");

		if (temp_byte & TEMP_OPEN_BIT)
			printf ("OPEN ");

		if (temp_byte & TEMP_ETHERM_BIT)
			printf ("ETHERM ");

		if (temp_byte & TEMP_LTHERM_BIT)
			printf ("LTHERM");

		printf ("\n");

		if (I2CAccess
		    (0x3, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		printf ("Config    : 0x%02x  ", temp_byte);

		if (I2CAccess
		    (0x4, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			printf ("\n");
			goto fail;
		}
		printf ("Conversion: 0x%02x\n", temp_byte);
		if (I2CAccess
		    (0x22, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		printf ("Cons Alert: 0x%02x  ", temp_byte);

		if (I2CAccess
		    (0x21, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			printf ("\n");
			goto fail;
		}
		printf ("Therm Hyst: %d\n", temp_byte);

		if (I2CAccess
		    (0x0, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		temp = temp_byte;
		if (I2CAccess
		    (0x6, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		low = temp_byte;
		if (I2CAccess
		    (0x5, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		high = temp_byte;
		if (I2CAccess
		    (0x20, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		therm = temp_byte;
		printf ("Local Temp: %2d     Low: %2d     High: %2d     THERM: %2d\n", temp, low, high, therm);

		if (I2CAccess
		    (0x1, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		temp = temp_byte;
		if (I2CAccess
		    (0x10, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		temp_low = temp_byte;
		if (I2CAccess
		    (0x8, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		low = temp_byte;
		if (I2CAccess
		    (0x14, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		low_low = temp_byte;
		if (I2CAccess
		    (0x7, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		high = temp_byte;
		if (I2CAccess
		    (0x13, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		high_low = temp_byte;
		if (I2CAccess
		    (0x19, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		therm = temp_byte;
		if (I2CAccess
		    (0x11, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &temp_byte, I2C_READ) != 0) {
			goto fail;
		}
		printf ("Ext Temp  : %2d.%03d Low: %2d.%03d High: %2d.%03d THERM: %2d Offset: %2d\n", temp, GET_DECIMAL (temp_low), low, GET_DECIMAL (low_low), high, GET_DECIMAL (high_low), therm, temp_byte);
		break;
	case 'l':		/* alter local limits : low, high, therm */
		if (argc < 3) {
			goto usage;
		}

		/* low */
		if (I2CAccess
		    (0xC, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &user_data[0], I2C_WRITE) != 0) {
			goto fail;
		}

		if (user_data_count > 1) {
			/* high */
			if (I2CAccess
			    (0xB, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
			     &user_data[1], I2C_WRITE) != 0) {
				goto fail;
			}
		}

		if (user_data_count > 2) {
			/* therm */
			if (I2CAccess
			    (0x20, I2C_SENSOR_DEV,
			     I2C_SENSOR_CHIP_SEL, &user_data[2],
			     I2C_WRITE) != 0) {
				goto fail;
			}
		}
		break;
	case 'e':		/* alter external limits: low, high, therm, offset */
		if (argc < 3) {
			goto usage;
		}

		/* low */
		if (I2CAccess
		    (0xE, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &user_data[0], I2C_WRITE) != 0) {
			goto fail;
		}

		if (user_data_count > 1) {
			/* high */
			if (I2CAccess
			    (0xD, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
			     &user_data[1], I2C_WRITE) != 0) {
				goto fail;
			}
		}

		if (user_data_count > 2) {
			/* therm */
			if (I2CAccess
			    (0x19, I2C_SENSOR_DEV,
			     I2C_SENSOR_CHIP_SEL, &user_data[2],
			     I2C_WRITE) != 0) {
				goto fail;
			}
		}

		if (user_data_count > 3) {
			/* offset */
			if (I2CAccess
			    (0x11, I2C_SENSOR_DEV,
			     I2C_SENSOR_CHIP_SEL, &user_data[3],
			     I2C_WRITE) != 0) {
				goto fail;
			}
		}
		break;
	case 'c':		/* alter config settings: config, conv, cons alert, therm hyst */
		if (argc < 3) {
			goto usage;
		}

		/* config */
		if (I2CAccess
		    (0x9, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
		     &user_data[0], I2C_WRITE) != 0) {
			goto fail;
		}

		if (user_data_count > 1) {
			/* conversion */
			if (I2CAccess
			    (0xA, I2C_SENSOR_DEV, I2C_SENSOR_CHIP_SEL,
			     &user_data[1], I2C_WRITE) != 0) {
				goto fail;
			}
		}

		if (user_data_count > 2) {
			/* cons alert */
			if (I2CAccess
			    (0x22, I2C_SENSOR_DEV,
			     I2C_SENSOR_CHIP_SEL, &user_data[2],
			     I2C_WRITE) != 0) {
				goto fail;
			}
		}

		if (user_data_count > 3) {
			/* therm hyst */
			if (I2CAccess
			    (0x21, I2C_SENSOR_DEV,
			     I2C_SENSOR_CHIP_SEL, &user_data[3],
			     I2C_WRITE) != 0) {
				goto fail;
			}
		}
		break;
	default:
		goto usage;
	}

	goto done;
fail:
	printf ("Access to sensor failed\n");
	ret_val = -1;
	goto done;
usage:
	printf ("Usage:\n%s\n", cmdtp->help);

done:
	return ret_val;
}

U_BOOT_CMD (temp, 6, 0, do_temp_sensor,
	    "temp    - interact with the temperature sensor\n",
	    "temp [s]\n"
	    "        - Show status.\n"
	    "temp l LOW [HIGH] [THERM]\n"
	    "        - Set local limits.\n"
	    "temp e LOW [HIGH] [THERM] [OFFSET]\n"
	    "        - Set external limits.\n"
	    "temp c CONFIG [CONVERSION] [CONS. ALERT] [THERM HYST]\n"
	    "        - Set config options.\n"
	    "\n"
	    "All values can be decimal or hex (hex preceded with 0x).\n"
	    "Only whole numbers are supported for external limits.\n");

#if 0
U_BOOT_CMD (loadace, 2, 0, do_loadace,
	    "loadace - load fpga configuration from System ACE compact flash\n",
	    "N\n"
	    "    - Load configuration N (0-7) from System ACE compact flash\n"
	    "loadace\n" "    - loads default configuration\n");
#endif

U_BOOT_CMD (swconfig, 2, 0, do_swconfigbyte,
	    "swconfig- display or modify the software configuration byte\n",
	    "N [ADDRESS]\n"
	    "    - set software configuration byte to N, optionally use ADDRESS as\n"
	    "      location of buffer for flash copy\n"
	    "swconfig\n" "    - display software configuration byte\n");

U_BOOT_CMD (pause, 2, 0, do_pause,
	    "pause   - sleep processor until any key is pressed with poll time of N seconds\n",
	    "N\n"
	    "    - sleep processor until any key is pressed with poll time of N seconds\n"
	    "pause\n"
	    "    - sleep processor until any key is pressed with poll time of 1 second\n");

U_BOOT_CMD (swrecon, 1, 0, do_swreconfig,
	    "swrecon - trigger a board reconfigure to the software selected configuration\n",
	    "\n"
	    "    - trigger a board reconfigure to the software selected configuration\n");
