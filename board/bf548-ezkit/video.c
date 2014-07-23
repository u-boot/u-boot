/*
 * video.c - run splash screen on lcd
 *
 * Copyright (c) 2007-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <stdarg.h>
#include <common.h>
#include <config.h>
#include <malloc.h>
#include <asm/blackfin.h>
#include <asm/gpio.h>
#include <asm/portmux.h>
#include <asm/mach-common/bits/dma.h>
#include <i2c.h>
#include <linux/types.h>
#include <stdio_dev.h>

#include <lzma/LzmaTypes.h>
#include <lzma/LzmaDec.h>
#include <lzma/LzmaTools.h>

#define DMA_SIZE16	2

#include <asm/mach-common/bits/eppi.h>

#include EASYLOGO_HEADER

#define LCD_X_RES		480	/*Horizontal Resolution */
#define LCD_Y_RES		272	/* Vertical Resolution */

#define LCD_BPP			24	/* Bit Per Pixel */
#define LCD_PIXEL_SIZE		(LCD_BPP / 8)
#define	DMA_BUS_SIZE		32
#define ACTIVE_VIDEO_MEM_OFFSET 0

/* 	-- Horizontal synchronizing --
 *
 * Timing characteristics taken from the SHARP LQ043T1DG01 datasheet
 * (LCY-W-06602A Page 9 of 22)
 *
 * Clock Frequency 	1/Tc Min 7.83 Typ 9.00 Max 9.26 MHz
 *
 * Period 		TH - 525 - Clock
 * Pulse width 		THp - 41 - Clock
 * Horizontal period 	THd - 480 - Clock
 * Back porch 		THb - 2 - Clock
 * Front porch 		THf - 2 - Clock
 *
 * -- Vertical synchronizing --
 * Period 		TV - 286 - Line
 * Pulse width 		TVp - 10 - Line
 * Vertical period 	TVd - 272 - Line
 * Back porch 		TVb - 2 - Line
 * Front porch 		TVf - 2 - Line
 */

#define	LCD_CLK         	(8*1000*1000)	/* 8MHz */

/* # active data to transfer after Horizontal Delay clock */
#define EPPI_HCOUNT		LCD_X_RES

/* # active lines to transfer after Vertical Delay clock */
#define EPPI_VCOUNT		LCD_Y_RES

/* Samples per Line = 480 (active data) + 45 (padding) */
#define EPPI_LINE		525

/* Lines per Frame = 272 (active data) + 14 (padding) */
#define EPPI_FRAME		286

/* FS1 (Hsync) Width (Typical)*/
#define EPPI_FS1W_HBL		41

/* FS1 (Hsync) Period (Typical) */
#define EPPI_FS1P_AVPL		EPPI_LINE

/* Horizontal Delay clock after assertion of Hsync (Typical) */
#define EPPI_HDELAY		43

/* FS2 (Vsync) Width    = FS1 (Hsync) Period * 10 */
#define EPPI_FS2W_LVB		(EPPI_LINE * 10)

 /* FS2 (Vsync) Period   = FS1 (Hsync) Period * Lines per Frame */
#define EPPI_FS2P_LAVF		(EPPI_LINE * EPPI_FRAME)

/* Vertical Delay after assertion of Vsync (2 Lines) */
#define EPPI_VDELAY		12

#define EPPI_CLIP		0xFF00FF00

/* EPPI Control register configuration value for RGB out
 * - EPPI as Output
 * GP 2 frame sync mode,
 * Internal Clock generation disabled, Internal FS generation enabled,
 * Receives samples on EPPI_CLK raising edge, Transmits samples on EPPI_CLK falling edge,
 * FS1 & FS2 are active high,
 * DLEN = 6 (24 bits for RGB888 out) or 5 (18 bits for RGB666 out)
 * DMA Unpacking disabled when RGB Formating is enabled, otherwise DMA unpacking enabled
 * Swapping Enabled,
 * One (DMA) Channel Mode,
 * RGB Formatting Enabled for RGB666 output, disabled for RGB888 output
 * Regular watermark - when FIFO is 100% full,
 * Urgent watermark - when FIFO is 75% full
 */

#define EPPI_CONTROL		(0x20136E2E)

static inline u16 get_eppi_clkdiv(u32 target_ppi_clk)
{
	u32 sclk = get_sclk();

	/* EPPI_CLK = (SCLK) / (2 * (EPPI_CLKDIV[15:0] + 1)) */

	return (((sclk / target_ppi_clk) / 2) - 1);
}

