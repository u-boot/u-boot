    make CROSS_COMPILE=arm-cortexm7-eabi- KCFLAGS="-fno-inline -Os"         HOSTLDLIBS="-lssl -lcrypto" %*
rem make CROSS_COMPILE=arm-cortexm7-eabi- KCFLAGS="-fno-inline -Og -DDEBUG" HOSTLDLIBS="-lssl -lcrypto" %*

dtc -I dtb -O dts dts/dt.dtb >u-boot.dts
