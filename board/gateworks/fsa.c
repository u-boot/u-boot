// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 Gateworks Corporation
 */

#include <command.h>
#include <hexdump.h>
#include <i2c.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h> // device_remove/device_unbind
#include <asm-generic/gpio.h>
#include <fdt_support.h>
#include <linux/delay.h>

#include "fsa.h"

static int fsa;
static struct udevice *fsa_gpiodevs[FSA_MAX] = { NULL };

/* find the ofnode of the FSA i2c bus */
static ofnode fsa_get_ofnode(int fsa)
{
	char str[32];

	/* by alias */
	snprintf(str, sizeof(str), "fsa%d", fsa);
	return ofnode_get_aliases_node(str);
}

static int fsa_get_dtnode(void *fdt, int fsa)
{
	char str[32];

	/* by alias */
	snprintf(str, sizeof(str), "fsa%d", fsa);
	return fdt_path_offset(fdt, fdt_get_alias(fdt, str));
}

static const char * const fsa_gpio_config_names[] = { "NC", "", "input", "output-low",
						      "output-high" };

static const char *fsa_gpio_config_name(struct fsa_gpio_desc *desc)
{
	if (desc->config < ARRAY_SIZE(fsa_gpio_config_names))
		return fsa_gpio_config_names[desc->config];
	return NULL;
};

static char *fsa_get_gpio_desc(struct fsa_gpio_desc *desc, char *str, int sz)
{
	str[0] = 0;
	if (desc->source == 0xff) {
		snprintf(str, sz, "fsa_gpio%d : %s %s",
			 desc->offset + 1,
			 desc->name,
			 fsa_gpio_config_name(desc));
	} else if (desc->config) {
		snprintf(str, sz, "gpio@%02x_%02d: %s %s",
			 desc->source,
			 desc->offset,
			 desc->name,
			 fsa_gpio_config_name(desc));
	}
	return str;
}

static void fsa_show_gpio_descs(const char *prefix, int fsa, struct fsa_board_info *board_info,
				struct fsa_user_info *user_info)
{
	char str[128];
	int i;

	/* display slot specific gpios */
	for (i = 0; i < board_info->sockgpios; i++) {
		fsa_get_gpio_desc(&user_info->gpios[i], str, sizeof(str));
		printf("%s%-2d: %s\n", prefix, i, str);
	}
	/* display io-expander specific gpios */
	if (fsa_gpiodevs[fsa]) {
		for (i = board_info->sockgpios;
		     i < (board_info->sockgpios + board_info->ioexpgpios);
		     i++) {
			fsa_get_gpio_desc(&user_info->gpios[i], str, sizeof(str));
			printf("%s%-2d: %s\n", prefix, i, str);
		}
	}
}

/* detect gpio expander by address and deal with enabling/disabling/adding gpio expander to dt */
static int fsa_get_gpiodev(int fsa, int addr, struct udevice **devp)
{
	struct udevice *bus, *dev;
	char gpio_name[32];
	int ret;

	ret = device_get_global_by_ofnode(fsa_get_ofnode(fsa), &bus);
	if (ret)
		return ret;

	sprintf(gpio_name, "gpio@%02x", addr);

	/* probe device on i2c bus */
	ret = dm_i2c_probe(bus, addr, 0, &dev);
	switch (ret) {
	case -EREMOTEIO: /* chip is not present on i2c bus */
		/* if device is in dt remove/unbind/disable it */
		ret = device_find_child_by_name(bus, gpio_name, &dev);
		if (ret)
			return ret;
		ret = ofnode_set_enabled(dev_ofnode(dev), false);
		if (ret)
			return ret;
		ret = device_unbind(dev);
		if (ret)
			return ret;
		ret = device_remove(dev, DM_REMOVE_NORMAL);
		if (ret)
			return ret;
		return ret;
	case -ENOSYS: /* chip found but driver invalid */
		/* if device is in not in dt add/bind it */
		return ret;
	case 0: /* chip responded and driver bound */
		break;
	}

	if (devp)
		*devp = dev;
	return 0;
}

