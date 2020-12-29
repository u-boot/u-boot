/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2018 Hannes Schmelzer <oe5hpm@oevsv.at> -
 * B&R Industrial Automation GmbH - http://www.br-automation.com
 */

#ifndef AM335X_FB_H
#define AM335X_FB_H

#if !CONFIG_IS_ENABLED(DM_VIDEO)

#define HSVS_CONTROL		BIT(25)	/*
					 * 0 = lcd_lp and lcd_fp are driven on
					 * opposite edges of pixel clock than
					 * the lcd_pixel_o
					 * 1 = lcd_lp and lcd_fp are driven
					 * according to bit 24 Note that this
					 * bit MUST be set to '0' for Passive
					 * Matrix displays the edge timing is
					 * fixed
					 */
#define HSVS_RISEFALL		BIT(24)	/*
					 * 0 = lcd_lp and lcd_fp are driven on
					 * the rising edge of pixel clock (bit
					 * 25 must be set to 1)
					 * 1 = lcd_lp and lcd_fp are driven on
					 * the falling edge of pixel clock (bit
					 * 25 must be set to 1)
					 */
#define DE_INVERT		BIT(23)	/*
					 * 0 = DE is low-active
					 * 1 = DE is high-active
					 */
#define PXCLK_INVERT		BIT(22)	/*
					 * 0 = pix-clk is high-active
					 * 1 = pic-clk is low-active
					 */
#define HSYNC_INVERT		BIT(21)	/*
					 * 0 = HSYNC is active high
					 * 1 = HSYNC is avtive low
					 */
#define VSYNC_INVERT		BIT(20)	/*
					 * 0 = VSYNC is active high
					 * 1 = VSYNC is active low
					 */

struct am335x_lcdpanel {
	unsigned int	hactive;	/* Horizontal active area */
	unsigned int	vactive;	/* Vertical active area */
	unsigned int	bpp;		/* bits per pixel */
	unsigned int	hfp;		/* Horizontal front porch */
	unsigned int	hbp;		/* Horizontal back porch */
	unsigned int	hsw;		/* Horizontal Sync Pulse Width */
	unsigned int	vfp;		/* Vertical front porch */
	unsigned int	vbp;		/* Vertical back porch */
	unsigned int	vsw;		/* Vertical Sync Pulse Width */
	unsigned int	pxl_clk;	/* Pixel clock */
	unsigned int	pol;		/* polarity of sync, clock signals */
	unsigned int	pup_delay;	/*
					 * time in ms after power on to
					 * initialization of lcd-controller
					 * (VCC ramp up time)
					 */
	unsigned int	pon_delay;	/*
					 * time in ms after initialization of
					 * lcd-controller (pic stabilization)
					 */
	void (*panel_power_ctrl)(int);	/* fp for power on/off display */
};

int am335xfb_init(struct am335x_lcdpanel *panel);

#else /* CONFIG_DM_VIDEO */

/**
 * tilcdc_panel_info: Panel parameters
 *
 * @ac_bias: AC Bias Pin Frequency
 * @ac_bias_intrpt: AC Bias Pin Transitions per Interrupt
 * @dma_burst_sz: DMA burst size
 * @bpp: Bits per pixel
 * @fdd: FIFO DMA Request Delay
 * @tft_alt_mode: TFT Alternative Signal Mapping (Only for active)
 * @invert_pxl_clk: Invert pixel clock
 * @sync_edge: Horizontal and Vertical Sync Edge: 0=rising 1=falling
 * @sync_ctrl: Horizontal and Vertical Sync: Control: 0=ignore
 * @raster_order: Raster Data Order Select: 1=Most-to-least 0=Least-to-most
 * @fifo_th: DMA FIFO threshold
 */
struct tilcdc_panel_info {
	u32 ac_bias;
	u32 ac_bias_intrpt;
	u32 dma_burst_sz;
	u32 bpp;
	u32 fdd;
	bool tft_alt_mode;
	bool invert_pxl_clk;
	u32 sync_edge;
	u32 sync_ctrl;
	u32 raster_order;
	u32 fifo_th;
};

#endif  /* CONFIG_DM_VIDEO */

#endif  /* AM335X_FB_H */
