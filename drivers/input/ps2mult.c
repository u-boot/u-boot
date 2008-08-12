/***********************************************************************
 *
 * (C) Copyright 2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 * All rights reserved.
 *
 * PS/2 multiplexer driver
 *
 * Originally from linux source (drivers/char/ps2mult.c)
 *
 * Uses simple serial driver (ps2ser.c) to access the multiplexer
 * Used by PS/2 keyboard driver (pc_keyb.c)
 *
 ***********************************************************************/

#include <common.h>

#include <pc_keyb.h>
#include <asm/atomic.h>
#include <ps2mult.h>

/* #define DEBUG_MULT */
/* #define DEBUG_KEYB */

#define KBD_STAT_DEFAULT		(KBD_STAT_SELFTEST | KBD_STAT_UNLOCKED)

#define PRINTF(format, args...)		printf("ps2mult.c: " format, ## args)

#ifdef DEBUG_MULT
#define PRINTF_MULT(format, args...)	printf("PS2MULT: " format, ## args)
#else
#define PRINTF_MULT(format, args...)
#endif

#ifdef DEBUG_KEYB
#define PRINTF_KEYB(format, args...)	printf("KEYB: " format, ## args)
#else
#define PRINTF_KEYB(format, args...)
#endif


static ulong start_time;
static int init_done = 0;

static int received_escape = 0;
static int received_bsync = 0;
static int received_selector = 0;

static int kbd_command_active = 0;
static int mouse_command_active = 0;
static int ctl_command_active = 0;

static u_char command_byte = 0;

static void (*keyb_handler)(void *dev_id);

static u_char ps2mult_buf [PS2BUF_SIZE];
static atomic_t ps2mult_buf_cnt;
static int ps2mult_buf_in_idx;
static int ps2mult_buf_out_idx;

static u_char ps2mult_buf_status [PS2BUF_SIZE];

#ifndef CONFIG_BOARD_EARLY_INIT_R
#error #define CONFIG_BOARD_EARLY_INIT_R and call ps2mult_early_init() in board_early_init_r()
#endif
void ps2mult_early_init (void)
{
	start_time = get_timer(0);
}

static void ps2mult_send_byte(u_char byte, u_char sel)
{
	ps2ser_putc(sel);

	if (sel == PS2MULT_KB_SELECTOR) {
		PRINTF_MULT("0x%02x send KEYBOARD\n", byte);
		kbd_command_active = 1;
	} else {
		PRINTF_MULT("0x%02x send MOUSE\n", byte);
		mouse_command_active = 1;
	}

	switch (byte) {
	case PS2MULT_ESCAPE:
	case PS2MULT_BSYNC:
	case PS2MULT_KB_SELECTOR:
	case PS2MULT_MS_SELECTOR:
	case PS2MULT_SESSION_START:
	case PS2MULT_SESSION_END:
		ps2ser_putc(PS2MULT_ESCAPE);
		break;
	default:
		break;
	}

	ps2ser_putc(byte);
}

static void ps2mult_receive_byte(u_char byte, u_char sel)
{
	u_char status = KBD_STAT_DEFAULT;

#if 1 /* Ignore mouse in U-Boot */
	if (sel == PS2MULT_MS_SELECTOR) return;
#endif

	if (sel == PS2MULT_KB_SELECTOR) {
		if (kbd_command_active) {
			if (!received_bsync) {
				PRINTF_MULT("0x%02x lost KEYBOARD !!!\n", byte);
				return;
			} else {
				kbd_command_active = 0;
				received_bsync = 0;
			}
		}
		PRINTF_MULT("0x%02x receive KEYBOARD\n", byte);
		status |= KBD_STAT_IBF | KBD_STAT_OBF;
	} else {
		if (mouse_command_active) {
			if (!received_bsync) {
				PRINTF_MULT("0x%02x lost MOUSE !!!\n", byte);
				return;
			} else {
				mouse_command_active = 0;
				received_bsync = 0;
			}
		}
		PRINTF_MULT("0x%02x receive MOUSE\n", byte);
		status |= KBD_STAT_IBF | KBD_STAT_OBF | KBD_STAT_MOUSE_OBF;
	}

	if (atomic_read(&ps2mult_buf_cnt) < PS2BUF_SIZE) {
		ps2mult_buf_status[ps2mult_buf_in_idx] = status;
		ps2mult_buf[ps2mult_buf_in_idx++] = byte;
		ps2mult_buf_in_idx &= (PS2BUF_SIZE - 1);
		atomic_inc(&ps2mult_buf_cnt);
	} else {
		PRINTF("buffer overflow\n");
	}

	if (received_bsync) {
		PRINTF("unexpected BSYNC\n");
		received_bsync = 0;
	}
}

