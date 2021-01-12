# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
#

ifdef CONFIG_DM_VIDEO
obj-$(CONFIG_AM335X_LCD) += tilcdc.o tilcdc-panel.o
else
obj-$(CONFIG_AM335X_LCD) += am335x-fb.o
endif
