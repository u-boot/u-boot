#
# (C) Copyright 2000, 2001, 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

HOSTARCH := $(shell uname -m | \
	sed -e s/i.86/i386/ \
	    -e s/sun4u/sparc64/ \
	    -e s/arm.*/arm/ \
	    -e s/sa110/arm/ \
	    -e s/powerpc/ppc/ \
	    -e s/macppc/ppc/)

HOSTOS := $(shell uname -s | tr A-Z a-z | \
	    sed -e 's/\(cygwin\).*/cygwin/')

export	HOSTARCH

# Deal with colliding definitions from tcsh etc.
VENDOR=

#########################################################################

TOPDIR	:= $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
export	TOPDIR

ifeq (include/config.mk,$(wildcard include/config.mk))
# load ARCH, BOARD, and CPU configuration
include include/config.mk
export	ARCH CPU BOARD VENDOR
# load other configuration
include $(TOPDIR)/config.mk

ifndef CROSS_COMPILE
ifeq ($(HOSTARCH),ppc)
CROSS_COMPILE =
else
ifeq ($(ARCH),ppc)
CROSS_COMPILE = ppc_8xx-
endif
ifeq ($(ARCH),arm)
CROSS_COMPILE = arm_920TDI-
endif
ifeq ($(ARCH),i386)
#CROSS_COMPILE = i386-elf-
endif
ifeq ($(ARCH),mips)
CROSS_COMPILE = mips_4KC-
endif
endif
endif

export	CROSS_COMPILE

# The "tools" are needed early, so put this first
SUBDIRS	= tools \
	  lib_generic \
	  lib_$(ARCH) \
	  cpu/$(CPU) \
	  board/$(BOARDDIR) \
	  common \
	  disk \
	  fs \
	  net \
	  rtc \
	  dtt \
	  drivers \
	  post \
	  post/cpu \
	  examples

#########################################################################
# U-Boot objects....order is important (i.e. start must be first)

OBJS  =	cpu/$(CPU)/start.o
ifeq ($(CPU),i386)
OBJS +=	cpu/$(CPU)/start16.o
OBJS +=	cpu/$(CPU)/reset.o
endif
ifeq ($(CPU),ppc4xx)
OBJS +=	cpu/$(CPU)/resetvec.o
endif

LIBS  =	board/$(BOARDDIR)/lib$(BOARD).a
LIBS += cpu/$(CPU)/lib$(CPU).a
LIBS += lib_$(ARCH)/lib$(ARCH).a
LIBS += fs/jffs2/libjffs2.a fs/fdos/libfdos.a
LIBS += net/libnet.a
LIBS += disk/libdisk.a
LIBS += rtc/librtc.a
LIBS += dtt/libdtt.a
LIBS += drivers/libdrivers.a
LIBS += post/libpost.a post/cpu/libcpu.a
LIBS += common/libcommon.a
LIBS += lib_generic/libgeneric.a

#########################################################################

all:		u-boot.srec u-boot.bin System.map

install:	all
		-cp u-boot.bin /tftpboot/u-boot.bin
		-cp u-boot.bin /net/denx/tftpboot/u-boot.bin

u-boot.srec:	u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O srec $< $@

u-boot.bin:	u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

u-boot.dis:	u-boot
		$(OBJDUMP) -d $< > $@

u-boot:		depend subdirs $(OBJS) $(LIBS) $(LDSCRIPT)
		$(LD) $(LDFLAGS) $(OBJS) \
			--start-group $(LIBS) --end-group \
			-Map u-boot.map -o u-boot

subdirs:
		@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir || exit 1 ; done

depend dep:
		@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir .depend ; done

tags:
		ctags -w `find $(SUBDIRS) include \
			\( -name CVS -prune \) -o \( -name '*.[ch]' -print \)`

etags:
		etags -a `find $(SUBDIRS) include \
			\( -name CVS -prune \) -o \( -name '*.[ch]' -print \)`

System.map:	u-boot
		@$(NM) $< | \
		grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
		sort > System.map

#########################################################################
else
all install u-boot u-boot.srec depend dep:
	@echo "System not configured - see README" >&2
	@ exit 1
endif

#########################################################################

unconfig:
	rm -f include/config.h include/config.mk

#========================================================================
# PowerPC
#========================================================================
#########################################################################
## MPC8xx Systems
#########################################################################

ADS860_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx fads

AMX860_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx amx860 westel

c2mon_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx c2mon

CCM_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx CCM siemens

cogent_mpc8xx_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx cogent

ELPT860_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx elpt860 LEOX

ESTEEM192E_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx esteem192e

ETX094_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx etx094

FADS823_config	\
FADS850SAR_config \
FADS860T_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx fads