/* add gpio's to gpio device: GPIO device must be probed before you can manipulate it */
static int fsa_config_gpios(int fsa, struct fsa_user_info *info, int gpios, struct udevice *dev)
{
	struct fsa_gpio_desc *desc;
	struct gpio_desc gdesc;
	struct udevice *gdev;
	int i, ret, flags;
	char name[32];

	/* configure GPIO's */
	for (i = 0; i < gpios; i++) {
		desc = &info->gpios[i];
		if (desc->config < FSA_GPIO_INPUT)
			continue;
		memset(&gdesc, 0, sizeof(gdesc));

		if (desc->source == 0xff) {
			/* Board specific IMX8M GPIO's: find dev of controller by line-name */
			sprintf(name, "fsa%d_gpio%d", fsa, desc->offset + 1);
			uclass_foreach_dev_probe(UCLASS_GPIO, gdev) {
				ret = dev_read_stringlist_search(gdev, "gpio-line-names", name);
				if (ret >= 0) {
					gdesc.dev = gdev;
					gdesc.offset = ret;
					break;
				}
			}
		} else {
			/* port expander GPIOs */
			gdesc.dev = dev;
			gdesc.offset = desc->offset;
		}

		if (!gdesc.dev)
			continue;

		sprintf(name, "fsa%d_%s", fsa, desc->name);
		switch (desc->config) {
		case FSA_GPIO_INPUT:
			flags = GPIOD_IS_IN;
			break;
		case FSA_GPIO_OUTPUT_LOW:
			flags = GPIOD_IS_OUT;
			break;
		case FSA_GPIO_OUTPUT_HIGH:
			flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE;
			break;
		}
		if (!dm_gpio_request(&gdesc, name))
			dm_gpio_clrset_flags(&gdesc, GPIOD_MASK_DIR, flags);
	}

	return 0;
}

static int fsa_read_board_config(int fsa, struct fsa_board_info *info)
{
	struct udevice *dev;
	int chksum;
	int i, ret;
	ofnode node;

	/* find eeprom dev */
	node = ofnode_find_subnode(fsa_get_ofnode(fsa), "eeprom@54");
	if (!ofnode_valid(node))
		return -EINVAL;
	ret = device_get_global_by_ofnode(node, &dev);
	if (ret)
		return ret;

	/* read eeprom */
	ret = dm_i2c_read(dev, 0, (uint8_t *)info, sizeof(*info));
	if (ret) {
		dev_err(dev, "read failed: %d\n", ret);
		return ret;
	}

	/* validate checksum */
	for (chksum = 0, i = 0; i < (int)sizeof(*info) - 2; i++)
		chksum += ((unsigned char *)info)[i];
	if ((info->chksum[0] != ((chksum >> 8) & 0xff)) ||
	    (info->chksum[1] != (chksum & 0xff))) {
		dev_err(dev, "FSA%d EEPROM: Invalid User Config Checksum\n", fsa);
		print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, info, sizeof(*info));
		memset(info, 0, sizeof(*info));
		return -EINVAL;
	}

	return 0;
}

static int fsa_read_user_config(int fsa, struct fsa_user_info *info)
{
	struct udevice *dev;
	int chksum;
	int i, ret;
	ofnode node;

	/* find eeprom dev */
	node = ofnode_find_subnode(fsa_get_ofnode(fsa), "eeprom@55");
	if (!ofnode_valid(node))
		return -EINVAL;
	ret = device_get_global_by_ofnode(node, &dev);
	if (ret)
		return ret;

	/* read eeprom */
	ret = dm_i2c_read(dev, 0, (uint8_t *)info, sizeof(*info));
	if (ret) {
		dev_err(dev, "read failed: %d\n", ret);
		return ret;
	}

	/* validate checksum */
	for (chksum = 0, i = 0; i < (int)sizeof(*info) - 2; i++)
		chksum += ((unsigned char *)info)[i];
	if ((info->chksum[0] != ((chksum >> 8) & 0xff)) ||
	    (info->chksum[1] != (chksum & 0xff))) {
		dev_err(dev, "FSA%d EEPROM: Invalid User Config Checksum\n", fsa);
		print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, info, sizeof(*info));
		memset(info, 0, sizeof(*info));
		return -EINVAL;
	}

	return 0;
}

