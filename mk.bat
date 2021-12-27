make CROSS_COMPILE=arm-cortexm7-eabi- KCFLAGS="-fno-inline -Og" HOSTLDLIBS="-lssl -lcrypto" %*

dtc -I dtb -O dts dts/dt.dtb >u-boot.dts
