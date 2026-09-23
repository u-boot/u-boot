// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 ASPEED Technology Inc.
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <errno.h>
#include <vsprintf.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/string.h>

#define SCU010			0x010
#define SCU200			0x200
#define SCU400			0x400
#define SCU404			0x404
#define SCU408			0x408
#define SCU410			0x410
/* One IO control register per ball, GPIO18A0..GPIO18B3 */
#define SCU480			0x480

/* Indexes into ast2700_soc0_pins[], in GPIO18A0..GPIO18B3 order */
enum {
	AC14,
	AE15,
	AD14,
	AE14,
	AF14,
	AB13,
	AB14,
	AF15,
	AF13,
	AC13,
	AD13,
	AE13,
};

#define USB_PORTA_ROUTE	BIT(9)
#define USB_PORTB_ROUTE	BIT(10)

#define PORTA_U3_MODE		GENMASK(1, 0)
#define PORTA_U2_MODE		GENMASK(3, 2)
#define PORTB_U3_MODE		GENMASK(5, 4)
#define PORTB_U2_MODE		GENMASK(7, 6)
#define PORTA_MODE		GENMASK(25, 24)
#define PORTB_MODE		GENMASK(29, 28)

#define PINMUX(_function, _group, _offset, _mask, _val)	\
	{							\
		.function = _function,				\
		.group = _group,				\
		.offset = _offset,				\
		.mask = _mask,					\
		.val = _val,					\
	}

struct ast2700_soc0_pinmux {
	const char *function;
	const char *group;
	u32 offset;
	u32 mask;
	u32 val;
};

struct ast2700_soc0_sig {
	u32 offset;
	u32 mask;
	const char *name;
};

struct ast2700_soc0_pin {
	const char *name;
	const struct ast2700_soc0_sig *sigs;
	unsigned int nsigs;
};

struct ast2700_soc0_pinctrl_priv {
	void __iomem *base;
};

struct ast2700_soc0_group {
	const char *name;
	const u8 *pins;
	unsigned int npins;
};

static const u8 ast2700_soc0_emmcg1_pins[] = { AC14, AE15, AD14 };
static const u8 ast2700_soc0_emmcg4_pins[] = {
	AC14, AE15, AD14, AE14, AF14, AB13,
};

static const u8 ast2700_soc0_emmcg8_pins[] = {
	AC14, AE15, AD14, AE14, AF14, AB13, AF13, AC13, AD13, AE13,
};

static const u8 ast2700_soc0_emmcwpn_pins[] = { AF15 };
static const u8 ast2700_soc0_emmccdn_pins[] = { AB14 };
static const u8 ast2700_soc0_vgaddc_pins[] = { AD13, AE13 };
static const u8 ast2700_soc0_vb1_pins[] = { AC14, AE15, AD14, AE14 };
static const u8 ast2700_soc0_vb0_pins[] = { AF15, AB14, AF13, AC13 };
static const u8 ast2700_soc0_tsprstn_pins[] = { AF13 };
static const u8 ast2700_soc0_ufsclki_pins[] = { AC13 };

#define AST2700_SOC0_GROUP(_name, _pins)				\
	{								\
		.name = _name,						\
		.pins = _pins,						\
		.npins = ARRAY_SIZE(_pins),				\
	}

/*
 * The USB, JTAG and PCIe RC groups control internal routing only; they
 * have no package balls, hence no group pins to configure.
 */
