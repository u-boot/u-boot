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
#include <asm/portmux.h>
#include <asm/mach-common/bits/dma.h>
#include <spi.h>
#include <linux/types.h>
#include <stdio_dev.h>

#include <asm/mach-common/bits/ppi.h>
#include <asm/mach-common/bits/timer.h>

#define LCD_X_RES		320	/* Horizontal Resolution */
#define LCD_Y_RES		240	/* Vertical Resolution */
#define DMA_BUS_SIZE		16

#ifdef CONFIG_BF527_EZKIT_REV_2_1 /* lq035q1 */

#if !defined(CONFIG_LQ035Q1_USE_RGB888_8_BIT_PPI) && \
    !defined(CONFIG_LQ035Q1_USE_RGB565_8_BIT_PPI)
# define CONFIG_LQ035Q1_USE_RGB565_8_BIT_PPI
#endif

/* Interface 16/18-bit TFT over an 8-bit wide PPI using a
 * small Programmable Logic Device (CPLD)
 * http://blackfin.uclinux.org/gf/project/stamp/frs/?action=FrsReleaseBrowse&frs_package_id=165
 */

#ifdef CONFIG_LQ035Q1_USE_RGB565_8_BIT_PPI
#include <asm/bfin_logo_rgb565_230x230.h>
#define LCD_BPP		16	/* Bit Per Pixel */
#define CLOCKS_PPIX	2	/* Clocks per pixel */
#define CPLD_DELAY	3	/* RGB565 pipeline delay */
#endif

#ifdef CONFIG_LQ035Q1_USE_RGB888_8_BIT_PPI
#include <asm/bfin_logo_230x230.h>
#define LCD_BPP		24	/* Bit Per Pixel */
#define CLOCKS_PPIX	3	/* Clocks per pixel */
#define CPLD_DELAY	5	/* RGB888 pipeline delay */
#endif

/*
 * HS and VS timing parameters (all in number of PPI clk ticks)
 */

#define H_ACTPIX	(LCD_X_RES * CLOCKS_PPIX)	/* active horizontal pixel */
#define H_PERIOD	(336 * CLOCKS_PPIX)		/* HS period */
#define H_PULSE		(2 * CLOCKS_PPIX)		/* HS pulse width */
#define H_START		(7 * CLOCKS_PPIX + CPLD_DELAY)	/* first valid pixel */

#define U_LINE		4				/* Blanking Lines */

#define V_LINES		(LCD_Y_RES + U_LINE)		/* total vertical lines */
#define V_PULSE		(2 * CLOCKS_PPIX)		/* VS pulse width (1-5 H_PERIODs) */
#define V_PERIOD	(H_PERIOD * V_LINES)		/* VS period */

#define ACTIVE_VIDEO_MEM_OFFSET	((U_LINE / 2) * LCD_X_RES * (LCD_BPP / 8))

/*
 * LCD Modes
 */
#define LQ035_RL	(0 << 8)	/* Right -> Left Scan */
#define LQ035_LR	(1 << 8)	/* Left -> Right Scan */
#define LQ035_TB	(1 << 9)	/* Top -> Botton Scan */
#define LQ035_BT	(0 << 9)	/* Botton -> Top Scan */
#define LQ035_BGR	(1 << 11)	/* Use BGR format */
#define LQ035_RGB	(0 << 11)	/* Use RGB format */
#define LQ035_NORM	(1 << 13)	/* Reversal */
#define LQ035_REV	(0 << 13)	/* Reversal */

#define LQ035_INDEX			0x74
#define LQ035_DATA			0x76

#define LQ035_DRIVER_OUTPUT_CTL		0x1
#define LQ035_SHUT_CTL			0x11

#define LQ035_DRIVER_OUTPUT_MASK	(LQ035_LR | LQ035_TB | LQ035_BGR | LQ035_REV)
#define LQ035_DRIVER_OUTPUT_DEFAULT	(0x2AEF & ~LQ035_DRIVER_OUTPUT_MASK)

#define LQ035_SHUT			(1 << 0)	/* Shutdown */
#define LQ035_ON			(0 << 0)	/* Shutdown */

#ifndef CONFIG_LQ035Q1_LCD_MODE
#define CONFIG_LQ035Q1_LCD_MODE		(LQ035_NORM | LQ035_RL | LQ035_TB | LQ035_BGR)
#endif

