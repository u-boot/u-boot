/*
 * (C) Copyright 2009
 * Werner Pfister <Pfister_Werner@intercontrol.de>
 *
 * (C) Copyright 2009 Semihalf, Grzegorz Bernacki
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
#include <mpc5xxx.h>
#include "spi.h"
#include "cmd_mtc.h"

DECLARE_GLOBAL_DATA_PTR;

static const char *led_names[] = {
	"diag",
	"can1",
	"can2",
	"can3",
	"can4",
	"usbpwr",
	"usbbusy",
	"user1",
	"user2",
	""
};

static int msp430_xfer(const void *dout, void *din)
{
	int err;

	err = spi_xfer(NULL, MTC_TRANSFER_SIZE, dout, din,
		       SPI_XFER_BEGIN | SPI_XFER_END);

	/* The MSP chip needs time to ready itself for the next command */
	udelay(1000);

	return err;
}

static void mtc_calculate_checksum(tx_msp_cmd *packet)
{
	int i;
	uchar *buff;

	buff = (uchar *) packet;

	for (i = 0; i < 6; i++)
		packet->cks += buff[i];
}

static int do_mtc_led(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	tx_msp_cmd pcmd;
	rx_msp_cmd prx;
	int err;
	int i;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return -1;
	}

	memset(&pcmd, 0, sizeof(pcmd));
	memset(&prx, 0, sizeof(prx));

	pcmd.cmd = CMD_SET_LED;

	pcmd.cmd_val0 = 0xff;
	for (i = 0; strlen(led_names[i]) != 0; i++) {
		if (strncmp(argv[1], led_names[i], strlen(led_names[i])) == 0) {
			pcmd.cmd_val0 = i;
			break;
		}
	}

	if (pcmd.cmd_val0 == 0xff) {
		printf("Usage:\n%s\n", cmdtp->help);
		return -1;
	}

	if (argc >= 3) {
		if (strncmp(argv[2], "red", 3) == 0)
			pcmd.cmd_val1 = 1;
		else if (strncmp(argv[2], "green", 5) == 0)
			pcmd.cmd_val1 = 2;
		else if (strncmp(argv[2], "orange", 6) == 0)
			pcmd.cmd_val1 = 3;
		else
			pcmd.cmd_val1 = 0;
	}

	if (argc >= 4)
		pcmd.cmd_val2 = simple_strtol(argv[3], NULL, 10);
	else
		pcmd.cmd_val2 = 0;

	mtc_calculate_checksum(&pcmd);
	err = msp430_xfer(&pcmd, &prx);

	return err;
}

static int do_mtc_key(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	tx_msp_cmd pcmd;
	rx_msp_cmd prx;
	int err;

	memset(&pcmd, 0, sizeof(pcmd));
	memset(&prx, 0, sizeof(prx));

	pcmd.cmd = CMD_GET_VIM;

	mtc_calculate_checksum(&pcmd);
	err = msp430_xfer(&pcmd, &prx);

	if (!err) {
		/* function returns '0' if key is pressed */
		err = (prx.input & 0x80) ? 0 : 1;
	}

	return err;
}

static int do_mtc_digout(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	tx_msp_cmd pcmd;
	rx_msp_cmd prx;
	int err;
	uchar channel_mask = 0;

	if (argc < 3) {
		cmd_usage(cmdtp);
		return -1;
	}

	if (strncmp(argv[1], "on", 2) == 0)
		channel_mask |= 1;
	if (strncmp(argv[2], "on", 2) == 0)
		channel_mask |= 2;

	memset(&pcmd, 0, sizeof(pcmd));
	memset(&prx, 0, sizeof(prx));

	pcmd.cmd = CMD_GET_VIM;
	pcmd.user_out = channel_mask;

	mtc_calculate_checksum(&pcmd);
	err = msp430_xfer(&pcmd, &prx);

	return err;
}

static int do_mtc_digin(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	tx_msp_cmd pcmd;
	rx_msp_cmd prx;
	int err;
	uchar channel_num = 0;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return -1;
	}

	channel_num = simple_strtol(argv[1], NULL, 10);
	if ((channel_num != 1) && (channel_num != 2)) {
		printf("mtc digin: invalid parameter - must be '1' or '2'\n");
		return -1;
	}

	memset(&pcmd, 0, sizeof(pcmd));
	memset(&prx, 0, sizeof(prx));

	pcmd.cmd = CMD_GET_VIM;

	mtc_calculate_checksum(&pcmd);
	err = msp430_xfer(&pcmd, &prx);

	if (!err) {
		/* function returns '0' when digin is on */
		err = (prx.input & channel_num) ? 0 : 1;
	}

	return err;
}

static int do_mtc_appreg(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	tx_msp_cmd pcmd;
	rx_msp_cmd prx;
	int err;
	char buf[5];

	/* read appreg */
	memset(&pcmd, 0, sizeof(pcmd));
	memset(&prx, 0, sizeof(prx));

	pcmd.cmd = CMD_WD_PARA;
	pcmd.cmd_val0 = 5;	/* max. Count */
	pcmd.cmd_val1 = 5;	/* max. Time */
	pcmd.cmd_val2 = 0;	/* =0 means read appreg */

	mtc_calculate_checksum(&pcmd);
	err = msp430_xfer(&pcmd, &prx);

	if (!err) {
		sprintf(buf, "%d", prx.ack2);
		setenv("appreg", buf);
	}

	return err;
}