static const struct ast2700_soc0_group ast2700_soc0_groups[] = {
	AST2700_SOC0_GROUP("EMMCG1", ast2700_soc0_emmcg1_pins),
	AST2700_SOC0_GROUP("EMMCG4", ast2700_soc0_emmcg4_pins),
	AST2700_SOC0_GROUP("EMMCG8", ast2700_soc0_emmcg8_pins),
	AST2700_SOC0_GROUP("EMMCWPN", ast2700_soc0_emmcwpn_pins),
	AST2700_SOC0_GROUP("EMMCCDN", ast2700_soc0_emmccdn_pins),
	AST2700_SOC0_GROUP("VGADDC", ast2700_soc0_vgaddc_pins),
	AST2700_SOC0_GROUP("VB1", ast2700_soc0_vb1_pins),
	AST2700_SOC0_GROUP("VB0", ast2700_soc0_vb0_pins),
	AST2700_SOC0_GROUP("TSPRSTN", ast2700_soc0_tsprstn_pins),
	AST2700_SOC0_GROUP("UFSCLKI", ast2700_soc0_ufsclki_pins),
	{ "USB3A" },
	{ "USB3AAP" },
	{ "USB3ABP" },
	{ "USB3B" },
	{ "USB3BAP" },
	{ "USB3BBP" },
	{ "USB2A" },
	{ "USB2AAP" },
	{ "USB2ABP" },
	{ "USB2ADAP" },
	{ "USB2AH" },
	{ "USB2AHAP" },
	{ "USB2B" },
	{ "USB2BBP" },
	{ "USB2BAP" },
	{ "USB2BDBP" },
	{ "USB2BH" },
	{ "USB2BHBP" },
	{ "JTAG0" },
	{ "PCIERC0PERST" },
	{ "PCIERC1PERST" },
};

static const char * const ast2700_soc0_functions[] = {
	"EMMC",
	"VB",
	"TSPRSTN",
	"UFSCLKI",
	"VGADDC",
	"USB3AXHD",
	"USB3AXHPD",
	"USB3AXH",
	"USB3AXHP",
	"USB3AXH2B",
	"USB3AXHP2B",
	"USB3BXHD",
	"USB3BXHPD",
	"USB3BXH",
	"USB3BXHP",
	"USB3BXH2A",
	"USB3BXHP2A",
	"USB2AXHD1",
	"USB2AXHPD1",
	"USB2AXH",
	"USB2AXHP",
	"USB2AXH2B",
	"USB2AXHP2B",
	"USB2AD1",
	"USB2AHPD0",
	"USB2AH",
	"USB2AHP",
	"USB2AD0",
	"USB2BXHD1",
	"USB2BXHPD1",
	"USB2BXH",
	"USB2BXHP",
	"USB2BXH2A",
	"USB2BXHP2A",
	"USB2BD1",
	"USB2BHPD0",
	"USB2BH",
	"USB2BHP",
	"USB2BD0",
	"JTAGPSP",
	"JTAGSSP",
	"JTAGTSP",
	"JTAGDDR",
	"JTAGUSB3A",
	"JTAGUSB3B",
	"JTAGPCIEA",
	"JTAGPCIEB",
	"JTAGM0",
	"PCIERC0PERST",
	"PCIERC1PERST",
};

