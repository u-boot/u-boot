/*
 * (C) Copyright 2000
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

/*
 *  Command Processor Table
 */

#include <common.h>
#include <command.h>
#include <cmd_cache.h>
#include <cmd_mem.h>
#include <cmd_boot.h>
#include <cmd_flash.h>
#include <cmd_bootm.h>
#include <cmd_net.h>
#include <cmd_nvedit.h>
#include <cmd_misc.h>
#include <cmd_kgdb.h>
#include <cmd_ide.h>
#include <cmd_disk.h>
#include <cmd_console.h>
#include <cmd_reginfo.h>
#include <cmd_pcmcia.h>
#include <cmd_autoscript.h>
#include <cmd_diag.h>

#include <cmd_eeprom.h>
#include <cmd_i2c.h>
#include <cmd_spi.h>
#include <cmd_immap.h>
#include <cmd_rtc.h>

#include <cmd_elf.h>
#include <cmd_fdc.h>		/* Floppy support */
#include <cmd_usb.h>		/* USB support */
#include <cmd_scsi.h>
#include <cmd_pci.h>
#include <cmd_mii.h>
#include <cmd_dcr.h>		/* 4xx DCR register access */
#include <cmd_doc.h>
#include <cmd_jffs2.h>
#include <cmd_fpga.h>

#include <cmd_bsp.h>		/* board special functions */

#include <cmd_bedbug.h>
#include <cmd_elf.h>

#include <cmd_dtt.h>

#include <cmd_vfd.h>		/* load a bitmap to the VFDs on TRAB */
#include <cmd_log.h>
#include <cmd_fdos.h>

#ifdef CONFIG_AMIGAONEG3SE
#include <cmd_menu.h>
#include <cmd_boota.h>
#endif

/*
 * HELP command
 */
#define	CMD_TBL_HELP	MK_CMD_TBL_ENTRY(					\
	"help",		1,	CFG_MAXARGS,	1,	do_help,		\
	"help    - print online help\n",					\
	"[command ...]\n"							\
	"    - show help information (for 'command')\n"				\
	"'help' prints online help for the monitor commands.\n\n"		\
	"Without arguments, it prints a short usage message for all commands.\n\n" \
	"To get detailed help information for specific commands you can type\n"	\
	"'help' with one or more command names as arguments.\n"			\
    ),

#define	CMD_TBL_QUES	MK_CMD_TBL_ENTRY(					\
	"?",		1,	CFG_MAXARGS,	1,	do_help,		\
	"?       - alias for 'help'\n",						\
	NULL									\
    ),

#define CMD_TBL_VERS	MK_CMD_TBL_ENTRY(					\
	"version",	4,	1,		1,	do_version,		\
	"version - print monitor version\n",					\
	NULL									\
    ),

#define CMD_TBL_ECHO	MK_CMD_TBL_ENTRY(					\
	"echo",		4,	CFG_MAXARGS,	1,	do_echo,		\
	"echo    - echo args to console\n",					\
	"[args..]\n"								\
	"    - echo args to console; \\c suppresses newline\n"			\
    ),

int
do_version (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	extern char version_string[];
	printf ("\n%s\n", version_string);
	return 0;
}

int
do_echo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i, putnl = 1;

	for (i = 1; i < argc; i++) {
		char *p = argv[i], c;

		if (i > 1)
			putc(' ');
		while ((c = *p++) != '\0')
			if (c == '\\' && *p == 'c') {
				putnl = 0;
				p++;
			}
			else
				putc(c);
	}

	if (putnl)
		putc('\n');
	return 0;
}

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */
int
do_help (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	int rcode = 0;

	if (argc == 1) {	/* print short help (usage) */

		for (cmdtp=&cmd_tbl[0]; cmdtp->name; cmdtp++) {
			/* allow user abort */
			if (ctrlc())
				return 1;

			if (cmdtp->usage == NULL)
				continue;
			puts (cmdtp->usage);
		}

		return 0;
	}

	/*
	 * command help (long version)
	 */
	for (i=1; i<argc; ++i) {
		if ((cmdtp = find_cmd(argv[i])) != NULL) {
#ifdef	CFG_LONGHELP
			/* found - print (long) help info */
			puts (cmdtp->name);
			putc (' ');
			if (cmdtp->help) {
				puts (cmdtp->help);
			} else {
				puts ("- No help available.\n");
				rcode = 1;
			}
			putc ('\n');
#else	/* no long help available */
			if (cmdtp->usage)
				puts (cmdtp->usage);
#endif	/* CFG_LONGHELP */
		}
		else {
			printf ("Unknown command '%s' - try 'help'"
				" without arguments for list of all"
				" known commands\n\n",
				argv[i]
			);
			rcode = 1;
		}
	}
	return rcode;
}

/***************************************************************************
 * find command table entry for a command
 */
