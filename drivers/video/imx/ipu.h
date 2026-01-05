/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Code fixes:
 *
 * (C) Copyright 2025
 * Brian Ruley, GE HealthCare, brian.ruley@gehealthcare.com
 *
 * Porting to u-boot:
 *
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * Linux IPU driver for MX51:
 *
 * (C) Copyright 2005-2010 Freescale Semiconductor, Inc.
 */

#ifndef __ASM_ARCH_IPU_H__
#define __ASM_ARCH_IPU_H__

#include <ipu_pixfmt.h>
#include <linux/types.h>

#define IDMA_CHAN_INVALID 0xFF
#define HIGH_RESOLUTION_WIDTH 1024

struct ipu_ctx;
struct ipu_di_config;

struct clk {
	const char *name;
	int id;
	/* The IPU context of this clock */
	struct ipu_ctx *ctx;
	/* Source clock this clk depends on */
	struct clk *parent;
	/* Secondary clock to enable/disable with this clock */
	struct clk *secondary;
	/* Current clock rate */
	unsigned long rate;
	/* Reference count of clock enable/disable */
	__s8 usecount;
	/* Register bit position for clock's enable/disable control. */
	u8 enable_shift;
	/* Register address for clock's enable/disable control. */
	void *enable_reg;
	u32 flags;
	/*
	 * Function ptr to recalculate the clock's rate based on parent
	 * clock's rate
	 */
	void (*recalc)(struct clk *clk);
	/*
	 * Function ptr to set the clock to a new rate. The rate must match a
	 * supported rate returned from round_rate. Leave blank if clock is not
	* programmable
	 */
	int (*set_rate)(struct clk *clk, unsigned long rate);
	/*
	 * Function ptr to round the requested clock rate to the nearest
	 * supported rate that is less than or equal to the requested rate.
	 */
	unsigned long (*round_rate)(struct clk *clk, unsigned long rate);
	/*
	 * Function ptr to enable the clock. Leave blank if clock can not
	 * be gated.
	 */
	int (*enable)(struct clk *clk);
	/*
	 * Function ptr to disable the clock. Leave blank if clock can not
	 * be gated.
	 */
	void (*disable)(struct clk *clk);
	/* Function ptr to set the parent clock of the clock. */
	int (*set_parent)(struct clk *clk, struct clk *parent);
};

struct udevice;

/*
 * Per-IPU context used by ipu_common to manage clocks and channel state.
 * Lifetime is owned by the IPU DM driver
 */
struct ipu_ctx {
	struct udevice *dev;
	int dev_id;

	struct clk *ipu_clk;
	struct clk *ldb_clk;
	unsigned char ipu_clk_enabled;
	struct clk *di_clk[2];
	struct clk *pixel_clk[2];

	u8 dc_di_assignment[10];
	u32 channel_init_mask;
	u32 channel_enable_mask;

	int ipu_dc_use_count;
	int ipu_dp_use_count;
	int ipu_dmfc_use_count;
	int ipu_di_use_count[2];
};

/**
 * @disp:	    The DI the panel is attached to.
 * @pixel_clk_rate: Desired pixel clock frequency in Hz.
 * @pixel_fmt:	    Input parameter for pixel format of buffer.
 *		    Pixel format is a FOURCC ASCII code.
 * @width:	    The width of panel in pixels.
 * @height:	    The height of panel in pixels.
 * @h_start_width:  The number of pixel clocks between the HSYNC
 *		    signal pulse and the start of valid data.
 * @h_sync_width:   The width of the HSYNC signal in units of pixel
 *		    clocks.
 * @h_end_width:    The number of pixel clocks between the end of
 *		    valid data and the HSYNC signal for next line.
 * @v_start_width:  The number of lines between the VSYNC
 *		    signal pulse and the start of valid data.
 * @v_sync_width:   The width of the VSYNC signal in units of lines
 * @v_end_width:    The number of lines between the end of valid
 *		    data and the VSYNC signal for next frame.
 * @ctx:	    The IPU context of the display.
 */
struct ipu_di_config {
	int disp;
	u32 pixel_clk_rate;
	u32 pixel_fmt;
	u16 width;
	u16 height;
	u16 h_start_width;
	u16 h_sync_width;
	u16 h_end_width;
	u16 v_start_width;
	u16 v_sync_width;
	u16 v_end_width;
	u32 v_to_h_sync;

