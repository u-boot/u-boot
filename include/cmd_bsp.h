/*
 * (C) Copyright 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifndef _CMD_BSP_H_
#define _CMD_BSP_H_

#include <common.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_BSP)

/* ----- LWMON ---------------------------------------------------------
 */
#if defined(CONFIG_LWMON)

#define	CMD_TBL_BSP	MK_CMD_TBL_ENTRY(					\
	"pic",	3,	4,	1,	do_pic,					\
	"pic     - read and write PIC registers\n",				\
	"read  reg      - read PIC register `reg'\n"				\
	"pic write reg val  - write value `val' to PIC register `reg'\n"	\
),  MK_CMD_TBL_ENTRY(								\
	"kbd",	3,	1,	1,	do_kbd,					\
	"kbd     - read keyboard status\n",					\
	NULL									\
),  MK_CMD_TBL_ENTRY(								\
	"lsb",	3,	2,	1,	do_lsb,					\
	"lsb     - check and set LSB switch\n",					\
	"on  - switch LSB on\n"							\
	"lsb off - switch LSB off\n"						\
	"lsb     - print current setting\n"					\
),
int do_pic (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_kbd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_lsb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif	/* CONFIG_LWMON */
/* --------------------------------------------------------------------	*/

/* ----- PCU E -------------------------------------------------------- */
#if defined(CONFIG_PCU_E)

#define CMD_TBL_BSP	MK_CMD_TBL_ENTRY(					\
	"puma",	4,	4,	1,	do_puma,				\
	"puma    - access PUMA FPGA\n",						\
	"status - print PUMA status\n"						\
	"puma load addr len - load PUMA configuration data\n"			\
),
int do_puma (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif	/* CONFIG_PCU_E */
/* --------------------------------------------------------------------	*/

/* ----- CCM/SCM ------------------------------------------------------ */
#if defined(CONFIG_CCM) || defined(CONFIG_SCM)

#define CMD_TBL_BSP	MK_CMD_TBL_ENTRY(					\
	"fpga",	4,	4,	1,	do_fpga,				\
	"fpga    - access FPGA(s)\n",						\
	"fpga status [name] - print FPGA status\n"				\
	"fpga reset  [name] - reset FPGA\n"					\
	"fpga load [name] addr - load FPGA configuration data\n"		\
),
int do_fpga (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif	/* CONFIG_CCM, CONFIG_SCM */
/* --------------------------------------------------------------------	*/

/* ----- PIP405 ------------------------------------------------------- */
#if defined(CONFIG_PIP405)

#define	CMD_TBL_BSP MK_CMD_TBL_ENTRY(				\
	"pip405",	4,	6,	1,	do_pip405,			\
	"pip405  - PIP405 specific Cmds\n",					\
	"flash mem [SrcAddr] - updates U-Boot with image in memory\n"					\
	"pip405 flash floppy [SrcAddr] - updates U-Boot with image from floppy\n"					\
	"pip405 flash mps - updates U-Boot with image from MPS\n"					\
),
int do_pip405 (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif /* CONFIG_PIP405 */
/* --------------------------------------------------------------------	*/

/* ----- MIP405 ------------------------------------------------------- */
#if defined(CONFIG_MIP405)

#define	CMD_TBL_BSP MK_CMD_TBL_ENTRY(				\
	"mip405",	4,	6,	1,	do_mip405,			\
	"mip405  - MIP405 specific Cmds\n",					\
	"flash mem [SrcAddr] - updates U-Boot with image in memory\n"					\
	"mip405 flash mps - updates U-Boot with image from MPS\n"					\
),
int do_mip405 (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif /* CONFIG_MIP405 */
/* ----- VCMA9 -----------------------------------------------------------------
 */
#if defined(CONFIG_VCMA9)

#define	CMD_TBL_BSP MK_CMD_TBL_ENTRY(				\
	"vcma9",	4,	6,	1,	do_vcma9,	\
	"vcma9   - VCMA9 specific Cmds\n",			\
	"flash mem [SrcAddr] - updates U-Boot with image in memory\n"					\
),
int do_vcma9 (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif /* CONFIG_VCMA9 */
/* ----------------------------------------------------------------------------*/

/* ----- DASA_SIM ----------------------------------------------------- */
#if defined(CONFIG_DASA_SIM)

#define	CMD_TBL_BSP MK_CMD_TBL_ENTRY(				                \
	"pci9054",	7,	3,	1,	do_pci9054,			\
	"pci9054 - PLX PCI9054 EEPROM access\n",				\
	"pci9054 info - print EEPROM values\n"		                        \
	"pci9054 update - updates EEPROM with default values\n"			\
),
int do_pci9054 (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif /* CONFIG_DASA_SIM */
/* --------------------------------------------------------------------	*/

/* ----- HYMOD -------------------------------------------------------- */
#if defined(CONFIG_HYMOD)

#define	CMD_TBL_BSP	MK_CMD_TBL_ENTRY(				\
	"fpga",	4,	6,	1,	do_fpga,			\
	"fpga    - FPGA sub-system\n",					\
	"load [type] addr size\n"					\
	"  - write the configuration data at memory address `addr',\n"	\
	"    size `size' bytes, into the FPGA of type `type' (either\n"	\
	"    `main' or `mezz', default `main'). e.g.\n"			\
	"        `fpga load 100000 7d8f'\n"				\
	"    loads the main FPGA with config data at address 100000\n"	\
	"    HEX, size 7d8f HEX (32143 DEC) bytes\n"			\
	"fpga tftp file addr\n"						\
	"  - transfers `file' from the tftp server into memory at\n"	\
	"    address `addr', then writes the entire file contents\n"	\
	"    into the main FPGA\n"					\
	"fpga store addr\n"						\
	"  - read configuration data from the main FPGA (the mezz\n"	\
	"    FPGA is write-only), into address `addr'. There must be\n"	\
	"    enough memory available at `addr' to hold all the config\n"\
	"    data - the size of which is determined by VC:???\n"	\
	"fpga info\n"							\
	"  - print information about the Hymod FPGA, namely the\n"	\
	"    memory addresses at which the four FPGA local bus\n"	\
	"    address spaces appear in the physical address space\n"	\
), MK_CMD_TBL_ENTRY(							\
	"eeclear", 4,	1,	0,	do_eecl,			\
	"eeclear - Clear the eeprom on a Hymod board\n",		\
	"[type]\n"							\
	"  - write zeroes into the EEPROM on the board of type `type'\n"\
	"    (`type' is either `main' or `mezz' - default `main')\n"	\
	"    Note: the EEPROM write enable jumper must be installed\n"	\
), MK_CMD_TBL_ENTRY(							\
	"htest", 5,	1,	0,	do_htest,			\
	"htest   - run HYMOD tests\n",					\
	NULL								\
),