static int fsa_write_user_config(int fsa, struct fsa_user_info *info)
{
	struct udevice *bus, *dev;
	int i, n, chunk, slave, base, ret;
	ofnode node;
	int chksum;

	/* create checksum */
	for (chksum = 0, i = 0; i < (int)sizeof(*info) - 2; i++)
		chksum += ((unsigned char *)info)[i];
	info->chksum[0] = chksum >> 8;
	info->chksum[1] = chksum & 0xff;

	/* find eeprom dev */
	node = ofnode_find_subnode(fsa_get_ofnode(fsa), "eeprom@55");
	ret = device_get_global_by_ofnode(node, &dev);
	if (ret)
		return ret;
	bus = dev->parent;
	base = dev_read_addr(dev);

	/* write in 16byte chunks (multi-byte writes fail larger than that) */
	chunk = 16;
	slave = -1;
	for (i = 0; i < sizeof(*info); i += chunk) {
		/* select device based on offset */
		if ((base + (i / 256)) != slave) {
			slave = base + (i / 256);
			ret = i2c_get_chip(bus, slave, 1, &dev);
			if (ret) {
				dev_err(bus, "failed to get eeprom@0x%02x: %d\n", slave, ret);
				return ret;
			}
		}
		/* select byte count */
		n = sizeof(*info) - i;
		if (n > chunk)
			n = chunk;
		ret = dm_i2c_write(dev, i % 256, (uint8_t *)info + i, n);
		if (ret) {
			dev_err(dev, "write failed: %d\n", ret);
			return ret;
		}
		mdelay(11);
	}

	return ret;
}

static int fsa_detect(int fsa, struct fsa_board_info *board_info, struct fsa_user_info *user_info,
		      bool gpio)
{
	int ret;

	ret = fsa_read_board_config(fsa, board_info);
	if (ret)
		return ret;
	if (user_info) {
		ret = fsa_read_user_config(fsa, user_info);
		if (ret)
			return ret;
		/* detect port expander */
		if (gpio && !fsa_get_gpiodev(fsa, 0x20, &fsa_gpiodevs[fsa]))
			fsa_config_gpios(fsa, user_info,
					 board_info->sockgpios + board_info->ioexpgpios,
					 fsa_gpiodevs[fsa]);
	}

	return ret;
}

static int ft_fixup_stringlist_elem(void *fdt, int offset, const char *prop, int elem,
				    const char *val)
{
	const char *list, *end;
	char *new, *buf;
	int length;
	int sz = 0;
	int i = 0;
	int ret;

	if (offset < 0 || elem < 0 || !val) {
		printf("%s -EINVAL\n", __func__);
		return -EINVAL;
	}

	list = fdt_getprop(fdt, offset, prop, &length);

	/* no property or invalid params */
	if (!list || length < 0) {
		printf("%s failed - no property\n", __func__);
		return -EINVAL;
	}

	/* create new buffer with enough space */
	buf = calloc(1, length + strlen(val));
	new = buf;

	/* iterate over current stringlist and build new list into buf */
	end = list + length;
	while (list < end) {
		length = strnlen(list, end - list) + 1;
		sz += length;
		/* insert new value into buf */
		if (elem == i) {
			strcpy(new, val);
			new += strlen(val) + 1;
		} else {
			strcpy(new, list);
			new += length;
		}
		list += length;
		i++;
	}
	length = new - buf;
	ret = fdt_setprop(fdt, offset, prop, buf, length);
	free(buf);
	if (ret)
		printf("%s failed %d\n", __func__, ret);

	return ret;
}

