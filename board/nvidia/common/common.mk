# common options for all tegra boards
COBJS-y	+= ../../nvidia/common/board.o
COBJS-$(CONFIG_SPI_UART_SWITCH) += ../../nvidia/common/uart-spi-switch.o
COBJS-$(CONFIG_TEGRA_CLOCK_SCALING) += ../../nvidia/common/emc.o