int do_fpga (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_eecl (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_htest(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif	/* CONFIG_HYMOD */
/* --------------------------------------------------------------------	*/

/* ----- CRAY405 (L1) ------------------------------------------------- */
#if defined (CONFIG_CRAYL1)
#define	CMD_TBL_BSP MK_CMD_TBL_ENTRY(						\
	"L1cmd",	5,	4,	1,	do_crayL1,			\
	"L1cmd  - L1 update, setup, commands \n",				\
	"L1cmd update - update flash images from host\n"			\
	"L1cmd boot - nfs or ramboot L1\n"					\
),
int do_crayL1 (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif /* CONFIG_CRAY405 */
/* --------------------------------------------------------------------	*/

/* ----- EVB64260 ----------------------------------------------------- */
#if defined (CONFIG_EVB64260)
#ifdef CONFIG_ZUMA_V2
#define CMD_TBL_BSP  ZUMA_TBL_ENTRY

#define ZUMA_TBL_ENTRY	MK_CMD_TBL_ENTRY(				\
	"zinit",	 5,	 1,	 0,	 do_zuma_init_pbb,	\
	"zinit   - init zuma pbb\n",					\
	"\n"								\
	"    - init zuma pbb\n"						\
), MK_CMD_TBL_ENTRY(							\
	"zdtest",	  6,	  3,	  1,	  do_zuma_test_dma,	\
	"zdtest  - run dma test\n",					\
	"[cmd [count]]\n"						\
	"    - run dma cmd (w=0,v=1,cp=2,cmp=3,wi=4,vi=5), count bytes\n" \
), MK_CMD_TBL_ENTRY(							\
	"zminit",	  5,	  1,	  0,	  do_zuma_init_mbox,	\
	"zminit  - init zuma mbox\n",				\
	"\n"								\
	"    - init zuma mbox\n"					\
),

int do_zuma_init_pbb  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_zuma_test_dma  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_zuma_init_mbox (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

void zuma_init_pbb(void);
int zuma_mbox_init(void);
int zuma_test_dma(int cmd, int size);
#else
#define CMD_TBL_BSP
#endif /* ZUMA_NTL */

#endif /* CONFIG_EVB64260 */
/* --------------------------------------------------------------------	*/

/* -----W7O------------------------------------------------------------ */
#if defined(CONFIG_W7O)

#define CMD_TBL_BSP MK_CMD_TBL_ENTRY(			\
	  "vpd",	3,	2,	1,	do_vpd, \
	  "vpd     - Read Vital Product Data\n",	\
	  "[dev_addr]\n"				\
	  "        - Read VPD Data from default address, or device address 'dev_addr'.\n" \
),

extern int do_vpd (cmd_tbl_t *, int, int, char *[]);

#endif	/* CONFIG_W7O */
/* --------------------------------------------------------------------	*/

/* ---- PCIPPC2 / PCIPPC6 --------------------------------------------- */
#if defined(CONFIG_PCIPPC2) || defined(CONFIG_PCIPPC6)
#if defined(CONFIG_WATCHDOG)

#define CMD_TBL_BSP MK_CMD_TBL_ENTRY(			\
	"wd",	3,	2,	1,	do_wd,		\
	"wd      - check and set watchdog\n",		\
	"on   - switch watchDog on\n"			\
	"wd off  - switch watchdog off\n"		\
	"wd      - print current status\n"		\
),

extern int do_wd (cmd_tbl_t *, int, int, char *[]);

#else
#define CMD_TBL_BSP
#endif  /* CONFIG_WATCHDOG */

#endif	/* CONFIG_PCIPPC2 , CONFIG_PCIPPC6 */
/* --------------------------------------------------------------------	*/

/* ----- PN62 --------------------------------------------------------- */
#if defined(CONFIG_PN62)

#define CMD_TBL_BSP MK_CMD_TBL_ENTRY(				\
	"loadpci",	5,	2,	1,	do_loadpci, 	\
	"loadpci - load binary file over PCI\n",		\
	"[addr]\n"						\
	"    - load binary file over PCI to address 'addr'\n"	\
), MK_CMD_TBL_ENTRY( 	 	 	 	 	 	\
	"led"    ,	3,	3,	1,	do_led, 	\
	"led     - set LED 0..11 on the PN62 board\n",		\
	"i fun\n"						\
	"    - set 'i'th LED to function 'fun'\n"		\
),