static int ft_fixup_fsa_gpio_name(void *fdt, int offset, int fsa, int gpio, const char *name)
{
	const char *prop = "gpio-line-names";
	char str[32];

	sprintf(str, "fsa%d_%s", fsa, name);

	if (!fdt_getprop(fdt, offset, prop, NULL)) {
		char buf[16] = { 0 };

		fdt_setprop(fdt, offset, prop, &buf, sizeof(buf));
	}

	return ft_fixup_stringlist_elem(fdt, offset, prop, gpio, str);
}

static void fsa_show_details(int fsa, struct fsa_board_info *board, struct fsa_user_info *user)
{
	printf("FSA%d: %s\n", fsa, board->model);
	printf("description: %s\n", user->desc);
	printf("overlay: %s\n", user->overlay);
	fsa_show_gpio_descs("\t", fsa, board, user);
}

int fsa_init(void)
{
	struct fsa_board_info board_info;
	struct fsa_user_info user_info;
	int fsa, ret;

	for (fsa = 1; fsa < FSA_MAX; fsa++) {
		ret = fsa_detect(fsa, &board_info, &user_info, true);
		if (!ret)
			printf("FSA%d:  %s %s\n", fsa, board_info.model, user_info.desc);
	}

	return 0;
}

int fsa_show(void)
{
	struct fsa_board_info board_info;
	int fsa, ret;

	for (fsa = 1; fsa < FSA_MAX; fsa++) {
		ret = fsa_detect(fsa, &board_info, NULL, false);
		if (!ret) {
			printf("FSA%d    : %s %d %02x-%02x-%02x%02x\n", fsa,
			       board_info.model, board_info.serial,
			       board_info.mfgdate[0], board_info.mfgdate[1],
			       board_info.mfgdate[2], board_info.mfgdate[3]);
		}
	}
	return 0;
}

/* fixup gpio line names for fsa gpios */
int fsa_ft_fixup(void *fdt)
{
	struct fsa_board_info board_info;
	struct fsa_user_info user_info;
	int fsa, i, ret;
	char path[128];
	char str[32];
	ofnode node;
	int off;

	/* iterate over FSA's and rename gpio's */
	for (fsa = 1; fsa < FSA_MAX; fsa++) {
		/* disable FSA ioexp node if disabled in controlling dt */
		off = fdt_subnode_offset(fdt, fsa_get_dtnode(fdt, fsa), "gpio@20");
		if (off >= 0) {
			if (!fdt_get_path(fdt, off, path, sizeof(path))) {
				node = ofnode_path(path);
				if (ofnode_valid(node) && !ofnode_is_enabled(node))
					fdt_setprop_string(fdt, off, "status", "disabled");
			}
		}

		/* detect FSA eeprom */
		if (fsa_detect(fsa, &board_info, &user_info, false))
			continue;

		/* configure GPIO's */
		for (i = 0; i < board_info.sockgpios + board_info.ioexpgpios; i++) {
			if (user_info.gpios[i].config < FSA_GPIO_INPUT)
				continue;

			if (user_info.gpios[i].source == 0xff) {
				/* Board specific IMX8M GPIO's */
				for (off = fdt_node_offset_by_prop_value(fdt, 0,
									 "gpio-controller", NULL,
									 0);
				     off >= 0;
				     off = fdt_node_offset_by_prop_value(fdt, off,
									 "gpio-controller", NULL,
									 0)
				    ) {
					sprintf(str, "fsa%d_gpio%d", fsa,
						user_info.gpios[i].offset + 1);
					ret = fdt_stringlist_search(fdt, off, "gpio-line-names",
								    str);
					if (ret >= 0) {
						ft_fixup_fsa_gpio_name(fdt, off, fsa, ret,
								       user_info.gpios[i].name);
						break;
					}
				}
			} else {
				/* port expander GPIOs */
				off = fdt_subnode_offset(fdt, fsa_get_dtnode(fdt, fsa), "gpio@20");
				ft_fixup_fsa_gpio_name(fdt, off, fsa, user_info.gpios[i].offset,
						       user_info.gpios[i].name);
			}
		}
	}

	return 0;
}

