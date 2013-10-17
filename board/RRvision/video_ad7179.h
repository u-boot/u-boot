/*
 * (C) Copyright 2003 Wolfgang Grandegger <wg@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define VIDEO_ENCODER_NAME	"Analog Devices AD7179"

#define VIDEO_ENCODER_I2C_RATE	100000	/* Max rate is 100Khz	*/
#define VIDEO_ENCODER_CB_Y_CR_Y		/* Use CB Y CR Y format...	*/

#define VIDEO_MODE_YUYV		/* The only mode supported by this encoder	*/
#undef	VIDEO_MODE_RGB
#define VIDEO_MODE_BPP		16

#ifdef	VIDEO_MODE_PAL
#define VIDEO_ACTIVE_COLS	720
#define VIDEO_ACTIVE_ROWS	576
#define VIDEO_VISIBLE_COLS	640
#define VIDEO_VISIBLE_ROWS	480
#else
#error "NTSC mode is not supported"
#endif

static unsigned char video_encoder_data[] = {
				0x05, /* Mode Register 0 */
				0x11, /* Mode Register 1 */
				0x20, /* Mode Register 2 */
				0x0C, /* Mode Register 3 */
				0x01, /* Mode Register 4 */
				0x00, /* Reserved */
				0x00, /* Reserved */
				0x04, /* Timing Register 0 */
				0x00, /* Timing Register 1 */
				0xCB, /* Subcarrier Frequency Register 0 */
				0x0A, /* Subcarrier Frequency Register 1 */
				0x09, /* Subcarrier Frequency Register 2 */
				0x2A, /* Subcarrier Frequency Register 3 */
				0x00, /* Subcarrier Phase */
				0x00, /* Closed Captioning Ext Reg 0 */
				0x00, /* Closed Captioning Ext Reg 1 */
				0x00, /* Closed Captioning Reg 0 */
				0x00, /* Closed Captioning Reg 1 */
				0x00, /* Pedestal Control Reg 0 */
				0x00, /* Pedestal Control Reg 1 */
				0x00, /* Pedestal Control Reg 2 */
				0x00, /* Pedestal Control Reg 3 */
				0x00, /* CGMS_WSS Reg 0 */
				0x00, /* CGMS_WSS Reg 0 */
				0x00, /* CGMS_WSS Reg 0 */
				0x00  /* Teletext Req. Control Reg */
} ;
