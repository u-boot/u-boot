ifndef CONFIG_SANDBOX

ifdef FTRACE
subdir-ccflags-y += -finstrument-functions -DFTRACE
endif

subdir-y += standalone
subdir-$(CONFIG_API) += api
endif
