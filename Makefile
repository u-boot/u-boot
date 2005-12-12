#
# (C) Copyright 2000-2005
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

HOSTOS := $(shell uname -s | tr '[:upper:]' '[:lower:]' | \
	    sed -e 's/\(cygwin\).*/cygwin/')

export	HOSTARCH HOSTOS

# Deal with colliding definitions from tcsh etc.
VENDOR=

#########################################################################

TOPDIR	:= $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
export	TOPDIR

ifeq (include/config.mk,$(wildcard include/config.mk))
# load ARCH, BOARD, and CPU configuration
include include/config.mk
export	ARCH CPU BOARD VENDOR SOC
# load other configuration
include $(TOPDIR)/config.mk

ifndef CROSS_COMPILE
ifeq ($(HOSTARCH),ppc)
CROSS_COMPILE =
else
ifeq ($(ARCH),ppc)
CROSS_COMPILE = powerpc-linux-
endif
ifeq ($(ARCH),arm)
CROSS_COMPILE = arm-linux-
endif
ifeq ($(ARCH),i386)
ifeq ($(HOSTARCH),i386)
CROSS_COMPILE =
else
CROSS_COMPILE = i386-linux-
endif
endif
ifeq ($(ARCH),mips)
CROSS_COMPILE = mips_4KC-
endif
ifeq ($(ARCH),nios)
CROSS_COMPILE = nios-elf-
endif
ifeq ($(ARCH),nios2)
CROSS_COMPILE = nios2-elf-
endif
ifeq ($(ARCH),m68k)
CROSS_COMPILE = m68k-elf-
endif
ifeq ($(ARCH),microblaze)
CROSS_COMPILE = mb-
endif
endif
endif

export	CROSS_COMPILE

#########################################################################
# U-Boot objects....order is important (i.e. start must be first)

OBJS  = cpu/$(CPU)/start.o
ifeq ($(CPU),i386)
OBJS += cpu/$(CPU)/start16.o
OBJS += cpu/$(CPU)/reset.o
endif
ifeq ($(CPU),ppc4xx)
OBJS += cpu/$(CPU)/resetvec.o
endif
ifeq ($(CPU),mpc83xx)
OBJS += cpu/$(CPU)/resetvec.o
endif
ifeq ($(CPU),mpc85xx)
OBJS += cpu/$(CPU)/resetvec.o
endif

LIBS  = lib_generic/libgeneric.a
LIBS += board/$(BOARDDIR)/lib$(BOARD).a
LIBS += cpu/$(CPU)/lib$(CPU).a
ifdef SOC
LIBS += cpu/$(CPU)/$(SOC)/lib$(SOC).a
endif
LIBS += lib_$(ARCH)/lib$(ARCH).a
LIBS += fs/cramfs/libcramfs.a fs/fat/libfat.a fs/fdos/libfdos.a fs/jffs2/libjffs2.a \
	fs/reiserfs/libreiserfs.a fs/ext2/libext2fs.a
LIBS += net/libnet.a
LIBS += disk/libdisk.a
LIBS += rtc/librtc.a
LIBS += dtt/libdtt.a
LIBS += drivers/libdrivers.a
LIBS += drivers/sk98lin/libsk98lin.a
LIBS += post/libpost.a post/cpu/libcpu.a
LIBS += common/libcommon.a
.PHONY : $(LIBS)

# Add GCC lib
PLATFORM_LIBS += -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc


# The "tools" are needed early, so put this first
# Don't include stuff already done in $(LIBS)
SUBDIRS	= tools \
	  examples \
	  post \
	  post/cpu
.PHONY : $(SUBDIRS)

#########################################################################
#########################################################################

ALL = u-boot.srec u-boot.bin System.map

all:		$(ALL)

u-boot.hex:	u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O ihex $< $@

u-boot.srec:	u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O srec $< $@

u-boot.bin:	u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

u-boot.img:	u-boot.bin
		./tools/mkimage -A $(ARCH) -T firmware -C none \
		-a $(TEXT_BASE) -e 0 \
		-n $(shell sed -n -e 's/.*U_BOOT_VERSION//p' include/version.h | \
			sed -e 's/"[	 ]*$$/ for $(BOARD) board"/') \
		-d $< $@

u-boot.dis:	u-boot
		$(OBJDUMP) -d $< > $@

u-boot:		depend $(SUBDIRS) $(OBJS) $(LIBS) $(LDSCRIPT)
		UNDEF_SYM=`$(OBJDUMP) -x $(LIBS) |sed  -n -e 's/.*\(__u_boot_cmd_.*\)/-u\1/p'|sort|uniq`;\
		$(LD) $(LDFLAGS) $$UNDEF_SYM $(OBJS) \
			--start-group $(LIBS) --end-group $(PLATFORM_LIBS) \
			-Map u-boot.map -o u-boot

$(LIBS):
		$(MAKE) -C `dirname $@`

$(SUBDIRS):
		$(MAKE) -C $@ all

gdbtools:
		$(MAKE) -C tools/gdb || exit 1

depend dep:
		@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir .depend ; done

tags:
		ctags -w `find $(SUBDIRS) include \
				lib_generic board/$(BOARDDIR) cpu/$(CPU) lib_$(ARCH) \
				fs/cramfs fs/fat fs/fdos fs/jffs2 \
				net disk rtc dtt drivers drivers/sk98lin common \
			\( -name CVS -prune \) -o \( -name '*.[ch]' -print \)`

