# common options for all tegra boards
obj-y	+= ../../nvidia/common/board.o
obj-$(CONFIG_TEGRA_CLOCK_SCALING) += ../../nvidia/common/emc.o
