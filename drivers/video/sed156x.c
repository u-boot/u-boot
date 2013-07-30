/*
 * (C) Copyright 2003
 *
 * Pantelis Antoniou <panto@intracom.gr>
 * Intracom S.A.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#include <common.h>
#include <watchdog.h>

#include <sed156x.h>

/* configure according to the selected display */
#if defined(CONFIG_SED156X_PG12864Q)
#define LCD_WIDTH	128
#define LCD_HEIGHT	64
#define LCD_LINES	64
#define LCD_PAGES	9
#define LCD_COLUMNS	132
#else
#error Unsupported SED156x configuration
#endif

/* include the font data */
#include <video_font.h>

#if VIDEO_FONT_WIDTH != 8 || VIDEO_FONT_HEIGHT != 16
#error Expecting VIDEO_FONT_WIDTH == 8 && VIDEO_FONT_HEIGHT == 16
#endif

#define LCD_BYTE_WIDTH		(LCD_WIDTH / 8)
#define VIDEO_FONT_BYTE_WIDTH	(VIDEO_FONT_WIDTH / 8)

#define LCD_TEXT_WIDTH	(LCD_WIDTH / VIDEO_FONT_WIDTH)
#define LCD_TEXT_HEIGHT (LCD_HEIGHT / VIDEO_FONT_HEIGHT)

#define LCD_BYTE_LINESZ		(LCD_BYTE_WIDTH * VIDEO_FONT_HEIGHT)

const int sed156x_text_width = LCD_TEXT_WIDTH;
const int sed156x_text_height = LCD_TEXT_HEIGHT;

/**************************************************************************************/

#define SED156X_SPI_RXD() (SED156X_SPI_RXD_PORT & SED156X_SPI_RXD_MASK)

#define SED156X_SPI_TXD(x) \
	do { \
		if (x) \
			SED156X_SPI_TXD_PORT |=	 SED156X_SPI_TXD_MASK; \
		else \
			SED156X_SPI_TXD_PORT &= ~SED156X_SPI_TXD_MASK; \
	} while(0)

#define SED156X_SPI_CLK(x) \
	do { \
		if (x) \
			SED156X_SPI_CLK_PORT |=	 SED156X_SPI_CLK_MASK; \
		else \
			SED156X_SPI_CLK_PORT &= ~SED156X_SPI_CLK_MASK; \
	} while(0)

#define SED156X_SPI_CLK_TOGGLE() (SED156X_SPI_CLK_PORT ^= SED156X_SPI_CLK_MASK)

#define SED156X_SPI_BIT_DELAY() /* no delay */

#define SED156X_CS(x) \
	do { \
		if (x) \
			SED156X_CS_PORT |=  SED156X_CS_MASK; \
		else \
			SED156X_CS_PORT &= ~SED156X_CS_MASK; \
	} while(0)

#define SED156X_A0(x) \
	do { \
		if (x) \
			SED156X_A0_PORT |=  SED156X_A0_MASK; \
		else \
			SED156X_A0_PORT &= ~SED156X_A0_MASK; \
	} while(0)

/**************************************************************************************/

/*** LCD Commands ***/

#define LCD_ON		0xAF	/* Display ON					      */
#define LCD_OFF		0xAE	/* Display OFF					      */
#define LCD_LADDR	0x40	/* Display start line set + (6-bit) address	      */
#define LCD_PADDR	0xB0	/* Page address set + (4-bit) page		      */
#define LCD_CADRH	0x10	/* Column address set upper + (4-bit) column hi	      */
#define LCD_CADRL	0x00	/* Column address set lower + (4-bit) column lo	      */
#define LCD_ADC_NRM	0xA0	/* ADC select Normal				      */
#define LCD_ADC_REV	0xA1	/* ADC select Reverse				      */
#define LCD_DSP_NRM	0xA6	/* LCD display Normal				      */
#define LCD_DSP_REV	0xA7	/* LCD display Reverse				      */
#define LCD_DPT_NRM	0xA4	/* Display all points Normal			      */
#define LCD_DPT_ALL	0xA5	/* Display all points ON			      */
#define LCD_BIAS9	0xA2	/* LCD bias set 1/9				      */
#define LCD_BIAS7	0xA3	/* LCD bias set 1/7				      */
#define LCD_CAINC	0xE0	/* Read/modify/write				      */
#define LCD_CAEND	0xEE	/* End						      */
#define LCD_RESET	0xE2	/* Reset					      */
#define LCD_C_NRM	0xC0	/* Common output mode select Normal direction	      */
#define LCD_C_RVS	0xC8	/* Common output mode select Reverse direction	      */
#define LCD_PWRMD	0x28	/* Power control set + (3-bit) mode		      */
#define LCD_RESRT	0x20	/* V5 v. reg. int. resistor ratio set + (3-bit) ratio */
#define LCD_EVSET	0x81	/* Electronic volume mode set + byte = (6-bit) volume */
#define LCD_SIOFF	0xAC	/* Static indicator OFF				      */
#define LCD_SION	0xAD	/* Static indicator ON + byte = (2-bit) mode	      */
#define LCD_NOP		0xE3	/* NOP						      */
#define LCD_TEST	0xF0	/* Test/Test mode reset (Note: *DO NOT USE*)	      */