etags:
		etags -a `find $(SUBDIRS) include \
				lib_generic board/$(BOARDDIR) cpu/$(CPU) lib_$(ARCH) \
				fs/cramfs fs/fat fs/fdos fs/jffs2 \
				net disk rtc dtt drivers drivers/sk98lin common \
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
	@rm -f include/config.h include/config.mk board/*/config.tmp

#========================================================================
# PowerPC
#========================================================================

#########################################################################
## MPC5xx Systems
#########################################################################

canmb_config:	unconfig
	@./mkconfig -a canmb ppc mpc5xxx canmb

cmi_mpc5xx_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc5xx cmi

PATI_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc5xx pati mpl

#########################################################################
## MPC5xxx Systems
#########################################################################

aev_config: unconfig
	@./mkconfig -a aev ppc mpc5xxx tqm5200

cpci5200_config:  unconfig
	@./mkconfig -a cpci5200  ppc mpc5xxx cpci5200 esd

hmi1001_config:         unconfig
	@./mkconfig hmi1001 ppc mpc5xxx hmi1001

Lite5200_config				\
Lite5200_LOWBOOT_config			\
Lite5200_LOWBOOT08_config		\
icecube_5200_config			\
icecube_5200_LOWBOOT_config		\
icecube_5200_LOWBOOT08_config		\
icecube_5200_DDR_config 		\
icecube_5200_DDR_LOWBOOT_config 	\
icecube_5200_DDR_LOWBOOT08_config	\
icecube_5100_config:			unconfig
	@ >include/config.h
	@[ -z "$(findstring LOWBOOT_,$@)" ] || \
		{ if [ "$(findstring DDR,$@)" ] ; \
			then echo "TEXT_BASE = 0xFF800000" >board/icecube/config.tmp ; \
			else echo "TEXT_BASE = 0xFF000000" >board/icecube/config.tmp ; \
		  fi ; \
		  echo "... with LOWBOOT configuration" ; \
		}
	@[ -z "$(findstring LOWBOOT08,$@)" ] || \
		{ echo "TEXT_BASE = 0xFF800000" >board/icecube/config.tmp ; \
		  echo "... with 8 MB flash only" ; \
		  echo "... with LOWBOOT configuration" ; \
		}
	@[ -z "$(findstring DDR,$@)" ] || \
		{ echo "#define CONFIG_MPC5200_DDR"	>>include/config.h ; \
		  echo "... DDR memory revision" ; \
		}
	@[ -z "$(findstring 5200,$@)" ] || \
		{ echo "#define CONFIG_MPC5200"		>>include/config.h ; \
		  echo "... with MPC5200 processor" ; \
		}
	@[ -z "$(findstring 5100,$@)" ] || \
		{ echo "#define CONFIG_MGT5100"		>>include/config.h ; \
		  echo "... with MGT5100 processor" ; \
		}
	@./mkconfig -a IceCube ppc mpc5xxx icecube

inka4x0_config:		unconfig
	@./mkconfig inka4x0 ppc mpc5xxx inka4x0

o2dnt_config:
	@./mkconfig -a o2dnt ppc mpc5xxx o2dnt

pf5200_config:  unconfig
	@./mkconfig -a pf5200  ppc mpc5xxx pf5200 esd

PM520_config \
PM520_DDR_config \
PM520_ROMBOOT_config \
PM520_ROMBOOT_DDR_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring DDR,$@)" ] || \
		{ echo "#define CONFIG_MPC5200_DDR"	>>include/config.h ; \
		  echo "... DDR memory revision" ; \
		}
	@[ -z "$(findstring ROMBOOT,$@)" ] || \
		{ echo "#define CONFIG_BOOT_ROM" >>include/config.h ; \
		  echo "... booting from 8-bit flash" ; \
		}
	@./mkconfig -a PM520 ppc mpc5xxx pm520

MINI5200_config	\
EVAL5200_config	\
TOP5200_config:	unconfig
	@ echo "#define CONFIG_$(@:_config=) 1"	>include/config.h
	@./mkconfig -a TOP5200 ppc mpc5xxx top5200 emk

Total5100_config		\
Total5200_config		\
Total5200_lowboot_config	\
Total5200_Rev2_config		\
Total5200_Rev2_lowboot_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring 5100,$@)" ] || \
		{ echo "#define CONFIG_MGT5100"		>>include/config.h ; \
		  echo "... with MGT5100 processor" ; \
		}
	@[ -z "$(findstring 5200,$@)" ] || \
		{ echo "#define CONFIG_MPC5200"		>>include/config.h ; \
		  echo "... with MPC5200 processor" ; \
		}
	@[ -n "$(findstring Rev,$@)" ] || \
		{ echo "#define CONFIG_TOTAL5200_REV 1"	>>include/config.h ; \
		  echo "... revision 1 board" ; \
		}
	@[ -z "$(findstring Rev2_,$@)" ] || \
		{ echo "#define CONFIG_TOTAL5200_REV 2"	>>include/config.h ; \
		  echo "... revision 2 board" ; \
		}
	@[ -z "$(findstring lowboot_,$@)" ] || \
		{ echo "TEXT_BASE = 0xFE000000" >board/total5200/config.tmp ; \
		  echo "... with lowboot configuration" ; \
		}
	@./mkconfig -a Total5200 ppc mpc5xxx total5200

TQM5200_auto_config	\
TQM5200_AA_config	\
TQM5200_AB_config	\
TQM5200_AC_config	\
MiniFAP_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring MiniFAP,$@)" ] || \
		{ echo "#define CONFIG_MINIFAP"	>>include/config.h ; \
		  echo "#define CONFIG_TQM5200_AC"	>>include/config.h ; \
		  echo "... TQM5200_AC on MiniFAP" ; \
		}
	@[ -z "$(findstring AA,$@)" ] || \
		{ echo "#define CONFIG_TQM5200_AA"	>>include/config.h ; \
		  echo "... with 4 MB Flash, 16 MB SDRAM, 32 kB EEPROM" ; \
		}
	@[ -z "$(findstring AB,$@)" ] || \
		{ echo "#define CONFIG_TQM5200_AB"	>>include/config.h ; \
		  echo "... with 64 MB Flash, 64 MB SDRAM, 32 kB EEPROM, 512 kB SRAM" ; \
		  echo "... with Graphics Controller"; \
		}
	@[ -z "$(findstring AC,$@)" ] || \
		{ echo "#define CONFIG_TQM5200_AC"	>>include/config.h ; \
		  echo "... with 4 MB Flash, 128 MB SDRAM" ; \
		  echo "... with Graphics Controller"; \
		}
	@[ -z "$(findstring auto,$@)" ] || \
		{ echo "#define CONFIG_CS_AUTOCONF"	>>include/config.h ; \
		  echo "... with automatic CS configuration" ; \
		}
	@./mkconfig -a TQM5200 ppc mpc5xxx tqm5200

spieval_config:	unconfig
	echo "#define CONFIG_CS_AUTOCONF">>include/config.h
	echo "... with automatic CS configuration"
	@./mkconfig -a spieval ppc mpc5xxx tqm5200

#########################################################################
## MPC8xx Systems
#########################################################################

Adder_config    \
Adder87x_config \
AdderII_config  \
	:		unconfig
	$(if $(findstring AdderII,$@), \
	@echo "#define CONFIG_MPC852T" > include/config.h)
	@./mkconfig -a Adder ppc mpc8xx adder

ADS860_config     \
FADS823_config    \
FADS850SAR_config \
MPC86xADS_config  \
MPC885ADS_config  \
FADS860T_config:	unconfig
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

FLAGADM_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx flagadm

xtract_GEN860T = $(subst _SC,,$(subst _config,,$1))

GEN860T_SC_config	\
GEN860T_config: unconfig
	@ >include/config.h
	@[ -z "$(findstring _SC,$@)" ] || \
		{ echo "#define CONFIG_SC" >>include/config.h ; \
		  echo "With reduced H/W feature set (SC)..." ; \
		}
	@./mkconfig -a $(call xtract_GEN860T,$@) ppc mpc8xx gen860t

GENIETV_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx genietv

GTH_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx gth

hermes_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx hermes

HMI10_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx tqm8xx

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
	@./mkconfig $(@:_config=) ppc mpc8xx kup4k kup

KUP4X_config    :       unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx kup4x kup

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

xtract_NETVIA = $(subst _V2,,$(subst _config,,$1))

NETVIA_V2_config \
NETVIA_config:		unconfig
	@ >include/config.h
	@[ -z "$(findstring NETVIA_config,$@)" ] || \
		 { echo "#define CONFIG_NETVIA_VERSION 1" >>include/config.h ; \
		  echo "... Version 1" ; \
		 }
	@[ -z "$(findstring NETVIA_V2_config,$@)" ] || \
		 { echo "#define CONFIG_NETVIA_VERSION 2" >>include/config.h ; \
		  echo "... Version 2" ; \
		 }
	@./mkconfig -a $(call xtract_NETVIA,$@) ppc mpc8xx netvia

xtract_NETPHONE = $(subst _V2,,$(subst _config,,$1))

NETPHONE_V2_config \
NETPHONE_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring NETPHONE_config,$@)" ] || \
		 { echo "#define CONFIG_NETPHONE_VERSION 1" >>include/config.h ; \
		 }
	@[ -z "$(findstring NETPHONE_V2_config,$@)" ] || \
		 { echo "#define CONFIG_NETPHONE_VERSION 2" >>include/config.h ; \
		 }
	@./mkconfig -a $(call xtract_NETPHONE,$@) ppc mpc8xx netphone

xtract_NETTA = $(subst _SWAPHOOK,,$(subst _6412,,$(subst _ISDN,,$(subst _config,,$1))))

NETTA_ISDN_6412_SWAPHOOK_config \
NETTA_ISDN_SWAPHOOK_config \
NETTA_6412_SWAPHOOK_config \
NETTA_SWAPHOOK_config \
NETTA_ISDN_6412_config \
NETTA_ISDN_config \
NETTA_6412_config \
NETTA_config:		unconfig
	@ >include/config.h
	@[ -z "$(findstring ISDN_,$@)" ] || \
		 { echo "#define CONFIG_NETTA_ISDN 1" >>include/config.h ; \
		 }
	@[ -n "$(findstring ISDN_,$@)" ] || \
		 { echo "#undef CONFIG_NETTA_ISDN" >>include/config.h ; \
		 }
	@[ -z "$(findstring 6412_,$@)" ] || \
		 { echo "#define CONFIG_NETTA_6412 1" >>include/config.h ; \
		 }
	@[ -n "$(findstring 6412_,$@)" ] || \
		 { echo "#undef CONFIG_NETTA_6412" >>include/config.h ; \
		 }
	@[ -z "$(findstring SWAPHOOK_,$@)" ] || \
		 { echo "#define CONFIG_NETTA_SWAPHOOK 1" >>include/config.h ; \
		 }
	@[ -n "$(findstring SWAPHOOK_,$@)" ] || \
		 { echo "#undef CONFIG_NETTA_SWAPHOOK" >>include/config.h ; \
		 }
	@./mkconfig -a $(call xtract_NETTA,$@) ppc mpc8xx netta

xtract_NETTA2 = $(subst _V2,,$(subst _config,,$1))

NETTA2_V2_config \
NETTA2_config:		unconfig
	@ >include/config.h
	@[ -z "$(findstring NETTA2_config,$@)" ] || \
		 { echo "#define CONFIG_NETTA2_VERSION 1" >>include/config.h ; \
		 }
	@[ -z "$(findstring NETTA2_V2_config,$@)" ] || \
		 { echo "#define CONFIG_NETTA2_VERSION 2" >>include/config.h ; \
		 }
	@./mkconfig -a $(call xtract_NETTA2,$@) ppc mpc8xx netta2

NC650_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx nc650

NX823_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx nx823

pcu_e_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx pcu_e siemens

QS850_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx qs850 snmc

QS823_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx qs850 snmc

QS860T_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx qs860t snmc

quantum_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx quantum

R360MPI_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx r360mpi

RBC823_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx rbc823

RPXClassic_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx RPXClassic

RPXlite_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx RPXlite

RPXlite_DW_64_config  		\
RPXlite_DW_LCD_config 		\
RPXlite_DW_64_LCD_config 	\
RPXlite_DW_NVRAM_config		\
RPXlite_DW_NVRAM_64_config      \
RPXlite_DW_NVRAM_LCD_config	\
RPXlite_DW_NVRAM_64_LCD_config  \
RPXlite_DW_config:         unconfig
	@ >include/config.h
	@[ -z "$(findstring _64,$@)" ] || \
		{ echo "#define RPXlite_64MHz"		>>include/config.h ; \
		  echo "... with 64MHz system clock ..."; \
		}
	@[ -z "$(findstring _LCD,$@)" ] || \
		{ echo "#define CONFIG_LCD"          	>>include/config.h ; \
		  echo "#define CONFIG_NEC_NL6448BC20"	>>include/config.h ; \
		  echo "... with LCD display ..."; \
		}
	@[ -z "$(findstring _NVRAM,$@)" ] || \
		{ echo "#define  CFG_ENV_IS_IN_NVRAM" 	>>include/config.h ; \
		  echo "... with ENV in NVRAM ..."; \
		}
	@./mkconfig -a RPXlite_DW ppc mpc8xx RPXlite_dw

rmu_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx rmu

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

stxxtc_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx stxxtc

svm_sc8xx_config:	unconfig
	@ >include/config.h
	@./mkconfig $(@:_config=) ppc mpc8xx svm_sc8xx

SXNI855T_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx sixnet

# EMK MPC8xx based modules
TOP860_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx top860 emk

# Play some tricks for configuration selection
# Only 855 and 860 boards may come with FEC
# and only 823 boards may have LCD support
xtract_8xx = $(subst _LCD,,$(subst _config,,$1))

FPS850L_config		\
FPS860L_config		\
NSCU_config		\
TQM823L_config		\
TQM823L_LCD_config	\
TQM850L_config		\
TQM855L_config		\
TQM860L_config		\
TQM862L_config		\
TQM823M_config		\
TQM850M_config		\
TQM855M_config		\
TQM860M_config		\
TQM862M_config		\
TQM866M_config:		unconfig
	@ >include/config.h
	@[ -z "$(findstring _LCD,$@)" ] || \
		{ echo "#define CONFIG_LCD"		>>include/config.h ; \
		  echo "#define CONFIG_NEC_NL6448BC20"	>>include/config.h ; \
		  echo "... with LCD display" ; \
		}
	@./mkconfig -a $(call xtract_8xx,$@) ppc mpc8xx tqm8xx

TTTech_config:	unconfig
	@echo "#define CONFIG_LCD" >include/config.h
	@echo "#define CONFIG_SHARP_LQ104V7DS01" >>include/config.h
	@./mkconfig -a TQM823L ppc mpc8xx tqm8xx

uc100_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx uc100

v37_config:	unconfig
	@echo "#define CONFIG_LCD" >include/config.h
	@echo "#define CONFIG_SHARP_LQ084V1DG21" >>include/config.h
	@./mkconfig $(@:_config=) ppc mpc8xx v37

wtk_config:	unconfig
	@echo "#define CONFIG_LCD" >include/config.h
	@echo "#define CONFIG_SHARP_LQ065T9DR51U" >>include/config.h
	@./mkconfig -a TQM823L ppc mpc8xx tqm8xx

#########################################################################
## PPC4xx Systems
#########################################################################
xtract_4xx = $(subst _25,,$(subst _33,,$(subst _BA,,$(subst _ME,,$(subst _HI,,$(subst _config,,$1))))))

ADCIOP_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx adciop esd

AP1000_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ap1000 amirix

APC405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx apc405 esd

AR405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ar405 esd

ASH405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ash405 esd

bamboo_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx bamboo amcc

bubinga_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx bubinga amcc

CANBT_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx canbt esd

CATcenter_config	\
CATcenter_25_config	\
CATcenter_33_config:	unconfig
	@ echo "/* CATcenter uses PPChameleon Model ME */"  > include/config.h
	@ echo "#define CONFIG_PPCHAMELEON_MODULE_MODEL 1" >> include/config.h
	@[ -z "$(findstring _25,$@)" ] || \
		{ echo "#define CONFIG_PPCHAMELEON_CLK_25" >>include/config.h ; \
		  echo "SysClk = 25MHz" ; \
		}
	@[ -z "$(findstring _33,$@)" ] || \
		{ echo "#define CONFIG_PPCHAMELEON_CLK_33" >>include/config.h ; \
		  echo "SysClk = 33MHz" ; \
		}
	@./mkconfig -a $(call xtract_4xx,$@) ppc ppc4xx PPChameleonEVB dave

CPCI2DP_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx cpci2dp esd

CPCI405_config	\
CPCI4052_config	\
CPCI405DT_config	\
CPCI405AB_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx cpci405 esd
	@echo "BOARD_REVISION = $(@:_config=)"	>>include/config.mk

CPCI440_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx cpci440 esd

CPCIISER4_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx cpciiser4 esd

CRAYL1_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx L1 cray

csb272_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx csb272

csb472_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx csb472

DASA_SIM_config: unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx dasa_sim esd

DP405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx dp405 esd

DU405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx du405 esd

ebony_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ebony amcc

ERIC_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx eric

EXBITGEN_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx exbitgen

G2000_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx g2000

HH405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx hh405 esd

HUB405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx hub405 esd

JSE_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx jse

KAREF_config: unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx karef sandburst

luan_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx luan amcc

METROBOX_config: unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx metrobox sandburst

MIP405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx mip405 mpl

MIP405T_config:	unconfig
	@echo "#define CONFIG_MIP405T" >include/config.h
	@echo "Enable subset config for MIP405T"
	@./mkconfig -a MIP405 ppc ppc4xx mip405 mpl

ML2_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ml2

ml300_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ml300 xilinx

ocotea_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ocotea amcc

OCRTC_config		\
ORSG_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ocrtc esd

p3p440_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx p3p440 prodrive

PCI405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx pci405 esd

PIP405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx pip405 mpl

PLU405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx plu405 esd

PMC405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx pmc405 esd

PPChameleonEVB_config		\
PPChameleonEVB_BA_25_config	\
PPChameleonEVB_ME_25_config	\
PPChameleonEVB_HI_25_config	\
PPChameleonEVB_BA_33_config	\
PPChameleonEVB_ME_33_config	\
PPChameleonEVB_HI_33_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring EVB_BA,$@)" ] || \
		{ echo "#define CONFIG_PPCHAMELEON_MODULE_MODEL 0" >>include/config.h ; \
		  echo "... BASIC model" ; \
		}
	@[ -z "$(findstring EVB_ME,$@)" ] || \
		{ echo "#define CONFIG_PPCHAMELEON_MODULE_MODEL 1" >>include/config.h ; \
		  echo "... MEDIUM model" ; \
		}
	@[ -z "$(findstring EVB_HI,$@)" ] || \
		{ echo "#define CONFIG_PPCHAMELEON_MODULE_MODEL 2" >>include/config.h ; \
		  echo "... HIGH-END model" ; \
		}
	@[ -z "$(findstring _25,$@)" ] || \
		{ echo "#define CONFIG_PPCHAMELEON_CLK_25" >>include/config.h ; \
		  echo "SysClk = 25MHz" ; \
		}
	@[ -z "$(findstring _33,$@)" ] || \
		{ echo "#define CONFIG_PPCHAMELEON_CLK_33" >>include/config.h ; \
		  echo "SysClk = 33MHz" ; \
		}
	@./mkconfig -a $(call xtract_4xx,$@) ppc ppc4xx PPChameleonEVB dave

sbc405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx sbc405

sycamore_config:	unconfig
	@echo "Configuring for sycamore board as subset of walnut..."
	@./mkconfig -a walnut ppc ppc4xx walnut amcc

VOH405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx voh405 esd

VOM405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx vom405 esd

CMS700_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx cms700 esd

W7OLMC_config	\
W7OLMG_config: unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx w7o

walnut_config: unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx walnut amcc

WUH405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx wuh405 esd

XPEDITE1K_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx xpedite1k

yosemite_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx yosemite amcc

yellowstone_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx yellowstone amcc

#########################################################################
## MPC8220 Systems
#########################################################################

Alaska8220_config	\
Yukon8220_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8220 alaska

sorcery_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8220 sorcery

#########################################################################
## MPC824x Systems
#########################################################################
xtract_82xx = $(subst _BIGFLASH,,$(subst _ROMBOOT,,$(subst _L2,,$(subst _266MHz,,$(subst _300MHz,,$(subst _config,,$1))))))

A3000_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x a3000

barco_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x barco

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

debris_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x debris etin

eXalion_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x eXalion

HIDDEN_DRAGON_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x hidden_dragon

MOUSSE_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x mousse

MUSENKI_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x musenki

MVBLUE_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc824x mvblue

OXC_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x oxc

PN62_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x pn62

Sandpoint8240_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x sandpoint

Sandpoint8245_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x sandpoint

sbc8240_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x sbc8240

SL8245_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x sl8245

utx8245_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x utx8245

#########################################################################
## MPC8260 Systems
#########################################################################

atc_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 atc

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

CPU87_config	\
CPU87_ROMBOOT_config: unconfig
	@./mkconfig $(call xtract_82xx,$@) ppc mpc8260 cpu87
	@cd ./include ;				\
	if [ "$(findstring _ROMBOOT_,$@)" ] ; then \
		echo "CONFIG_BOOT_ROM = y" >> config.mk ; \
		echo "... booting from 8-bit flash" ; \
	else \
		echo "CONFIG_BOOT_ROM = n" >> config.mk ; \
		echo "... booting from 64-bit flash" ; \
	fi; \
	echo "export CONFIG_BOOT_ROM" >> config.mk;

ep8248_config	\
ep8248E_config	:	unconfig
	@./mkconfig ep8248 ppc mpc8260 ep8248

ep8260_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 ep8260

gw8260_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 gw8260

hymod_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 hymod

IDS8247_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 ids8247

IPHASE4539_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 iphase4539

ISPAN_config		\
ISPAN_REVB_config:	unconfig
	@if [ "$(findstring _REVB_,$@)" ] ; then \
		echo "#define CFG_REV_B" > include/config.h ; \
	fi
	@./mkconfig -a ISPAN ppc mpc8260 ispan

MPC8260ADS_config	\
MPC8260ADS_lowboot_config	\
MPC8260ADS_33MHz_config	\
MPC8260ADS_33MHz_lowboot_config	\
MPC8260ADS_40MHz_config	\
MPC8260ADS_40MHz_lowboot_config	\
MPC8272ADS_config	\
MPC8272ADS_lowboot_config	\
PQ2FADS_config		\
PQ2FADS_lowboot_config		\
PQ2FADS-VR_config	\
PQ2FADS-VR_lowboot_config	\
PQ2FADS-ZU_config	\
PQ2FADS-ZU_lowboot_config	\
PQ2FADS-ZU_66MHz_config	\
PQ2FADS-ZU_66MHz_lowboot_config	\
	:		unconfig
	$(if $(findstring PQ2FADS,$@), \
	@echo "#define CONFIG_ADSTYPE CFG_PQ2FADS" > include/config.h, \
	@echo "#define CONFIG_ADSTYPE CFG_"$(subst MPC,,$(word 1,$(subst _, ,$@))) > include/config.h)
	$(if $(findstring MHz,$@), \
	@echo "#define CONFIG_8260_CLKIN" $(subst MHz,,$(word 2,$(subst _, ,$@)))"000000" >> include/config.h, \
	$(if $(findstring VR,$@), \
	@echo "#define CONFIG_8260_CLKIN 66000000" >> include/config.h))
	@[ -z "$(findstring lowboot_,$@)" ] || \
		{ echo "TEXT_BASE = 0xFF800000" >board/mpc8260ads/config.tmp ; \
		  echo "... with lowboot configuration" ; \
		}
	@./mkconfig -a MPC8260ADS ppc mpc8260 mpc8260ads

MPC8266ADS_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 mpc8266ads

# PM825/PM826 default configuration:  small (= 8 MB) Flash / boot from 64-bit flash
PM825_config	\
PM825_ROMBOOT_config	\
PM825_BIGFLASH_config	\
PM825_ROMBOOT_BIGFLASH_config	\
PM826_config	\
PM826_ROMBOOT_config	\
PM826_BIGFLASH_config	\
PM826_ROMBOOT_BIGFLASH_config:	unconfig
	@if [ "$(findstring PM825_,$@)" ] ; then \
		echo "#define CONFIG_PCI"	>include/config.h ; \
	else \
		>include/config.h ; \
	fi
	@if [ "$(findstring _ROMBOOT_,$@)" ] ; then \
		echo "... booting from 8-bit flash" ; \
		echo "#define CONFIG_BOOT_ROM" >>include/config.h ; \
		echo "TEXT_BASE = 0xFF800000" >board/pm826/config.tmp ; \
		if [ "$(findstring _BIGFLASH_,$@)" ] ; then \
			echo "... with 32 MB Flash" ; \
			echo "#define CONFIG_FLASH_32MB" >>include/config.h ; \
		fi; \
	else \
		echo "... booting from 64-bit flash" ; \
		if [ "$(findstring _BIGFLASH_,$@)" ] ; then \
			echo "... with 32 MB Flash" ; \
			echo "#define CONFIG_FLASH_32MB" >>include/config.h ; \
			echo "TEXT_BASE = 0x40000000" >board/pm826/config.tmp ; \
		else \
			echo "TEXT_BASE = 0xFF000000" >board/pm826/config.tmp ; \
		fi; \
	fi
	@./mkconfig -a PM826 ppc mpc8260 pm826

PM828_config	\
PM828_PCI_config	\
PM828_ROMBOOT_config	\
PM828_ROMBOOT_PCI_config:	unconfig
	@if [ -z "$(findstring _PCI_,$@)" ] ; then \
		echo "#define CONFIG_PCI"  >>include/config.h ; \
		echo "... with PCI enabled" ; \
	else \
		>include/config.h ; \
	fi
	@if [ "$(findstring _ROMBOOT_,$@)" ] ; then \
		echo "... booting from 8-bit flash" ; \
		echo "#define CONFIG_BOOT_ROM" >>include/config.h ; \
		echo "TEXT_BASE = 0xFF800000" >board/pm826/config.tmp ; \
	fi
	@./mkconfig -a PM828 ppc mpc8260 pm828

ppmc8260_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 ppmc8260

Rattler8248_config	\
Rattler_config:		unconfig
	$(if $(findstring 8248,$@), \
	@echo "#define CONFIG_MPC8248" > include/config.h)
	@./mkconfig -a Rattler ppc mpc8260 rattler

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

TQM8255_AA_config \
TQM8260_AA_config \
TQM8260_AB_config \
TQM8260_AC_config \
TQM8260_AD_config \
TQM8260_AE_config \
TQM8260_AF_config \
TQM8260_AG_config \
TQM8260_AH_config \
TQM8265_AA_config:  unconfig
	@case "$@" in \
	TQM8255_AA_config) CTYPE=MPC8255; CFREQ=300; CACHE=no;  BMODE=8260;;  \
	TQM8260_AA_config) CTYPE=MPC8260; CFREQ=200; CACHE=no;  BMODE=8260;; \
	TQM8260_AB_config) CTYPE=MPC8260; CFREQ=200; CACHE=yes; BMODE=60x;;  \
	TQM8260_AC_config) CTYPE=MPC8260; CFREQ=200; CACHE=yes; BMODE=60x;;  \
	TQM8260_AD_config) CTYPE=MPC8260; CFREQ=300; CACHE=no;  BMODE=60x;;  \
	TQM8260_AE_config) CTYPE=MPC8260; CFREQ=266; CACHE=no;  BMODE=8260;; \
	TQM8260_AF_config) CTYPE=MPC8260; CFREQ=300; CACHE=no;  BMODE=60x;;  \
	TQM8260_AG_config) CTYPE=MPC8260; CFREQ=300; CACHE=no;  BMODE=8260;; \
	TQM8260_AH_config) CTYPE=MPC8260; CFREQ=300; CACHE=yes; BMODE=60x;;  \
	TQM8265_AA_config) CTYPE=MPC8265; CFREQ=300; CACHE=no;  BMODE=60x;;  \
	esac; \
	>include/config.h ; \
	if [ "$${CTYPE}" != "MPC8260" ] ; then \
		echo "#define CONFIG_$${CTYPE}"	>>include/config.h ; \
	fi; \
	echo "#define CONFIG_$${CFREQ}MHz"	>>include/config.h ; \
	echo "... with $${CFREQ}MHz system clock" ; \
	if [ "$${CACHE}" == "yes" ] ; then \
		echo "#define CONFIG_L2_CACHE"	>>include/config.h ; \
		echo "... with L2 Cache support" ; \
	else \
		echo "#undef CONFIG_L2_CACHE"	>>include/config.h ; \
		echo "... without L2 Cache support" ; \
	fi; \
	if [ "$${BMODE}" == "60x" ] ; then \
		echo "#define CONFIG_BUSMODE_60x" >>include/config.h ; \
		echo "... with 60x Bus Mode" ; \
	else \
		echo "#undef CONFIG_BUSMODE_60x"  >>include/config.h ; \
		echo "... without 60x Bus Mode" ; \
	fi
	@./mkconfig -a TQM8260 ppc mpc8260 tqm8260

VoVPN-GW_66MHz_config	\
VoVPN-GW_100MHz_config:		unconfig
	@echo "#define CONFIG_CLKIN_$(word 2,$(subst _, ,$@))" > include/config.h
	@./mkconfig -a VoVPN-GW ppc mpc8260 vovpn-gw funkwerk

ZPC1900_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 zpc1900

#========================================================================
# M68K
#========================================================================
#########################################################################
## Coldfire
#########################################################################

cobra5272_config :		unconfig
	@./mkconfig $(@:_config=) m68k mcf52x2 cobra5272

M5272C3_config :		unconfig
	@./mkconfig $(@:_config=) m68k mcf52x2 m5272c3

M5282EVB_config :		unconfig
	@./mkconfig $(@:_config=) m68k mcf52x2 m5282evb

TASREG_config :		unconfig
	@./mkconfig $(@:_config=) m68k mcf52x2 tasreg esd

#########################################################################
## MPC83xx Systems
#########################################################################

MPC8349ADS_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc83xx mpc8349ads

TQM834x_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc83xx tqm834x

#########################################################################
## MPC85xx Systems
#########################################################################

MPC8540ADS_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc85xx mpc8540ads

MPC8540EVAL_config \
MPC8540EVAL_33_config \
MPC8540EVAL_66_config \
MPC8540EVAL_33_slave_config \
MPC8540EVAL_66_slave_config:      unconfig
	@echo "" >include/config.h ; \
	if [ "$(findstring _33_,$@)" ] ; then \
		echo -n "... 33 MHz PCI" ; \
	else \
		echo "#define CONFIG_SYSCLK_66M" >>include/config.h ; \
		echo -n "... 66 MHz PCI" ; \
	fi ; \
	if [ "$(findstring _slave_,$@)" ] ; then \
		echo "#define CONFIG_PCI_SLAVE" >>include/config.h ; \
		echo " slave" ; \
	else \
		echo " host" ; \
	fi
	@./mkconfig -a MPC8540EVAL ppc mpc85xx mpc8540eval

MPC8560ADS_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc85xx mpc8560ads

MPC8541CDS_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc85xx mpc8541cds cds

MPC8548CDS_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc85xx mpc8548cds cds

MPC8555CDS_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc85xx mpc8555cds cds

PM854_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc85xx pm854

PM856_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc85xx pm856

sbc8540_config \
sbc8540_33_config \
sbc8540_66_config:	unconfig
	@if [ "$(findstring _66_,$@)" ] ; then \
		echo "#define CONFIG_PCI_66"	>>include/config.h ; \
		echo "... 66 MHz PCI" ; \
	else \
		>include/config.h ; \
		echo "... 33 MHz PCI" ; \
	fi
	@./mkconfig -a SBC8540 ppc mpc85xx sbc8560

sbc8560_config \
sbc8560_33_config \
sbc8560_66_config:      unconfig
	@if [ "$(findstring _66_,$@)" ] ; then \
		echo "#define CONFIG_PCI_66"	>>include/config.h ; \
		echo "... 66 MHz PCI" ; \
	else \
		>include/config.h ; \
		echo "... 33 MHz PCI" ; \
	fi
	@./mkconfig -a sbc8560 ppc mpc85xx sbc8560

stxgp3_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc85xx stxgp3

TQM8540_config		\
TQM8541_config		\
TQM8555_config		\
TQM8560_config:		unconfig
	@CTYPE=$(subst TQM,,$(@:_config=)); \
	>include/config.h ; \
	echo "... TQM"$${CTYPE}; \
	echo "#define CONFIG_MPC$${CTYPE}">>include/config.h; \
	echo "#define CONFIG_TQM$${CTYPE}">>include/config.h; \
	echo "#define CONFIG_HOSTNAME tqm$${CTYPE}">>include/config.h; \
	echo "#define CONFIG_BOARDNAME \"TQM$${CTYPE}\"">>include/config.h; \
	echo "#define CFG_BOOTFILE \"bootfile=/tftpboot/tqm$${CTYPE}/uImage\0\"">>include/config.h
	@./mkconfig -a TQM85xx ppc mpc85xx tqm85xx

#########################################################################
## 74xx/7xx Systems
#########################################################################

AmigaOneG3SE_config:	unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx AmigaOneG3SE MAI

BAB7xx_config: unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx bab7xx eltec

CPCI750_config:        unconfig
	@./mkconfig CPCI750 ppc 74xx_7xx cpci750 esd

DB64360_config:  unconfig
	@./mkconfig DB64360 ppc 74xx_7xx db64360 Marvell

DB64460_config:  unconfig
	@./mkconfig DB64460 ppc 74xx_7xx db64460 Marvell

ELPPC_config: unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx elppc eltec

EVB64260_config	\
EVB64260_750CX_config:	unconfig
	@./mkconfig EVB64260 ppc 74xx_7xx evb64260

P3G4_config: unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx evb64260

PCIPPC2_config \
PCIPPC6_config: unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx pcippc2

ZUMA_config:	unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx evb64260

#========================================================================
# ARM
#========================================================================
#########################################################################
## StrongARM Systems
#########################################################################

assabet_config	:	unconfig
	@./mkconfig $(@:_config=) arm sa1100 assabet

dnp1110_config	:	unconfig
	@./mkconfig $(@:_config=) arm sa1100 dnp1110

gcplus_config	:	unconfig
	@./mkconfig $(@:_config=) arm sa1100 gcplus

lart_config	:	unconfig
	@./mkconfig $(@:_config=) arm sa1100 lart

shannon_config	:	unconfig
	@./mkconfig $(@:_config=) arm sa1100 shannon

#########################################################################
## ARM92xT Systems
#########################################################################

xtract_trab = $(subst _bigram,,$(subst _bigflash,,$(subst _old,,$(subst _config,,$1))))

xtract_omap1610xxx = $(subst _cs0boot,,$(subst _cs3boot,,$(subst _cs_autoboot,,$(subst _config,,$1))))

xtract_omap730p2 = $(subst _cs0boot,,$(subst _cs3boot,, $(subst _config,,$1)))

at91rm9200dk_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t at91rm9200dk NULL at91rm9200

cmc_pu2_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t cmc_pu2 NULL at91rm9200

csb637_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t csb637 NULL at91rm9200

mp2usb_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t mp2usb NULL at91rm9200


########################################################################
## ARM Integrator boards - see doc/README-integrator for more info.
integratorap_config	\
ap_config		\
ap966_config		\
ap922_config		\
ap922_XA10_config	\
ap7_config		\
ap720t_config  		\
ap920t_config		\
ap926ejs_config		\
ap946es_config: unconfig
	@board/integratorap/split_by_variant.sh $@

integratorcp_config	\
cp_config		\
cp920t_config		\
cp926ejs_config		\
cp946es_config		\
cp1136_config		\
cp966_config		\
cp922_config		\
cp922_XA10_config	\
cp1026_config: unconfig
	@board/integratorcp/split_by_variant.sh $@

kb9202_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t kb9202 NULL at91rm9200

lpd7a400_config \
lpd7a404_config:	unconfig
	@./mkconfig $(@:_config=) arm lh7a40x lpd7a40x

mx1ads_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t mx1ads NULL imx

mx1fs2_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t mx1fs2 NULL imx

omap1510inn_config :	unconfig
	@./mkconfig $(@:_config=) arm arm925t omap1510inn

omap5912osk_config :	unconfig
	@./mkconfig $(@:_config=) arm arm926ejs omap5912osk

omap1610inn_config \
omap1610inn_cs0boot_config \
omap1610inn_cs3boot_config \
omap1610inn_cs_autoboot_config \
omap1610h2_config \
omap1610h2_cs0boot_config \
omap1610h2_cs3boot_config \
omap1610h2_cs_autoboot_config:	unconfig
	@if [ "$(findstring _cs0boot_, $@)" ] ; then \
		echo "#define CONFIG_CS0_BOOT" >> ./include/config.h ; \
		echo "... configured for CS0 boot"; \
	elif [ "$(findstring _cs_autoboot_, $@)" ] ; then \
		echo "#define CONFIG_CS_AUTOBOOT" >> ./include/config.h ; \
		echo "... configured for CS_AUTO boot"; \
	else \
		echo "#define CONFIG_CS3_BOOT" >> ./include/config.h ; \
		echo "... configured for CS3 boot"; \
	fi;
	@./mkconfig -a $(call xtract_omap1610xxx,$@) arm arm926ejs omap1610inn

omap730p2_config \
omap730p2_cs0boot_config \
omap730p2_cs3boot_config :	unconfig
	@if [ "$(findstring _cs0boot_, $@)" ] ; then \
		echo "#define CONFIG_CS0_BOOT" >> ./include/config.h ; \
		echo "... configured for CS0 boot"; \
	else \
		echo "#define CONFIG_CS3_BOOT" >> ./include/config.h ; \
		echo "... configured for CS3 boot"; \
	fi;
	@./mkconfig -a $(call xtract_omap730p2,$@) arm arm926ejs omap730p2

scb9328_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t scb9328 NULL imx

smdk2400_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t smdk2400 NULL s3c24x0

smdk2410_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t smdk2410 NULL s3c24x0

SX1_config :		unconfig
	@./mkconfig $(@:_config=) arm arm925t sx1

# TRAB default configuration:	8 MB Flash, 32 MB RAM
trab_config \
trab_bigram_config \
trab_bigflash_config \
trab_old_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring _bigram,$@)" ] || \
		{ echo "#define CONFIG_FLASH_8MB"  >>include/config.h ; \
		  echo "#define CONFIG_RAM_32MB"   >>include/config.h ; \
		  echo "... with 8 MB Flash, 32 MB RAM" ; \
		}
	@[ -z "$(findstring _bigflash,$@)" ] || \
		{ echo "#define CONFIG_FLASH_16MB" >>include/config.h ; \
		  echo "#define CONFIG_RAM_16MB"   >>include/config.h ; \
		  echo "... with 16 MB Flash, 16 MB RAM" ; \
		  echo "TEXT_BASE = 0x0CF40000" >board/trab/config.tmp ; \
		}
	@[ -z "$(findstring _old,$@)" ] || \
		{ echo "#define CONFIG_FLASH_8MB"  >>include/config.h ; \
		  echo "#define CONFIG_RAM_16MB"   >>include/config.h ; \
		  echo "... with 8 MB Flash, 16 MB RAM" ; \
		  echo "TEXT_BASE = 0x0CF40000" >board/trab/config.tmp ; \
		}
	@./mkconfig -a $(call xtract_trab,$@) arm arm920t trab NULL s3c24x0

VCMA9_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t vcma9 mpl s3c24x0

#========================================================================
# ARM supplied Versatile development boards
#========================================================================
versatile_config	\
versatileab_config	\
versatilepb_config :	unconfig
	@board/versatile/split_by_variant.sh $@

voiceblue_smallflash_config	\
voiceblue_config:	unconfig
	@if [ "$(findstring _smallflash_,$@)" ] ; then \
		echo "... boot from lower flash bank" ; \
		echo "#define VOICEBLUE_SMALL_FLASH" >>include/config.h ; \
		echo "VOICEBLUE_SMALL_FLASH=y" >board/voiceblue/config.tmp ; \
	else \
		echo "... boot from upper flash bank" ; \
		>include/config.h ; \
		echo "VOICEBLUE_SMALL_FLASH=n" >board/voiceblue/config.tmp ; \
	fi
	@./mkconfig -a voiceblue arm arm925t voiceblue

cm4008_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t cm4008 NULL ks8695

cm41xx_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t cm41xx NULL ks8695

#########################################################################
## S3C44B0 Systems
#########################################################################

B2_config	:	unconfig
	@./mkconfig $(@:_config=) arm s3c44b0 B2 dave

#########################################################################
## ARM720T Systems
#########################################################################

armadillo_config:	unconfig
	@./mkconfig $(@:_config=) arm arm720t armadillo

ep7312_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm720t ep7312

impa7_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm720t impa7

modnet50_config :	unconfig
	@./mkconfig $(@:_config=) arm arm720t modnet50

evb4510_config :	unconfig
	@./mkconfig $(@:_config=) arm arm720t evb4510

#########################################################################
## XScale Systems
#########################################################################

adsvix_config	:	unconfig
	@./mkconfig $(@:_config=) arm pxa adsvix

cerf250_config :	unconfig
	@./mkconfig $(@:_config=) arm pxa cerf250

cradle_config	:	unconfig
	@./mkconfig $(@:_config=) arm pxa cradle

csb226_config	:	unconfig
	@./mkconfig $(@:_config=) arm pxa csb226

innokom_config	:	unconfig
	@./mkconfig $(@:_config=) arm pxa innokom

ixdp425_config	:	unconfig
	@./mkconfig $(@:_config=) arm ixp ixdp425

lubbock_config	:	unconfig
	@./mkconfig $(@:_config=) arm pxa lubbock

logodl_config	:	unconfig
	@./mkconfig $(@:_config=) arm pxa logodl

pxa255_idp_config:	unconfig
	@./mkconfig $(@:_config=) arm pxa pxa255_idp

wepep250_config	:	unconfig
	@./mkconfig $(@:_config=) arm pxa wepep250

xaeniax_config	:	unconfig
	@./mkconfig $(@:_config=) arm pxa xaeniax

xm250_config	:	unconfig
	@./mkconfig $(@:_config=) arm pxa xm250

xsengine_config :	unconfig
	@./mkconfig $(@:_config=) arm pxa xsengine

#########################################################################
## ARM1136 Systems
#########################################################################
omap2420h4_config :    unconfig
	@./mkconfig $(@:_config=) arm arm1136 omap2420h4

#========================================================================
# i386
#========================================================================
#########################################################################
## AMD SC520 CDP
#########################################################################
sc520_cdp_config	:	unconfig
	@./mkconfig $(@:_config=) i386 i386 sc520_cdp

sc520_spunk_config	:	unconfig
	@./mkconfig $(@:_config=) i386 i386 sc520_spunk

sc520_spunk_rel_config	:	unconfig
	@./mkconfig $(@:_config=) i386 i386 sc520_spunk

#========================================================================
# MIPS
#========================================================================
#########################################################################
## MIPS32 4Kc
#########################################################################

xtract_incaip = $(subst _100MHz,,$(subst _133MHz,,$(subst _150MHz,,$(subst _config,,$1))))

incaip_100MHz_config	\
incaip_133MHz_config	\
incaip_150MHz_config	\
incaip_config: unconfig
	@ >include/config.h
	@[ -z "$(findstring _100MHz,$@)" ] || \
		{ echo "#define CPU_CLOCK_RATE 100000000" >>include/config.h ; \
		  echo "... with 100MHz system clock" ; \
		}
	@[ -z "$(findstring _133MHz,$@)" ] || \
		{ echo "#define CPU_CLOCK_RATE 133000000" >>include/config.h ; \
		  echo "... with 133MHz system clock" ; \
		}
	@[ -z "$(findstring _150MHz,$@)" ] || \
		{ echo "#define CPU_CLOCK_RATE 150000000" >>include/config.h ; \
		  echo "... with 150MHz system clock" ; \
		}
	@./mkconfig -a $(call xtract_incaip,$@) mips mips incaip

tb0229_config: unconfig
	@./mkconfig $(@:_config=) mips mips tb0229

#########################################################################
## MIPS32 AU1X00
#########################################################################
dbau1000_config		: 	unconfig
	@ >include/config.h
	@echo "#define CONFIG_DBAU1000 1" >>include/config.h
	@./mkconfig -a dbau1x00 mips mips dbau1x00

dbau1100_config		: 	unconfig
	@ >include/config.h
	@echo "#define CONFIG_DBAU1100 1" >>include/config.h
	@./mkconfig -a dbau1x00 mips mips dbau1x00

dbau1500_config		: 	unconfig
	@ >include/config.h
	@echo "#define CONFIG_DBAU1500 1" >>include/config.h
	@./mkconfig -a dbau1x00 mips mips dbau1x00

dbau1550_config		:	unconfig
	@ >include/config.h
	@echo "#define CONFIG_DBAU1550 1" >>include/config.h
	@./mkconfig -a dbau1x00 mips mips dbau1x00

dbau1550_el_config	:	unconfig
	@ >include/config.h
	@echo "#define CONFIG_DBAU1550 1" >>include/config.h
	@./mkconfig -a dbau1x00 mips mips dbau1x00

pb1000_config		: 	unconfig
	@ >include/config.h
	@echo "#define CONFIG_PB1000 1" >>include/config.h
	@./mkconfig -a pb1x00 mips mips pb1x00

#########################################################################
## MIPS64 5Kc
#########################################################################

purple_config :		unconfig
	@./mkconfig $(@:_config=) mips mips purple

#========================================================================
# Nios
#========================================================================
#########################################################################
## Nios32
#########################################################################

DK1C20_safe_32_config		\
DK1C20_standard_32_config	\
DK1C20_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring _safe_32,$@)" ] || \
		{ echo "#define CONFIG_NIOS_SAFE_32 1" >>include/config.h ; \
		  echo "... NIOS 'safe_32' configuration" ; \
		}
	@[ -z "$(findstring _standard_32,$@)" ] || \
		{ echo "#define CONFIG_NIOS_STANDARD_32 1" >>include/config.h ; \
		  echo "... NIOS 'standard_32' configuration" ; \
		}
	@[ -z "$(findstring DK1C20_config,$@)" ] || \
		{ echo "#define CONFIG_NIOS_STANDARD_32 1" >>include/config.h ; \
		  echo "... NIOS 'standard_32' configuration (DEFAULT)" ; \
		}
	@./mkconfig -a DK1C20 nios nios dk1c20 altera

DK1S10_safe_32_config		\
DK1S10_standard_32_config	\
DK1S10_mtx_ldk_20_config	\
DK1S10_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring _safe_32,$@)" ] || \
		{ echo "#define CONFIG_NIOS_SAFE_32 1" >>include/config.h ; \
		  echo "... NIOS 'safe_32' configuration" ; \
		}
	@[ -z "$(findstring _standard_32,$@)" ] || \
		{ echo "#define CONFIG_NIOS_STANDARD_32 1" >>include/config.h ; \
		  echo "... NIOS 'standard_32' configuration" ; \
		}
	@[ -z "$(findstring _mtx_ldk_20,$@)" ] || \
		{ echo "#define CONFIG_NIOS_MTX_LDK_20 1" >>include/config.h ; \
		  echo "... NIOS 'mtx_ldk_20' configuration" ; \
		}
	@[ -z "$(findstring DK1S10_config,$@)" ] || \
		{ echo "#define CONFIG_NIOS_STANDARD_32 1" >>include/config.h ; \
		  echo "... NIOS 'standard_32' configuration (DEFAULT)" ; \
		}
	@./mkconfig -a DK1S10 nios nios dk1s10 altera

ADNPESC1_DNPEVA2_base_32_config	\
ADNPESC1_base_32_config		\
ADNPESC1_config: unconfig
	@ >include/config.h
	@[ -z "$(findstring _DNPEVA2,$@)" ] || \
		{ echo "#define CONFIG_DNPEVA2 1" >>include/config.h ; \
		  echo "... DNP/EVA2 configuration" ; \
		}
	@[ -z "$(findstring _base_32,$@)" ] || \
		{ echo "#define CONFIG_NIOS_BASE_32 1" >>include/config.h ; \
		  echo "... NIOS 'base_32' configuration" ; \
		}
	@[ -z "$(findstring ADNPESC1_config,$@)" ] || \
		{ echo "#define CONFIG_NIOS_BASE_32 1" >>include/config.h ; \
		  echo "... NIOS 'base_32' configuration (DEFAULT)" ; \
		}
	@./mkconfig -a ADNPESC1 nios nios adnpesc1 ssv

#########################################################################
## Nios-II
#########################################################################

PK1C20_config : unconfig
	@./mkconfig  PK1C20 nios2 nios2 pk1c20 psyent

PCI5441_config : unconfig
	@./mkconfig  PCI5441 nios2 nios2 pci5441 psyent

#========================================================================
# MicroBlaze
#========================================================================
#########################################################################
## Microblaze
#########################################################################
suzaku_config:	unconfig
	@ >include/config.h
	@echo "#define CONFIG_SUZAKU 1" >> include/config.h
	@./mkconfig -a $(@:_config=) microblaze microblaze suzaku AtmarkTechno

#########################################################################
#########################################################################

clean:
	find . -type f \
		\( -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'  -o -name '*.a'  \) -print \
		| xargs rm -f
	rm -f examples/hello_world examples/timer \
	      examples/eepro100_eeprom examples/sched \
	      examples/mem_to_mem_idma2intr examples/82559_eeprom \
	      examples/test_burst
	rm -f tools/img2srec tools/mkimage tools/envcrc tools/gen_eth_addr
	rm -f tools/mpc86x_clk tools/ncb
	rm -f tools/easylogo/easylogo tools/bmp_logo
	rm -f tools/gdb/astest tools/gdb/gdbcont tools/gdb/gdbsend
	rm -f tools/env/fw_printenv tools/env/fw_setenv
	rm -f board/cray/L1/bootscript.c board/cray/L1/bootscript.image
	rm -f board/trab/trab_fkt board/voiceblue/eeprom
	rm -f board/integratorap/u-boot.lds board/integratorcp/u-boot.lds

clobber:	clean
	find . -type f \( -name .depend \
		-o -name '*.srec' -o -name '*.bin' -o -name u-boot.img \) \
		-print0 \
		| xargs -0 rm -f
	rm -f $(OBJS) *.bak tags TAGS
	rm -fr *.*~
	rm -f u-boot u-boot.map u-boot.hex $(ALL)
	rm -f tools/crc32.c tools/environment.c tools/env/crc32.c
	rm -f tools/inca-swap-bytes cpu/mpc824x/bedbug_603e.c
	rm -f include/asm/proc include/asm/arch include/asm

mrproper \
distclean:	clobber unconfig

backup:
	F=`basename $(TOPDIR)` ; cd .. ; \
	gtar --force-local -zcvf `date "+$$F-%Y-%m-%d-%T.tar.gz"` $$F

#########################################################################