extern int do_loadpci (cmd_tbl_t *, int, int, char *[]);
extern int do_led (cmd_tbl_t *, int, int, char *[]);
#endif /* CONFIG_PN62 */
/* --------------------------------------------------------------------	*/

/* ----- TRAB --------------------------------------------------------- */
#if defined(CONFIG_TRAB)

#define	CMD_TBL_BSP	MK_CMD_TBL_ENTRY(			\
	"kbd",	3,	1,	1,	do_kbd,			\
	"kbd     - read keyboard status\n",			\
	NULL							\
),

int do_kbd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif	/* CONFIG_TRAB */
/* --------------------------------------------------------------------	*/

/* ----- R360MPI ------------------------------------------------------ */
#if defined(CONFIG_R360MPI)

#define	CMD_TBL_BSP	MK_CMD_TBL_ENTRY(			\
	"kbd",	3,	1,	1,	do_kbd,			\
	"kbd     - read keyboard status\n",			\
	NULL							\
),

int do_kbd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif	/* CONFIG_R360MPI */
/* --------------------------------------------------------------------	*/

/* ------ AMIGAONEG3SE ------------------------------------------------ */
#if defined(CONFIG_AMIGAONEG3SE)

#define CMD_TBL_BSP	/* dummy */

#endif  /* AmigaOneG3SE */
/* ----- PCI405 ------------------------------------------------------- */
#if defined(CONFIG_PCI405)

#define	CMD_TBL_BSP	MK_CMD_TBL_ENTRY(			\
	"loadpci",	7,	1,	1,	do_loadpci,	\
	"loadpci - wait for sync and boot image\n",		\
	NULL							\
),

int do_loadpci (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif	/* CONFIG_PCI405 */
/* -------------------------------------------------------------------- */

#else
#define CMD_TBL_BSP
#endif	/* CFG_CMD_BSP */

#endif	/* _CMD_BSP_H_ */