/*-------------------------------------------------------------------------------
  Compound commands
  -------------------------------------------------------------------------------
  Command	Description			Commands
  ----------	------------------------	-------------------------------------
  POWS_ON	POWER SAVER ON command		LCD_OFF, LCD_D_ALL
  POWS_OFF	POWER SAVER OFF command		LCD_D_NRM
  SLEEPON	SLEEP mode			LCD_SIOFF, POWS_ON
  SLEEPOFF	SLEEP mode cancel		LCD_D_NRM, LCD_SION, LCD_SIS_???
  STDBYON	STAND BY mode			LCD_SION, POWS_ON
  STDBYOFF	STAND BY mode cancel		LCD_D_NRM
  -------------------------------------------------------------------------------*/

/*** LCD various parameters ***/
#define LCD_PPB		8	/* Pixels per byte (display is B/W, 1 bit per pixel) */

/*** LCD Status byte masks ***/
#define LCD_S_BUSY	0x80	/* Status Read - BUSY mask   */
#define LCD_S_ADC	0x40	/* Status Read - ADC mask    */
#define LCD_S_ONOFF	0x20	/* Status Read - ON/OFF mask */
#define LCD_S_RESET	0x10	/* Status Read - RESET mask  */

/*** LCD commands parameter masks ***/
#define LCD_M_LADDR	0x3F	/* Display start line (6-bit) address mask	     */
#define LCD_M_PADDR	0x0F	/* Page address (4-bit) page mask		     */
#define LCD_M_CADRH	0x0F	/* Column address upper (4-bit) column hi mask	     */
#define LCD_M_CADRL	0x0F	/* Column address lower (4-bit) column lo mask	     */
#define LCD_M_PWRMD	0x07	/* Power control (3-bit) mode mask		     */
#define LCD_M_RESRT	0x07	/* V5 v. reg. int. resistor ratio (3-bit) ratio mask */
#define LCD_M_EVSET	0x3F	/* Electronic volume mode byte (6-bit) volume mask   */
#define LCD_M_SION	0x03	/* Static indicator ON (2-bit) mode mask	     */

/*** LCD Power control cirquits control masks ***/
#define LCD_PWRBSTR	0x04	/* Power control mode - Booster cirquit ON	     */
#define LCD_PWRVREG	0x02	/* Power control mode - Voltage regulator cirquit ON */
#define LCD_PWRVFOL	0x01	/* Power control mode - Voltage follower cirquit ON  */

/*** LCD Static indicator states ***/
#define LCD_SIS_OFF	0x00	/* Static indicator register set - OFF state		 */
#define LCD_SIS_BL	0x01	/* Static indicator register set - 1s blink state	 */
#define LCD_SIS_RBL	0x02	/* Static indicator register set - .5s rapid blink state */
#define LCD_SIS_ON	0x03	/* Static indicator register set - constantly on state	 */

/*** LCD functions special parameters (commands) ***/
#define LCD_PREVP	0x80	/* Page number for moving to previous */
#define LCD_NEXTP	0x81	/* or next page */
#define LCD_ERR_P	0xFF	/* Error in page number */

/*** LCD initialization settings ***/
#define LCD_BIAS	LCD_BIAS9	/* Bias: 1/9		      */
#define LCD_ADCMODE	LCD_ADC_NRM	/* ADC mode: normal	      */
#define LCD_COMDIR	LCD_C_NRM	/* Common output mode: normal */
#define LCD_RRATIO	0		/* Resistor ratio: 0	      */
#define LCD_CNTRST	0x1C		/* electronic volume: 1Ch     */
#define LCD_POWERM	(LCD_PWRBSTR | LCD_PWRVREG | LCD_PWRVFOL)	/* Power mode: All on */

/**************************************************************************************/

