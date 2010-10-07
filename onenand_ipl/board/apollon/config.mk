#
# (C) Copyright 2005-2008 Samsung Electronics
# Kyungmin Park <kyungmin.park@samsung.com>
#
# Samsung Apollon board with OMAP2420 (ARM1136) cpu
#
# Apollon has 1 bank of 128MB mDDR-SDRAM on CS0
# Physical Address:
# 8000'0000 (bank0)
# 8800'0000 (bank1)
# Linux-Kernel is expected to be at 8000'8000, entry 8000'8000
# (mem base + reserved)

CONFIG_SYS_TEXT_BASE = 0x00000000
