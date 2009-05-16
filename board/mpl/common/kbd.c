/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
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
 *
 *
 * Source partly derived from:
 * linux/drivers/char/pc_keyb.c
 *
 *
 */
#include <common.h>
#include <asm/processor.h>
#include <stdio_dev.h>
#include "isa.h"
#include "kbd.h"


unsigned char kbd_read_status(void);
unsigned char kbd_read_input(void);
void kbd_send_data(unsigned char data);
void disable_8259A_irq(unsigned int irq);
void enable_8259A_irq(unsigned int irq);

/* used only by send_data - set by keyboard_interrupt */


#undef KBG_DEBUG

#ifdef KBG_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#define KBD_STAT_KOBF		0x01
#define KBD_STAT_IBF		0x02
#define KBD_STAT_SYS		0x04
#define KBD_STAT_CD		0x08
#define KBD_STAT_LOCK		0x10
#define KBD_STAT_MOBF		0x20
#define KBD_STAT_TI_OUT		0x40
#define KBD_STAT_PARERR		0x80

#define KBD_INIT_TIMEOUT	1000	/* Timeout in ms for initializing the keyboard */
#define KBC_TIMEOUT		250	/* Timeout in ms for sending to keyboard controller */
#define KBD_TIMEOUT		2000	/* Timeout in ms for keyboard command acknowledge */
/*
 *	Keyboard Controller Commands
 */

#define KBD_CCMD_READ_MODE	0x20	/* Read mode bits */
#define KBD_CCMD_WRITE_MODE	0x60	/* Write mode bits */
#define KBD_CCMD_GET_VERSION	0xA1	/* Get controller version */
#define KBD_CCMD_MOUSE_DISABLE	0xA7	/* Disable mouse interface */
#define KBD_CCMD_MOUSE_ENABLE	0xA8	/* Enable mouse interface */
#define KBD_CCMD_TEST_MOUSE	0xA9	/* Mouse interface test */
#define KBD_CCMD_SELF_TEST	0xAA	/* Controller self test */
#define KBD_CCMD_KBD_TEST	0xAB	/* Keyboard interface test */
#define KBD_CCMD_KBD_DISABLE	0xAD	/* Keyboard interface disable */
#define KBD_CCMD_KBD_ENABLE	0xAE	/* Keyboard interface enable */
#define KBD_CCMD_WRITE_AUX_OBUF	0xD3    /* Write to output buffer as if
					   initiated by the auxiliary device */
#define KBD_CCMD_WRITE_MOUSE	0xD4	/* Write the following byte to the mouse */

/*
 *	Keyboard Commands
 */

#define KBD_CMD_SET_LEDS	0xED	/* Set keyboard leds */
#define KBD_CMD_SET_RATE	0xF3	/* Set typematic rate */
#define KBD_CMD_ENABLE		0xF4	/* Enable scanning */
#define KBD_CMD_DISABLE		0xF5	/* Disable scanning */
#define KBD_CMD_RESET		0xFF	/* Reset */

/*
 *	Keyboard Replies
 */

#define KBD_REPLY_POR		0xAA	/* Power on reset */
#define KBD_REPLY_ACK		0xFA	/* Command ACK */
#define KBD_REPLY_RESEND	0xFE	/* Command NACK, send the cmd again */

/*
 *	Status Register Bits
 */

#define KBD_STAT_OBF		0x01	/* Keyboard output buffer full */
#define KBD_STAT_IBF		0x02	/* Keyboard input buffer full */
#define KBD_STAT_SELFTEST	0x04	/* Self test successful */
#define KBD_STAT_CMD		0x08	/* Last write was a command write (0=data) */
#define KBD_STAT_UNLOCKED	0x10	/* Zero if keyboard locked */
#define KBD_STAT_MOUSE_OBF	0x20	/* Mouse output buffer full */
#define KBD_STAT_GTO		0x40	/* General receive/xmit timeout */
#define KBD_STAT_PERR		0x80	/* Parity error */

#define AUX_STAT_OBF (KBD_STAT_OBF | KBD_STAT_MOUSE_OBF)

/*
 *	Controller Mode Register Bits
 */

#define KBD_MODE_KBD_INT	0x01	/* Keyboard data generate IRQ1 */
#define KBD_MODE_MOUSE_INT	0x02	/* Mouse data generate IRQ12 */
#define KBD_MODE_SYS		0x04	/* The system flag (?) */
#define KBD_MODE_NO_KEYLOCK	0x08	/* The keylock doesn't affect the keyboard if set */
#define KBD_MODE_DISABLE_KBD	0x10	/* Disable keyboard interface */
#define KBD_MODE_DISABLE_MOUSE	0x20	/* Disable mouse interface */
#define KBD_MODE_KCC		0x40	/* Scan code conversion to PC format */
#define KBD_MODE_RFU		0x80