static inline unsigned int sed156x_transfer(unsigned int val)
{
	unsigned int rx;
	int b;

	rx = 0; b = 8;
	while (--b >= 0) {
		SED156X_SPI_TXD(val & 0x80);
		val <<= 1;
		SED156X_SPI_CLK_TOGGLE();
		SED156X_SPI_BIT_DELAY();
		rx <<= 1;
		if (SED156X_SPI_RXD())
			rx |= 1;
		SED156X_SPI_CLK_TOGGLE();
		SED156X_SPI_BIT_DELAY();
	}

	return rx;
}

unsigned int sed156x_data_transfer(unsigned int val)
{
	unsigned int rx;

	SED156X_SPI_CLK(1);
	SED156X_CS(0);
	SED156X_A0(1);

	rx = sed156x_transfer(val);

	SED156X_CS(1);

	return rx;
}

void sed156x_data_block_transfer(const u8 *p, int size)
{
	SED156X_SPI_CLK(1);
	SED156X_CS(0);
	SED156X_A0(1);

	while (--size >= 0)
		sed156x_transfer(*p++);

	SED156X_CS(1);
}

unsigned int sed156x_cmd_transfer(unsigned int val)
{
	unsigned int rx;

	SED156X_SPI_CLK(1);
	SED156X_CS(0);
	SED156X_A0(0);

	rx = sed156x_transfer(val);

	SED156X_CS(1);
	SED156X_A0(1);

	return rx;
}

/******************************************************************************/

static u8 hw_screen[LCD_PAGES][LCD_COLUMNS];
static u8 last_hw_screen[LCD_PAGES][LCD_COLUMNS];
static u8 sw_screen[LCD_BYTE_WIDTH * LCD_HEIGHT];

void sed156x_sync(void)
{
	int i, j, last_page;
	u8 *d;
	const u8 *s, *e, *b, *r;
	u8 v0, v1, v2, v3, v4, v5, v6, v7;

	/* copy and rotate sw_screen to hw_screen */
	for (i = 0; i < LCD_HEIGHT / 8; i++) {

		d = &hw_screen[i][0];
		s = &sw_screen[LCD_BYTE_WIDTH * 8 * i + LCD_BYTE_WIDTH - 1];

		for (j = 0; j < LCD_WIDTH / 8; j++) {

			v0 = s[0 * LCD_BYTE_WIDTH];
			v1 = s[1 * LCD_BYTE_WIDTH];
			v2 = s[2 * LCD_BYTE_WIDTH];
			v3 = s[3 * LCD_BYTE_WIDTH];
			v4 = s[4 * LCD_BYTE_WIDTH];
			v5 = s[5 * LCD_BYTE_WIDTH];
			v6 = s[6 * LCD_BYTE_WIDTH];
			v7 = s[7 * LCD_BYTE_WIDTH];

			d[0] =	((v7 & 0x01) << 7) |
				((v6 & 0x01) << 6) |
				((v5 & 0x01) << 5) |
				((v4 & 0x01) << 4) |
				((v3 & 0x01) << 3) |
				((v2 & 0x01) << 2) |
				((v1 & 0x01) << 1) |
				 (v0 & 0x01)	   ;

			d[1] =	((v7 & 0x02) << 6) |
				((v6 & 0x02) << 5) |
				((v5 & 0x02) << 4) |
				((v4 & 0x02) << 3) |
				((v3 & 0x02) << 2) |
				((v2 & 0x02) << 1) |
				((v1 & 0x02) << 0) |
				((v0 & 0x02) >> 1) ;

			d[2] =	((v7 & 0x04) << 5) |
				((v6 & 0x04) << 4) |
				((v5 & 0x04) << 3) |
				((v4 & 0x04) << 2) |
				((v3 & 0x04) << 1) |
				 (v2 & 0x04)	   |
				((v1 & 0x04) >> 1) |
				((v0 & 0x04) >> 2) ;

			d[3] =	((v7 & 0x08) << 4) |
				((v6 & 0x08) << 3) |
				((v5 & 0x08) << 2) |
				((v4 & 0x08) << 1) |
				 (v3 & 0x08)	   |
				((v2 & 0x08) >> 1) |
				((v1 & 0x08) >> 2) |
				((v0 & 0x08) >> 3) ;

			d[4] =	((v7 & 0x10) << 3) |
				((v6 & 0x10) << 2) |
				((v5 & 0x10) << 1) |
				 (v4 & 0x10)	   |
				((v3 & 0x10) >> 1) |
				((v2 & 0x10) >> 2) |
				((v1 & 0x10) >> 3) |
				((v0 & 0x10) >> 4) ;

			d[5] =	((v7 & 0x20) << 2) |
				((v6 & 0x20) << 1) |
				 (v5 & 0x20)	   |
				((v4 & 0x20) >> 1) |
				((v3 & 0x20) >> 2) |
				((v2 & 0x20) >> 3) |
				((v1 & 0x20) >> 4) |
				((v0 & 0x20) >> 5) ;

			d[6] =	((v7 & 0x40) << 1) |
				 (v6 & 0x40)	   |
				((v5 & 0x40) >> 1) |
				((v4 & 0x40) >> 2) |
				((v3 & 0x40) >> 3) |
				((v2 & 0x40) >> 4) |
				((v1 & 0x40) >> 5) |
				((v0 & 0x40) >> 6) ;

			d[7] =	 (v7 & 0x80)	   |
				((v6 & 0x80) >> 1) |
				((v5 & 0x80) >> 2) |
				((v4 & 0x80) >> 3) |
				((v3 & 0x80) >> 4) |
				((v2 & 0x80) >> 5) |
				((v1 & 0x80) >> 6) |
				((v0 & 0x80) >> 7) ;

			d += 8;
			s--;
		}
	}

	/* and now output only the differences */
	for (i = 0; i < LCD_PAGES; i++) {

		b = &hw_screen[i][0];
		e = &hw_screen[i][LCD_COLUMNS];

		d = &last_hw_screen[i][0];
		s = b;

		last_page = -1;

		/* update only the differences */
		do {
			while (s < e && *s == *d) {
				s++;
				d++;
			}
			if (s == e)
				break;
			r = s;
			while (s < e && *s != *d)
				*d++ = *s++;

			j = r - b;

			if (i != last_page) {
				sed156x_cmd_transfer(LCD_PADDR | i);
				last_page = i;
			}

			sed156x_cmd_transfer(LCD_CADRH | ((j >> 4) & 0x0F));
			sed156x_cmd_transfer(LCD_CADRL | (j & 0x0F));
			sed156x_data_block_transfer(r, s - r);

		} while (s < e);
	}

/********
	for (i = 0; i < LCD_PAGES; i++) {
		sed156x_cmd_transfer(LCD_PADDR | i);
		sed156x_cmd_transfer(LCD_CADRH | 0);
		sed156x_cmd_transfer(LCD_CADRL | 0);
		sed156x_data_block_transfer(&hw_screen[i][0], LCD_COLUMNS);
	}
	memcpy(last_hw_screen, hw_screen, sizeof(last_hw_screen));
********/
}