static const struct ast2700_soc0_pinmux ast2700_soc0_pinmuxes[] = {
	PINMUX("EMMC", "EMMCG1", SCU400, GENMASK(2, 0), GENMASK(2, 0)),
	PINMUX("EMMC", "EMMCG4", SCU400, GENMASK(5, 0), GENMASK(5, 0)),
	PINMUX("EMMC", "EMMCG8", SCU400,
	       GENMASK(5, 0) | GENMASK(11, 8),
	       GENMASK(5, 0) | GENMASK(11, 8)),
	PINMUX("EMMC", "EMMCWPN", SCU400, BIT(7), BIT(7)),
	PINMUX("EMMC", "EMMCCDN", SCU400, BIT(6), BIT(6)),
	PINMUX("VB", "VB1", SCU404, GENMASK(3, 0), GENMASK(3, 0)),
	PINMUX("VB", "VB0", SCU010, BIT(17), BIT(17)),
	PINMUX("TSPRSTN", "TSPRSTN", SCU010, BIT(9), BIT(9)),
	PINMUX("UFSCLKI", "UFSCLKI", SCU010, BIT(19), BIT(19)),
	PINMUX("VGADDC", "VGADDC", SCU404, GENMASK(11, 10),
	       GENMASK(11, 10)),

	/*
	 * The Linux driver models the USB2/USB3 routing with additional
	 * virtual PHY pins (PORTA/PORTB_U2_PHY, PORTA/PORTB_U3_PHY) purely
	 * to reject conflicting cross-coupled selections; they carry no
	 * register writes. U-Boot applies the states given by the device
	 * tree without strict conflict checking, so only the real selector
	 * fields are programmed here while the group names stay identical.
	 */
	PINMUX("USB3AXHD", "USB3A", SCU410,
	       PORTA_U3_MODE | USB_PORTA_ROUTE, USB_PORTA_ROUTE),
	PINMUX("USB3AXHPD", "USB3A", SCU410,
	       PORTA_U3_MODE | USB_PORTA_ROUTE, 0),
	PINMUX("USB3AXH", "USB3AAP", SCU410,
	       PORTA_U3_MODE | USB_PORTA_ROUTE, (2 << 0) | USB_PORTA_ROUTE),
	PINMUX("USB3AXHP", "USB3AAP", SCU410,
	       PORTA_U3_MODE | USB_PORTA_ROUTE, 2 << 0),
	PINMUX("USB3AXH2B", "USB3ABP", SCU410,
	       PORTA_U3_MODE | USB_PORTA_ROUTE, (3 << 0) | USB_PORTA_ROUTE),
	PINMUX("USB3AXHP2B", "USB3ABP", SCU410,
	       PORTA_U3_MODE | USB_PORTA_ROUTE, 3 << 0),

	PINMUX("USB3BXHD", "USB3B", SCU410,
	       PORTB_U3_MODE | USB_PORTB_ROUTE, USB_PORTB_ROUTE),
	PINMUX("USB3BXHPD", "USB3B", SCU410,
	       PORTB_U3_MODE | USB_PORTB_ROUTE, 0),
	PINMUX("USB3BXH", "USB3BBP", SCU410,
	       PORTB_U3_MODE | USB_PORTB_ROUTE, (2 << 4) | USB_PORTB_ROUTE),
	PINMUX("USB3BXHP", "USB3BBP", SCU410,
	       PORTB_U3_MODE | USB_PORTB_ROUTE, 2 << 4),
	PINMUX("USB3BXH2A", "USB3BAP", SCU410,
	       PORTB_U3_MODE | USB_PORTB_ROUTE, (3 << 4) | USB_PORTB_ROUTE),
	PINMUX("USB3BXHP2A", "USB3BAP", SCU410,
	       PORTB_U3_MODE | USB_PORTB_ROUTE, 3 << 4),

	PINMUX("USB2AXHD1", "USB2A", SCU410,
	       PORTA_U2_MODE | USB_PORTA_ROUTE, USB_PORTA_ROUTE),
	PINMUX("USB2AXHPD1", "USB2A", SCU410,
	       PORTA_U2_MODE | USB_PORTA_ROUTE, 0),
	PINMUX("USB2AXH", "USB2AAP", SCU410,
	       PORTA_U2_MODE | USB_PORTA_ROUTE, (2 << 2) | USB_PORTA_ROUTE),
	PINMUX("USB2AXHP", "USB2AAP", SCU410,
	       PORTA_U2_MODE | USB_PORTA_ROUTE, 2 << 2),
	PINMUX("USB2AXH2B", "USB2ABP", SCU410,
	       PORTA_U2_MODE | USB_PORTA_ROUTE, (3 << 2) | USB_PORTA_ROUTE),
	PINMUX("USB2AXHP2B", "USB2ABP", SCU410,
	       PORTA_U2_MODE | USB_PORTA_ROUTE, 3 << 2),
	PINMUX("USB2AD1", "USB2ADAP", SCU410, PORTA_U2_MODE, 1 << 2),
	PINMUX("USB2AHPD0", "USB2AH", SCU410, PORTA_MODE, 0),
	PINMUX("USB2AH", "USB2AHAP", SCU410, PORTA_MODE, 2 << 24),
	PINMUX("USB2AHP", "USB2AHAP", SCU410, PORTA_MODE, 3 << 24),
	PINMUX("USB2AD0", "USB2AHAP", SCU410, PORTA_MODE, 1 << 24),

	PINMUX("USB2BXHD1", "USB2B", SCU410,
	       PORTB_U2_MODE | USB_PORTB_ROUTE, USB_PORTB_ROUTE),
	PINMUX("USB2BXHPD1", "USB2B", SCU410,
	       PORTB_U2_MODE | USB_PORTB_ROUTE, 0),
	PINMUX("USB2BXH", "USB2BBP", SCU410,
	       PORTB_U2_MODE | USB_PORTB_ROUTE, (2 << 6) | USB_PORTB_ROUTE),
	PINMUX("USB2BXHP", "USB2BBP", SCU410,
	       PORTB_U2_MODE | USB_PORTB_ROUTE, 2 << 6),
	PINMUX("USB2BXH2A", "USB2BAP", SCU410,
	       PORTB_U2_MODE | USB_PORTB_ROUTE, (3 << 6) | USB_PORTB_ROUTE),
	PINMUX("USB2BXHP2A", "USB2BAP", SCU410,
	       PORTB_U2_MODE | USB_PORTB_ROUTE, 3 << 6),
	PINMUX("USB2BD1", "USB2BDBP", SCU410, PORTB_U2_MODE, 1 << 6),
	PINMUX("USB2BHPD0", "USB2BH", SCU410, PORTB_MODE, 0),
	PINMUX("USB2BH", "USB2BHBP", SCU410, PORTB_MODE, 2 << 28),
	PINMUX("USB2BHP", "USB2BHBP", SCU410, PORTB_MODE, 3 << 28),
	PINMUX("USB2BD0", "USB2BHBP", SCU410, PORTB_MODE, 1 << 28),

	PINMUX("JTAGPSP", "JTAG0", SCU408, GENMASK(12, 5), 0x00 << 5),
	PINMUX("JTAGSSP", "JTAG0", SCU408, GENMASK(12, 5), 0x41 << 5),
	PINMUX("JTAGTSP", "JTAG0", SCU408, GENMASK(12, 5), 0x42 << 5),
	PINMUX("JTAGDDR", "JTAG0", SCU408, GENMASK(12, 5), 0x43 << 5),
	PINMUX("JTAGUSB3A", "JTAG0", SCU408, GENMASK(12, 5), 0x44 << 5),
	PINMUX("JTAGUSB3B", "JTAG0", SCU408, GENMASK(12, 5), 0x45 << 5),
	PINMUX("JTAGPCIEA", "JTAG0", SCU408, GENMASK(12, 5), 0x46 << 5),
	PINMUX("JTAGPCIEB", "JTAG0", SCU408, GENMASK(12, 5), 0x47 << 5),
	PINMUX("JTAGM0", "JTAG0", SCU408, GENMASK(12, 5), 0x08 << 5),
	PINMUX("PCIERC0PERST", "PCIERC0PERST", SCU200, BIT(21), BIT(21)),
	PINMUX("PCIERC1PERST", "PCIERC1PERST", SCU200, BIT(19), BIT(19)),
};