static int do_mtc_version(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	tx_msp_cmd pcmd;
	rx_msp_cmd prx;
	int err;

	memset(&pcmd, 0, sizeof(pcmd));
	memset(&prx, 0, sizeof(prx));

	pcmd.cmd = CMD_FW_VERSION;

	mtc_calculate_checksum(&pcmd);
	err = msp430_xfer(&pcmd, &prx);

	if (!err) {
		printf("FW V%d.%d.%d / HW %d\n",
		       prx.ack0, prx.ack1, prx.ack3, prx.ack2);
	}

	return err;
}

static int do_mtc_state(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	tx_msp_cmd pcmd;
	rx_msp_cmd prx;
	int err;

	memset(&pcmd, 0, sizeof(pcmd));
	memset(&prx, 0, sizeof(prx));

	pcmd.cmd = CMD_WD_WDSTATE;
	pcmd.cmd_val2 = 1;

	mtc_calculate_checksum(&pcmd);
	err = msp430_xfer(&pcmd, &prx);

	if (!err) {
		printf("State     %02Xh\n", prx.state);
		printf("Input     %02Xh\n", prx.input);
		printf("UserWD    %02Xh\n", prx.ack2);
		printf("Sys WD    %02Xh\n", prx.ack3);
		printf("WD Timout %02Xh\n", prx.ack0);
		printf("eSysState %02Xh\n", prx.ack1);
	}

	return err;
}

static int do_mtc_help(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

cmd_tbl_t cmd_mtc_sub[] = {
	U_BOOT_CMD_MKENT(led, 3, 1, do_mtc_led,
	"set state of leds",
	"[ledname] [state] [blink]\n"
	" - lednames: diag can1 can2 can3 can4 usbpwr usbbusy user1 user2\n"
	" - state: off red green orange\n"
	" - blink: blink interval in 100ms steps (1 - 10; 0 = static)\n"),
	U_BOOT_CMD_MKENT(key, 0, 1, do_mtc_key,
	"returns state of user key", ""),
	U_BOOT_CMD_MKENT(version, 0, 1, do_mtc_version,
	"returns firmware version of supervisor uC", ""),
	U_BOOT_CMD_MKENT(appreg, 0, 1, do_mtc_appreg,
	"reads appreg value and stores in environment variable 'appreg'", ""),
	U_BOOT_CMD_MKENT(digin, 1, 1, do_mtc_digin,
	"returns state of digital input",
	"<channel_num> - get state of digital input (1 or 2)\n"),
	U_BOOT_CMD_MKENT(digout, 2, 1, do_mtc_digout,
	"sets digital outputs",
	"<on|off> <on|off>- set state of digital output 1 and 2\n"),
	U_BOOT_CMD_MKENT(state, 0, 1, do_mtc_state,
	"displays state", ""),
	U_BOOT_CMD_MKENT(help, 4, 1, do_mtc_help, "get help",
	"[command] - get help for command\n"),
};

static int do_mtc_help(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	extern int _do_help(cmd_tbl_t *cmd_start, int cmd_items,
			    cmd_tbl_t *cmdtp, int flag,
			    int argc, char *argv[]);
#ifdef CONFIG_SYS_LONGHELP
	puts("mtc ");
#endif
	return _do_help(&cmd_mtc_sub[0],
			ARRAY_SIZE(cmd_mtc_sub), cmdtp, flag, argc, argv);
}

/* Relocate the command table function pointers when running in RAM */
int mtc_cmd_init_r(void)
{
	cmd_tbl_t *cmdtp;

	for (cmdtp = &cmd_mtc_sub[0]; cmdtp !=
	     &cmd_mtc_sub[ARRAY_SIZE(cmd_mtc_sub)]; cmdtp++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
		cmdtp->cmd =
		    (int (*)(struct cmd_tbl_s *, int, int, char *[]))addr;

		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (char *)addr;

		if (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (char *)addr;
		}
#ifdef CONFIG_SYS_LONGHELP
		if (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (char *)addr;
		}
#endif
	}
	return 0;
}

int cmd_mtc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	cmd_tbl_t *c;
	int err = 0;

	c = find_cmd_tbl(argv[1], &cmd_mtc_sub[0], ARRAY_SIZE(cmd_mtc_sub));
	if (c) {
		argc--;
		argv++;
		return c->cmd(c, flag, argc, argv);
	} else {
		/* Unrecognized command */
		cmd_usage(cmdtp);
		return 1;
	}

	return err;
}

U_BOOT_CMD(mtc, 5, 1, cmd_mtc,
	"special commands for digsyMTC",
	"[subcommand] [args...]\n"
	"Subcommands list:\n"
	"led [ledname] [state] [blink] - set state of leds\n"
	"  [ledname]: diag can1 can2 can3 can4 usbpwr usbbusy user1 user2\n"
	"  [state]: off red green orange\n"
	"  [blink]: blink interval in 100ms steps (1 - 10; 0 = static)\n"
	"key - returns state of user key\n"
	"version - returns firmware version of supervisor uC\n"
	"appreg - reads appreg value and stores in environment variable"
	" 'appreg'\n"
	"digin [channel] - returns state of digital input (1 or 2)\n"
	"digout <on|off> <on|off> - sets state of two digital outputs\n"
	"state - displays state\n"
	"help [subcommand] - get help for subcommand\n"
);