#else /* t350mcqb */
#include <asm/bfin_logo_230x230.h>

#define LCD_BPP		24	/* Bit Per Pixel */
#define CLOCKS_PPIX	3	/* Clocks per pixel */

/* HS and VS timing parameters (all in number of PPI clk ticks) */
#define H_ACTPIX	(LCD_X_RES * CLOCKS_PPIX)	/* active horizontal pixel */
#define H_PERIOD	(408 * CLOCKS_PPIX)		/* HS period */
#define H_PULSE		90				/* HS pulse width */
#define H_START		204				/* first valid pixel */

#define U_LINE		1				/* Blanking Lines */

#define V_LINES		(LCD_Y_RES + U_LINE)		/* total vertical lines */
#define V_PULSE		(3 * H_PERIOD)			/* VS pulse width (1-5 H_PERIODs) */
#define V_PERIOD	(H_PERIOD * V_LINES)		/* VS period */

#define ACTIVE_VIDEO_MEM_OFFSET	(U_LINE * H_ACTPIX)
#endif

#define LCD_PIXEL_SIZE		(LCD_BPP / 8)
#define DMA_SIZE16		2

#define PPI_TX_MODE		0x2
#define PPI_XFER_TYPE_11	0xC
#define PPI_PORT_CFG_01		0x10
#define PPI_PACK_EN		0x80
#define PPI_POLS_1		0x8000

#ifdef CONFIG_BF527_EZKIT_REV_2_1
static struct spi_slave *slave;
static int lq035q1_control(unsigned char reg, unsigned short value)
{
	int ret;
	u8 regs[3] = {LQ035_INDEX, 0, 0};
	u8 data[3] = {LQ035_DATA, 0, 0};
	u8 dummy[3];

	regs[2] = reg;
	data[1] = value >> 8;
	data[2] = value & 0xFF;

	if (!slave) {
		/* FIXME: Verify the max SCK rate */
		slave = spi_setup_slave(CONFIG_LQ035Q1_SPI_BUS,
				CONFIG_LQ035Q1_SPI_CS, 20000000,
				SPI_MODE_3);
		if (!slave)
			return -1;
	}

	if (spi_claim_bus(slave))
		return -1;

	ret = spi_xfer(slave, 24, regs, dummy, SPI_XFER_BEGIN | SPI_XFER_END);
	ret |= spi_xfer(slave, 24, data, dummy, SPI_XFER_BEGIN | SPI_XFER_END);

	spi_release_bus(slave);

	return ret;
}
#endif

/* enable and disable PPI functions */
void EnablePPI(void)
{
	bfin_write_PPI_CONTROL(bfin_read_PPI_CONTROL() | PORT_EN);
}

void DisablePPI(void)
{
	bfin_write_PPI_CONTROL(bfin_read_PPI_CONTROL() & ~PORT_EN);
}

void Init_Ports(void)
{
	const unsigned short pins[] = {
		P_PPI0_D0, P_PPI0_D1, P_PPI0_D2, P_PPI0_D3, P_PPI0_D4,
		P_PPI0_D5, P_PPI0_D6, P_PPI0_D7, P_PPI0_FS2, 0,
	};
	peripheral_request_list(pins, "lcd");
}

void Init_PPI(void)
{

	bfin_write_PPI_DELAY(H_START);
	bfin_write_PPI_COUNT(H_ACTPIX - 1);
	bfin_write_PPI_FRAME(V_LINES);

	/* PPI control, to be replaced with definitions */
	bfin_write_PPI_CONTROL(
			PPI_TX_MODE		|	/* output mode , PORT_DIR */
			PPI_XFER_TYPE_11	|	/* sync mode XFR_TYPE */
			PPI_PORT_CFG_01		|	/* two frame sync PORT_CFG */
			PPI_PACK_EN		|	/* packing enabled PACK_EN */
			PPI_POLS_1			/* faling edge syncs POLS */
	);
}