#define KDB_DATA_PORT		0x60
#define KDB_COMMAND_PORT	0x64

#define	LED_SCR			0x01	/* scroll lock led */
#define	LED_CAP			0x04	/* caps lock led */
#define	LED_NUM			0x02	/* num lock led */

#define	KBD_BUFFER_LEN		0x20	/* size of the keyboardbuffer */


static volatile char kbd_buffer[KBD_BUFFER_LEN];
static volatile int in_pointer = 0;
static volatile int out_pointer = 0;


static unsigned char num_lock = 0;
static unsigned char caps_lock = 0;
static unsigned char scroll_lock = 0;
static unsigned char shift = 0;
static unsigned char ctrl = 0;
static unsigned char alt = 0;
static unsigned char e0 = 0;
static unsigned char leds = 0;

#define DEVNAME "kbd"

/* Simple translation table for the keys */

static unsigned char kbd_plain_xlate[] = {
	0xff,0x1b, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=','\b','\t',	/* 0x00 - 0x0f */
	 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']','\r',0xff, 'a', 's',	/* 0x10 - 0x1f */
	 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';','\'', '`',0xff,'\\', 'z', 'x', 'c', 'v',	/* 0x20 - 0x2f */
	 'b', 'n', 'm', ',', '.', '/',0xff,0xff,0xff, ' ',0xff,0xff,0xff,0xff,0xff,0xff,	/* 0x30 - 0x3f */
	0xff,0xff,0xff,0xff,0xff,0xff,0xff, '7', '8', '9', '-', '4', '5', '6', '+', '1',	/* 0x40 - 0x4f */
	 '2', '3', '0', '.',0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  /* 0x50 - 0x5F */
	'\r',0xff,0xff
	};

static unsigned char kbd_shift_xlate[] = {
	0xff,0x1b, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+','\b','\t',	/* 0x00 - 0x0f */
	 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}','\r',0xff, 'A', 'S',	/* 0x10 - 0x1f */
	 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',0xff, '|', 'Z', 'X', 'C', 'V',	/* 0x20 - 0x2f */
	 'B', 'N', 'M', '<', '>', '?',0xff,0xff,0xff, ' ',0xff,0xff,0xff,0xff,0xff,0xff,	/* 0x30 - 0x3f */
	0xff,0xff,0xff,0xff,0xff,0xff,0xff, '7', '8', '9', '-', '4', '5', '6', '+', '1',	/* 0x40 - 0x4f */
	 '2', '3', '0', '.',0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  /* 0x50 - 0x5F */
	'\r',0xff,0xff
	};

static unsigned char kbd_ctrl_xlate[] = {
	0xff,0x1b, '1',0x00, '3', '4', '5',0x1E, '7', '8', '9', '0',0x1F, '=','\b','\t',	/* 0x00 - 0x0f */
	0x11,0x17,0x05,0x12,0x14,0x18,0x15,0x09,0x0f,0x10,0x1b,0x1d,'\n',0xff,0x01,0x13,	/* 0x10 - 0x1f */
	0x04,0x06,0x08,0x09,0x0a,0x0b,0x0c, ';','\'', '~',0x00,0x1c,0x1a,0x18,0x03,0x16,	/* 0x20 - 0x2f */
	0x02,0x0e,0x0d, '<', '>', '?',0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0xff,	/* 0x30 - 0x3f */
	0xff,0xff,0xff,0xff,0xff,0xff,0xff, '7', '8', '9', '-', '4', '5', '6', '+', '1',	/* 0x40 - 0x4f */
	 '2', '3', '0', '.',0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  /* 0x50 - 0x5F */
	'\r',0xff,0xff
	};

/******************************************************************
 * Init
 ******************************************************************/
int isa_kbd_init(void)
{
	char* result;
	result=kbd_initialize();
	if(result==NULL) {
		PRINTF("AT Keyboard initialized\n");
		irq_install_handler(25, (interrupt_handler_t *)handle_isa_int, NULL);
		isa_irq_install_handler(KBD_INTERRUPT, (interrupt_handler_t *)kbd_interrupt, NULL);
		return (1);
	} else {
		printf("%s\n",result);
		return (-1);
	}
}

