/*
 * Copyright (c) 2017 Theobroma Systems Design und Consulting GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __RK_HDMI_H__
#define __RK_HDMI_H__

struct rkhdmi_driverdata {
	/* configuration */
	u8 i2c_clk_high;
	u8 i2c_clk_low;
	const char * const *regulator_names;
	u32 regulator_names_cnt;
	/* setters/getters */
	int (*set_input_vop)(struct udevice *dev);
	int (*clk_config)(struct udevice *dev);
};

struct rk_hdmi_priv {
	struct dw_hdmi hdmi;
	void *grf;
};

int rk_hdmi_read_edid(struct udevice *dev, u8 *buf, int buf_size);
void rk_hdmi_probe_regulators(struct udevice *dev,
			      const char * const *names, int cnt);
int rk_hdmi_ofdata_to_platdata(struct udevice *dev);
int rk_hdmi_probe(struct udevice *dev);

#endif