void Init_DMA(void *dst)
{
	bfin_write_DMA0_START_ADDR(dst);

	/* X count */
	bfin_write_DMA0_X_COUNT(H_ACTPIX / 2);
	bfin_write_DMA0_X_MODIFY(DMA_BUS_SIZE / 8);

	/* Y count */
	bfin_write_DMA0_Y_COUNT(V_LINES);
	bfin_write_DMA0_Y_MODIFY(DMA_BUS_SIZE / 8);

	/* DMA Config */
	bfin_write_DMA0_CONFIG(
		WDSIZE_16	|	/* 16 bit DMA */
		DMA2D 		|	/* 2D DMA */
		FLOW_AUTO		/* autobuffer mode */
	);
}

void EnableDMA(void)
{
	bfin_write_DMA0_CONFIG(bfin_read_DMA0_CONFIG() | DMAEN);
}

void DisableDMA(void)
{
	bfin_write_DMA0_CONFIG(bfin_read_DMA0_CONFIG() & ~DMAEN);
}

/* Init TIMER0 as Frame Sync 1 generator */
void InitTIMER0(void)
{
	bfin_write_TIMER_DISABLE(TIMDIS0);			/* disable Timer */
	SSYNC();
	bfin_write_TIMER_STATUS(TIMIL0 | TOVF_ERR0 | TRUN0);	/* clear status */
	SSYNC();

	bfin_write_TIMER0_PERIOD(H_PERIOD);
	SSYNC();
	bfin_write_TIMER0_WIDTH(H_PULSE);
	SSYNC();

	bfin_write_TIMER0_CONFIG(
				PWM_OUT |
				PERIOD_CNT   |
				TIN_SEL      |
				CLK_SEL      |
				EMU_RUN
	);
	SSYNC();
}

void EnableTIMER0(void)
{
	bfin_write_TIMER_ENABLE(TIMEN0);
	SSYNC();
}

void DisableTIMER0(void)
{
	bfin_write_TIMER_DISABLE(TIMDIS0);
	SSYNC();
}


void InitTIMER1(void)
{
	bfin_write_TIMER_DISABLE(TIMDIS1);			/* disable Timer */
	SSYNC();
	bfin_write_TIMER_STATUS(TIMIL1 | TOVF_ERR1 | TRUN1);	/* clear status */
	SSYNC();

	bfin_write_TIMER1_PERIOD(V_PERIOD);
	SSYNC();
	bfin_write_TIMER1_WIDTH(V_PULSE);
	SSYNC();

	bfin_write_TIMER1_CONFIG(
				PWM_OUT |
				PERIOD_CNT   |
				TIN_SEL      |
				CLK_SEL      |
				EMU_RUN
	);
	SSYNC();
}

void EnableTIMER1(void)
{
	bfin_write_TIMER_ENABLE(TIMEN1);
	SSYNC();
}

void DisableTIMER1(void)
{
	bfin_write_TIMER_DISABLE(TIMDIS1);
	SSYNC();
}

void EnableTIMER12(void)
{
	bfin_write_TIMER_ENABLE(TIMEN1 | TIMEN0);
	SSYNC();
}

int video_init(void *dst)
{

#ifdef CONFIG_BF527_EZKIT_REV_2_1
	lq035q1_control(LQ035_SHUT_CTL, LQ035_ON);
	lq035q1_control(LQ035_DRIVER_OUTPUT_CTL, (CONFIG_LQ035Q1_LCD_MODE &
		LQ035_DRIVER_OUTPUT_MASK) | LQ035_DRIVER_OUTPUT_DEFAULT);
#endif
	Init_Ports();
	Init_DMA(dst);
	EnableDMA();
	InitTIMER0();
	InitTIMER1();
	Init_PPI();
	EnablePPI();

#ifdef CONFIG_BF527_EZKIT_REV_2_1
	EnableTIMER12();
#else
	/* Frame sync 2 (VS) needs to start at least one PPI clk earlier */
	EnableTIMER1();
	/* Add Some Delay ... */
	SSYNC();
	SSYNC();
	SSYNC();
	SSYNC();

	/* now start frame sync 1 */
	EnableTIMER0();
#endif

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

void video_stop(void)
{
	DisablePPI();
	DisableDMA();
	DisableTIMER0();
	DisableTIMER1();
#ifdef CONFIG_BF527_EZKIT_REV_2_1
	lq035q1_control(LQ035_SHUT_CTL, LQ035_SHUT);
#endif
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