/*
 * Signals muxable on each package ball, strongest first. A pin acts as
 * GPIO when none of its signal enable bits are set; gpio_request_enable
 * clears them all. The pin order follows the GPIO18A0..GPIO18B3 numbering
 * used by the gpio-ranges of the SoC0 GPIO controller.
 */
static const struct ast2700_soc0_sig ast2700_soc0_ac14_sigs[] = {
	{ SCU400, BIT(0), "EMMCCLK" },
	{ SCU404, BIT(0), "VB1CS" },
};

static const struct ast2700_soc0_sig ast2700_soc0_ae15_sigs[] = {
	{ SCU400, BIT(1), "EMMCCMD" },
	{ SCU404, BIT(1), "VB1CK" },
};

static const struct ast2700_soc0_sig ast2700_soc0_ad14_sigs[] = {
	{ SCU400, BIT(2), "EMMCDAT0" },
	{ SCU404, BIT(2), "VB1MOSI" },
};

static const struct ast2700_soc0_sig ast2700_soc0_ae14_sigs[] = {
	{ SCU400, BIT(3), "EMMCDAT1" },
	{ SCU404, BIT(3), "VB1MISO" },
};

static const struct ast2700_soc0_sig ast2700_soc0_af14_sigs[] = {
	{ SCU400, BIT(4), "EMMCDAT2" },
};