#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern int overwrite_console (void);
#else
int overwrite_console (void)
{
	return (0);
}
#endif

int drv_isa_kbd_init (void)
{
	int error;
	struct stdio_dev kbddev ;
	char *stdinname  = getenv ("stdin");

	if(isa_kbd_init()==-1)
		return -1;
	memset (&kbddev, 0, sizeof(kbddev));
	strcpy(kbddev.name, DEVNAME);
	kbddev.flags =  DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	kbddev.putc = NULL ;
	kbddev.puts = NULL ;
	kbddev.getc = kbd_getc ;
	kbddev.tstc = kbd_testc ;

	error = stdio_register (&kbddev);
	if(error==0) {
		/* check if this is the standard input device */
		if(strcmp(stdinname,DEVNAME)==0) {
			/* reassign the console */
			if(overwrite_console()) {
				return 1;
			}
			error=console_assign(stdin,DEVNAME);
			if(error==0)
				return 1;
			else
				return error;
		}
		return 1;
	}
	return error;
}

/******************************************************************
 * Queue handling
 ******************************************************************/
/* puts character in the queue and sets up the in and out pointer */
void kbd_put_queue(char data)
{
	if((in_pointer+1)==KBD_BUFFER_LEN) {
		if(out_pointer==0) {
			return; /* buffer full */
		} else{
			in_pointer=0;
		}
	} else {
		if((in_pointer+1)==out_pointer)
			return; /* buffer full */
		in_pointer++;
	}
	kbd_buffer[in_pointer]=data;
	return;
}

/* test if a character is in the queue */
int kbd_testc(void)
{
	if(in_pointer==out_pointer)
		return(0); /* no data */
	else
		return(1);
}
/* gets the character from the queue */
int kbd_getc(void)
{
	char c;
	while(in_pointer==out_pointer);
	if((out_pointer+1)==KBD_BUFFER_LEN)
		out_pointer=0;
	else
		out_pointer++;
	c=kbd_buffer[out_pointer];
	return (int)c;

}


/* set LEDs */

void kbd_set_leds(void)
{
	if(caps_lock==0)
		leds&=~LED_CAP; /* switch caps_lock off */
	else
		leds|=LED_CAP; /* switch on LED */
	if(num_lock==0)
		leds&=~LED_NUM; /* switch LED off */
	else
		leds|=LED_NUM;  /* switch on LED */
	if(scroll_lock==0)
		leds&=~LED_SCR; /* switch LED off */
	else
		leds|=LED_SCR; /* switch on LED */
	kbd_send_data(KBD_CMD_SET_LEDS);
	kbd_send_data(leds);
}