FLAGADM_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx flagadm

GEN860T_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx gen860t

GENIETV_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx genietv

GTH_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx gth

hermes_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx hermes

IAD210_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx IAD210 siemens

xtract_ICU862 = $(subst _100MHz,,$(subst _config,,$1))

ICU862_100MHz_config	\
ICU862_config: unconfig
	@ >include/config.h
	@[ -z "$(findstring _100MHz,$@)" ] || \
		{ echo "#define CONFIG_100MHz"	>>include/config.h ; \
		  echo "... with 100MHz system clock" ; \
		}
	@./mkconfig -a $(call xtract_ICU862,$@) ppc mpc8xx icu862

IP860_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx ip860

IVML24_256_config \
IVML24_128_config \
IVML24_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring IVML24_config,$@)" ] || \
		 { echo "#define CONFIG_IVML24_16M"	>>include/config.h ; \
		 }
	@[ -z "$(findstring IVML24_128_config,$@)" ] || \
		 { echo "#define CONFIG_IVML24_32M"	>>include/config.h ; \
		 }
	@[ -z "$(findstring IVML24_256_config,$@)" ] || \
		 { echo "#define CONFIG_IVML24_64M"	>>include/config.h ; \
		 }
	@./mkconfig -a IVML24 ppc mpc8xx ivm

IVMS8_256_config \
IVMS8_128_config \
IVMS8_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring IVMS8_config,$@)" ] || \
		 { echo "#define CONFIG_IVMS8_16M"	>>include/config.h ; \
		 }
	@[ -z "$(findstring IVMS8_128_config,$@)" ] || \
		 { echo "#define CONFIG_IVMS8_32M"	>>include/config.h ; \
		 }
	@[ -z "$(findstring IVMS8_256_config,$@)" ] || \
		 { echo "#define CONFIG_IVMS8_64M"	>>include/config.h ; \
		 }
	@./mkconfig -a IVMS8 ppc mpc8xx ivm

KUP4K_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx kup4k

LANTEC_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx lantec

lwmon_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx lwmon

MBX_config	\
MBX860T_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx mbx8xx

MHPC_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx mhpc eltec

MVS1_config :		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx mvs1

NETVIA_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx netvia

NX823_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx nx823

pcu_e_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx pcu_e siemens

R360MPI_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx r360mpi

RPXClassic_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx RPXClassic

RPXlite_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx RPXlite

RRvision_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx RRvision

RRvision_LCD_config:	unconfig
	@echo "#define CONFIG_LCD" >include/config.h
	@echo "#define CONFIG_SHARP_LQ104V7DS01" >>include/config.h
	@./mkconfig -a RRvision ppc mpc8xx RRvision

SM850_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx tqm8xx

SPD823TS_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx spd8xx

SXNI855T_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx sixnet

# EMK MPC8xx based modules
TOP860_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx top860 emk

# Play some tricks for configuration selection
# All boards can come with 50 MHz (default), 66MHz or 80MHz clock,
# but only 855 and 860 boards may come with FEC
# and 823 boards may have LCD support
xtract_8xx = $(subst _66MHz,,$(subst _80MHz,,$(subst _LCD,,$(subst _FEC,,$(subst _config,,$1)))))

FPS850L_config		\
FPS860L_config		\
TQM823L_config		\
TQM823L_66MHz_config	\
TQM823L_80MHz_config	\
TQM823L_LCD_config	\
TQM823L_LCD_66MHz_config \
TQM823L_LCD_80MHz_config \
TQM850L_config		\
TQM850L_66MHz_config	\
TQM850L_80MHz_config	\
TQM855L_config		\
TQM855L_66MHz_config	\
TQM855L_80MHz_config	\
TQM855L_FEC_config	\
TQM855L_FEC_66MHz_config \
TQM855L_FEC_80MHz_config \
TQM860L_config		\
TQM860L_66MHz_config	\
TQM860L_80MHz_config	\
TQM860L_FEC_config	\
TQM860L_FEC_66MHz_config	\
TQM860L_FEC_80MHz_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring _FEC,$@)" ] || \
		{ echo "#define CONFIG_FEC_ENET"	>>include/config.h ; \
		  echo "... with FEC support" ; \
		}
	@[ -z "$(findstring _66MHz,$@)" ] || \
		{ echo "#define CONFIG_66MHz"		>>include/config.h ; \
		  echo "... with 66MHz system clock" ; \
		}
	@[ -z "$(findstring _80MHz,$@)" ] || \
		{ echo "#define CONFIG_80MHz"		>>include/config.h ; \
		  echo "... with 80MHz system clock" ; \
		}
	@[ -z "$(findstring _LCD,$@)" ] || \
		{ echo "#define CONFIG_LCD"		>>include/config.h ; \
		  echo "#define CONFIG_NEC_NL6648BC20"	>>include/config.h ; \
		  echo "... with LCD display" ; \
		}
	@./mkconfig -a $(call xtract_8xx,$@) ppc mpc8xx tqm8xx