static const struct ast2700_soc0_sig ast2700_soc0_ab13_sigs[] = {
	{ SCU400, BIT(5), "EMMCDAT3" },
};

static const struct ast2700_soc0_sig ast2700_soc0_ab14_sigs[] = {
	{ SCU400, BIT(6), "EMMCCDN" },
	{ SCU010, BIT(17), "VB0CS" },
};

static const struct ast2700_soc0_sig ast2700_soc0_af15_sigs[] = {
	{ SCU400, BIT(7), "EMMCWPN" },
	{ SCU010, BIT(17), "VB0CK" },
};

static const struct ast2700_soc0_sig ast2700_soc0_af13_sigs[] = {
	{ SCU010, BIT(9), "TSPRSTN" },
	{ SCU400, BIT(8), "EMMCDAT4" },
	{ SCU010, BIT(17), "VB0MOSI" },
};

static const struct ast2700_soc0_sig ast2700_soc0_ac13_sigs[] = {
	{ SCU010, BIT(19), "UFSCLKI" },
	{ SCU400, BIT(9), "EMMCDAT5" },
	{ SCU010, BIT(17), "VB0MISO" },
};

static const struct ast2700_soc0_sig ast2700_soc0_ad13_sigs[] = {
	{ SCU400, BIT(10), "EMMCDAT6" },
	{ SCU404, BIT(10), "DDCCLK" },
};

static const struct ast2700_soc0_sig ast2700_soc0_ae13_sigs[] = {
	{ SCU400, BIT(11), "EMMCDAT7" },
	{ SCU404, BIT(11), "DDCDAT" },
};

#define AST2700_SOC0_PIN(_name, _sigs)					\
	{								\
		.name = _name,						\
		.sigs = _sigs,						\
		.nsigs = ARRAY_SIZE(_sigs),				\
	}

static const struct ast2700_soc0_pin ast2700_soc0_pins[] = {
	AST2700_SOC0_PIN("AC14", ast2700_soc0_ac14_sigs),
	AST2700_SOC0_PIN("AE15", ast2700_soc0_ae15_sigs),
	AST2700_SOC0_PIN("AD14", ast2700_soc0_ad14_sigs),
	AST2700_SOC0_PIN("AE14", ast2700_soc0_ae14_sigs),
	AST2700_SOC0_PIN("AF14", ast2700_soc0_af14_sigs),
	AST2700_SOC0_PIN("AB13", ast2700_soc0_ab13_sigs),
	AST2700_SOC0_PIN("AB14", ast2700_soc0_ab14_sigs),
	AST2700_SOC0_PIN("AF15", ast2700_soc0_af15_sigs),
	AST2700_SOC0_PIN("AF13", ast2700_soc0_af13_sigs),
	AST2700_SOC0_PIN("AC13", ast2700_soc0_ac13_sigs),
	AST2700_SOC0_PIN("AD13", ast2700_soc0_ad13_sigs),
	AST2700_SOC0_PIN("AE13", ast2700_soc0_ae13_sigs),
};

