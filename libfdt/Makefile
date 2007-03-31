PREFIX = /usr/local
TARGETLIBS = libfdt.a
LIBOBJS = fdt.o fdt_ro.o fdt_wip.o fdt_sw.o fdt_rw.o fdt_strerror.o

SOURCE = $(shell find . -maxdepth 1 ! -name version.h -a -name '*.[h]')
SOURCE += *.c Makefile
NODEPTARGETS=<clean>

CPPFLAGS = -I.
CFLAGS = -Wall -g

LIBDIR = $(PREFIX)/$(LIB32)

EXTRA_DIST = \
	README \
	HOWTO \
	LGPL-2.1

ifdef V
VECHO = :
else
VECHO = echo "	"
ARFLAGS = rc
.SILENT:
endif

DEPFILES = $(LIBOBJS:%.o=%.d)

all:	libs tests

.PHONY:	tests libs

libs:	$(TARGETLIBS)

tests:	tests/all

tests/%: libs
	$(MAKE) -C tests $*

check:	all
	cd tests; ./run_tests.sh

checkv:	all
	cd tests; ./run_tests.sh -v

func:	all
	cd tests; ./run_tests.sh -t func

funcv:	all
	cd tests; ./run_tests.sh -t func -v

stress:	all
	cd tests; ./run_tests.sh -t stress

stressv: all
	cd tests; ./run_tests.sh -t stress -v

%.o: %.c
	@$(VECHO) CC $@
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

libfdt.a: $(LIBOBJS)
	@$(VECHO) AR $@
	$(AR) $(ARFLAGS) $@ $^

%.i:	%.c
	@$(VECHO) CPP $@
	$(CC) $(CPPFLAGS) -E $< > $@

%.s:	%.c
	@$(VECHO) CC -S $@
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -S $<

clean:
	@$(VECHO) CLEAN
	rm -f *~ *.o *.so *.a *.d *.i *.s core a.out $(VERSION)
	$(MAKE) -C tests clean

%.d: %.c
	@$(CC) $(CPPFLAGS) -MM -MT "$*.o $@" $< > $@

# Workaround: Don't build dependencies for certain targets
#    When the include below is executed, make will use the %.d target above to
# generate missing files.  For certain targets (clean, version.h, etc) we don't
# need or want these dependency files, so don't include them in this case.
ifeq (,$(findstring <$(MAKECMDGOALS)>,$(NODEPTARGETS)))
-include $(DEPFILES)
endif