void handle_keyboard_event (unsigned char scancode)
{
	unsigned char keycode;

	/*  Convert scancode to keycode */
	PRINTF ("scancode %x\n", scancode);
	if (scancode == 0xe0) {
		e0 = 1;		/* special charakters */
		return;
	}
	if (e0 == 1) {
		e0 = 0;		/* delete flag */
		if (!(((scancode & 0x7F) == 0x38) ||	/* the right ctrl key */
		      ((scancode & 0x7F) == 0x1D) ||	/* the right alt key */
		      ((scancode & 0x7F) == 0x35) ||	/* the right '/' key */
		      ((scancode & 0x7F) == 0x1C)))
			/* the right enter key */
			/* we swallow unknown e0 codes */
			return;
	}
	/* special cntrl keys */
	switch (scancode) {
	case 0x2A:
	case 0x36:		/* shift pressed */
		shift = 1;
		return;		/* do nothing else */
	case 0xAA:
	case 0xB6:		/* shift released */
		shift = 0;
		return;		/* do nothing else */
	case 0x38:		/* alt pressed */
		alt = 1;
		return;		/* do nothing else */
	case 0xB8:		/* alt released */
		alt = 0;
		return;		/* do nothing else */
	case 0x1d:		/* ctrl pressed */
		ctrl = 1;
		return;		/* do nothing else */
	case 0x9d:		/* ctrl released */
		ctrl = 0;
		return;		/* do nothing else */
	case 0x46:		/* scrollock pressed */
		scroll_lock = ~scroll_lock;
		kbd_set_leds ();
		return;		/* do nothing else */
	case 0x3A:		/* capslock pressed */
		caps_lock = ~caps_lock;
		kbd_set_leds ();
		return;
	case 0x45:		/* numlock pressed */
		num_lock = ~num_lock;
		kbd_set_leds ();
		return;
	case 0xC6:		/* scroll lock released */
	case 0xC5:		/* num lock released */
	case 0xBA:		/* caps lock released */
		return;		/* just swallow */
	}
	if ((scancode & 0x80) == 0x80)	/* key released */
		return;
	/* now, decide which table we need */
	if (scancode > (sizeof (kbd_plain_xlate) / sizeof (kbd_plain_xlate[0]))) {	/* scancode not in list */
		PRINTF ("unkown scancode %X\n", scancode);
		return;		/* swallow it */
	}
	/* setup plain code first */
	keycode = kbd_plain_xlate[scancode];
	if (caps_lock == 1) {	/* caps_lock is pressed, overwrite plain code */
		if (scancode > (sizeof (kbd_shift_xlate) / sizeof (kbd_shift_xlate[0]))) {	/* scancode not in list */
			PRINTF ("unkown caps-locked scancode %X\n", scancode);
			return;	/* swallow it */
		}
		keycode = kbd_shift_xlate[scancode];
		if (keycode < 'A') {	/* we only want the alphas capital */
			keycode = kbd_plain_xlate[scancode];
		}
	}
	if (shift == 1) {	/* shift overwrites caps_lock */
		if (scancode > (sizeof (kbd_shift_xlate) / sizeof (kbd_shift_xlate[0]))) {	/* scancode not in list */
			PRINTF ("unkown shifted scancode %X\n", scancode);
			return;	/* swallow it */
		}
		keycode = kbd_shift_xlate[scancode];
	}
	if (ctrl == 1) {	/* ctrl overwrites caps_lock and shift */
		if (scancode > (sizeof (kbd_ctrl_xlate) / sizeof (kbd_ctrl_xlate[0]))) {	/* scancode not in list */
			PRINTF ("unkown ctrl scancode %X\n", scancode);
			return;	/* swallow it */
		}
		keycode = kbd_ctrl_xlate[scancode];
	}
	/* check if valid keycode */
	if (keycode == 0xff) {
		PRINTF ("unkown scancode %X\n", scancode);
		return;		/* swallow unknown codes */
	}

	kbd_put_queue (keycode);
	PRINTF ("%x\n", keycode);
}

/*
 * This reads the keyboard status port, and does the
 * appropriate action.
 *
 */
unsigned char handle_kbd_event(void)
{
	unsigned char status = kbd_read_status();
	unsigned int work = 10000;

	while ((--work > 0) && (status & KBD_STAT_OBF)) {
		unsigned char scancode;

		scancode = kbd_read_input();

		/* Error bytes must be ignored to make the
		   Synaptics touchpads compaq use work */
		/* Ignore error bytes */
		if (!(status & (KBD_STAT_GTO | KBD_STAT_PERR)))
		{
			if (status & KBD_STAT_MOUSE_OBF)
				; /* not supported: handle_mouse_event(scancode); */
			else
				handle_keyboard_event(scancode);
		}
		status = kbd_read_status();
	}
	if (!work)
		PRINTF("pc_keyb: controller jammed (0x%02X).\n", status);
	return status;
}


/******************************************************************************
 * Lowlevel Part of keyboard section
 */
unsigned char kbd_read_status(void)
{
	return(in8(CONFIG_SYS_ISA_IO_BASE_ADDRESS + KDB_COMMAND_PORT));
}

unsigned char kbd_read_input(void)
{
	return(in8(CONFIG_SYS_ISA_IO_BASE_ADDRESS + KDB_DATA_PORT));
}

void kbd_write_command(unsigned char cmd)
{
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS + KDB_COMMAND_PORT,cmd);
}

void kbd_write_output(unsigned char data)
{
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS + KDB_DATA_PORT, data);
}

int kbd_read_data(void)
{
	int val;
	unsigned char status;

	val = -1;
	status = kbd_read_status();
	if (status & KBD_STAT_OBF) {
		val = kbd_read_input();
		if (status & (KBD_STAT_GTO | KBD_STAT_PERR))
			val = -2;
	}
	return val;
}

int kbd_wait_for_input(void)
{
	unsigned long timeout;
	int val;

	timeout = KBD_TIMEOUT;
	val=kbd_read_data();
	while(val < 0)
	{
		if(timeout--==0)
			return -1;
		udelay(1000);
		val=kbd_read_data();
	}
	return val;
}


