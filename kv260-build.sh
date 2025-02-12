#!/bin/bash

set -e

[ -e .config ] || {
	make xilinx_zynqmp_kria_defconfig
}

grep -q CONFIG_TRACE=y .config && ftrace_opt="FTRACE=1"

make -j$(nproc) CROSS_COMPILE="ccache aarch64-linux-gnu-" ${ftrace_opt} BL31=bl31.bin BINMAN_ALLOW_MISSING=1

ls -l qspi.bin

# qspi.bin may be flashed via the board's recovery app (press and hold FWUEN,
# press and release RESET, release FWUEN, connect to http://192.168.0.111/)
# Another way to flash is from U-Boot itself:
#  ZynqMP> setenv autoload no; dhcp; tftpboot 20000000 192.168.0.16:qspi.bin; sf probe 0;sf erase 200000 +$filesize; sf write 20000000 200000 $filesize; zynqmp reboot 40
#  ZynqMP> saveenv
#  ZynqMP> run flash
# For the above to work, run a TFTP server in the main U-Boot directory:
#  $ sudo atftpd --daemon --no-fork `pwd`