static int do_fsa_dev(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct fsa_board_info board_info;
	struct fsa_user_info user_info;
	int i;

	if (argc < 2) {
		/* list FSAs */
		printf("detecting FSA Adapters:\n");
		for (i = 1; i < FSA_MAX; i++) {
			if (!fsa_read_board_config(i, &board_info) &&
			    !fsa_read_user_config(i, &user_info))
				printf("FSA%d    : %s %s\n", i, board_info.model, user_info.desc);
		}
	} else {
		/* select FSA */
		fsa = simple_strtoul(argv[1], NULL, 10);
	}

	if (fsa) {
		/* read FSA */
		if (!fsa_read_board_config(fsa, &board_info) &&
		    !fsa_read_user_config(fsa, &user_info)) {
			printf("selected:\n");
			fsa_show_details(fsa, &board_info, &user_info);
		} else {
			printf("FSA%d not detected\n", fsa);
			fsa = 0;
		}
	} else {
		printf("no FSA currently selected\n");
	}

	return 0;
}

static int do_fsa_desc(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct fsa_board_info board_info;
	struct fsa_user_info user_info;

	/* strip off leading cmd arg */
	argc--;
	argv++;

	if (!fsa) {
		printf("No FSA selected\n");
		return CMD_RET_USAGE;
	}

	if (fsa_read_board_config(fsa, &board_info) || fsa_read_user_config(fsa, &user_info)) {
		printf("can't detect FSA%d\n", fsa);
		return CMD_RET_USAGE;
	}

	/* set */
	if (argc) {
		strlcpy(user_info.desc, argv[0], sizeof(user_info.desc));
		if (fsa_write_user_config(fsa, &user_info))
			return CMD_RET_FAILURE;
	}

	/* show */
	fsa_show_details(fsa, &board_info, &user_info);

	return CMD_RET_SUCCESS;
}

static int do_fsa_overlay(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct fsa_board_info board_info;
	struct fsa_user_info user_info;

	/* strip off leading cmd arg */
	argc--;
	argv++;

	if (!fsa) {
		printf("No FSA selected\n");
		return CMD_RET_USAGE;
	}

	if (fsa_read_board_config(fsa, &board_info) || fsa_read_user_config(fsa, &user_info)) {
		printf("can't detect FSA%d\n", fsa);
		return CMD_RET_USAGE;
	}

	/* set */
	if (argc) {
		strlcpy(user_info.overlay, argv[0], sizeof(user_info.overlay));
		if (fsa_write_user_config(fsa, &user_info))
			return CMD_RET_FAILURE;
	}

	/* show */
	fsa_show_details(fsa, &board_info, &user_info);

	return CMD_RET_SUCCESS;
}

