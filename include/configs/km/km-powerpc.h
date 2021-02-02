/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#ifndef __CONFIG_KEYMILE_POWERPC_H
#define __CONFIG_KEYMILE_POWERPC_H

/* Do boardspecific init for all boards */

/* EEprom support 24C08, 24C16, 24C64 */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_ENABLE
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3  /* 8 Byte write page */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */

/* Reserve 4 MB for malloc */
#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)

/* Increase max size of compressed kernel */
#define CONFIG_SYS_BOOTM_LEN		0x2000000     /* 32 MB */

/******************************************************************************
 * (PRAM usage)
 * ... -------------------------------------------------------
 * ... |ROOTFSSIZE | PNVRAM |PHRAM |RESERVED_PRAM | END_OF_RAM
 * ... |<------------------- pram -------------------------->|
 * ... -------------------------------------------------------
 * @END_OF_RAM:
 * @CONFIG_KM_RESERVED_PRAM: reserved pram for special purpose
 * @CONFIG_KM_PHRAM: address for /var
 * @CONFIG_KM_PNVRAM: address for PNVRAM (for the application)
 * @CONFIG_KM_ROOTFSSIZE: address for rootfilesystem in RAM
 */

/* size of rootfs in RAM */
#define CONFIG_KM_ROOTFSSIZE	0x0
/* set the default PRAM value to at least PNVRAM + PHRAM when pram env variable
 * is not valid yet, which is the case for when u-boot copies itself to RAM */
#define CONFIG_PRAM		((CONFIG_KM_PNVRAM + CONFIG_KM_PHRAM)>>10)

/* architecture specific default bootargs */
#define CONFIG_KM_DEF_BOOT_ARGS_CPU		""

#define CONFIG_KM_DEF_ENV_CPU						\
	"u-boot="CONFIG_HOSTNAME "/u-boot.bin\0"		\
	"update="							\
		"protect off " __stringify(BOOTFLASH_START) " +${filesize} && "\
		"erase " __stringify(BOOTFLASH_START) "  +${filesize} && "\
		"cp.b ${load_addr_r} " __stringify(BOOTFLASH_START)	\
		"  ${filesize} && "					\
		"protect on " __stringify(BOOTFLASH_START) "  +${filesize}\0"\
	"set_fdthigh=true\0"						\
	"checkfdt=true\0"						\
	"bootm_mapsize=" __stringify(CONFIG_SYS_BOOTM_LEN) "\0"		\
	""

#endif /* __CONFIG_KEYMILE_POWERPC_H */
