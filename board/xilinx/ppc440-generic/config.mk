# need to strip off double quotes
ifneq ($(CONFIG_SYS_LDSCRIPT),)
LDSCRIPT := $(subst ",,$(CONFIG_SYS_LDSCRIPT))
endif
