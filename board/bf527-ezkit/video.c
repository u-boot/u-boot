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
#include <asm/mach-common/bits/dma.h>
#include <i2c.h>
#include <linux/types.h>
#include <stdio_dev.h>

int gunzip(void *, int, unsigned char *, unsigned long *);

#define DMA_SIZE16	2

#include <asm/mach-common/bits/ppi.h>
#include <asm/mach-common/bits/timer.h>

#include <asm/bfin_logo_230x230.h>

#define LCD_X_RES		320	/* Horizontal Resolution */
#define LCD_Y_RES		240	/* Vertical Resolution */
#define LCD_BPP			24	/* Bit Per Pixel */
#define LCD_PIXEL_SIZE		(LCD_BPP / 8)

#define	DMA_BUS_SIZE		16
#define	LCD_CLK         	(12*1000*1000)	/* 12MHz */

#define CLOCKS_PER_PIX		3

/* HS and VS timing parameters (all in number of PPI clk ticks) */
#define H_ACTPIX	(LCD_X_RES * CLOCKS_PER_PIX)	/* active horizontal pixel */
#define H_PERIOD	(408 * CLOCKS_PER_PIX)		/* HS period */
#define H_PULSE		90				/* HS pulse width */
#define H_START		204				/* first valid pixel */

#define U_LINE		1				/* Blanking Lines */

#define	V_LINES		(LCD_Y_RES + U_LINE)		/* total vertical lines */
#define V_PULSE		(3 * H_PERIOD)			/* VS pulse width (1-5 H_PERIODs) */
#define V_PERIOD	(H_PERIOD * V_LINES)		/* VS period */

#define ACTIVE_VIDEO_MEM_OFFSET	(U_LINE * H_ACTPIX)

#define PPI_TX_MODE		0x2
#define PPI_XFER_TYPE_11	0xC
#define PPI_PORT_CFG_01		0x10
#define PPI_PACK_EN		0x80
#define PPI_POLS_1		0x8000

/* enable and disable PPI functions */
void EnablePPI(void)
{
	*pPPI_CONTROL |= PORT_EN;
}

void DisablePPI(void)
{
	*pPPI_CONTROL &= ~PORT_EN;
}

void Init_Ports(void)
{
	*pPORTF_MUX &= ~PORT_x_MUX_0_MASK;
	*pPORTF_MUX |= PORT_x_MUX_0_FUNC_1;
	*pPORTF_FER |= PF0 | PF1 | PF2 | PF3 | PF4 | PF5 | PF6 | PF7;

	*pPORTG_MUX &= ~PORT_x_MUX_1_MASK;
	*pPORTG_MUX |= PORT_x_MUX_1_FUNC_1;
	*pPORTG_FER |= PG5;
}

void Init_PPI(void)
{

	*pPPI_DELAY = H_START;
	*pPPI_COUNT = (H_ACTPIX-1);
	*pPPI_FRAME = 0;

	/* PPI control, to be replaced with definitions */
	*pPPI_CONTROL = PPI_TX_MODE		|	/* output mode , PORT_DIR */
			PPI_XFER_TYPE_11	|	/* sync mode XFR_TYPE */
			PPI_PORT_CFG_01		|	/* two frame sync PORT_CFG */
			PPI_PACK_EN		|	/* packing enabled PACK_EN */
			PPI_POLS_1;			/* faling edge syncs POLS */
}

void Init_DMA(void *dst)
{
	*pDMA0_START_ADDR = dst;

	/* X count */
	*pDMA0_X_COUNT = H_ACTPIX / 2;
	*pDMA0_X_MODIFY = DMA_BUS_SIZE / 8;

	/* Y count */
	*pDMA0_Y_COUNT = V_LINES;
	*pDMA0_Y_MODIFY = DMA_BUS_SIZE / 8;

	/* DMA Config */
	*pDMA0_CONFIG =
		WDSIZE_16	|	/* 16 bit DMA */
		DMA2D 		|	/* 2D DMA */
		FLOW_AUTO;		/* autobuffer mode */
}


