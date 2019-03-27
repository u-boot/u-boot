# This is an example file to generate boot.scr - a boot script for U-Boot
# This exmaple only target for qspi boot, sameway it can be created for boot
# devices like nand.
# Generate boot.scr:
# ./tools/mkimage -c none -A arm -T script -d sdboot.cmd boot.scr
#
# It requires a list of environment variables used below to be defined
# before load
#
mmc dev $sdbootdev && mmcinfo && run uenvboot || run sdroot$sdbootdev;load mmc $sdbootdev:$partid $fdt_addr system.dtb && load mmc $sdbootdev:$partid $kernel_addr Image && booti $kernel_addr - $fdt_addr
