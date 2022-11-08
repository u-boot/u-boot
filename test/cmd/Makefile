# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2013 Google, Inc

ifdef CONFIG_HUSH_PARSER
obj-$(CONFIG_CONSOLE_RECORD) += test_echo.o
endif
ifdef CONFIG_CONSOLE_RECORD
obj-$(CONFIG_CMD_PAUSE) += test_pause.o
endif
obj-y += mem.o
obj-$(CONFIG_CMD_ADDRMAP) += addrmap.o
obj-$(CONFIG_CMD_FDT) += fdt.o
obj-$(CONFIG_CONSOLE_TRUETYPE) += font.o
obj-$(CONFIG_CMD_LOADM) += loadm.o
obj-$(CONFIG_CMD_MEM_SEARCH) += mem_search.o
obj-$(CONFIG_CMD_PINMUX) += pinmux.o
obj-$(CONFIG_CMD_PWM) += pwm.o
ifdef CONFIG_SANDBOX
obj-$(CONFIG_CMD_SETEXPR) += setexpr.o
endif
obj-$(CONFIG_CMD_TEMPERATURE) += temperature.o
obj-$(CONFIG_CMD_WGET) += wget.o