static int do_fsa_gpio(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct fsa_board_info board_info;
	struct fsa_user_info user_info;
	struct fsa_gpio_desc desc;
	char str[64];
	int i, j;

	/* strip off leading cmd arg */
	argc--;
	argv++;

	if (!fsa) {
		printf("No FSA selected\n");
		return CMD_RET_USAGE;
	}

	if (fsa_read_board_config(fsa, &board_info) || fsa_read_user_config(fsa, &user_info)) {
		printf("can't detect FSA%d\n", fsa);
		return CMD_RET_USAGE;
	}

	if (!argc) {
		/* show all gpios */
		fsa_show_gpio_descs("\t", fsa, &board_info, &user_info);
		return CMD_RET_SUCCESS;
	}

	if (!isdigit(argv[0][0])) {
		printf("invalid gpio offset: %s\n", argv[0]);
		return CMD_RET_USAGE;
	}

	memset(&desc, 0, sizeof(desc));
	i = simple_strtoul(argv[0], NULL, 10);

	if (i >= 0 && i < board_info.sockgpios) {
		desc.offset = i;
		desc.source = 0xff;
	} else if (i >= board_info.sockgpios &&
		 i < (board_info.sockgpios + board_info.ioexpgpios) &&
		 fsa_gpiodevs[fsa]) {
		desc.offset = i - board_info.sockgpios;
		desc.source = 0x20;
	} else {
		printf("invalid index %d", i);
		return CMD_RET_FAILURE;
	}

	if (argc > 1) {
		if (user_info.gpios[i].config == FSA_GPIO_NC) {
			printf("can not alter NC gpio\n");
			return CMD_RET_FAILURE;
		}
		strlcpy(desc.name, argv[1], sizeof(desc.name));
		if (!*desc.name) {
			printf("FSA%d %s erasing gpio %d\n", fsa, board_info.model, i);
			memset(&user_info.gpios[i], 0, sizeof(desc));
			if (fsa_write_user_config(fsa, &user_info))
				return CMD_RET_FAILURE;
			return CMD_RET_SUCCESS;
		}
	}
	if (argc > 2) {
		if (user_info.gpios[i].config == FSA_GPIO_NC) {
			printf("can not alter NC gpio\n");
			return CMD_RET_FAILURE;
		}
		for (j = 1; j < ARRAY_SIZE(fsa_gpio_config_names); j++) {
			if (!strcasecmp(argv[2], fsa_gpio_config_names[j])) {
				desc.config = j;
				break;
			}
		};
		if (j >= ARRAY_SIZE(fsa_gpio_config_names)) {
			printf("invalid config type '%s\n", argv[2]);
			return CMD_RET_FAILURE;
		}
	}

	/* show a specific gpio */
	if (argc == 1) {
		printf("FSA%d %s showing gpio %d\n", fsa, board_info.model, i);
		printf("%s\n", fsa_get_gpio_desc(&user_info.gpios[i], str, sizeof(str)));
		return CMD_RET_SUCCESS;
	}

	/* set a specific gpio */
	else if (argc == 3) {
		printf("FSA%d %s updating gpio %d\n", fsa, board_info.model, i);
		memcpy(&user_info.gpios[i], &desc, sizeof(desc));
		if (fsa_write_user_config(fsa, &user_info))
			return CMD_RET_FAILURE;
		return CMD_RET_SUCCESS;
	}

	return CMD_RET_USAGE;
}

static struct cmd_tbl cmd_fsa_sub[] = {
	U_BOOT_CMD_MKENT(dev, 1, 1, do_fsa_dev, "", ""),
	U_BOOT_CMD_MKENT(gpio, 4, 1, do_fsa_gpio, "", ""),
	U_BOOT_CMD_MKENT(description, 1, 1, do_fsa_desc, "", ""),
	U_BOOT_CMD_MKENT(overlay, 1, 1, do_fsa_overlay, "", ""),
};

static int do_fsa(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cmd_tbl *c;

	/* strip off leading fsa arg */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_fsa_sub, ARRAY_SIZE(cmd_fsa_sub));
	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	return CMD_RET_USAGE;
}

U_BOOT_LONGHELP(fsa,
		"dev [dev] - show or set current FSA adapter\n"
		"fsa gpio - show current gpio descriptors\n"
		"fsa gpio [<offset>]|[<offset> <source>] - show a specific gpio descriptor\n"
		"fsa gpio [<offset> <name> <input|output-low|output-high> [source]] - set a gpio descriptor\n"
		"fsa description [description] - show or set the FSA user description string\n"
		"fsa overlay [overlay] - show or set the FSA overlay string\n"
);

U_BOOT_CMD(fsa, 6, 1, do_fsa,
	   "Flexible Socket Adapter",
	   fsa_help_text
);
