/*
 * (C) Copyright 2002
 * Wolfgang Grandegger, DENX Software Engineering, wg@denx.de.
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
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <pci.h>
#include <command.h>
#include "pn62.h"

#if defined(CONFIG_CMD_BSP)

/*
 * Command led: controls the various LEDs 0..11 on the PN62 card.
 */
int do_led(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int number, function;

	if (argc != 3)
		return cmd_usage(cmdtp);

	number = simple_strtoul(argv[1], NULL, 10);
	if (number > PN62_LED_MAX)
		return 1;

	function = simple_strtoul(argv[2], NULL, 16);
	set_led(number, function);
	return 0;
}
U_BOOT_CMD(
	led    ,	3,	1,	do_led,
	"set LED 0..11 on the PN62 board",
	"i fun"
	"    - set 'i'th LED to function 'fun'"
);

/*
 * Command loadpci: loads a image over PCI.
 */
#define CMD_MOVE_WINDOW 0x1
#define CMD_BOOT_IMAGE  0x2

int do_loadpci (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char *s;
    ulong addr = 0, count = 0;
    u32 off;
    int cmd, rcode = 0;

    /* pre-set load_addr */
    if ((s = getenv("loadaddr")) != NULL) {
	addr = simple_strtoul(s, NULL, 16);
    }

    switch (argc) {
    case 1:
	break;
    case 2:
	addr = simple_strtoul(argv[1], NULL, 16);
	break;
    default:
	return cmd_usage(cmdtp);
    }

    printf ("## Ready for image download ...\n");

    show_startup_phase(12);

    while (1) {
	/* Alive indicator */
	i2155x_write_scrapad(BOOT_PROTO, BOOT_PROTO_READY);

	/* Toggle status LEDs */
	cmd = (count / 200) % 4; /* downscale */
	set_led(4, cmd == 0 ? LED_1 : LED_0);
	set_led(5, cmd == 1 ? LED_1 : LED_0);
	set_led(6, cmd == 2 ? LED_1 : LED_0);
	set_led(7, cmd == 3 ? LED_1 : LED_0);
	udelay(1000);
	count++;

	cmd = i2155x_read_scrapad(BOOT_CMD);

	if (cmd == BOOT_CMD_MOVE) {
	    off = i2155x_read_scrapad(BOOT_DATA);
	    off += addr;
	    i2155x_set_bar_base(3, off);
	    printf ("## BAR3 Addr moved = 0x%08x\n", off);
	    i2155x_write_scrapad(BOOT_CMD, ~cmd);
	    show_startup_phase(13);
	}
	else if (cmd == BOOT_CMD_BOOT) {
	    set_led(4, LED_1);
	    set_led(5, LED_1);
	    set_led(6, LED_1);
	    set_led(7, LED_1);

	    i2155x_write_scrapad(BOOT_CMD, ~cmd);
	    show_startup_phase(14);
	    break;
	}

	/* Abort if ctrl-c was pressed */
	if (ctrlc()) {
	    printf("\nAbort\n");
	    return 0;
	}

    }

    /* Repoint to the default shared memory */
    i2155x_set_bar_base(3, PN62_SMEM_DEFAULT);

    load_addr = addr;
    printf ("## Start Addr      = 0x%08lx\n", addr);

    show_startup_phase(15);

    /* Loading ok, check if we should attempt an auto-start */
    if (((s = getenv("autostart")) != NULL) && (strcmp(s,"yes") == 0)) {
	char *local_args[2];
	local_args[0] = argv[0];
	local_args[1] = NULL;

	printf ("Automatic boot of image at addr 0x%08lX ...\n",
		load_addr);
	rcode = do_bootm (cmdtp, 0, 1, local_args);
    }

    return rcode;
}

U_BOOT_CMD(
	loadpci,	2,	1,	do_loadpci,
	"load binary file over PCI",
	"[addr]\n"
	"    - load binary file over PCI to address 'addr'"
);

#endif