TTTech_config:	unconfig
	@echo "#define CONFIG_LCD" >include/config.h
	@echo "#define CONFIG_SHARP_LQ104V7DS01" >>include/config.h
	@./mkconfig -a TQM823L ppc mpc8xx tqm8xx

v37_config:	unconfig
	@echo "#define CONFIG_LCD" >include/config.h
	@echo "#define CONFIG_SHARP_LQ084V1DG21" >>include/config.h
	@./mkconfig $(@:_config=) ppc mpc8xx v37

#########################################################################
## PPC4xx Systems
#########################################################################

ADCIOP_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx adciop esd

AR405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ar405 esd

CANBT_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx canbt esd

CPCI405_config		\
CPCI4052_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx cpci405 esd
	@echo "BOARD_REVISION = $(@:_config=)"	>>include/config.mk

CPCI440_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx cpci440 esd

CPCIISER4_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx cpciiser4 esd

CRAYL1_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx L1 cray

DASA_SIM_config: unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx dasa_sim esd

DU405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx du405 esd

EBONY_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ebony

ERIC_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx eric

MIP405_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx mip405 mpl

ML2_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ml2

OCRTC_config		\
ORSG_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ocrtc esd

PCI405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx pci405 esd

PIP405_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx pip405 mpl

W7OLMC_config	\
W7OLMG_config: unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx w7o

WALNUT405_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx walnut405

#########################################################################
## MPC824x Systems
#########################################################################
xtract_82xx = $(subst _ROMBOOT,,$(subst _L2,,$(subst _266MHz,,$(subst _300MHz,,$(subst _config,,$1)))))

BMW_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x bmw

CPC45_config	\
CPC45_ROMBOOT_config:	unconfig
	@./mkconfig $(call xtract_82xx,$@) ppc mpc824x cpc45
	@cd ./include ;				\
	if [ "$(findstring _ROMBOOT_,$@)" ] ; then \
		echo "CONFIG_BOOT_ROM = y" >> config.mk ; \
		echo "... booting from 8-bit flash" ; \
	else \
		echo "CONFIG_BOOT_ROM = n" >> config.mk ; \
		echo "... booting from 64-bit flash" ; \
	fi; \
	echo "export CONFIG_BOOT_ROM" >> config.mk;

CU824_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x cu824

MOUSSE_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x mousse

MUSENKI_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x musenki

OXC_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x oxc

PN62_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x pn62

Sandpoint8240_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x sandpoint

Sandpoint8245_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x sandpoint

utx8245_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x utx8245

#########################################################################
## MPC8260 Systems
#########################################################################

cogent_mpc8260_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 cogent

CPU86_config	\
CPU86_ROMBOOT_config: unconfig
	@./mkconfig $(call xtract_82xx,$@) ppc mpc8260 cpu86
	@cd ./include ;				\
	if [ "$(findstring _ROMBOOT_,$@)" ] ; then \
		echo "CONFIG_BOOT_ROM = y" >> config.mk ; \
		echo "... booting from 8-bit flash" ; \
	else \
		echo "CONFIG_BOOT_ROM = n" >> config.mk ; \
		echo "... booting from 64-bit flash" ; \
	fi; \
	echo "export CONFIG_BOOT_ROM" >> config.mk;

ep8260_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 ep8260

gw8260_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 gw8260

hymod_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 hymod

IPHASE4539_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 iphase4539

MPC8260ADS_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 mpc8260ads

MPC8266ADS_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 mpc8266ads

PM826_config	\
PM826_ROMBOOT_config: unconfig
	@./mkconfig $(call xtract_82xx,$@) ppc mpc8260 pm826
	@cd ./include ;				\
	if [ "$(findstring _ROMBOOT_,$@)" ] ; then \
		echo "CONFIG_BOOT_ROM = y" >> config.mk ; \
		echo "... booting from 8-bit flash" ; \
	else \
		echo "CONFIG_BOOT_ROM = n" >> config.mk ; \
		echo "... booting from 64-bit flash" ; \
	fi; \
	echo "export CONFIG_BOOT_ROM" >> config.mk; \

ppmc8260_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 ppmc8260

RPXsuper_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 rpxsuper

rsdproto_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 rsdproto

sacsng_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 sacsng

sbc8260_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 sbc8260

SCM_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 SCM siemens

