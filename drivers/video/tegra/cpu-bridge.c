// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 *
 * This driver uses 8-bit CPU interface found in Tegra 2
 * and Tegra 3 to drive MIPI DSI panel.
 */

#include <dm.h>
#include <dm/ofnode_graph.h>
#include <log.h>
#include <mipi_display.h>
#include <mipi_dsi.h>
#include <backlight.h>
#include <panel.h>
#include <video_bridge.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <asm/gpio.h>
#include <asm/io.h>

#include "dc.h"

struct tegra_cpu_bridge_priv {
	struct dc_ctlr *dc;

	struct mipi_dsi_host host;
	struct mipi_dsi_device device;

	struct udevice *panel;
	struct display_timing timing;

	struct gpio_desc dc_gpio;
	struct gpio_desc rw_gpio;
	struct gpio_desc cs_gpio;

	struct gpio_desc data_gpios[8];

	u32 pixel_format;
	u32 spi_init_seq[4];
};

#define TEGRA_CPU_BRIDGE_COMM 0
#define TEGRA_CPU_BRIDGE_DATA 1

static void tegra_cpu_bridge_write(struct tegra_cpu_bridge_priv *priv,
				   u8 type, u8 value)
{
	int i;

	dm_gpio_set_value(&priv->dc_gpio, type);

	dm_gpio_set_value(&priv->cs_gpio, 0);
	dm_gpio_set_value(&priv->rw_gpio, 0);

	for (i = 0; i < 8; i++)
		dm_gpio_set_value(&priv->data_gpios[i],
				  (value >> i) & 0x1);

	dm_gpio_set_value(&priv->cs_gpio, 1);
	dm_gpio_set_value(&priv->rw_gpio, 1);

	udelay(10);

	log_debug("%s: type 0x%x, val 0x%x\n",
		  __func__, type, value);
}

static ssize_t tegra_cpu_bridge_transfer(struct mipi_dsi_host *host,
					 const struct mipi_dsi_msg *msg)
{
	struct udevice *dev = (struct udevice *)host->dev;
	struct tegra_cpu_bridge_priv *priv = dev_get_priv(dev);
	u8 command = *(u8 *)msg->tx_buf;
	const u8 *data = msg->tx_buf;
	int i;

	tegra_cpu_bridge_write(priv, TEGRA_CPU_BRIDGE_COMM, command);

	for (i = 1; i < msg->tx_len; i++)
		tegra_cpu_bridge_write(priv, TEGRA_CPU_BRIDGE_DATA, data[i]);

	return 0;
}

static const struct mipi_dsi_host_ops tegra_cpu_bridge_host_ops = {
	.transfer	= tegra_cpu_bridge_transfer,
};

