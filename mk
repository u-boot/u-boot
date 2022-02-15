make CROSS_COMPILE=/home/engdahl/cross/arm-cortexm7-eabi/bin/arm-cortexm7-eabi- KCFLAGS="-fno-inline -Os" $*

dtc -I dtb -O dts dts/dt.dtb >u-boot.dts