int kb_wait(void)
{
	unsigned long timeout = KBC_TIMEOUT * 10;

	do {
		unsigned char status = handle_kbd_event();
		if (!(status & KBD_STAT_IBF))
			return 0; /* ok */
		udelay(1000);
		timeout--;
	} while (timeout);
	return 1;
}

void kbd_write_command_w(int data)
{
	if(kb_wait())
		PRINTF("timeout in kbd_write_command_w\n");
	kbd_write_command(data);
}

void kbd_write_output_w(int data)
{
	if(kb_wait())
		PRINTF("timeout in kbd_write_output_w\n");
	kbd_write_output(data);
}

void kbd_send_data(unsigned char data)
{
	unsigned char status;
	disable_8259A_irq(1); /* disable interrupt */
	kbd_write_output_w(data);
	status = kbd_wait_for_input();
	if (status == KBD_REPLY_ACK)
		enable_8259A_irq(1); /* enable interrupt */
}


char * kbd_initialize(void)
{
	int status;

	in_pointer = 0; /* delete in Buffer */
	out_pointer = 0;
	/*
	 * Test the keyboard interface.
	 * This seems to be the only way to get it going.
	 * If the test is successful a x55 is placed in the input buffer.
	 */
	kbd_write_command_w(KBD_CCMD_SELF_TEST);
	if (kbd_wait_for_input() != 0x55)
		return "Kbd:   failed self test";
	/*
	 * Perform a keyboard interface test.  This causes the controller
	 * to test the keyboard clock and data lines.  The results of the
	 * test are placed in the input buffer.
	 */
	kbd_write_command_w(KBD_CCMD_KBD_TEST);
	if (kbd_wait_for_input() != 0x00)
		return "Kbd:   interface failed self test";
	/*
	 * Enable the keyboard by allowing the keyboard clock to run.
	 */
	kbd_write_command_w(KBD_CCMD_KBD_ENABLE);
	status = kbd_wait_for_input();
	/*
	 * Reset keyboard. If the read times out
	 * then the assumption is that no keyboard is
	 * plugged into the machine.
	 * This defaults the keyboard to scan-code set 2.
	 *
	 * Set up to try again if the keyboard asks for RESEND.
	 */
	do {
		kbd_write_output_w(KBD_CMD_RESET);
		status = kbd_wait_for_input();
		if (status == KBD_REPLY_ACK)
			break;
		if (status != KBD_REPLY_RESEND) {
			PRINTF("status: %X\n",status);
			return "Kbd:   reset failed, no ACK";
		}
	} while (1);
	if (kbd_wait_for_input() != KBD_REPLY_POR)
		return "Kbd:   reset failed, no POR";

	/*
	 * Set keyboard controller mode. During this, the keyboard should be
	 * in the disabled state.
	 *
	 * Set up to try again if the keyboard asks for RESEND.
	 */
	do {
		kbd_write_output_w(KBD_CMD_DISABLE);
		status = kbd_wait_for_input();
		if (status == KBD_REPLY_ACK)
			break;
		if (status != KBD_REPLY_RESEND)
			return "Kbd:   disable keyboard: no ACK";
	} while (1);

	kbd_write_command_w(KBD_CCMD_WRITE_MODE);
	kbd_write_output_w(KBD_MODE_KBD_INT
			      | KBD_MODE_SYS
			      | KBD_MODE_DISABLE_MOUSE
			      | KBD_MODE_KCC);

	/* AMCC powerpc portables need this to use scan-code set 1 -- Cort */
	kbd_write_command_w(KBD_CCMD_READ_MODE);
	if (!(kbd_wait_for_input() & KBD_MODE_KCC)) {
		/*
		 * If the controller does not support conversion,
		 * Set the keyboard to scan-code set 1.
		 */
		kbd_write_output_w(0xF0);
		kbd_wait_for_input();
		kbd_write_output_w(0x01);
		kbd_wait_for_input();
	}
	kbd_write_output_w(KBD_CMD_ENABLE);
	if (kbd_wait_for_input() != KBD_REPLY_ACK)
		return "Kbd:   enable keyboard: no ACK";

	/*
	 * Finally, set the typematic rate to maximum.
	 */
	kbd_write_output_w(KBD_CMD_SET_RATE);
	if (kbd_wait_for_input() != KBD_REPLY_ACK)
		return "Kbd:   Set rate: no ACK";
	kbd_write_output_w(0x00);
	if (kbd_wait_for_input() != KBD_REPLY_ACK)
		return "Kbd:   Set rate: no ACK";
	return NULL;
}

void kbd_interrupt(void)
{
	handle_kbd_event();
}
