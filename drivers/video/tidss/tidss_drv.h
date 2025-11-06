/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2023 Texas Instruments Incorporated - https://www.ti.com/
 * Nikhil M Jain, n-jain1@ti.com
 *
 * based on the linux tidss driver, which is
 *
 * (C) Copyright 2018 Texas Instruments Incorporated - https://www.ti.com/
 * Author: Tomi Valkeinen <tomi.valkeinen@ti.com>
 */

#ifndef __TIDSS_DRV_H__
#define __TIDSS_DRV_H__

#include <media_bus_format.h>
#include <syscon.h>
#include <regmap.h>

#define TIDSS_MAX_PORTS 4
#define TIDSS_MAX_PLANES 4
#define TIDSS_MAX_OLDI_TXES 2

enum dss_vp_bus_type {
	DSS_VP_DPI,		/* DPI output */
	DSS_VP_OLDI,		/* OLDI (LVDS) output */
	DSS_VP_INTERNAL,	/* SoC internal routing */
	DSS_VP_MAX_BUS_TYPE,
};

enum tidss_oldi_link_type {
	OLDI_MODE_UNSUPPORTED,
	OLDI_MODE_SINGLE_LINK,
	OLDI_MODE_CLONE_SINGLE_LINK,
	OLDI_MODE_SECONDARY_CLONE_SINGLE_LINK,
	OLDI_MODE_DUAL_LINK,
	OLDI_MODE_SECONDARY_DUAL_LINK,
};

enum oldi_mode_reg_val { SPWG_18 = 0, JEIDA_24 = 1, SPWG_24 = 2 };

struct dss_features_scaling {
	u32 in_width_max_5tap_rgb;
	u32 in_width_max_3tap_rgb;
	u32 in_width_max_5tap_yuv;
	u32 in_width_max_3tap_yuv;
	u32 upscale_limit;
	u32 downscale_limit_5tap;
	u32 downscale_limit_3tap;
	u32 xinc_max;
};

enum tidss_gamma_type { TIDSS_GAMMA_8BIT, TIDSS_GAMMA_10BIT };

/* choose specific DSS based on the board */
enum dss_subrevision {
	DSS_K2G,
	DSS_AM65X,
	DSS_J721E,
	DSS_AM625,
};

struct tidss_vp_feat {
	struct tidss_vp_color_feat {
		u32 gamma_size;
		enum tidss_gamma_type gamma_type;
		bool has_ctm;
	} color;
};

struct dss_color_lut {
	/*
	 * Data is U0.16 fixed point format.
	 */
	__u16 red;
	__u16 green;
	__u16 blue;
	__u16 reserved;
};

struct dss_vp_data {
	u32 *gamma_table;
};

struct dss_features {
	int min_pclk_khz;
	int max_pclk_khz[DSS_VP_MAX_BUS_TYPE];

	struct dss_features_scaling scaling;

	enum dss_subrevision subrev;

	const char *common;
	const u16 *common_regs;
	u32 num_vps;
	const char *vp_name[TIDSS_MAX_PORTS]; /* Should match dt reg names */
	const char *ovr_name[TIDSS_MAX_PORTS]; /* Should match dt reg names */
	const char *vpclk_name[TIDSS_MAX_PORTS]; /* Should match dt clk names */
	const enum dss_vp_bus_type vp_bus_type[TIDSS_MAX_PORTS];
	struct tidss_vp_feat vp_feat;
	u32 num_planes;
	const char *vid_name[TIDSS_MAX_PLANES]; /* Should match dt reg names */
	bool vid_lite[TIDSS_MAX_PLANES];
	u32 vid_order[TIDSS_MAX_PLANES];
};

struct dss_bus_format {
	u32 bus_fmt;
	u32 data_width;
	bool is_oldi_fmt;
	enum oldi_mode_reg_val oldi_mode_reg_val;
};

struct tidss_drv_priv {
	struct udevice *dev;
	void __iomem *base_common; /* common register region of dss*/
	void __iomem *base_vid[TIDSS_MAX_PLANES]; /* plane register region of dss*/
	void __iomem *base_ovr[TIDSS_MAX_PORTS]; /* overlay register region of dss*/
	void __iomem *base_vp[TIDSS_MAX_PORTS]; /* video port register region of dss*/
	struct regmap *oldi_io_ctrl;
	struct clk vp_clk[TIDSS_MAX_PORTS];
	const struct dss_features *feat;
	struct clk fclk;
	struct dss_vp_data vp_data[TIDSS_MAX_PORTS];
	enum tidss_oldi_link_type oldi_mode;
	struct dss_bus_format *bus_format;
	u32 pixel_format;
	u32 memory_bandwidth_limit;
	unsigned int num_oldis;
	struct tidss_oldi *oldis[TIDSS_MAX_OLDI_TXES];
	u8 active_hw_vps[TIDSS_MAX_PORTS];
	u8 active_pipelines;
};

struct tidss_oldi;
#endif