cmd_tbl_t *find_cmd(const char *cmd)
{
	cmd_tbl_t *cmdtp;

	/* Search command table - Use linear search - it's a small table */
	for (cmdtp = &cmd_tbl[0]; cmdtp->name; cmdtp++) {
		if (strncmp (cmd, cmdtp->name, cmdtp->lmin) == 0)
			return cmdtp;
	}
	return NULL;	/* not found */
}

/*
 * The commands in this table are sorted alphabetically by the
 * command name and in descending order by the command name string
 * length. This is to prevent conflicts in command name parsing.
 * Please ensure that new commands are added according to that rule.
 * Please use $(TOPDIR)/doc/README.commands as a reference AND make
 * sure it gets updated.
 */

cmd_tbl_t cmd_tbl[] = {
	CMD_TBL_ASKENV
	CMD_TBL_ASM
	CMD_TBL_AUTOSCRIPT
	CMD_TBL_BASE
	CMD_TBL_BDINFO
#ifdef CONFIG_AMIGAONEG3SE
	CMD_TBL_BOOTA
#endif
	CMD_TBL_BOOTELF
	CMD_TBL_BOOTM
	CMD_TBL_BOOTP
	CMD_TBL_BOOTVX
	CMD_TBL_BOOTD
	CMD_TBL_BREAK
	CMD_TBL_BRGINFO
	CMD_TBL_CARINFO
	CMD_TBL_JFFS2_CHPART
	CMD_TBL_CMP
	CMD_TBL_CONINFO
	CMD_TBL_CONTINUE
	CMD_TBL_CP
	CMD_TBL_CRC
	CMD_TBL_DATE
	CMD_TBL_DCACHE
	CMD_TBL_DHCP
	CMD_TBL_DIAG
	CMD_TBL_DISK
	CMD_TBL_DMAINFO
	CMD_TBL_DIS
	CMD_TBL_DOCBOOT
	CMD_TBL_DOC
	CMD_TBL_DTT
	CMD_TBL_ECHO
	CMD_TBL_EEPROM
	CMD_TBL_FCCINFO
	CMD_TBL_FLERASE
	CMD_TBL_FDC
        CMD_TBL_FDOS_BOOT
        CMD_TBL_FDOS_LS
	CMD_TBL_FLINFO
	CMD_TBL_FPGA
	CMD_TBL_JFFS2_FSINFO
	CMD_TBL_JFFS2_FSLOAD
	CMD_TBL_GETDCR
	CMD_TBL_GO
	CMD_TBL_HELP
	CMD_TBL_HWFLOW
	CMD_TBL_I2CINFO
	CMD_TBL_ICACHE
#ifdef CONFIG_8260
	CMD_TBL_ICINFO
#endif
	CMD_TBL_IMD
	CMD_TBL_IMM
	CMD_TBL_INM
	CMD_TBL_IMW
	CMD_TBL_ICRC
	CMD_TBL_IPROBE
	CMD_TBL_ILOOP
	CMD_TBL_ISDRAM
	CMD_TBL_IDE
	CMD_TBL_IMINFO
	CMD_TBL_IOPINFO
	CMD_TBL_IOPSET
	CMD_TBL_IRQINFO
	CMD_TBL_KGDB
	CMD_TBL_LOADB
	CMD_TBL_LOADS
	CMD_TBL_LOG
	CMD_TBL_LOOP
	CMD_TBL_JFFS2_LS
	CMD_TBL_MCCINFO
	CMD_TBL_MD
	CMD_TBL_MEMCINFO
#ifdef CONFIG_AMIGAONEG3SE
	CMD_TBL_MENU
#endif
	CMD_TBL_MII
	CMD_TBL_MM
	CMD_TBL_MTEST
	CMD_TBL_MUXINFO
	CMD_TBL_MW
	CMD_TBL_NEXT
	CMD_TBL_NM
	CMD_TBL_PCI
	CMD_TBL_PRINTENV
	CMD_TBL_PROTECT
	CMD_TBL_RARPB
	CMD_TBL_RDUMP
	CMD_TBL_PINIT
	CMD_TBL_REGINFO
	CMD_TBL_RESET
	CMD_TBL_RUN
	CMD_TBL_SAVEENV
	CMD_TBL_SAVES
	CMD_TBL_SCCINFO
	CMD_TBL_SCSIBOOT
	CMD_TBL_SCSI
	CMD_TBL_SETDCR
	CMD_TBL_SETENV
	CMD_TBL_SIINFO
	CMD_TBL_SITINFO
	CMD_TBL_SIUINFO
	CMD_TBL_MISC		/* sleep */
	CMD_TBL_SMCINFO
	CMD_TBL_SPIINFO
	CMD_TBL_SPI
	CMD_TBL_STACK
	CMD_TBL_STEP
	CMD_TBL_TFTPB
	CMD_TBL_USBBOOT
	CMD_TBL_USB
	CMD_TBL_VERS
	CMD_TBL_BSP
	CMD_TBL_VFD
	CMD_TBL_QUES		/* keep this ("help") the last entry */
	/* the following entry terminates this table */
	MK_CMD_TBL_ENTRY( NULL, 0, 0, 0, NULL, NULL, NULL )
};