static int ast2700_soc0_pinctrl_probe(struct udevice *dev)
{
	struct ast2700_soc0_pinctrl_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr(dev->parent);
	if (!priv->base)
		return -ENOMEM;

	return 0;
}

static int ast2700_soc0_pinctrl_get_groups_count(struct udevice *dev)
{
	return ARRAY_SIZE(ast2700_soc0_groups);
}

static const char *ast2700_soc0_pinctrl_get_group_name(struct udevice *dev,
						       unsigned int selector)
{
	return ast2700_soc0_groups[selector].name;
}

static int ast2700_soc0_pinctrl_get_functions_count(struct udevice *dev)
{
	return ARRAY_SIZE(ast2700_soc0_functions);
}

static const char *ast2700_soc0_pinctrl_get_function_name(struct udevice *dev,
							  unsigned int selector)
{
	return ast2700_soc0_functions[selector];
}

static int ast2700_soc0_pinctrl_group_set(struct udevice *dev,
					  unsigned int group_selector,
					  unsigned int func_selector)
{
	struct ast2700_soc0_pinctrl_priv *priv = dev_get_priv(dev);
	const struct ast2700_soc0_pinmux *pinmux;
	const char *function;
	const char *group;
	u32 i;

	if (group_selector >= ARRAY_SIZE(ast2700_soc0_groups) ||
	    func_selector >= ARRAY_SIZE(ast2700_soc0_functions))
		return -EINVAL;

	function = ast2700_soc0_functions[func_selector];
	group = ast2700_soc0_groups[group_selector].name;

	for (i = 0; i < ARRAY_SIZE(ast2700_soc0_pinmuxes); i++) {
		pinmux = &ast2700_soc0_pinmuxes[i];
		if (strcmp(pinmux->function, function) ||
		    strcmp(pinmux->group, group))
			continue;

		clrsetbits_le32(priv->base + pinmux->offset, pinmux->mask,
				pinmux->val);
		return 0;
	}

	return -EINVAL;
}

static int ast2700_soc0_pinctrl_gpio_request_enable(struct udevice *dev,
						    unsigned int selector)
{
	struct ast2700_soc0_pinctrl_priv *priv = dev_get_priv(dev);
	const struct ast2700_soc0_pin *pin;
	u32 i;

	if (selector >= ARRAY_SIZE(ast2700_soc0_pins))
		return -EINVAL;

	pin = &ast2700_soc0_pins[selector];
	for (i = 0; i < pin->nsigs; i++)
		clrbits_le32(priv->base + pin->sigs[i].offset,
			     pin->sigs[i].mask);

	return 0;
}

static int ast2700_soc0_pinctrl_get_pins_count(struct udevice *dev)
{
	return ARRAY_SIZE(ast2700_soc0_pins);
}

static const char *ast2700_soc0_pinctrl_get_pin_name(struct udevice *dev,
						     unsigned int selector)
{
	if (selector >= ARRAY_SIZE(ast2700_soc0_pins))
		return NULL;

	return ast2700_soc0_pins[selector].name;
}

static int ast2700_soc0_pinctrl_get_pin_muxing(struct udevice *dev,
					       unsigned int selector,
					       char *buf, int size)
{
	struct ast2700_soc0_pinctrl_priv *priv = dev_get_priv(dev);
	const struct ast2700_soc0_pin *pin;
	const struct ast2700_soc0_sig *sig;
	u32 i;

	if (selector >= ARRAY_SIZE(ast2700_soc0_pins))
		return -EINVAL;

	pin = &ast2700_soc0_pins[selector];
	for (i = 0; i < pin->nsigs; i++) {
		sig = &pin->sigs[i];
		if ((readl(priv->base + sig->offset) & sig->mask) ==
		    sig->mask) {
			snprintf(buf, size, "%s", sig->name);
			return 0;
		}
	}

	snprintf(buf, size, "GPIO");

	return 0;
}

