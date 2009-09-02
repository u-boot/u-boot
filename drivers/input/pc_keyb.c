/***********************************************************************
 *
 * (C) Copyright 2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 *
 * PS/2 keyboard driver
 *
 * Originally from linux source (drivers/char/pc_keyb.c)
 *
 ***********************************************************************/

#include <common.h>

#include <keyboard.h>
#include <pc_keyb.h>

#undef KBG_DEBUG

#ifdef KBG_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif


/*
 * This reads the keyboard status port, and does the
 * appropriate action.
 *
 */
static unsigned char handle_kbd_event(void)
{
	unsigned char status = kbd_read_status();
	unsigned int work = 10000;

	while ((--work > 0) && (status & KBD_STAT_OBF)) {
		unsigned char scancode;

		scancode = kbd_read_input();

		/* Error bytes must be ignored to make the
		   Synaptics touchpads compaq use work */
		/* Ignore error bytes */
		if (!(status & (KBD_STAT_GTO | KBD_STAT_PERR))) {
			if (status & KBD_STAT_MOUSE_OBF)
				; /* not supported: handle_mouse_event(scancode); */
			else
				handle_scancode(scancode);
		}
		status = kbd_read_status();
	}
	if (!work)
		PRINTF("pc_keyb: controller jammed (0x%02X).\n", status);
	return status;
}


static int kbd_read_data(void)
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

static int kbd_wait_for_input(void)
{
	unsigned long timeout;
	int val;

	timeout = KBD_TIMEOUT;
	val=kbd_read_data();
	while(val < 0) {
		if(timeout--==0)
			return -1;
		udelay(1000);
		val=kbd_read_data();
	}
	return val;
}


static int kb_wait(void)
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

static void kbd_write_command_w(int data)
{
	if(kb_wait())
		PRINTF("timeout in kbd_write_command_w\n");
	kbd_write_command(data);
}

static void kbd_write_output_w(int data)
{
	if(kb_wait())
		PRINTF("timeout in kbd_write_output_w\n");
	kbd_write_output(data);
}

static void kbd_send_data(unsigned char data)
{
	kbd_write_output_w(data);
	kbd_wait_for_input();
}


static char * kbd_initialize(void)
{
	int status;

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

static void kbd_interrupt(void *dev_id)
{
	handle_kbd_event();
}

/******************************************************************
 * Init
 ******************************************************************/

int kbd_init_hw(void)
{
	char* result;

	kbd_request_region();

	result=kbd_initialize();
	if (result==NULL) {
		PRINTF("AT Keyboard initialized\n");
		kbd_request_irq(kbd_interrupt);
		return (1);
	} else {
		printf("%s\n",result);
		return (-1);
	}
}

void pckbd_leds(unsigned char leds)
{
	kbd_send_data(KBD_CMD_SET_LEDS);
	kbd_send_data(leds);
}
