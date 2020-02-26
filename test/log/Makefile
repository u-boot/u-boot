# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2017 Google, Inc

obj-$(CONFIG_LOG_TEST) += log_test.o

ifdef CONFIG_UT_LOG

obj-y += test-main.o

ifdef CONFIG_SANDBOX
obj-$(CONFIG_LOG_SYSLOG) += syslog_test.o
endif

ifndef CONFIG_LOG
obj-$(CONFIG_CONSOLE_RECORD) += nolog_test.o
endif

endif # CONFIG_UT_LOG
