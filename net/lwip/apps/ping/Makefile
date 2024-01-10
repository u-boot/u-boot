ccflags-y += -I$(srctree)/net/lwip/port/include
ccflags-y += -I$(srctree)/net/lwip/lwip-external/src/include -I$(srctree)/net/lwip
ccflags-y += -I$(obj)

# ping.c includes "ping.h", copy it to local directory, to override ping.h
.PHONY: $(obj)/ping.c
$(obj)/ping.o: $(obj)/ping.c
$(obj)/ping.c:
	cp $(srctree)/net/lwip/lwip-external/contrib/apps/ping/ping.c $(obj)/ping.c

obj-y += ping.o
obj-y += lwip_ping.o
