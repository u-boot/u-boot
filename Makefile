#
# (C) Copyright 2000-2004
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
ifeq ($(ARCH),m68k)
CROSS_COMPILE = m68k-elf-
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
ifeq ($(CPU),mpc85xx)
OBJS += cpu/$(CPU)/resetvec.o
endif

LIBS  = lib_generic/libgeneric.a
LIBS += board/$(BOARDDIR)/lib$(BOARD).a
LIBS += cpu/$(CPU)/lib$(CPU).a
LIBS += lib_$(ARCH)/lib$(ARCH).a
LIBS += fs/cramfs/libcramfs.a fs/fat/libfat.a fs/fdos/libfdos.a fs/jffs2/libjffs2.a
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
			--start-group $(LIBS) $(PLATFORM_LIBS) --end-group \
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
## MPC5xx Systems
#########################################################################

cmi_mpc5xx_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc5xx cmi

PATI_config:unconfig
	@./mkconfig $(@:_config=) ppc mpc5xx pati mpl

#########################################################################
## MPC5xxx Systems
#########################################################################
MPC5200LITE_config		\
MPC5200LITE_LOWBOOT_config	\
MPC5200LITE_LOWBOOT08_config	\
icecube_5200_DDR_LOWBOOT_config	\
icecube_5200_DDR_config		\
IceCube_5200_DDR_config		\
icecube_5200_config		\
IceCube_5200_config		\
IceCube_5100_config:		unconfig
	@ >include/config.h
	@[ -z "$(findstring LOWBOOT,$@)" ] || \
		{ echo "TEXT_BASE = 0xFF000000" >board/icecube/config.tmp ; \
		  echo "... with LOWBOOT configuration" ; \
		}
	@[ -z "$(findstring LOWBOOT08,$@)" ] || \
		{ echo "TEXT_BASE = 0xFF800000" >board/icecube/config.tmp ; \
		  echo "... with 8 MB flash only" ; \
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

MINI5200_config	\
EVAL5200_config	\
TOP5200_config:	unconfig
	@ echo "#define CONFIG_$(@:_config=) 1"	>include/config.h
	@./mkconfig -a TOP5200 ppc mpc5xxx top5200 emk

#########################################################################
## MPC8xx Systems
#########################################################################

AdderII_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx adderII

ADS860_config     \
DUET_ADS_config   \
FADS823_config    \
FADS850SAR_config \
MPC86xADS_config  \
FADS860T_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx fads

AMX860_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx amx860 westel

bms2003_config	:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx tqm8xx

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

R360MPI_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx r360mpi

RBC823_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx rbc823

RPXClassic_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx RPXClassic

RPXlite_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx RPXlite

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

svm_sc8xx_config:	unconfig
	@ >include/config.h
	@./mkconfig $(@:_config=) ppc mpc8xx svm_sc8xx

SXNI855T_config:	unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx sixnet

# EMK MPC8xx based modules
TOP860_config:		unconfig
	@./mkconfig $(@:_config=) ppc mpc8xx top860 emk

# Play some tricks for configuration selection
# All boards can come with 50 MHz (default), 66MHz, 80MHz or 100 MHz clock,
# but only 855 and 860 boards may come with FEC
# and 823 boards may have LCD support
xtract_8xx = $(subst _66MHz,,$(subst _80MHz,,$(subst _100MHz,,$(subst _133MHz,,$(subst _LCD,,$(subst _config,,$1))))))

FPS850L_config		\
FPS860L_config		\
NSCU_config		\
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
TQM860L_config		\
TQM860L_66MHz_config	\
TQM860L_80MHz_config	\
TQM862L_config		\
TQM862L_66MHz_config	\
TQM862L_80MHz_config	\
TQM823M_config		\
TQM823M_66MHz_config	\
TQM823M_80MHz_config	\
TQM850M_config		\
TQM850M_66MHz_config	\
TQM850M_80MHz_config	\
TQM855M_config		\
TQM855M_66MHz_config	\
TQM855M_80MHz_config	\
TQM860M_config		\
TQM860M_66MHz_config	\
TQM860M_80MHz_config	\
TQM862M_config		\
TQM862M_66MHz_config	\
TQM862M_80MHz_config	\
TQM862M_100MHz_config	\
TQM866M_config:		unconfig
	@ >include/config.h
	@[ -z "$(findstring _66MHz,$@)" ] || \
		{ echo "#define CONFIG_66MHz"		>>include/config.h ; \
		  echo "... with 66MHz system clock" ; \
		}
	@[ -z "$(findstring _80MHz,$@)" ] || \
		{ echo "#define CONFIG_80MHz"		>>include/config.h ; \
		  echo "... with 80MHz system clock" ; \
		}
	@[ -z "$(findstring _100MHz,$@)" ] || \
		{ echo "#define CONFIG_100MHz"		>>include/config.h ; \
		  echo "... with 100MHz system clock" ; \
		}
	@[ -z "$(findstring _133MHz,$@)" ] || \
		{ echo "#define CONFIG_133MHz"		>>include/config.h ; \
		  echo "... with 133MHz system clock" ; \
		}
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
xtract_4xx = $(subst _MODEL_BA,,$(subst _MODEL_ME,,$(subst _MODEL_HI,,$(subst _config,,$1))))

ADCIOP_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx adciop esd

AR405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ar405 esd

ASH405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ash405 esd

BUBINGA405EP_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx bubinga405ep

CANBT_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx canbt esd

CPCI405_config	\
CPCI4052_config	\
CPCI405AB_config:	unconfig
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

DP405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx dp405 esd

DU405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx du405 esd

EBONY_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ebony

ERIC_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx eric

EXBITGEN_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx exbitgen

HUB405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx hub405 esd

MIP405_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx mip405 mpl

MIP405T_config:unconfig
	@echo "#define CONFIG_MIP405T" >include/config.h
	@echo "Enable subset config for MIP405T"
	@./mkconfig -a MIP405 ppc ppc4xx mip405 mpl

ML2_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ml2

OCRTC_config		\
ORSG_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx ocrtc esd

PCI405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx pci405 esd

PIP405_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx pip405 mpl

PLU405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx plu405 esd

PMC405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx pmc405 esd

PPChameleonEVB_MODEL_BA_config	\
PPChameleonEVB_MODEL_ME_config	\
PPChameleonEVB_MODEL_HI_config	\
PPChameleonEVB_config:	unconfig
	@ >include/config.h
	@[ -z "$(findstring _MODEL_BA,$@)" ] || \
		{ echo "#define CONFIG_PPCHAMELEON_MODULE_MODEL 0" >>include/config.h ; \
		  echo "... BASIC model" ; \
		}
	@[ -z "$(findstring _MODEL_ME,$@)" ] || \
		{ echo "#define CONFIG_PPCHAMELEON_MODULE_MODEL 1" >>include/config.h ; \
		  echo "... MEDIUM model" ; \
		}
	@[ -z "$(findstring _MODEL_HI,$@)" ] || \
		{ echo "#define CONFIG_PPCHAMELEON_MODULE_MODEL 2" >>include/config.h ; \
		  echo "... HIGH-END model" ; \
		}
	@./mkconfig -a $(call xtract_4xx,$@) ppc ppc4xx PPChameleonEVB dave

VOH405_config:	unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx voh405 esd

W7OLMC_config	\
W7OLMG_config: unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx w7o

WALNUT405_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx walnut405

XPEDITE1K_config:unconfig
	@./mkconfig $(@:_config=) ppc ppc4xx xpedite1k

#########################################################################
## MPC824x Systems
#########################################################################
xtract_82xx = $(subst _ROMBOOT,,$(subst _L2,,$(subst _266MHz,,$(subst _300MHz,,$(subst _config,,$1)))))

A3000_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x a3000

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

PM825_config	\
PM825_ROMBOOT_config: unconfig
	@echo "#define CONFIG_PCI"	>include/config.h
	@./mkconfig -a PM826 ppc mpc8260 pm826
	@cd ./include ;				\
	if [ "$(findstring _ROMBOOT_,$@)" ] ; then \
		echo "CONFIG_BOOT_ROM = y" >> config.mk ; \
		echo "... booting from 8-bit flash" ; \
	else \
		echo "CONFIG_BOOT_ROM = n" >> config.mk ; \
		echo "... booting from 64-bit flash" ; \
	fi; \
	echo "export CONFIG_BOOT_ROM" >> config.mk; \

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

ZPC1900_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc8260 zpc1900

#========================================================================
# M68K
#========================================================================
#########################################################################
## Coldfire
#########################################################################

M5272C3_config :		unconfig
	@./mkconfig $(@:_config=) m68k mcf52x2 m5272c3

M5282EVB_config :		unconfig
	@./mkconfig $(@:_config=) m68k mcf52x2 m5282evb

#########################################################################
## MPC85xx Systems
#########################################################################

MPC8540ADS_config:      unconfig
	@./mkconfig $(@:_config=) ppc mpc85xx mpc8540ads

MPC8560ADS_config:      unconfig
	@./mkconfig $(@:_config=) ppc mpc85xx mpc8560ads

#########################################################################
## 74xx/7xx Systems
#########################################################################

AmigaOneG3SE_config:	unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx AmigaOneG3SE MAI

BAB7xx_config: unconfig
	@./mkconfig $(@:_config=) ppc 74xx_7xx bab7xx eltec

DB64360_config:  unconfig
	@./mkconfig DB64360 ppc 74xx_7xx db64360 Marvell

DB64460_config:  unconfig
	@./mkconfig DB64460 ppc 74xx_7xx db64460 Marvell

debris_config: unconfig
	@./mkconfig $(@:_config=) ppc mpc824x debris etin

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

at91rm9200dk_config	:	unconfig
	@./mkconfig $(@:_config=) arm at91rm9200 at91rm9200dk

lart_config	:	unconfig
	@./mkconfig $(@:_config=) arm sa1100 lart

dnp1110_config	:	unconfig
	@./mkconfig $(@:_config=) arm sa1100 dnp1110

shannon_config	:	unconfig
	@./mkconfig $(@:_config=) arm sa1100 shannon

#########################################################################
## ARM92xT Systems
#########################################################################

xtract_trab = $(subst _bigram,,$(subst _bigflash,,$(subst _old,,$(subst _config,,$1))))

omap1510inn_config :	unconfig
	@./mkconfig $(@:_config=) arm arm925t omap1510inn

omap1610inn_config :	unconfig
	@./mkconfig $(@:_config=) arm arm926ejs omap1610inn

smdk2400_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t smdk2400

smdk2410_config	:	unconfig
	@./mkconfig $(@:_config=) arm arm920t smdk2410

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

wepep250_config	:	unconfig
	@./mkconfig $(@:_config=) arm pxa wepep250

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

#########################################################################
#########################################################################

clean:
	find . -type f \
		\( -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'  -o -name '*.a'  \) -print \
		| xargs rm -f
	rm -f examples/hello_world examples/timer \
	      examples/eepro100_eeprom examples/sched \
	      examples/mem_to_mem_idma2intr examples/82559_eeprom

	rm -f tools/img2srec tools/mkimage tools/envcrc tools/gen_eth_addr
	rm -f tools/easylogo/easylogo tools/bmp_logo
	rm -f tools/gdb/astest tools/gdb/gdbcont tools/gdb/gdbsend
	rm -f tools/env/fw_printenv tools/env/fw_setenv
	rm -f board/cray/L1/bootscript.c board/cray/L1/bootscript.image
	rm -f board/trab/trab_fkt board/*/config.tmp

clobber:	clean
	find . -type f \
		\( -name .depend -o -name '*.srec' -o -name '*.bin' \) \
		-print \
		| xargs rm -f
	rm -f $(OBJS) *.bak tags TAGS
	rm -fr *.*~
	rm -f u-boot u-boot.map $(ALL)
	rm -f tools/crc32.c tools/environment.c tools/env/crc32.c
	rm -f tools/inca-swap-bytes cpu/mpc824x/bedbug_603e.c
	rm -f include/asm/proc include/asm/arch include/asm

mrproper \
distclean:	clobber unconfig

backup:
	F=`basename $(TOPDIR)` ; cd .. ; \
	gtar --force-local -zcvf `date "+$$F-%Y-%m-%d-%T.tar.gz"` $$F

#########################################################################
