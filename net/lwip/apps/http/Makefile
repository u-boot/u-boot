ccflags-y += -I$(srctree)/net/lwip/port/include
ccflags-y += -I$(srctree)/net/lwip/lwip-external/src/include -I$(srctree)/net/lwip
ccflags-y += -I$(srctree)/net/lwip/lwip-external/src/include/lwip/apps

obj-$(CONFIG_CMD_WGET) += ../../lwip-external/src/apps/http/http_client.o
obj-$(CONFIG_CMD_WGET) += lwip-wget.o
