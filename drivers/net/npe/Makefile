#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB := $(obj)libnpe.o

LOCAL_CFLAGS  += -I$(TOPDIR)/drivers/net/npe/include -DCONFIG_IXP425_COMPONENT_ETHDB -D__linux
CFLAGS  += $(LOCAL_CFLAGS)
CPPFLAGS  += $(LOCAL_CFLAGS) # needed for depend
HOSTCFLAGS  += $(LOCAL_CFLAGS)

COBJS-$(CONFIG_IXP4XX_NPE) := npe.o \
	miiphy.o \
	IxOsalBufferMgt.o \
	IxOsalIoMem.o \
	IxOsalOsCacheMMU.o \
	IxOsalOsMsgQ.o \
	IxOsalOsSemaphore.o \
	IxOsalOsServices.o \
	IxOsalOsThread.o \
	IxEthAcc.o \
	IxEthAccCommon.o \
	IxEthAccControlInterface.o \
	IxEthAccDataPlane.o \
	IxEthAccMac.o \
	IxEthAccMii.o \
	IxEthDBAPI.o \
	IxEthDBAPISupport.o \
	IxEthDBCore.o \
	IxEthDBEvents.o \
	IxEthDBFeatures.o \
	IxEthDBFirewall.o \
	IxEthDBHashtable.o \
	IxEthDBLearning.o \
	IxEthDBMem.o \
	IxEthDBNPEAdaptor.o \
	IxEthDBPortUpdate.o \
	IxEthDBReports.o \
	IxEthDBSearch.o \
	IxEthDBSpanningTree.o \
	IxEthDBUtil.o \
	IxEthDBVlan.o \
	IxEthDBWiFi.o \
	IxEthMii.o \
	IxQMgrAqmIf.o \
	IxQMgrDispatcher.o \
	IxQMgrInit.o \
	IxQMgrQAccess.o \
	IxQMgrQCfg.o \
	IxFeatureCtrl.o \
	IxNpeDl.o \
	IxNpeDlImageMgr.o \
	IxNpeDlNpeMgr.o \
	IxNpeDlNpeMgrUtils.o \
	IxNpeMh.o \
	IxNpeMhConfig.o \
	IxNpeMhReceive.o \
	IxNpeMhSend.o \
	IxNpeMhSolicitedCbMgr.o \
	IxNpeMhUnsolicitedCbMgr.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
