ccflags-y += -I$(srctree)/lib/lwip/lwip/src/include -I$(srctree)/lib/lwip/u-boot

obj-$(CONFIG_$(SPL_TPL_)BOOTDEV_ETH) += ../net/eth_bootdev.o
obj-$(CONFIG_DM_MDIO)  += ../net/mdio-uclass.o
obj-$(CONFIG_DM_MDIO_MUX) += ../net/mdio-mux-uclass.o
obj-$(CONFIG_$(SPL_)DM_ETH) += ../net/eth_common.o
obj-$(CONFIG_$(SPL_)DM_ETH) += ../net/eth-uclass.o
obj-$(CONFIG_$(SPL_)DM_ETH) += net-lwip.o
obj-$(CONFIG_CMD_DHCP) += dhcp.o
obj-$(CONFIG_CMD_DNS) += dns.o
obj-$(CONFIG_CMD_PING) += ping.o
obj-$(CONFIG_CMD_TFTPBOOT) += tftp.o
obj-$(CONFIG_CMD_WGET) += wget.o

# Disable this warning as it is triggered by:
# sprintf(buf, index ? "foo%d" : "foo", index)
# and this is intentional usage.
CFLAGS_eth_common.o += -Wno-format-extra-args
