/*
 * (C) Copyright 2005-2008
 * Matthias Fuchs, esd GmbH Germany, matthias.fuchs@esd-electronics.com
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
#if !defined(CONFIG_440)
#include <asm/4xx_pci.h>
#endif

#if defined(CONFIG_CMD_BSP)

extern int do_bootm (cmd_tbl_t *, int, int, char *[]);
extern int do_source (cmd_tbl_t *, int, int, char *[]);

#define ADDRMASK 0xfffff000

/*
 * Command loadpci: wait for signal from host and boot image.
 */
int do_loadpci(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 *ptr = 0;
	int count = 0;
	int count2 = 0;
	char addr[16];
	char str[] = "\\|/-";
	char *local_args[2];
	u32 la, ptm1la;

#if defined(CONFIG_440)
	ptm1la = in32r(PCIX0_PTM1LA);
#else
	ptm1la = in32r(PTM1LA);
#endif
	while(1) {
		/*
		 * Mark sync address
		 */
		ptr = (u32 *)ptm1la;
		memset(ptr, 0, 0x20);

		*ptr = 0xffffffff;
		puts("\nWaiting for action from pci host -");

		/*
		 * Wait for host to write the start address
		 */
		while (*ptr == 0xffffffff) {
			count++;
			if (!(count % 100)) {
				count2++;
				putc(0x08); /* backspace */
				putc(str[count2 % 4]);
			}

			/* Abort if ctrl-c was pressed */
			if (ctrlc()) {
				puts("\nAbort\n");
				return 0;
			}

			udelay(1000);
		}

		printf("\nGot bootcode %08x: ", *ptr);
		la = ptm1la + (*ptr & ADDRMASK);
		sprintf(addr, "%08x", la);

		switch (*ptr & ~ADDRMASK) {
		case 0:
			/*
			 * Boot image via bootm
			 */
			printf("booting image at addr 0x%s ...\n", addr);
			setenv("loadaddr", addr);
			do_bootm(cmdtp, 0, 0, NULL);
			break;

		case 1:
			/*
			 * Boot image via "source" command
			 */
			printf("executing script at addr 0x%s ...\n", addr);
			local_args[0] = addr;
			local_args[1] = NULL;
			do_source(cmdtp, 0, 1, local_args);
			break;

		case 2:
			/*
			 * Call run_cmd
			 */
			printf("running command at addr 0x%s ...\n", addr);
			run_command((char*)la, 0);
			break;

		default:
			printf("unhandled boot method\n");
			break;
		}
	}
}

U_BOOT_CMD(
	loadpci,	1,	1,	do_loadpci,
	"Wait for pci bootcmd and boot it",
	""
);

#endif
