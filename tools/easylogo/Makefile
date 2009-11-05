include $(TOPDIR)/config.mk

all: $(obj)easylogo

$(obj)easylogo: $(SRCTREE)/tools/easylogo/easylogo.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^

clean:
	rm -f $(obj)easylogo

.PHONY: all clean
