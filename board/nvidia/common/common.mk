# common options for all tegra boards
COBJS-y	+= ../../nvidia/common/board.o
COBJS-$(CONFIG_TEGRA_CLOCK_SCALING) += ../../nvidia/common/emc.o