	struct ipu_ctx *ctx;
};

/*
 * Enumeration of Synchronous (Memory-less) panel types
 */
typedef enum {
	IPU_PANEL_SHARP_TFT,
	IPU_PANEL_TFT,
} ipu_panel_t;

/*
 * IPU Driver channels definitions.
 * Note these are different from IDMA channels
 */
#define IPU_MAX_CH 32
#define _MAKE_CHAN(num, v_in, g_in, a_in, out) \
	((num << 24) | (v_in << 18) | (g_in << 12) | (a_in << 6) | out)
#define _MAKE_ALT_CHAN(ch) (ch | (IPU_MAX_CH << 24))
#define IPU_CHAN_ID(ch) (ch >> 24)
#define IPU_CHAN_ALT(ch) (ch & 0x02000000)
#define IPU_CHAN_ALPHA_IN_DMA(ch) ((u32)(ch >> 6) & 0x3F)
#define IPU_CHAN_GRAPH_IN_DMA(ch) ((u32)(ch >> 12) & 0x3F)
#define IPU_CHAN_VIDEO_IN_DMA(ch) ((u32)(ch >> 18) & 0x3F)
#define IPU_CHAN_OUT_DMA(ch) ((u32)(ch & 0x3F))
#define NO_DMA 0x3F
#define ALT 1

/*
 * Enumeration of IPU logical channels. An IPU logical channel is defined as a
 * combination of an input (memory to IPU), output (IPU to memory), and/or
 * secondary input IDMA channels and in some cases an Image Converter task.
 * Some channels consist of only an input or output.
 */
typedef enum {
	CHAN_NONE = -1,

	MEM_DC_SYNC = _MAKE_CHAN(7, 28, NO_DMA, NO_DMA, NO_DMA),
	MEM_DC_ASYNC = _MAKE_CHAN(8, 41, NO_DMA, NO_DMA, NO_DMA),
	MEM_BG_SYNC = _MAKE_CHAN(9, 23, NO_DMA, 51, NO_DMA),
	MEM_FG_SYNC = _MAKE_CHAN(10, 27, NO_DMA, 31, NO_DMA),

	MEM_BG_ASYNC0 = _MAKE_CHAN(11, 24, NO_DMA, 52, NO_DMA),
	MEM_FG_ASYNC0 = _MAKE_CHAN(12, 29, NO_DMA, 33, NO_DMA),
	MEM_BG_ASYNC1 = _MAKE_ALT_CHAN(MEM_BG_ASYNC0),
	MEM_FG_ASYNC1 = _MAKE_ALT_CHAN(MEM_FG_ASYNC0),

	DIRECT_ASYNC0 = _MAKE_CHAN(13, NO_DMA, NO_DMA, NO_DMA, NO_DMA),
	DIRECT_ASYNC1 = _MAKE_CHAN(14, NO_DMA, NO_DMA, NO_DMA, NO_DMA),

} ipu_channel_t;

/*
 * Enumeration of types of buffers for a logical channel.
 */
typedef enum {
	IPU_OUTPUT_BUFFER = 0, /*< Buffer for output from IPU */
	IPU_ALPHA_IN_BUFFER = 1, /*< Buffer for input to IPU */
	IPU_GRAPH_IN_BUFFER = 2, /*< Buffer for input to IPU */
	IPU_VIDEO_IN_BUFFER = 3, /*< Buffer for input to IPU */
	IPU_INPUT_BUFFER = IPU_VIDEO_IN_BUFFER,
	IPU_SEC_INPUT_BUFFER = IPU_GRAPH_IN_BUFFER,
} ipu_buffer_t;

#define IPU_PANEL_SERIAL 1
#define IPU_PANEL_PARALLEL 2

struct ipu_channel {
	u8 video_in_dma;
	u8 alpha_in_dma;
	u8 graph_in_dma;
	u8 out_dma;
};

enum ipu_dmfc_type {
	DMFC_NORMAL = 0,
	DMFC_HIGH_RESOLUTION_DC,
	DMFC_HIGH_RESOLUTION_DP,
	DMFC_HIGH_RESOLUTION_ONLY_DP,
};

/*
 * Union of initialization parameters for a logical channel.
 */