void Init_PPI(void)
{
	u16 eppi_clkdiv = get_eppi_clkdiv(LCD_CLK);

	bfin_write_EPPI0_FS1W_HBL(EPPI_FS1W_HBL);
	bfin_write_EPPI0_FS1P_AVPL(EPPI_FS1P_AVPL);
	bfin_write_EPPI0_FS2W_LVB(EPPI_FS2W_LVB);
	bfin_write_EPPI0_FS2P_LAVF(EPPI_FS2P_LAVF);
	bfin_write_EPPI0_CLIP(EPPI_CLIP);

	bfin_write_EPPI0_FRAME(EPPI_FRAME);
	bfin_write_EPPI0_LINE(EPPI_LINE);

	bfin_write_EPPI0_HCOUNT(EPPI_HCOUNT);
	bfin_write_EPPI0_HDELAY(EPPI_HDELAY);
	bfin_write_EPPI0_VCOUNT(EPPI_VCOUNT);
	bfin_write_EPPI0_VDELAY(EPPI_VDELAY);

	bfin_write_EPPI0_CLKDIV(eppi_clkdiv);

/*
 * DLEN = 6 (24 bits for RGB888 out) or 5 (18 bits for RGB666 out)
 * RGB Formatting Enabled for RGB666 output, disabled for RGB888 output
 */
#if defined(CONFIG_VIDEO_RGB666)
		bfin_write_EPPI0_CONTROL((EPPI_CONTROL & ~DLENGTH) | DLEN_18 |
					 RGB_FMT_EN);
#else
		bfin_write_EPPI0_CONTROL(((EPPI_CONTROL & ~DLENGTH) | DLEN_24) &
					 ~RGB_FMT_EN);
#endif

}

#define               DEB2_URGENT  0x2000     /* DEB2 Urgent */

void Init_DMA(void *dst)
{

#if defined(CONFIG_DEB_DMA_URGENT)
	bfin_write_EBIU_DDRQUE(bfin_read_EBIU_DDRQUE() | DEB2_URGENT);
#endif

	bfin_write_DMA12_START_ADDR(dst);

	/* X count */
	bfin_write_DMA12_X_COUNT((LCD_X_RES * LCD_BPP) / DMA_BUS_SIZE);
	bfin_write_DMA12_X_MODIFY(DMA_BUS_SIZE / 8);

	/* Y count */
	bfin_write_DMA12_Y_COUNT(LCD_Y_RES);
	bfin_write_DMA12_Y_MODIFY(DMA_BUS_SIZE / 8);

	/* DMA Config */
	bfin_write_DMA12_CONFIG(
		WDSIZE_32	|	/* 32 bit DMA */
		DMA2D 		|	/* 2D DMA */
		FLOW_AUTO		/* autobuffer mode */
	);
}

void Init_Ports(void)
{
	const unsigned short pins[] = {
		P_PPI0_D0, P_PPI0_D1, P_PPI0_D2, P_PPI0_D3, P_PPI0_D4,
		P_PPI0_D5, P_PPI0_D6, P_PPI0_D7, P_PPI0_D8, P_PPI0_D9,
		P_PPI0_D10, P_PPI0_D11, P_PPI0_D12, P_PPI0_D13, P_PPI0_D14,
		P_PPI0_D15, P_PPI0_D16, P_PPI0_D17,
#if !defined(CONFIG_VIDEO_RGB666)
		P_PPI0_D18, P_PPI0_D19, P_PPI0_D20, P_PPI0_D21, P_PPI0_D22,
		P_PPI0_D23,
#endif
		P_PPI0_CLK, P_PPI0_FS1, P_PPI0_FS2, 0,
	};
	peripheral_request_list(pins, "lcd");

	gpio_request(GPIO_PE3, "lcd-disp");
	gpio_direction_output(GPIO_PE3, 1);
}

void EnableDMA(void)
{
	bfin_write_DMA12_CONFIG(bfin_read_DMA12_CONFIG() | DMAEN);
}

void DisableDMA(void)
{
	bfin_write_DMA12_CONFIG(bfin_read_DMA12_CONFIG() & ~DMAEN);
}

/* enable and disable PPI functions */
void EnablePPI(void)
{
	bfin_write_EPPI0_CONTROL(bfin_read_EPPI0_CONTROL() | EPPI_EN);
}

void DisablePPI(void)
{
	bfin_write_EPPI0_CONTROL(bfin_read_EPPI0_CONTROL() & ~EPPI_EN);
}

int video_init(void *dst)
{
	Init_Ports();
	Init_DMA(dst);
	EnableDMA();
	Init_PPI();
	EnablePPI();

	return 0;
}

