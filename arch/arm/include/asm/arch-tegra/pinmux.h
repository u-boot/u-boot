/*
 * (C) Copyright 2010-2014
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _TEGRA_PINMUX_H_
#define _TEGRA_PINMUX_H_

#include <asm/arch/tegra.h>

/* The pullup/pulldown state of a pin group */
enum pmux_pull {
	PMUX_PULL_NORMAL = 0,
	PMUX_PULL_DOWN,
	PMUX_PULL_UP,
};

/* Defines whether a pin group is tristated or in normal operation */
enum pmux_tristate {
	PMUX_TRI_NORMAL = 0,
	PMUX_TRI_TRISTATE = 1,
};

#ifdef TEGRA_PMX_HAS_PIN_IO_BIT_ETC
enum pmux_pin_io {
	PMUX_PIN_OUTPUT = 0,
	PMUX_PIN_INPUT = 1,
	PMUX_PIN_NONE,
};

enum pmux_pin_lock {
	PMUX_PIN_LOCK_DEFAULT = 0,
	PMUX_PIN_LOCK_DISABLE,
	PMUX_PIN_LOCK_ENABLE,
};

enum pmux_pin_od {
	PMUX_PIN_OD_DEFAULT = 0,
	PMUX_PIN_OD_DISABLE,
	PMUX_PIN_OD_ENABLE,
};

enum pmux_pin_ioreset {
	PMUX_PIN_IO_RESET_DEFAULT = 0,
	PMUX_PIN_IO_RESET_DISABLE,
	PMUX_PIN_IO_RESET_ENABLE,
};

#ifdef TEGRA_PMX_HAS_RCV_SEL
enum pmux_pin_rcv_sel {
	PMUX_PIN_RCV_SEL_DEFAULT = 0,
	PMUX_PIN_RCV_SEL_NORMAL,
	PMUX_PIN_RCV_SEL_HIGH,
};
#endif /* TEGRA_PMX_HAS_RCV_SEL */
#endif /* TEGRA_PMX_HAS_PIN_IO_BIT_ETC */

/*
 * This defines the configuration for a pin, including the function assigned,
 * pull up/down settings and tristate settings. Having set up one of these
 * you can call pinmux_config_pingroup() to configure a pin in one step. Also
 * available is pinmux_config_table() to configure a list of pins.
 */
struct pingroup_config {
	enum pmux_pingrp pingroup;	/* pin group PINGRP_...             */
	enum pmux_func func;		/* function to assign FUNC_...      */
	enum pmux_pull pull;		/* pull up/down/normal PMUX_PULL_...*/
	enum pmux_tristate tristate;	/* tristate or normal PMUX_TRI_...  */
#ifdef TEGRA_PMX_HAS_PIN_IO_BIT_ETC
	enum pmux_pin_io io;		/* input or output PMUX_PIN_...  */
	enum pmux_pin_lock lock;	/* lock enable/disable PMUX_PIN...  */
	enum pmux_pin_od od;		/* open-drain or push-pull driver  */
	enum pmux_pin_ioreset ioreset;	/* input/output reset PMUX_PIN...  */
#ifdef TEGRA_PMX_HAS_RCV_SEL
	enum pmux_pin_rcv_sel rcv_sel;	/* select between High and Normal  */
					/* VIL/VIH receivers */
#endif
#endif
};

/* Set the mux function for a pin group */
void pinmux_set_func(enum pmux_pingrp pin, enum pmux_func func);

/* Set the pull up/down feature for a pin group */
void pinmux_set_pullupdown(enum pmux_pingrp pin, enum pmux_pull pupd);

/* Set a pin group to tristate or normal */
void pinmux_set_tristate(enum pmux_pingrp pin, int enable);

/* Set a pin group to tristate */
void pinmux_tristate_enable(enum pmux_pingrp pin);

/* Set a pin group to normal (non tristate) */
void pinmux_tristate_disable(enum pmux_pingrp pin);

#ifdef TEGRA_PMX_HAS_PIN_IO_BIT_ETC
/* Set a pin group as input or output */
void pinmux_set_io(enum pmux_pingrp pin, enum pmux_pin_io io);
#endif

/* Set the complete configuration for a pin group */
void pinmux_config_pingroup(const struct pingroup_config *config);

/**
 * Configure a list of pin groups
 *
 * @param config	List of config items
 * @param len		Number of config items in list
 */
void pinmux_config_table(const struct pingroup_config *config, int len);

#ifdef TEGRA_PMX_HAS_PADGRPS

#define PGRP_SLWF_MIN	0
#define PGRP_SLWF_MAX	3
#define PGRP_SLWF_NONE	-1

#define PGRP_SLWR_MIN	0
#define PGRP_SLWR_MAX	3
#define PGRP_SLWR_NONE	-1

#define PGRP_DRVUP_MIN	0
#define PGRP_DRVUP_MAX	127
#define PGRP_DRVUP_NONE	-1

#define PGRP_DRVDN_MIN	0
#define PGRP_DRVDN_MAX	127
#define PGRP_DRVDN_NONE	-1

/* Defines a pin group cfg's low-power mode select */
enum pgrp_lpmd {
	PGRP_LPMD_X8 = 0,
	PGRP_LPMD_X4,
	PGRP_LPMD_X2,
	PGRP_LPMD_X,
	PGRP_LPMD_NONE = -1,
};

/* Defines whether a pin group cfg's schmidt is enabled or not */
enum pgrp_schmt {
	PGRP_SCHMT_DISABLE = 0,
	PGRP_SCHMT_ENABLE = 1,
	PGRP_SCHMT_NONE = -1,
};

/* Defines whether a pin group cfg's high-speed mode is enabled or not */
enum pgrp_hsm {
	PGRP_HSM_DISABLE = 0,
	PGRP_HSM_ENABLE = 1,
	PGRP_HSM_NONE = -1,
};

/*
 * This defines the configuration for a pin group's pad control config
 */
struct padctrl_config {
	enum pdrive_pingrp padgrp;	/* pin group PDRIVE_PINGRP_x */
	int slwf;			/* falling edge slew         */
	int slwr;			/* rising edge slew          */
	int drvup;			/* pull-up drive strength    */
	int drvdn;			/* pull-down drive strength  */
	enum pgrp_lpmd lpmd;		/* low-power mode selection  */
	enum pgrp_schmt schmt;		/* schmidt enable            */
	enum pgrp_hsm hsm;		/* high-speed mode enable    */
};

/**
 * Set the GP pad configs
 *
 * @param config	List of config items
 * @param len		Number of config items in list
 */
void padgrp_config_table(const struct padctrl_config *config, int len);

#endif /* TEGRA_PMX_HAS_PADGRPS */

struct tegra_pingroup_desc {
	enum pmux_func funcs[4];
#if defined(CONFIG_TEGRA20)
	u32 ctl_id;
	u32 pull_id;
#endif /* CONFIG_TEGRA20 */
};

extern const struct tegra_pingroup_desc *tegra_soc_pingroups;

#endif /* _TEGRA_PINMUX_H_ */