void sed156x_clear(void)
{
	memset(sw_screen, 0, sizeof(sw_screen));
}

void sed156x_output_at(int x, int y, const char *str, int size)
{
	int i, j;
	u8 *p;
	const u8 *s;

	if ((unsigned int)y >= LCD_TEXT_HEIGHT || (unsigned int)x >= LCD_TEXT_WIDTH)
		return;

	p = &sw_screen[y * VIDEO_FONT_HEIGHT * LCD_BYTE_WIDTH + x * VIDEO_FONT_BYTE_WIDTH];

	while (--size >= 0) {

		s = &video_fontdata[((int)*str++ & 0xff) * VIDEO_FONT_BYTE_WIDTH * VIDEO_FONT_HEIGHT];
		for (i = 0; i < VIDEO_FONT_HEIGHT; i++) {
			for (j = 0; j < VIDEO_FONT_BYTE_WIDTH; j++)
				*p++ = *s++;
			p += LCD_BYTE_WIDTH - VIDEO_FONT_BYTE_WIDTH;
		}
		p -= (LCD_BYTE_LINESZ - VIDEO_FONT_BYTE_WIDTH);

		if (x >= LCD_TEXT_WIDTH)
			break;
		x++;
	}
}

void sed156x_reverse_at(int x, int y, int size)
{
	int i, j;
	u8 *p;

	if ((unsigned int)y >= LCD_TEXT_HEIGHT || (unsigned int)x >= LCD_TEXT_WIDTH)
		return;

	p = &sw_screen[y * VIDEO_FONT_HEIGHT * LCD_BYTE_WIDTH + x * VIDEO_FONT_BYTE_WIDTH];

	while (--size >= 0) {

		for (i = 0; i < VIDEO_FONT_HEIGHT; i++) {
			for (j = 0; j < VIDEO_FONT_BYTE_WIDTH; j++, p++)
				*p = ~*p;
			p += LCD_BYTE_WIDTH - VIDEO_FONT_BYTE_WIDTH;
		}
		p -= (LCD_BYTE_LINESZ - VIDEO_FONT_BYTE_WIDTH);

		if (x >= LCD_TEXT_WIDTH)
			break;
		x++;
	}
}