void video_stop(void)
{
	DisablePPI();
	DisableDMA();
}

static void dma_bitblit(void *dst, fastimage_t *logo, int x, int y)
{
	if (dcache_status())
		blackfin_dcache_flush_range(logo->data, logo->data + logo->size);

	bfin_write_MDMA_D0_IRQ_STATUS(DMA_DONE | DMA_ERR);

	/* Setup destination start address */
	bfin_write_MDMA_D0_START_ADDR(dst + ((x & -2) * LCD_PIXEL_SIZE)
					+ (y * LCD_X_RES * LCD_PIXEL_SIZE));
	/* Setup destination xcount */
	bfin_write_MDMA_D0_X_COUNT(logo->width * LCD_PIXEL_SIZE / DMA_SIZE16);
	/* Setup destination xmodify */
	bfin_write_MDMA_D0_X_MODIFY(DMA_SIZE16);

	/* Setup destination ycount */
	bfin_write_MDMA_D0_Y_COUNT(logo->height);
	/* Setup destination ymodify */
	bfin_write_MDMA_D0_Y_MODIFY((LCD_X_RES - logo->width) * LCD_PIXEL_SIZE + DMA_SIZE16);


	/* Setup Source start address */
	bfin_write_MDMA_S0_START_ADDR(logo->data);
	/* Setup Source xcount */
	bfin_write_MDMA_S0_X_COUNT(logo->width * LCD_PIXEL_SIZE / DMA_SIZE16);
	/* Setup Source xmodify */
	bfin_write_MDMA_S0_X_MODIFY(DMA_SIZE16);

	/* Setup Source ycount */
	bfin_write_MDMA_S0_Y_COUNT(logo->height);
	/* Setup Source ymodify */
	bfin_write_MDMA_S0_Y_MODIFY(DMA_SIZE16);


	/* Enable source DMA */
	bfin_write_MDMA_S0_CONFIG(DMAEN | WDSIZE_16 | DMA2D);
	SSYNC();
	bfin_write_MDMA_D0_CONFIG(WNR | DMAEN  | WDSIZE_16 | DMA2D);

	while (bfin_read_MDMA_D0_IRQ_STATUS() & DMA_RUN);

	bfin_write_MDMA_S0_IRQ_STATUS(bfin_read_MDMA_S0_IRQ_STATUS() | DMA_DONE | DMA_ERR);
	bfin_write_MDMA_D0_IRQ_STATUS(bfin_read_MDMA_D0_IRQ_STATUS() | DMA_DONE | DMA_ERR);

}

int drv_video_init(void)
{
	int error, devices = 1;
	struct stdio_dev videodev;

	u8 *dst;
	u32 fbmem_size = LCD_X_RES * LCD_Y_RES * LCD_PIXEL_SIZE + ACTIVE_VIDEO_MEM_OFFSET;

	dst = malloc(fbmem_size);

	if (dst == NULL) {
		printf("Failed to alloc FB memory\n");
		return -1;
	}

#ifdef EASYLOGO_ENABLE_GZIP
	unsigned char *data = EASYLOGO_DECOMP_BUFFER;
	unsigned long src_len = EASYLOGO_ENABLE_GZIP;
	error = gunzip(data, bfin_logo.size, bfin_logo.data, &src_len);
	bfin_logo.data = data;
#elif defined(EASYLOGO_ENABLE_LZMA)
	unsigned char *data = EASYLOGO_DECOMP_BUFFER;
	SizeT lzma_len = bfin_logo.size;
	error = lzmaBuffToBuffDecompress(data, &lzma_len,
		bfin_logo.data, EASYLOGO_ENABLE_LZMA);
	bfin_logo.data = data;
#else
	error = 0;
#endif

	if (error) {
		puts("Failed to decompress logo\n");
		free(dst);
		return -1;
	}

	memset(dst + ACTIVE_VIDEO_MEM_OFFSET, bfin_logo.data[0], fbmem_size - ACTIVE_VIDEO_MEM_OFFSET);

	dma_bitblit(dst + ACTIVE_VIDEO_MEM_OFFSET, &bfin_logo,
			(LCD_X_RES - bfin_logo.width) / 2,
			(LCD_Y_RES - bfin_logo.height) / 2);

	video_init(dst);		/* Video initialization */

	memset(&videodev, 0, sizeof(videodev));

	strcpy(videodev.name, "video");
	videodev.ext = DEV_EXT_VIDEO;	/* Video extensions */
	videodev.flags = DEV_FLAGS_SYSTEM;	/* No Output */

	error = stdio_register(&videodev);

	return (error == 0) ? devices : error;
}