typedef union {
	struct {
		u32 di;
		unsigned char interlaced;
	} mem_dc_sync;
	struct {
		u32 temp;
	} mem_sdc_fg;
	struct {
		u32 di;
		unsigned char interlaced;
		u32 in_pixel_fmt;
		u32 out_pixel_fmt;
		unsigned char alpha_chan_en;
	} mem_dp_bg_sync;
	struct {
		u32 temp;
	} mem_sdc_bg;
	struct {
		u32 di;
		unsigned char interlaced;
		u32 in_pixel_fmt;
		u32 out_pixel_fmt;
		unsigned char alpha_chan_en;
	} mem_dp_fg_sync;
} ipu_channel_params_t;

/*
 * Enumeration of IPU interrupts.
 */
enum ipu_irq_line {
	IPU_IRQ_DP_SF_END = 448 + 3,
	IPU_IRQ_DC_FC_1 = 448 + 9,
};

/*
 * Bitfield of Display Interface signal polarities.
 */
typedef struct {
	unsigned datamask_en : 1;
	unsigned ext_clk : 1;
	unsigned interlaced : 1;
	unsigned odd_field_first : 1;
	unsigned clksel_en : 1;
	unsigned clkidle_en : 1;
	unsigned data_pol : 1; /* true = inverted */
	unsigned clk_pol : 1; /* true = rising edge */
	unsigned enable_pol : 1;
	unsigned hsync_pol : 1; /* true = active high */
	unsigned vsync_pol : 1;
} ipu_di_signal_cfg_t;

typedef enum { RGB, YCBCR, YUV } ipu_color_space_t;

/* Common IPU API */
int32_t ipu_init_channel(struct ipu_ctx *ctx, ipu_channel_t channel,
			 ipu_channel_params_t *params);
void ipu_uninit_channel(struct ipu_ctx *ctx, ipu_channel_t channel);

int32_t ipu_init_channel_buffer(ipu_channel_t channel, ipu_buffer_t type,
				u32 pixel_fmt, u16 width, u16 height,
				u32 stride, dma_addr_t phyaddr_0,
				dma_addr_t phyaddr_1, u32 u_offset,
				u32 v_offset);

void ipu_clear_buffer_ready(ipu_channel_t channel, ipu_buffer_t type,
			    u32 buf_num);
int32_t ipu_enable_channel(struct ipu_ctx *ctx, ipu_channel_t channel);
int32_t ipu_disable_channel(struct ipu_ctx *ctx, ipu_channel_t channel);

int32_t ipu_init_sync_panel(struct ipu_di_config *di, ipu_di_signal_cfg_t sig);

int32_t ipu_disp_set_global_alpha(ipu_channel_t channel, unsigned char enable,
				  u8 alpha);
int32_t ipu_disp_set_color_key(ipu_channel_t channel, unsigned char enable,
			       u32 color_key);

u32 bytes_per_pixel(u32 fmt);

void clk_enable(struct clk *clk);
void clk_disable(struct clk *clk);
u32 clk_get_rate(struct clk *clk);
int clk_set_rate(struct clk *clk, unsigned long rate);
long clk_round_rate(struct clk *clk, unsigned long rate);
int clk_set_parent(struct clk *clk, struct clk *parent);
int clk_get_usecount(struct clk *clk);
struct clk *clk_get_parent(struct clk *clk);

void ipu_dump_registers(void);
struct ipu_ctx *ipu_probe(struct udevice *dev);
bool ipu_clk_enabled(struct ipu_ctx *ctx);

void ipu_dmfc_init(int dmfc_type, int first);
void ipu_init_dc_mappings(void);
void ipu_dmfc_set_wait4eot(int dma_chan, int width);
void ipu_dc_init(int dc_chan, int di, unsigned char interlaced);
void ipu_dc_uninit(int dc_chan);
void ipu_dp_dc_enable(struct ipu_ctx *ctx, ipu_channel_t channel);
int ipu_dp_init(ipu_channel_t channel, u32 in_pixel_fmt, u32 out_pixel_fmt);
void ipu_dp_uninit(ipu_channel_t channel);
void ipu_dp_dc_disable(struct ipu_ctx *ctx, ipu_channel_t channel,
		       unsigned char swap);
ipu_color_space_t format_to_colorspace(u32 fmt);
#endif