static int tegra_cpu_bridge_get_format(enum mipi_dsi_pixel_format format, u32 *fmt)
{
	switch (format) {
	case MIPI_DSI_FMT_RGB888:
	case MIPI_DSI_FMT_RGB666_PACKED:
		*fmt = BASE_COLOR_SIZE_888;
		break;

	case MIPI_DSI_FMT_RGB666:
		*fmt = BASE_COLOR_SIZE_666;
		break;

	case MIPI_DSI_FMT_RGB565:
		*fmt = BASE_COLOR_SIZE_565;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int tegra_cpu_bridge_attach(struct udevice *dev)
{
	struct tegra_cpu_bridge_priv *priv = dev_get_priv(dev);
	struct dc_disp_reg *disp = &priv->dc->disp;
	struct dc_cmd_reg *cmd = &priv->dc->cmd;
	struct dc_com_reg *com = &priv->dc->com;
	u32 value;
	int ret;

	writel(CTRL_MODE_STOP << CTRL_MODE_SHIFT, &cmd->disp_cmd);
	writel(0, &disp->disp_win_opt);
	writel(GENERAL_UPDATE, &cmd->state_ctrl);
	writel(GENERAL_ACT_REQ, &cmd->state_ctrl);

	/* TODO: parametrize if needed */
	writel(V_PULSE1_ENABLE, &disp->disp_signal_opt0);
	writel(PULSE_POLARITY_LOW, &disp->v_pulse1.v_pulse_ctrl);

	writel(PULSE_END(1), &disp->v_pulse1.v_pulse_pos[V_PULSE0_POSITION_A]);
	writel(0, &disp->v_pulse1.v_pulse_pos[V_PULSE0_POSITION_B]);
	writel(0, &disp->v_pulse1.v_pulse_pos[V_PULSE0_POSITION_C]);

	ret = dev_read_u32_array(dev, "nvidia,init-sequence", priv->spi_init_seq, 4);
	if (!ret) {
		value = 1 << FRAME_INIT_SEQ_CYCLES_SHIFT |
			DC_SIGNAL_VPULSE1 << INIT_SEQ_DC_SIGNAL_SHIFT |
			INIT_SEQUENCE_MODE_PLCD | SEND_INIT_SEQUENCE;
		writel(value, &disp->seq_ctrl);

		writel(priv->spi_init_seq[0], &disp->spi_init_seq_data_a);
		writel(priv->spi_init_seq[1], &disp->spi_init_seq_data_b);
		writel(priv->spi_init_seq[2], &disp->spi_init_seq_data_c);
		writel(priv->spi_init_seq[3], &disp->spi_init_seq_data_d);
	}

	value = readl(&cmd->disp_cmd);
	value &= ~CTRL_MODE_MASK;
	value |= CTRL_MODE_C_DISPLAY << CTRL_MODE_SHIFT;
	writel(value, &cmd->disp_cmd);

	/* set LDC pin to V Pulse 1 */
	value = readl(&com->pin_output_sel[6]) | LDC_OUTPUT_SELECT_V_PULSE1;
	writel(value, &com->pin_output_sel[6]);

	value = readl(&disp->disp_interface_ctrl);
	value |= DATA_ALIGNMENT_LSB << DATA_ALIGNMENT_SHIFT;
	writel(value, &disp->disp_interface_ctrl);

	value = SC_H_QUALIFIER_NONE << SC1_H_QUALIFIER_SHIFT |
		SC_V_QUALIFIER_VACTIVE << SC0_V_QUALIFIER_SHIFT |
		SC_H_QUALIFIER_HACTIVE << SC0_H_QUALIFIER_SHIFT;
	writel(value, &disp->shift_clk_opt);

	value = readl(&disp->disp_color_ctrl);
	value |= priv->pixel_format;
	writel(value, &disp->disp_color_ctrl);

	/* Perform panel setup */
	panel_enable_backlight(priv->panel);

	dm_gpio_set_value(&priv->cs_gpio, 0);

	dm_gpio_free(dev, &priv->dc_gpio);
	dm_gpio_free(dev, &priv->rw_gpio);
	dm_gpio_free(dev, &priv->cs_gpio);

	gpio_free_list(dev, priv->data_gpios, 8);

	return 0;
}

static int tegra_cpu_bridge_set_panel(struct udevice *dev, int percent)
{
	struct tegra_cpu_bridge_priv *priv = dev_get_priv(dev);

	return panel_set_backlight(priv->panel, percent);
}

static int tegra_cpu_bridge_panel_timings(struct udevice *dev,
					  struct display_timing *timing)
{
	struct tegra_cpu_bridge_priv *priv = dev_get_priv(dev);

	memcpy(timing, &priv->timing, sizeof(*timing));

	return 0;
}

static int tegra_cpu_bridge_hw_init(struct udevice *dev)
{
	struct tegra_cpu_bridge_priv *priv = dev_get_priv(dev);

	dm_gpio_set_value(&priv->cs_gpio, 1);

	dm_gpio_set_value(&priv->rw_gpio, 1);
	dm_gpio_set_value(&priv->dc_gpio, 0);

	return 0;
}

static int tegra_cpu_bridge_get_links(struct udevice *dev)
{
	struct tegra_cpu_bridge_priv *priv = dev_get_priv(dev);
	int i, ret;

	u32 num = ofnode_graph_get_port_count(dev_ofnode(dev));

	for (i = 0; i < num; i++) {
		ofnode remote = ofnode_graph_get_remote_node(dev_ofnode(dev), i, -1);

		/* Look for DC source */
		if (ofnode_name_eq(remote, "rgb")) {
			ofnode dc = ofnode_get_parent(remote);

			priv->dc = (struct dc_ctlr *)ofnode_get_addr(dc);
			if (!priv->dc) {
				log_err("%s: failed to get DC controller\n", __func__);
				return -EINVAL;
			}
		}

		/* Look for driven panel */
		ret = uclass_get_device_by_ofnode(UCLASS_PANEL, remote, &priv->panel);
		if (!ret)
			return 0;
	}

	/* If this point is reached, no panels were found */
	return -ENODEV;
}

static int tegra_cpu_bridge_probe(struct udevice *dev)
{
	struct tegra_cpu_bridge_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_device *device = &priv->device;
	struct mipi_dsi_panel_plat *mipi_plat;
	int ret;

	ret = tegra_cpu_bridge_get_links(dev);
	if (ret) {
		log_debug("%s: links not found, ret %d\n", __func__, ret);
		return ret;
	}

	panel_get_display_timing(priv->panel, &priv->timing);

	mipi_plat = dev_get_plat(priv->panel);
	mipi_plat->device = device;

	priv->host.dev = (struct device *)dev;
	priv->host.ops = &tegra_cpu_bridge_host_ops;

	device->host = &priv->host;
	device->lanes = mipi_plat->lanes;
	device->format = mipi_plat->format;
	device->mode_flags = mipi_plat->mode_flags;

	tegra_cpu_bridge_get_format(device->format, &priv->pixel_format);

	/* get control gpios */
	ret = gpio_request_by_name(dev, "dc-gpios", 0,
				   &priv->dc_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: could not decode dc-gpios (%d)\n", __func__, ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "rw-gpios", 0,
				   &priv->rw_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: could not decode rw-gpios (%d)\n", __func__, ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "cs-gpios", 0,
				   &priv->cs_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: could not decode cs-gpios (%d)\n", __func__, ret);
		return ret;
	}

	/* get data gpios */
	ret = gpio_request_list_by_name(dev, "data-gpios",
					priv->data_gpios, 8,
					GPIOD_IS_OUT);
	if (ret < 0) {
		log_debug("%s: could not decode data-gpios (%d)\n", __func__, ret);
		return ret;
	}

	return tegra_cpu_bridge_hw_init(dev);
}

static const struct video_bridge_ops tegra_cpu_bridge_ops = {
	.attach			= tegra_cpu_bridge_attach,
	.set_backlight		= tegra_cpu_bridge_set_panel,
	.get_display_timing	= tegra_cpu_bridge_panel_timings,
};

static const struct udevice_id tegra_cpu_bridge_ids[] = {
	{ .compatible = "nvidia,tegra-8bit-cpu" },
	{ }
};

U_BOOT_DRIVER(tegra_8bit_cpu) = {
	.name		= "tegra_8bit_cpu",
	.id		= UCLASS_VIDEO_BRIDGE,
	.of_match	= tegra_cpu_bridge_ids,
	.ops		= &tegra_cpu_bridge_ops,
	.bind		= dm_scan_fdt_dev,
	.probe		= tegra_cpu_bridge_probe,
	.priv_auto	= sizeof(struct tegra_cpu_bridge_priv),
};
