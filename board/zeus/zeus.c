/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <malloc.h>
#include <environment.h>
#include <logbuff.h>
#include <post.h>

#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx-gpio.h>

DECLARE_GLOBAL_DATA_PTR;

#define REBOOT_MAGIC	0x07081967
#define REBOOT_NOP	0x00000000
#define REBOOT_DO_POST	0x00000001

extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/
extern env_t *env_ptr;
extern uchar default_environment[];

ulong flash_get_size(ulong base, int banknum);
void env_crc_update(void);

static u32 start_time;

int board_early_init_f(void)
{
	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(UIC0ER, 0x00000000);	/* disable all ints */
	mtdcr(UIC0CR, 0x00000000);
	mtdcr(UIC0PR, 0xFFFF7F00);	/* set int polarities */
	mtdcr(UIC0TR, 0x00000000);	/* set int trigger levels */
	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(UIC0VCR, 0x00000001);	/* set vect base=0,INT0 highest priority */

	/*
	 * Configure CPC0_PCI to enable PerWE as output
	 */
	mtdcr(CPC0_PCI, CPC0_PCI_SPE);

	return 0;
}

int misc_init_r(void)
{
	u32 pbcr;
	int size_val = 0;
	u32 post_magic;
	u32 post_val;

	post_magic = in_be32((void *)CONFIG_SYS_POST_MAGIC);
	post_val = in_be32((void *)CONFIG_SYS_POST_VAL);
	if ((post_magic == REBOOT_MAGIC) && (post_val == REBOOT_DO_POST)) {
		/*
		 * Set special bootline bootparameter to pass this POST boot
		 * mode to Linux to reset the username/password
		 */
		setenv("addmisc", "setenv bootargs \\${bootargs} factory_reset=yes");

		/*
		 * Normally don't run POST tests, only when enabled
		 * via the sw-reset button. So disable further tests
		 * upon next bootup here.
		 */
		out_be32((void *)CONFIG_SYS_POST_VAL, REBOOT_NOP);
	} else {
		/*
		 * Only run POST when initiated via the sw-reset button mechanism
		 */
		post_word_store(0);
	}

	/*
	 * Get current time
	 */
	start_time = get_timer(0);

	/*
	 * FLASH stuff...
	 */

	/* Re-do sizing to get full correct info */

	/* adjust flash start and offset */
	mfebc(PB0CR, pbcr);
	switch (gd->bd->bi_flashsize) {
	case 1 << 20:
		size_val = 0;
		break;
	case 2 << 20:
		size_val = 1;
		break;
	case 4 << 20:
		size_val = 2;
		break;
	case 8 << 20:
		size_val = 3;
		break;
	case 16 << 20:
		size_val = 4;
		break;
	case 32 << 20:
		size_val = 5;
		break;
	case 64 << 20:
		size_val = 6;
		break;
	case 128 << 20:
		size_val = 7;
		break;
	}
	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
	mtebc(PB0CR, pbcr);

	/*
	 * Re-check to get correct base address
	 */
	flash_get_size(gd->bd->bi_flashstart, 0);

	/* Monitor protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    -CONFIG_SYS_MONITOR_LEN,
			    0xffffffff,
			    &flash_info[0]);

	/* Env protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    CONFIG_ENV_ADDR_REDUND,
			    CONFIG_ENV_ADDR_REDUND + 2*CONFIG_ENV_SECT_SIZE - 1,
			    &flash_info[0]);

	return 0;
}

/*
 * Check Board Identity:
 */
int checkboard(void)
{
	char *s = getenv("serial#");

	puts("Board: Zeus-");

	if (in_be32((void *)GPIO0_IR) & GPIO_VAL(CONFIG_SYS_GPIO_ZEUS_PE))
		puts("PE");
	else
		puts("CE");

	puts(" of BulletEndPoint");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	/* both LED's off */
	gpio_write_bit(CONFIG_SYS_GPIO_LED_RED, 0);
	gpio_write_bit(CONFIG_SYS_GPIO_LED_GREEN, 0);
	udelay(10000);
	/* and on again */
	gpio_write_bit(CONFIG_SYS_GPIO_LED_RED, 1);
	gpio_write_bit(CONFIG_SYS_GPIO_LED_GREEN, 1);

	return (0);
}

static int default_env_var(char *buf, char *var)
{
	char *ptr;
	char *val;

	/*
	 * Find env variable
	 */
	ptr = strstr(buf + 4, var);
	if (ptr == NULL) {
		printf("ERROR: %s not found!\n", var);
		return -1;
	}
	ptr += strlen(var) + 1;

	/*
	 * Now the ethaddr needs to be updated in the "normal"
	 * environment storage -> redundant flash.
	 */
	val = ptr;
	setenv(var, val);
	printf("Updated %s from eeprom to %s!\n", var, val);

	return 0;
}

static int restore_default(void)
{
	char *buf;
	char *buf_save;
	u32 crc;

	set_default_env("");

	gd->env_valid = 1;

	/*
	 * Read board specific values from I2C EEPROM
	 * and set env variables accordingly
	 * -> ethaddr, eth1addr, serial#
	 */
	buf = buf_save = malloc(FACTORY_RESET_ENV_SIZE);
	if (buf == NULL) {
		printf("ERROR: malloc() failed\n");
		return -1;
	}
	if (eeprom_read(FACTORY_RESET_I2C_EEPROM, FACTORY_RESET_ENV_OFFS,
			(u8 *)buf, FACTORY_RESET_ENV_SIZE)) {
		puts("\nError reading EEPROM!\n");
	} else {
		crc = crc32(0, (u8 *)(buf + 4), FACTORY_RESET_ENV_SIZE - 4);
		if (crc != *(u32 *)buf) {
			printf("ERROR: crc mismatch %08x %08x\n", crc, *(u32 *)buf);
			return -1;
		}

		default_env_var(buf, "ethaddr");
		buf += 8 + 18;
		default_env_var(buf, "eth1addr");
		buf += 9 + 18;
		default_env_var(buf, "serial#");
	}

	/*
	 * Finally save updated env variables back to flash
	 */
	saveenv();

	free(buf_save);

	return 0;
}

int do_set_default(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *buf;
	char *buf_save;
	char str[32];
	u32 crc;
	char var[32];

	if (argc < 4) {
		puts("ERROR!\n");
		return -1;
	}

	buf = buf_save = malloc(FACTORY_RESET_ENV_SIZE);
	memset(buf, 0, FACTORY_RESET_ENV_SIZE);

	strcpy(var, "ethaddr");
	printf("Setting %s to %s\n", var, argv[1]);
	sprintf(str, "%s=%s", var, argv[1]);
	strcpy(buf + 4, str);
	buf += strlen(str) + 1;

	strcpy(var, "eth1addr");
	printf("Setting %s to %s\n", var, argv[2]);
	sprintf(str, "%s=%s", var, argv[2]);
	strcpy(buf + 4, str);
	buf += strlen(str) + 1;

	strcpy(var, "serial#");
	printf("Setting %s to %s\n", var, argv[3]);
	sprintf(str, "%s=%s", var, argv[3]);
	strcpy(buf + 4, str);

	crc = crc32(0, (u8 *)(buf_save + 4), FACTORY_RESET_ENV_SIZE - 4);
	*(u32 *)buf_save = crc;

	if (eeprom_write(FACTORY_RESET_I2C_EEPROM, FACTORY_RESET_ENV_OFFS,
			 (u8 *)buf_save, FACTORY_RESET_ENV_SIZE)) {
		puts("\nError writing EEPROM!\n");
		return -1;
	}

	free(buf_save);

	return 0;
}

U_BOOT_CMD(
	setdef,	4,	1,	do_set_default,
	"write board-specific values to EEPROM (ethaddr...)",
	"ethaddr eth1addr serial#\n    - write board-specific values to EEPROM"
	);

static inline int sw_reset_pressed(void)
{
	return !(in_be32((void *)GPIO0_IR) & GPIO_VAL(CONFIG_SYS_GPIO_SW_RESET));
}

int do_chkreset(cmd_tbl_t* cmdtp, int flag, int argc, char * const argv[])
{
	int delta;
	int count = 0;
	int post = 0;
	int factory_reset = 0;

	if (!sw_reset_pressed()) {
		printf("SW-Reset already high (Button released)\n");
		printf("-> No action taken!\n");
		return 0;
	}

	printf("Waiting for SW-Reset button to be released.");

	while (1) {
		delta = get_timer(start_time);
		if (!sw_reset_pressed())
			break;

		if ((delta > CONFIG_SYS_TIME_POST) && !post) {
			printf("\nWhen released now, POST tests will be started.");
			gpio_write_bit(CONFIG_SYS_GPIO_LED_GREEN, 0);
			post = 1;
		}

		if ((delta > CONFIG_SYS_TIME_FACTORY_RESET) && !factory_reset) {
			printf("\nWhen released now, factory default values"
			       " will be restored.");
			gpio_write_bit(CONFIG_SYS_GPIO_LED_RED, 0);
			factory_reset = 1;
		}

		udelay(1000);
		if (!(count++ % 1000))
			printf(".");
	}


	printf("\nSW-Reset Button released after %d milli-seconds!\n", delta);

	if (delta > CONFIG_SYS_TIME_FACTORY_RESET) {
		printf("Starting factory reset value restoration...\n");

		/*
		 * Restore default setting
		 */
		restore_default();

		/*
		 * Reset the board for default to become valid
		 */
		do_reset(NULL, 0, 0, NULL);

		return 0;
	}

	if (delta > CONFIG_SYS_TIME_POST) {
		printf("Starting POST configuration...\n");

		/*
		 * Enable POST upon next bootup
		 */
		out_be32((void *)CONFIG_SYS_POST_MAGIC, REBOOT_MAGIC);
		out_be32((void *)CONFIG_SYS_POST_VAL, REBOOT_DO_POST);
		post_bootmode_init();

		/*
		 * Reset the logbuffer for a clean start
		 */
		logbuff_reset();

		do_reset(NULL, 0, 0, NULL);

		return 0;
	}

	return 0;
}

U_BOOT_CMD (
	chkreset, 1, 1, do_chkreset,
	"Check for status of SW-reset button and act accordingly",
	""
);

#if defined(CONFIG_POST)
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{
	u32 post_magic;
	u32 post_val;

	post_magic = in_be32((void *)CONFIG_SYS_POST_MAGIC);
	post_val = in_be32((void *)CONFIG_SYS_POST_VAL);

	if ((post_magic == REBOOT_MAGIC) && (post_val == REBOOT_DO_POST))
		return 1;
	else
		return 0;
}
#endif /* CONFIG_POST */