void sed156x_scroll_line(void)
{
	memmove(&sw_screen[0],
			&sw_screen[LCD_BYTE_LINESZ],
			LCD_BYTE_WIDTH * (LCD_HEIGHT - VIDEO_FONT_HEIGHT));
}

void sed156x_scroll(int dx, int dy)
{
	u8 *p1 = NULL, *p2 = NULL, *p3 = NULL;	/* pacify gcc */
	int adx, ady, i, sz;

	adx = dx > 0 ? dx : -dx;
	ady = dy > 0 ? dy : -dy;

	/* overscroll? erase everything */
	if (adx >= LCD_TEXT_WIDTH || ady >= LCD_TEXT_HEIGHT) {
		memset(sw_screen, 0, sizeof(sw_screen));
		return;
	}

	sz = LCD_BYTE_LINESZ * ady;
	if (dy > 0) {
		p1 = &sw_screen[0];
		p2 = &sw_screen[sz];
		p3 = &sw_screen[LCD_BYTE_WIDTH * LCD_HEIGHT - sz];
	} else if (dy < 0) {
		p1 = &sw_screen[sz];
		p2 = &sw_screen[0];
		p3 = &sw_screen[0];
	}

	if (ady > 0) {
		memmove(p1, p2, LCD_BYTE_WIDTH * LCD_HEIGHT - sz);
		memset(p3, 0, sz);
	}

	sz = VIDEO_FONT_BYTE_WIDTH * adx;
	if (dx > 0) {
		p1 = &sw_screen[0];
		p2 = &sw_screen[0] + sz;
		p3 = &sw_screen[0] + LCD_BYTE_WIDTH - sz;
	} else if (dx < 0) {
		p1 = &sw_screen[0] + sz;
		p2 = &sw_screen[0];
		p3 = &sw_screen[0];
	}

	/* xscroll */
	if (adx > 0) {
		for (i = 0; i < LCD_HEIGHT; i++) {
			memmove(p1, p2, LCD_BYTE_WIDTH - sz);
			memset(p3, 0, sz);
			p1 += LCD_BYTE_WIDTH;
			p2 += LCD_BYTE_WIDTH;
			p3 += LCD_BYTE_WIDTH;
		}
	}
}

void sed156x_init(void)
{
	int i;

	SED156X_CS(1);
	SED156X_A0(1);

	/* Send initialization commands to the LCD */
	sed156x_cmd_transfer(LCD_OFF);			/* Turn display OFF	  */
	sed156x_cmd_transfer(LCD_BIAS);			/* set the LCD Bias,	  */
	sed156x_cmd_transfer(LCD_ADCMODE);		/* ADC mode,		  */
	sed156x_cmd_transfer(LCD_COMDIR);		/* common output mode,	  */
	sed156x_cmd_transfer(LCD_RESRT | LCD_RRATIO);	/* resistor ratio,	  */
	sed156x_cmd_transfer(LCD_EVSET);		/* electronic volume,	  */
	sed156x_cmd_transfer(LCD_CNTRST);
	sed156x_cmd_transfer(LCD_PWRMD | LCD_POWERM);	/* and power mode	  */
	sed156x_cmd_transfer(LCD_PADDR | 0);		/* cursor home		  */
	sed156x_cmd_transfer(LCD_CADRH | 0);
	sed156x_cmd_transfer(LCD_CADRL | 0);
	sed156x_cmd_transfer(LCD_LADDR | 0);		/* and display start line */
	sed156x_cmd_transfer(LCD_DSP_NRM);		/* LCD display Normal	  */

	/* clear everything */
	memset(sw_screen, 0, sizeof(sw_screen));
	memset(hw_screen, 0, sizeof(hw_screen));
	memset(last_hw_screen, 0, sizeof(last_hw_screen));

	for (i = 0; i < LCD_PAGES; i++) {
		sed156x_cmd_transfer(LCD_PADDR | i);
		sed156x_cmd_transfer(LCD_CADRH | 0);
		sed156x_cmd_transfer(LCD_CADRL | 0);
		sed156x_data_block_transfer(&hw_screen[i][0], LCD_COLUMNS);
	}

	sed156x_clear();
	sed156x_sync();
	sed156x_cmd_transfer(LCD_ON);			/* Turn display ON	  */
}
