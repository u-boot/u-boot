# SPDX-License-Identifier: GPL-2.0+
obj-$(CONFIG_AUTOBOOT) += test_autoboot.o

ifneq ($(CONFIG_$(XPL_)BLOBLIST),)
ifdef CONFIG_BLOBLIST_FIXED
obj-$(CONFIG_$(XPL_)CMDLINE) += bloblist.o
endif
endif

obj-$(CONFIG_CYCLIC) += cyclic.o
obj-$(CONFIG_EVENT_DYNAMIC) += event.o
obj-y += cread.o
obj-$(CONFIG_$(XPL_)CMDLINE) += print.o
