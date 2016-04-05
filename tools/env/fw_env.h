/*
 * (C) Copyright 2002-2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <aes.h>
#include <stdint.h>

/* Pull in the current config to define the default environment */
#include <linux/kconfig.h>

#ifndef __ASSEMBLY__
#define __ASSEMBLY__ /* get only #defines from config.h */
#include <config.h>
#undef	__ASSEMBLY__
#else
#include <config.h>
#endif

/*
 * To build the utility with the static configuration
 * comment out the next line.
 * See included "fw_env.config" sample file
 * for notes on configuration.
 */
#define CONFIG_FILE     "/etc/fw_env.config"

#ifndef CONFIG_FILE
#define HAVE_REDUND /* For systems with 2 env sectors */
#define DEVICE1_NAME      "/dev/mtd1"
#define DEVICE2_NAME      "/dev/mtd2"
#define DEVICE1_OFFSET    0x0000
#define ENV1_SIZE         0x4000
#define DEVICE1_ESIZE     0x4000
#define DEVICE1_ENVSECTORS     2
#define DEVICE2_OFFSET    0x0000
#define ENV2_SIZE         0x4000
#define DEVICE2_ESIZE     0x4000
#define DEVICE2_ENVSECTORS     2
#endif

#ifndef CONFIG_BAUDRATE
#define CONFIG_BAUDRATE		115200
#endif

#ifndef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#ifndef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND							\
	"bootp; "								\
	"setenv bootargs root=/dev/nfs nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "	\
	"bootm"
#endif

struct env_opts {
#ifdef CONFIG_FILE
	char *config_file;
#endif
	int aes_flag; /* Is AES encryption used? */
	uint8_t aes_key[AES_KEY_LENGTH];
};

int parse_aes_key(char *key, uint8_t *bin_key);

int fw_printenv(int argc, char *argv[], int value_only, struct env_opts *opts);
char *fw_getenv(char *name);
int fw_setenv(int argc, char *argv[], struct env_opts *opts);
int fw_parse_script(char *fname, struct env_opts *opts);
int fw_env_open(struct env_opts *opts);
int fw_env_write(char *name, char *value);
int fw_env_close(struct env_opts *opts);

unsigned long crc32(unsigned long, const unsigned char *, unsigned);