void EnableDMA(void)
{
	*pDMA0_CONFIG |= DMAEN;
}

void DisableDMA(void)
{
	*pDMA0_CONFIG &= ~DMAEN;
}


/* Init TIMER0 as Frame Sync 1 generator */
void InitTIMER0(void)
{
	*pTIMER_DISABLE |= TIMDIS0;			/* disable Timer */
	SSYNC();
	*pTIMER_STATUS  |= TIMIL0 | TOVF_ERR0 | TRUN0;	/* clear status */
	SSYNC();

	*pTIMER0_PERIOD  = H_PERIOD;
	SSYNC();
	*pTIMER0_WIDTH   = H_PULSE;
	SSYNC();

	*pTIMER0_CONFIG  = PWM_OUT |
				PERIOD_CNT   |
				TIN_SEL      |
				CLK_SEL      |
				EMU_RUN;
	SSYNC();
}

void EnableTIMER0(void)
{
	*pTIMER_ENABLE  |= TIMEN0;
	SSYNC();
}

void DisableTIMER0(void)
{
	*pTIMER_DISABLE  |= TIMDIS0;
	SSYNC();
}


void InitTIMER1(void)
{
	*pTIMER_DISABLE |= TIMDIS1;			/* disable Timer */
	SSYNC();
	*pTIMER_STATUS  |= TIMIL1 | TOVF_ERR1 | TRUN1;	/* clear status */
	SSYNC();


	*pTIMER1_PERIOD  = V_PERIOD;
	SSYNC();
	*pTIMER1_WIDTH   = V_PULSE;
	SSYNC();

	*pTIMER1_CONFIG  = PWM_OUT |
				PERIOD_CNT   |
				TIN_SEL      |
				CLK_SEL      |
				EMU_RUN;
	SSYNC();
}

void EnableTIMER1(void)
{
	*pTIMER_ENABLE  |= TIMEN1;
	SSYNC();
}

void DisableTIMER1(void)
{
	*pTIMER_DISABLE  |= TIMDIS1;
	SSYNC();
}

int video_init(void *dst)
{

	Init_Ports();
	Init_DMA(dst);
	EnableDMA();
	InitTIMER0();
	InitTIMER1();
	Init_PPI();
	EnablePPI();

	/* Frame sync 2 (VS) needs to start at least one PPI clk earlier */
	EnableTIMER1();
	/* Add Some Delay ... */
	SSYNC();
	SSYNC();
	SSYNC();
	SSYNC();

	/* now start frame sync 1 */
	EnableTIMER0();

	return 0;
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

void video_putc(const char c)
{
}

void video_puts(const char *s)
{
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
	if (gunzip(data, bfin_logo.size, bfin_logo.data, &src_len)) {
		puts("Failed to decompress logo\n");
		free(dst);
		return -1;
	}
	bfin_logo.data = data;
#endif

	memset(dst + ACTIVE_VIDEO_MEM_OFFSET, bfin_logo.data[0], fbmem_size - ACTIVE_VIDEO_MEM_OFFSET);

	dma_bitblit(dst + ACTIVE_VIDEO_MEM_OFFSET, &bfin_logo,
			(LCD_X_RES - bfin_logo.width) / 2,
			(LCD_Y_RES - bfin_logo.height) / 2);

	video_init(dst);		/* Video initialization */

	memset(&videodev, 0, sizeof(videodev));

	strcpy(videodev.name, "video");
	videodev.ext = DEV_EXT_VIDEO;	/* Video extensions */
	videodev.flags = DEV_FLAGS_SYSTEM;	/* No Output */
	videodev.putc = video_putc;	/* 'putc' function */
	videodev.puts = video_puts;	/* 'puts' function */

	error = stdio_register(&videodev);

	return (error == 0) ? devices : error;
}
