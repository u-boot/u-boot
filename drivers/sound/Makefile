# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2012 Samsung Electronics
# R. Chandrasekar <rcsekar@samsung.com>

obj-$(CONFIG_SOUND)	+= sound.o
obj-$(CONFIG_DM_SOUND)	+= codec-uclass.o
obj-$(CONFIG_DM_SOUND)	+= i2s-uclass.o
obj-$(CONFIG_DM_SOUND)	+= sound-uclass.o
obj-$(CONFIG_I2S_SAMSUNG)	+= samsung-i2s.o
obj-$(CONFIG_SOUND_SANDBOX)	+= sandbox.o
ifdef CONFIG_DM_SOUND
obj-$(CONFIG_I2S_SAMSUNG)	+= samsung_sound.o
else
obj-$(CONFIG_I2S)	+= sound-i2s.o
endif
obj-$(CONFIG_SOUND_WM8994)	+= wm8994.o
obj-$(CONFIG_SOUND_MAX98095)	+= max98095.o