TQM8260_config	\
TQM8260_L2_config	\
TQM8260_266MHz_config	\
TQM8260_L2_266MHz_config \
TQM8260_300MHz_config:	unconfig
	@ >include/config.h
	@if [ "$(findstring _L2_,$@)" ] ; then \
		echo "#define CONFIG_L2_CACHE"	>>include/config.h ; \
		echo "... with L2 Cache support (60x Bus Mode)" ; \
	else \
		echo "#undef CONFIG_L2_CACHE"	>>include/config.h ; \
		echo "... without L2 Cache support" ; \
	fi
	@[ -z "$(findstring _266MHz,$@)" ] || \
		{ echo "#define CONFIG_266MHz"	>>include/config.h ; \
		  echo "... with 266MHz system clock" ; \
		}
	@[ -z "$(findstring _300MHz,$@)" ] || \
		{ echo "#define CONFIG_300MHz"	>>include/config.h ; \
		  echo "... with 300MHz system clock" ; \
		}
	@./mkconfig -a $(call xtract_82xx,$@) ppc mpc8260 tqm8260

#########################################################################
## 74xx/7xx Systems
#########################################################################

AmigaOneG3SE_config:	unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx AmigaOneG3SE MAI

EVB64260_config	\
EVB64260_750CX_config:	unconfig
	@./mkconfig EVB64260 ppc 74xx_7xx evb64260

ZUMA_config:	unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx evb64260

PCIPPC2_config \
PCIPPC6_config: unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx pcippc2

BAB7xx_config: unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx bab7xx eltec

ELPPC_config: unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx elppc eltec

#========================================================================
# ARM
#========================================================================
#########################################################################
## StrongARM Systems
#########################################################################

lart_config	:	unconfig
	@./mkconfig $(@:_config=) arm sa1100 lart

dnp1110_config	:	unconfig
	@./mkconfig $(@:_config=) arm sa1100 dnp1110

shannon_config	:	unconfig
	@./mkconfig $(@:_config=) arm sa1100 shannon

#########################################################################
## ARM920T Systems
#########################################################################

xtract_trab = $(subst _big_flash,,$(subst _config,,$1))

smdk2400_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t smdk2400

smdk2410_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t smdk2410

trab_config \
trab_big_flash_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring _big_flash,$@)" ] || \
		{ echo "#define CONFIG_BIG_FLASH" >>include/config.h ; \
		  echo "... with big flash support" ; \
		}
	@./mkconfig -a $(call xtract_trab,$@) arm arm920t trab

VCMA9_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t vcma9 mpl

#########################################################################
## ARM720T Systems
#########################################################################

impa7_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm720t impa7

ep7312_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm720t ep7312

#########################################################################
## XScale Systems
#########################################################################

cradle_config	:	unconfig
	@./mkconfig $(@:_config=) arm xscale cradle

csb226_config	:	unconfig
	@./mkconfig $(@:_config=) arm xscale csb226

innokom_config	:	unconfig
	@./mkconfig $(@:_config=) arm xscale innokom

lubbock_config	:	unconfig
	@./mkconfig $(@:_config=) arm xscale lubbock

#========================================================================
# i386
#========================================================================
#########################################################################
## AMD SC520 CDP
#########################################################################
sc520_cdp_config	:	unconfig
	@./mkconfig $(@:_config=) i386 i386 sc520_cdp

#========================================================================
# MIPS
#========================================================================
#########################################################################
## MIPS32 4Kc
#########################################################################

incaip_config :		unconfig
	@./mkconfig $(@:_config=) mips mips incaip



clean:
	find . -type f \
		\( -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'  -o -name '*.a'  \) -print \
		| xargs rm -f
	rm -f examples/hello_world examples/timer examples/eepro100_eeprom
	rm -f tools/img2srec tools/mkimage tools/envcrc tools/gen_eth_addr
	rm -f tools/easylogo/easylogo tools/bmp_logo
	rm -f tools/gdb/astest tools/gdb/gdbcont tools/gdb/gdbsend
	rm -f tools/env/fw_printenv tools/env/fw_setenv

clobber:	clean
	find . -type f \
		\( -name .depend -o -name '*.srec' -o -name '*.bin' \) \
		-print \
		| xargs rm -f
	rm -f $(OBJS) *.bak tags TAGS
	rm -fr *.*~
	rm -f u-boot u-boot.bin u-boot.elf u-boot.srec u-boot.map System.map
	rm -f tools/crc32.c tools/environment.c tools/env/crc32.c
	rm -f cpu/mpc824x/bedbug_603e.c
	rm -f include/asm/arch include/asm

mrproper \
distclean:	clobber unconfig

backup:
	F=`basename $(TOPDIR)` ; cd .. ; \
	gtar --force-local -zcvf `date "+$$F-%Y-%m-%d-%T.tar.gz"` $$F

#########################################################################