void ps2mult_callback (int in_cnt)
{
	int i;
	u_char byte;
	static int keyb_handler_active = 0;

	if (!init_done) {
		return;
	}

	for (i = 0; i < in_cnt; i ++) {
		byte = ps2ser_getc();

		if (received_escape) {
			ps2mult_receive_byte(byte, received_selector);
			received_escape = 0;
		} else switch (byte) {
		case PS2MULT_ESCAPE:
			PRINTF_MULT("ESCAPE receive\n");
			received_escape = 1;
			break;

		case PS2MULT_BSYNC:
			PRINTF_MULT("BSYNC receive\n");
			received_bsync = 1;
			break;

		case PS2MULT_KB_SELECTOR:
		case PS2MULT_MS_SELECTOR:
			PRINTF_MULT("%s receive\n",
			    byte == PS2MULT_KB_SELECTOR ? "KB_SEL" : "MS_SEL");
			received_selector = byte;
			break;

		case PS2MULT_SESSION_START:
		case PS2MULT_SESSION_END:
			PRINTF_MULT("%s receive\n",
			    byte == PS2MULT_SESSION_START ?
			    "SESSION_START" : "SESSION_END");
			break;

		default:
			ps2mult_receive_byte(byte, received_selector);
		}
	}

	if (keyb_handler && !keyb_handler_active &&
	    atomic_read(&ps2mult_buf_cnt)) {
		keyb_handler_active = 1;
		keyb_handler(NULL);
		keyb_handler_active = 0;
	}
}

u_char ps2mult_read_status(void)
{
	u_char byte;

	if (atomic_read(&ps2mult_buf_cnt) == 0) {
		ps2ser_check();
	}

	if (atomic_read(&ps2mult_buf_cnt)) {
		byte = ps2mult_buf_status[ps2mult_buf_out_idx];
	} else {
		byte = KBD_STAT_DEFAULT;
	}
	PRINTF_KEYB("read_status()=0x%02x\n", byte);
	return byte;
}

u_char ps2mult_read_input(void)
{
	u_char byte = 0;

	if (atomic_read(&ps2mult_buf_cnt) == 0) {
		ps2ser_check();
	}

	if (atomic_read(&ps2mult_buf_cnt)) {
		byte = ps2mult_buf[ps2mult_buf_out_idx++];
		ps2mult_buf_out_idx &= (PS2BUF_SIZE - 1);
		atomic_dec(&ps2mult_buf_cnt);
	}
	PRINTF_KEYB("read_input()=0x%02x\n", byte);
	return byte;
}

void ps2mult_write_output(u_char val)
{
	int i;

	PRINTF_KEYB("write_output(0x%02x)\n", val);

	for (i = 0; i < KBD_TIMEOUT; i++) {
		if (!kbd_command_active && !mouse_command_active) {
			break;
		}
		udelay(1000);
		ps2ser_check();
	}

	if (kbd_command_active) {
		PRINTF("keyboard command not acknoledged\n");
		kbd_command_active = 0;
	}

	if (mouse_command_active) {
		PRINTF("mouse command not acknoledged\n");
		mouse_command_active = 0;
	}

	if (ctl_command_active) {
		switch (ctl_command_active) {
		case KBD_CCMD_WRITE_MODE:
			  /* Scan code conversion not supported */
			command_byte = val & ~KBD_MODE_KCC;
			break;

		case KBD_CCMD_WRITE_AUX_OBUF:
			ps2mult_receive_byte(val, PS2MULT_MS_SELECTOR);
			break;

		case KBD_CCMD_WRITE_MOUSE:
			ps2mult_send_byte(val, PS2MULT_MS_SELECTOR);
			break;

		default:
			PRINTF("invalid controller command\n");
			break;
		}

		ctl_command_active = 0;
		return;
	}

	ps2mult_send_byte(val, PS2MULT_KB_SELECTOR);
}

