/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __LINUX_PINCTRL_PINCTRL_H
#define __LINUX_PINCTRL_PINCTRL_H

#include <linux/types.h>

/**
 * struct pingroup - provides information on pingroup
 * @name: a name for pingroup
 * @pins: an array of pins in the pingroup
 * @npins: number of pins in the pingroup
 */
struct pingroup {
	const char *name;
	const unsigned int *pins;
	size_t npins;
};

/* Convenience macro to define a single named or anonymous pingroup */
#define PINCTRL_PINGROUP(_name, _pins, _npins)	\
(struct pingroup) {				\
	.name = _name,				\
	.pins = _pins,				\
	.npins = _npins,			\
}

/**
 * struct pinctrl_pin_desc - boards/machines provide information on their
 * pins, pads or other muxable units in this struct
 * @number: unique pin number from the global pin number space
 * @name: a name for this pin
 * @drv_data: driver-defined per-pin data. pinctrl core does not touch this
 */
struct pinctrl_pin_desc {
	unsigned int number;
	const char *name;
	void *drv_data;
};

/* Convenience macro to define a single named or anonymous pin descriptor */
#define PINCTRL_PIN(_number, _name)		\
(struct pinctrl_pin_desc) {			\
	.number = _number,			\
	.name = _name,				\
}

#define PINCTRL_PIN_ANON(_number)		\
(struct pinctrl_pin_desc) {			\
	.number = _number,			\
}

/**
 * struct pinfunction - Description about a function
 * @name: Name of the function
 * @groups: An array of groups for this function
 * @ngroups: Number of groups in @groups
 * @flags: Additional pin function flags
 */
struct pinfunction {
	const char *name;
	const char * const *groups;
	size_t ngroups;
};

/* Convenience macro to define a single named pinfunction */
#define PINCTRL_PINFUNCTION(_name, _groups, _ngroups)	\
(struct pinfunction) {					\
	.name = (_name),				\
	.groups = (_groups),				\
	.ngroups = (_ngroups),				\
}

#endif /* __LINUX_PINCTRL_PINCTRL_H */
