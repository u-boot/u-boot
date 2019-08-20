# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_TARGET_P1020RDB_PC) += p1020rdb-pc.dtb p1020rdb-pc_36b.dtb
dtb-$(CONFIG_TARGET_P1020RDB_PD) += p1020rdb-pd.dtb
dtb-$(CONFIG_TARGET_P2020RDB) += p2020rdb-pc.dtb p2020rdb-pc_36b.dtb
dtb-$(CONFIG_TARGET_T1024RDB) += t1024rdb.dtb
dtb-$(CONFIG_TARGET_T1042D4RDB) += t1042d4rdb.dtb
dtb-$(CONFIG_TARGET_T2080QDS) += t2080qds.dtb
dtb-$(CONFIG_TARGET_T2080RDB) += t2080rdb.dtb
dtb-$(CONFIG_TARGET_T4240RDB) += t4240rdb.dtb
dtb-$(CONFIG_TARGET_MCR3000) += mcr3000.dtb
dtb-$(CONFIG_TARGET_GAZERBEAM) += gazerbeam.dtb

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