void ps2mult_write_command(u_char val)
{
	ctl_command_active = 0;

	PRINTF_KEYB("write_command(0x%02x)\n", val);

	switch (val) {
	case KBD_CCMD_READ_MODE:
		ps2mult_receive_byte(command_byte, PS2MULT_KB_SELECTOR);
		break;

	case KBD_CCMD_WRITE_MODE:
		ctl_command_active = val;
		break;

	case KBD_CCMD_MOUSE_DISABLE:
		break;

	case KBD_CCMD_MOUSE_ENABLE:
		break;

	case KBD_CCMD_SELF_TEST:
		ps2mult_receive_byte(0x55, PS2MULT_KB_SELECTOR);
		break;

	case KBD_CCMD_KBD_TEST:
		ps2mult_receive_byte(0x00, PS2MULT_KB_SELECTOR);
		break;

	case KBD_CCMD_KBD_DISABLE:
		break;

	case KBD_CCMD_KBD_ENABLE:
		break;

	case KBD_CCMD_WRITE_AUX_OBUF:
		ctl_command_active = val;
		break;

	case KBD_CCMD_WRITE_MOUSE:
		ctl_command_active = val;
		break;

	default:
		PRINTF("invalid controller command\n");
		break;
	}
}

static int ps2mult_getc_w (void)
{
	int res = -1;
	int i;

	for (i = 0; i < KBD_TIMEOUT; i++) {
		if (ps2ser_check()) {
			res = ps2ser_getc();
			break;
		}
		udelay(1000);
	}

	switch (res) {
	case PS2MULT_KB_SELECTOR:
	case PS2MULT_MS_SELECTOR:
		received_selector = res;
		break;
	default:
		break;
	}

	return res;
}

int ps2mult_init (void)
{
	int byte;
	int kbd_found = 0;
	int mouse_found = 0;

	while (get_timer(start_time) < CONFIG_PS2MULT_DELAY);

	ps2ser_init();

	ps2ser_putc(PS2MULT_SESSION_START);

	ps2ser_putc(PS2MULT_KB_SELECTOR);
	ps2ser_putc(KBD_CMD_RESET);

	do {
		byte = ps2mult_getc_w();
	} while (byte >= 0 && byte != KBD_REPLY_ACK);

	if (byte == KBD_REPLY_ACK) {
		byte = ps2mult_getc_w();
		if (byte == 0xaa) {
			kbd_found = 1;
			puts("keyboard");
		}
	}

	if (!kbd_found) {
		while (byte >= 0) {
			byte = ps2mult_getc_w();
		}
	}

#if 1 /* detect mouse */
	ps2ser_putc(PS2MULT_MS_SELECTOR);
	ps2ser_putc(AUX_RESET);

	do {
		byte = ps2mult_getc_w();
	} while (byte >= 0 && byte != AUX_ACK);

	if (byte == AUX_ACK) {
		byte = ps2mult_getc_w();
		if (byte == 0xaa) {
			byte = ps2mult_getc_w();
			if (byte == 0x00) {
				mouse_found = 1;
				puts(", mouse");
			}
		}
	}

	if (!mouse_found) {
		while (byte >= 0) {
			byte = ps2mult_getc_w();
		}
	}
#endif

	if (mouse_found || kbd_found) {
		if (!received_selector) {
			if (mouse_found) {
				received_selector = PS2MULT_MS_SELECTOR;
			} else {
				received_selector = PS2MULT_KB_SELECTOR;
			}
		}

		init_done = 1;
	} else {
		puts("No device found");
	}

	puts("\n");

#if 0 /* for testing */
	{
		int i;
		u_char key[] = {
			0x1f, 0x12, 0x14, 0x12, 0x31, 0x2f, 0x39,	/* setenv */
			0x1f, 0x14, 0x20, 0x17, 0x31, 0x39,		/* stdin */
			0x1f, 0x12, 0x13, 0x17, 0x1e, 0x26, 0x1c,	/* serial */
		};

		for (i = 0; i < sizeof (key); i++) {
			ps2mult_receive_byte (key[i],	     PS2MULT_KB_SELECTOR);
			ps2mult_receive_byte (key[i] | 0x80, PS2MULT_KB_SELECTOR);
		}
	}
#endif

	return init_done ? 0 : -1;
}

int ps2mult_request_irq(void (*handler)(void *))
{
	keyb_handler = handler;

	return 0;
}
