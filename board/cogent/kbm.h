/* keyboard/mouse not implemented yet */

extern int cma_kbm_not_implemented;

/**************** DEFINES for H8542B Keyboard/Mouse Controller ***************/

/*
 * note the auxillary port is used to control the mouse
 */

/* 8542B Commands (Sent to the Command Port) */
#define HT8542_CMD_SET_BYTE	0x60	/* Set the command byte */
#define HT8542_CMD_GET_BYTE	0x20	/* Get the command byte */
#define HT8542_CMD_KBD_OBUFF	0xD2	/* Write to HT8542 Kbd Output Buffer */
#define HT8542_CMD_AUX_OBUFF	0xD3	/* Write to HT8542 Mse Output Buffer */
#define HT8542_CMD_AUX_WRITE	0xD4	/* Write to Mouse Port */
#define HT8542_CMD_AUX_OFF	0xA7	/* Disable Mouse Port */
#define HT8542_CMD_AUX_ON	0xA8	/* Re-Enable Mouse Port */
#define HT8542_CMD_AUX_TEST	0xA9	/* Test for the presence of a Mouse */
#define HT8542_CMD_DIAG		0xAA	/* Start Diagnostics */
#define HT8542_CMD_KBD_TEST	0xAB	/* Test for presence of a keyboard */
#define HT8542_CMD_KBD_OFF	0xAD	/* Disable Kbd Port (use KBD_DAT_ON) */
#define HT8542_CMD_KBD_ON	0xAE	/* Enable Kbd Port (use KBD_DAT_OFF) */

/* HT8542B cmd byte set by KBD_CMD_SET_BYTE and retrieved by KBD_CMD_GET_BYTE */
#define HT8542_CMD_BYTE_TRANS	0x40
#define HT8542_CMD_BYTE_AUX_OFF	0x20	/* 1 = mse port disabled, 0 = enabled */
#define HT8542_CMD_BYTE_KBD_OFF	0x10	/* 1 = kbd port disabled, 0 = enabled */
#define HT8542_CMD_BYTE_OVER	0x08	/* 1 = override keyboard lock */
#define HT8542_CMD_BYTE_RES	0x04	/* reserved */
#define HT8542_CMD_BYTE_AUX_INT	0x02	/* 1 = enable mouse interrupt */
#define HT8542_CMD_BYTE_KBD_INT	0x01	/* 1 = enable keyboard interrupt */

/* Keyboard Commands (Sent to the Data Port) */
#define KBD_CMD_LED		0xED	/* Set Keyboard LEDS with next byte */
#define KBD_CMD_ECHO		0xEE	/* Echo - we get 0xFA, 0xEE back */
#define KBD_CMD_MODE		0xF0	/* set scan code mode with next byte */
#define KBD_CMD_ID		0xF2	/* get keyboard/mouse ID */
#define KBD_CMD_RPT		0xF3	/* Set Repeat Rate and Delay 2nd Byte */
#define KBD_CMD_ON		0xF4	/* Enable keyboard */
#define KBD_CMD_OFF		0xF5	/* Disables Scanning, Resets to Def */
#define KBD_CMD_DEF		0xF6	/* Reverts kbd to default settings */
#define KBD_CMD_RST		0xFF	/* Reset - should get 0xFA, 0xAA back */

/* Set LED second bit defines */
#define KBD_CMD_LED_SCROLL	0x01	/* Set SCROLL LOCK LED on */
#define KBD_CMD_LED_NUM		0x02	/* Set NUM LOCK LED on */
#define KBD_CMD_LED_CAPS	0x04	/* Set CAPS LOCK LED on */

/* Set Mode second byte defines */
#define KBD_CMD_MODE_STAT	0x00	/* get current scan code mode */
#define KBD_CMD_MODE_SCAN1	0x01	/* set mode to scan code 1 */
#define KBD_CMD_MODE_SCAN2	0x02	/* set mode to scan code 2 */
#define KBD_CMD_MODE_SCAN3	0x03	/* set mode to scan code 3 */

/* Keyboard/Mouse ID Codes */
#define KBD_CMD_ID_1ST		0xAB	/* 1st byte is 0xAB, 2nd is actual ID */
#define KBD_CMD_ID_KBD		0x83	/* Keyboard */
#define KBD_CMD_ID_MOUSE	0x00	/* Mouse */

/* Keyboard Data Return Defines */
#define KBD_STAT_OVER		0x00	/* Buffer Overrun */
#define KBD_STAT_DIAG_OK	0x55	/* Internal Self Test OK */
#define KBD_STAT_RST_OK		0xAA	/* Reset Complete */
#define KBD_STAT_ECHO		0xEE	/* Echo Command Return */
#define KBD_STAT_BRK		0xF0	/* Prefix for Break Key Code */
#define KBD_STAT_ACK		0xFA	/* Received after all commands */
#define KBD_STAT_DIAG_FAIL	0xFD	/* Internal Self Test Failed */
#define KBD_STAT_RESEND		0xFE	/* Resend Last Command */

/* HT8542B Status Register Bit Defines */
#define HT8542_STAT_OBF		0x01	/* 1 = output buffer is full */
#define HT8542_STAT_IBF		0x02	/* 1 = input buffer is full */
#define HT8542_STAT_SYS		0x04	/* system flag - unused */
#define HT8542_STAT_CMD		0x08	/* 1 = cmd in input buffer, 0 = data */
#define HT8542_STAT_INH		0x10	/* 1 = Inhibit - unused */
#define HT8542_STAT_TX		0x20	/* 1 = Transmit Timeout has occured */
#define HT8542_STAT_RX		0x40	/* 1 = Receive Timeout has occured */
#define HT8542_STAT_PERR	0x80	/* 1 = Parity Error from Keyboard */