/* Drive strength in mA selectable by the IO control register [3:0] */
static const u8 ast2700_soc0_drv_ma[] = {
	3, 6, 8, 11, 16, 18, 20, 23, 30, 32, 33, 35, 37, 38, 39, 41,
};

static const struct pinconf_param ast2700_soc0_pinconf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 0 },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 0 },
};

static int ast2700_soc0_pinctrl_pinconf_set(struct udevice *dev,
					    unsigned int selector,
					    unsigned int param,
					    unsigned int arg)
{
	struct ast2700_soc0_pinctrl_priv *priv = dev_get_priv(dev);
	u32 mask, val, i;

	if (selector >= ARRAY_SIZE(ast2700_soc0_pins))
		return -EINVAL;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		mask = BIT(5);
		val = 0;
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		mask = GENMASK(5, 4);
		val = 2 << 4;
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		mask = GENMASK(5, 4);
		val = 3 << 4;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		for (i = 0; i < ARRAY_SIZE(ast2700_soc0_drv_ma); i++) {
			if (ast2700_soc0_drv_ma[i] == arg)
				break;
		}
		if (i == ARRAY_SIZE(ast2700_soc0_drv_ma))
			return -EINVAL;
		mask = GENMASK(3, 0);
		val = i;
		break;
	default:
		return -ENOTSUPP;
	}

	clrsetbits_le32(priv->base + SCU480 + selector * 4, mask, val);

	return 0;
}

static int ast2700_soc0_pinctrl_pinconf_group_set(struct udevice *dev,
						  unsigned int selector,
						  unsigned int param,
						  unsigned int arg)
{
	const struct ast2700_soc0_group *group;
	u32 i;
	int ret;

	if (selector >= ARRAY_SIZE(ast2700_soc0_groups))
		return -EINVAL;

	group = &ast2700_soc0_groups[selector];
	if (!group->npins)
		return -ENOTSUPP;

	for (i = 0; i < group->npins; i++) {
		ret = ast2700_soc0_pinctrl_pinconf_set(dev, group->pins[i],
						       param, arg);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct pinctrl_ops ast2700_soc0_pinctrl_ops = {
	.set_state = pinctrl_generic_set_state,
	.get_pins_count = ast2700_soc0_pinctrl_get_pins_count,
	.get_pin_name = ast2700_soc0_pinctrl_get_pin_name,
	.get_pin_muxing = ast2700_soc0_pinctrl_get_pin_muxing,
	.get_groups_count = ast2700_soc0_pinctrl_get_groups_count,
	.get_group_name = ast2700_soc0_pinctrl_get_group_name,
	.get_functions_count = ast2700_soc0_pinctrl_get_functions_count,
	.get_function_name = ast2700_soc0_pinctrl_get_function_name,
	.pinmux_group_set = ast2700_soc0_pinctrl_group_set,
	.gpio_request_enable = ast2700_soc0_pinctrl_gpio_request_enable,
	.pinconf_num_params = ARRAY_SIZE(ast2700_soc0_pinconf_params),
	.pinconf_params = ast2700_soc0_pinconf_params,
	.pinconf_set = ast2700_soc0_pinctrl_pinconf_set,
	.pinconf_group_set = ast2700_soc0_pinctrl_pinconf_group_set,
};

static const struct udevice_id ast2700_soc0_pinctrl_ids[] = {
	{ .compatible = "aspeed,ast2700-soc0-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_ast2700_soc0) = {
	.name = "aspeed_ast2700_soc0_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = ast2700_soc0_pinctrl_ids,
	.priv_auto = sizeof(struct ast2700_soc0_pinctrl_priv),
	.ops = &ast2700_soc0_pinctrl_ops,
	.probe = ast2700_soc0_pinctrl_probe,
};
