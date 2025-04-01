ccflags-y += -I$(srctree)/lib/lwip/lwip/src/include -I$(srctree)/lib/lwip/u-boot

obj-$(CONFIG_$(PHASE_)DM_ETH) += net-lwip.o
obj-$(CONFIG_CMD_DHCP) += dhcp.o
obj-$(CONFIG_CMD_DNS) += dns.o
obj-$(CONFIG_CMD_PING) += ping.o
obj-$(CONFIG_CMD_TFTPBOOT) += tftp.o
obj-$(CONFIG_WGET) += wget.o

ifeq (y,$(CONFIG_WGET_BUILTIN_CACERT))
$(obj)/builtin_cacert.c: $(CONFIG_WGET_BUILTIN_CACERT_PATH:"%"=%) FORCE
	$(call if_changed,bin2c,builtin_cacert)
obj-y += builtin_cacert.o
endif
