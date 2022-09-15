#!/bin/bash -e
# SPDX-License-Identifier: GPL-2.0+
# (C) 2020 Pali Rohár <pali@kernel.org>

# External tools needed for this test:
echo '
	wget
	git
	truncate
	tar
	dpkg
	dd
	make
	gcc
	arm-linux-gnueabi-gcc
	fakeroot		(homepage http://fakeroot-ng.lingnu.com/)
	mcopy			(from mtools, homepage http://www.gnu.org/software/mtools/)
	mformat			(from mtools, homepage http://www.gnu.org/software/mtools/)
	/usr/sbin/mkfs.ubifs	(from mtd-utils, homepage http://www.linux-mtd.infradead.org/)
	/usr/sbin/ubinize	(from mtd-utils, homepage http://www.linux-mtd.infradead.org/)
	/lib/ld-linux.so.2	(32-bit x86 version of LD loader, needed for qflasher)
' | while read tool info; do
	if test -z "$tool"; then continue; fi
	if ! which $tool 1>/dev/null 2>&1; then
		echo "Tool $tool was not found and is required to run this test"
		echo "First install $tool $info"
		exit 1
	fi
done || exit 1

echo
echo "============================================================"
echo "========== Compiling U-Boot for Nokia RX-51 board =========="
echo "============================================================"
echo

# First compile u-boot-ubifs.bin binary with UBI/UBIFS support for Nokia RX-51 board according to doc/board/nokia/rx51.rst
make nokia_rx51_config
cat >> .config << EOF
CONFIG_CMD_UBI=y
CONFIG_CMD_UBIFS=y
CONFIG_MTD_UBI_BEB_LIMIT=10
EOF
make olddefconfig
make -j4 u-boot.bin CROSS_COMPILE=arm-linux-gnueabi-
mv u-boot.bin u-boot-ubifs.bin

# Then compile standard u-boot.bin binary for Nokia RX-51 board
make nokia_rx51_config
make -j4 u-boot.bin CROSS_COMPILE=arm-linux-gnueabi-

# And then do all stuff in temporary directory
mkdir -p nokia_rx51_tmp
cd nokia_rx51_tmp

test -f mkimage || ln -s ../tools/mkimage .
test -f u-boot.bin || ln -s ../u-boot.bin .
test -f u-boot-ubifs.bin || ln -s ../u-boot-ubifs.bin .

echo
echo "=========================================================================="
echo "========== Downloading and compiling qemu from qemu-linaro fork =========="
echo "=========================================================================="
echo

# Download and compile linaro version qemu which has support for n900 machine
# Last working commit is 8f8d8e0796efe1a6f34cdd83fb798f3c41217ec1
if ! test -f qemu-system-arm; then
	test -d qemu-linaro || git clone https://git.linaro.org/qemu/qemu-linaro.git
	cd qemu-linaro
	git checkout 8f8d8e0796efe1a6f34cdd83fb798f3c41217ec1
	./configure --enable-system --target-list=arm-softmmu --python=/usr/bin/python2.7 --disable-sdl --disable-gtk --disable-curses --audio-drv-list= --audio-card-list= --disable-werror --disable-xen --disable-xen-pci-passthrough --disable-brlapi --disable-vnc --disable-curl --disable-slirp --disable-kvm --disable-user --disable-linux-user --disable-bsd-user --disable-guest-base --disable-uuid --disable-vde --disable-linux-aio --disable-cap-ng --disable-attr --disable-blobs --disable-docs --disable-spice --disable-libiscsi --disable-smartcard-nss --disable-usb-redir --disable-guest-agent --disable-seccomp --disable-glusterfs --disable-nptl --disable-fdt
	make -j4
	cd ..
	ln -s qemu-linaro/arm-softmmu/qemu-system-arm .
fi

echo
echo "==================================================="
echo "========== Downloading external binaries =========="
echo "==================================================="
echo

# Download qflasher and nolo images
# This is proprietary qemu flasher tool with first stage images, but license allows non-commercial redistribution
wget -c http://repository.maemo.org/qemu-n900/qemu-n900.tar.gz
tar -xf qemu-n900.tar.gz

# Download Maemo script u-boot-gen-combined
if ! test -f u-boot-gen-combined; then
	test -d u-boot-maemo || git clone https://github.com/pali/u-boot-maemo.git
	chmod +x u-boot-maemo/debian/u-boot-gen-combined
	ln -s u-boot-maemo/debian/u-boot-gen-combined .
fi

# Download Maemo fiasco kernel
wget -c http://repository.maemo.org/pool/maemo5.0/free/k/kernel/kernel_2.6.28-20103103+0m5_armel.deb
dpkg -x kernel_2.6.28-20103103+0m5_armel.deb kernel_2.6.28

# Download Maemo libc
wget -c http://repository.maemo.org/pool/maemo5.0/free/g/glibc/libc6_2.5.1-1eglibc27+0m5_armel.deb
dpkg -x libc6_2.5.1-1eglibc27+0m5_armel.deb libc6_2.5.1

# Download Maemo busybox
wget -c http://repository.maemo.org/pool/maemo5.0/free/b/busybox/busybox_1.10.2.legal-1osso30+0m5_armel.deb
dpkg -x busybox_1.10.2.legal-1osso30+0m5_armel.deb busybox_1.10.2

echo
echo "======================================="
echo "========== Generating images =========="
echo "======================================="
echo

# Generate kernel image in zImage and uImage format from FIASCO format
dd if=kernel_2.6.28/boot/zImage-2.6.28-20103103+0m5.fiasco of=zImage-2.6.28-omap1 skip=95 bs=1
./mkimage -A arm -O linux -T kernel -C none -a 80008000 -e 80008000 -n zImage-2.6.28-omap1 -d zImage-2.6.28-omap1 uImage-2.6.28-omap1

# Generate rootfs directory
mkdir -p rootfs
mkdir -p rootfs/dev/
mkdir -p rootfs/bin/
mkdir -p rootfs/sbin/
mkdir -p rootfs/lib/
cp -a busybox_1.10.2/bin/busybox rootfs/bin/
cp -a libc6_2.5.1/lib/ld-linux.so.3 rootfs/lib/
cp -a libc6_2.5.1/lib/ld-2.5.so rootfs/lib/
cp -a libc6_2.5.1/lib/libc.so.6 rootfs/lib/
cp -a libc6_2.5.1/lib/libc-2.5.so rootfs/lib/
cp -a libc6_2.5.1/lib/libcrypt.so.1 rootfs/lib/
cp -a libc6_2.5.1/lib/libcrypt-2.5.so rootfs/lib/
test -f rootfs/bin/sh || ln -sf busybox rootfs/bin/sh
test -f rootfs/sbin/poweroff || ln -sf ../bin/busybox rootfs/sbin/poweroff
cat > rootfs/sbin/preinit << EOF
#!/bin/sh
echo
echo "Successfully booted"
echo
/sbin/poweroff -f
EOF
chmod +x rootfs/sbin/preinit

# Generate ubifs image from rootfs directory
# NOTE: Character device on host filesystem can be created only by root
#       But we do not need it on host filesystem, just in ubifs image
#       So run mknod and mkfs.ubifs commands under fakeroot program
#       which via LD_PRELOAD simulate mknod() and stat() functions
#       so mkfs.ubifs will see dev/console as character device and
#       put it correctly as character device into final ubifs image
#       Therefore we can run whole script as non-root nobody user
fakeroot sh -c '
	rm -f rootfs/dev/console;
	mknod rootfs/dev/console c 5 1;
	/usr/sbin/mkfs.ubifs -m 2048 -e 129024 -c 2047 -r rootfs ubifs.img;
'

# Generate ubi image with rootfs on first volume
cat > ubi.ini << EOF
[rootfs]
mode=ubi
image=ubifs.img
vol_id=0
vol_size=230MiB # 1870 LEBs
vol_type=dynamic
vol_name=rootfs
vol_alignment=1
vol_flags=autoresize
EOF
/usr/sbin/ubinize -o ubi.img -p 128KiB -m 2048 -s 512 ubi.ini

# Generate ubi image with rootfs on first volume and kernel in zImage format on second volume for UBI booting
cp ubi.ini ubi_with_kernel.ini
cat >> ubi_with_kernel.ini << EOF
[kernel]
mode=ubi
image=zImage-2.6.28-omap1
vol_id=1
vol_size=2MiB
vol_type=dynamic
vol_name=kernel
vol_alignment=1
EOF
/usr/sbin/ubinize -o ubi_with_kernel.img -p 128KiB -m 2048 -s 512 ubi_with_kernel.ini

# Generate bootmenu for U-Boot serial console testing
cat > bootmenu_uboot << EOF
setenv bootmenu_0 'Serial console test=echo; echo "Testing serial console"; echo; echo "Successfully booted"; echo; poweroff';
setenv bootmenu_1;
setenv bootmenu_delay 1;
setenv bootdelay 1;
EOF
./mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n bootmenu_uboot -d bootmenu_uboot bootmenu_uboot.scr

# Generate bootmenu for eMMC booting (uImage)
cat > bootmenu_emmc << EOF
setenv bootmenu_0 'uImage-2.6.28-omap1 from eMMC=setenv mmcnum 1; setenv mmcpart 1; setenv mmctype fat; setenv bootargs; setenv setup_omap_atag 1; setenv mmckernfile uImage-2.6.28-omap1; run trymmckernboot';
setenv bootmenu_1;
setenv bootmenu_delay 1;
setenv bootdelay 1;
EOF
./mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n bootmenu_emmc -d bootmenu_emmc bootmenu_emmc.scr

# Generate bootmenu for eMMC booting (zImage)
cat > bootmenu_emmc2 << EOF
setenv bootmenu_0 'zImage-2.6.28-omap1 from eMMC=setenv mmcnum 1; setenv mmcpart 1; setenv mmctype fat; setenv bootargs; setenv setup_omap_atag 1; setenv mmckernfile zImage-2.6.28-omap1; run trymmckernboot';
setenv bootmenu_1;
setenv bootmenu_delay 1;
setenv bootdelay 1;
EOF
./mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n bootmenu_emmc2 -d bootmenu_emmc2 bootmenu_emmc2.scr

# Generate bootmenu for OneNAND booting (uImage)
cat > bootmenu_nand << EOF
setenv bootmenu_0 'uImage-2.6.28-omap1 from OneNAND=setenv bootargs; setenv setup_omap_atag 1; mtd read initfs \${kernaddr} && bootm \${kernaddr}';
setenv bootmenu_1;
setenv bootmenu_delay 1;
setenv bootdelay 1;
EOF
./mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n bootmenu_nand -d bootmenu_nand bootmenu_nand.scr

# Generate bootmenu for UBI booting (zImage)
cat > bootmenu_ubi << EOF
setenv bootmenu_0 'zImage-2.6.28-omap1 from UBI=setenv bootargs; setenv setup_omap_atag 1; ubi part rootfs && ubi read \${kernaddr} kernel && bootz \${kernaddr}';
setenv bootmenu_1;
setenv bootmenu_delay 1;
setenv bootdelay 1;
EOF
./mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n bootmenu_ubi -d bootmenu_ubi bootmenu_ubi.scr

# Generate bootmenu for default booting
cat > bootmenu_default << EOF
setenv bootmenu_delay 1;
setenv bootdelay 1;
EOF
./mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n bootmenu_default -d bootmenu_default bootmenu_default.scr

# Generate combined image from u-boot and Maemo fiasco kernel
./u-boot-gen-combined u-boot.bin zImage-2.6.28-omap1 combined_zimage.bin
./u-boot-gen-combined u-boot.bin uImage-2.6.28-omap1 combined_uimage.bin

# Generate combined hack image from u-boot and Maemo fiasco kernel (kernel starts at 2MB offset and qflasher puts 2kB header before supplied image)
cp u-boot.bin combined_hack.bin
dd if=uImage-2.6.28-omap1 of=combined_hack.bin bs=1024 seek=$((2048-2))

# Generate FAT32 eMMC image for U-Boot serial console testing
truncate -s 50MiB emmc_uboot.img
mformat -m 0xf8 -F -h 4 -s 16 -c 1 -t $((50*1024*1024/(4*16*512))) :: -i emmc_uboot.img
mcopy bootmenu_uboot.scr ::/bootmenu.scr -i emmc_uboot.img

# Generate FAT32 eMMC image for eMMC booting (uImage)
truncate -s 50MiB emmc_emmc.img
mformat -m 0xf8 -F -h 4 -s 16 -c 1 -t $((50*1024*1024/(4*16*512))) :: -i emmc_emmc.img
mcopy uImage-2.6.28-omap1 ::/uImage-2.6.28-omap1 -i emmc_emmc.img
mcopy bootmenu_emmc.scr ::/bootmenu.scr -i emmc_emmc.img

# Generate FAT32 eMMC image for eMMC booting (zImage)
truncate -s 50MiB emmc_emmc2.img
mformat -m 0xf8 -F -h 4 -s 16 -c 1 -t $((50*1024*1024/(4*16*512))) :: -i emmc_emmc2.img
mcopy zImage-2.6.28-omap1 ::/zImage-2.6.28-omap1 -i emmc_emmc2.img
mcopy bootmenu_emmc2.scr ::/bootmenu.scr -i emmc_emmc2.img

# Generate FAT32 eMMC image for OneNAND booting (uImage)
truncate -s 50MiB emmc_nand.img
mformat -m 0xf8 -F -h 4 -s 16 -c 1 -t $((50*1024*1024/(4*16*512))) :: -i emmc_nand.img
mcopy bootmenu_nand.scr ::/bootmenu.scr -i emmc_nand.img

# Generate FAT32 eMMC image for UBI booting (zImage)
truncate -s 50MiB emmc_ubi.img
mformat -m 0xf8 -F -h 4 -s 16 -c 1 -t $((50*1024*1024/(4*16*512))) :: -i emmc_ubi.img
mcopy bootmenu_ubi.scr ::/bootmenu.scr -i emmc_ubi.img

# Generate FAT32 eMMC image for default booting
truncate -s 50MiB emmc_default.img
mformat -m 0xf8 -F -h 4 -s 16 -c 1 -t $((50*1024*1024/(4*16*512))) :: -i emmc_default.img
mcopy bootmenu_default.scr ::/bootmenu.scr -i emmc_default.img

# Generate MTD image for U-Boot serial console testing
rm -f mtd_uboot.img
./qflasher -v -x xloader-qemu.bin -s secondary-qemu.bin -k u-boot.bin -m rx51 -o mtd_uboot.img

# Generate MTD image for RAM booting from bootloader nolo images, compiled image and rootfs image
rm -f mtd_ram.img
./qflasher -v -x xloader-qemu.bin -s secondary-qemu.bin -k combined_uimage.bin -r ubi.img -m rx51 -o mtd_ram.img
rm -f mtd_ram2.img
./qflasher -v -x xloader-qemu.bin -s secondary-qemu.bin -k combined_zimage.bin -r ubi.img -m rx51 -o mtd_ram2.img

# Generate MTD image for eMMC booting from bootloader nolo images, u-boot image and rootfs image
rm -f mtd_emmc.img
./qflasher -v -x xloader-qemu.bin -s secondary-qemu.bin -k u-boot.bin -r ubi.img -m rx51 -o mtd_emmc.img

# Generate MTD image for OneNAND booting from bootloader nolo images, combined hacked image and rootfs image
# Kernel image is put into initfs area, but qflasher reject to copy kernel image into initfs area because it does not have initfs signature
# This is hack to workaround this problem, tell qflasher that kernel area for u-boot is bigger and put big combined hacked image (u-boot + kernel with correct offset)
rm -f mtd_nand.img
./qflasher -v -x xloader-qemu.bin -s secondary-qemu.bin -k combined_hack.bin -r ubi.img -m rx51 -p k=4094,i=2 -o mtd_nand.img

# Generate MTD image for UBI booting from bootloader nolo images, u-boot image with UBI/UBIFS support and rootfs image with kernel volume
rm -f mtd_ubi.img
./qflasher -v -x xloader-qemu.bin -s secondary-qemu.bin -k u-boot-ubifs.bin -r ubi_with_kernel.img -m rx51 -o mtd_ubi.img

echo
echo "======================================================"
echo "========== Running test images in n900 qemu =========="
echo "======================================================"
echo

# Run MTD image in qemu and wait for 300s if U-Boot prints testing string to serial console and poweroff
rm -f qemu_uboot.log
./qemu-system-arm -M n900 -mtdblock mtd_uboot.img -sd emmc_uboot.img -serial /dev/stdout -display none > qemu_uboot.log &
qemu_pid=$!
tail -F qemu_uboot.log &
tail_pid=$!
sleep 300 &
sleep_pid=$!
wait -n $sleep_pid $qemu_pid || true
kill -9 $tail_pid $sleep_pid $qemu_pid 2>/dev/null || true
wait || true

# Run MTD image in qemu and wait for 300s if uImage kernel from RAM is correctly booted
rm -f qemu_ram.log
./qemu-system-arm -M n900 -mtdblock mtd_ram.img -serial /dev/stdout -display none > qemu_ram.log &
qemu_pid=$!
tail -F qemu_ram.log &
tail_pid=$!
sleep 300 &
sleep_pid=$!
wait -n $sleep_pid $qemu_pid || true
kill -9 $tail_pid $sleep_pid $qemu_pid 2>/dev/null || true
wait || true

# Run MTD image in qemu and wait for 300s if zImage kernel from RAM is correctly booted
rm -f qemu_ram2.log
./qemu-system-arm -M n900 -mtdblock mtd_ram2.img -sd emmc_default.img -serial /dev/stdout -display none > qemu_ram2.log &
qemu_pid=$!
tail -F qemu_ram2.log &
tail_pid=$!
sleep 300 &
sleep_pid=$!
wait -n $sleep_pid $qemu_pid || true
kill -9 $tail_pid $sleep_pid $qemu_pid 2>/dev/null || true
wait || true

# Run MTD image in qemu and wait for 300s if uImage kernel from eMMC is correctly booted
rm -f qemu_emmc.log
./qemu-system-arm -M n900 -mtdblock mtd_emmc.img -sd emmc_emmc.img -serial /dev/stdout -display none > qemu_emmc.log &
qemu_pid=$!
tail -F qemu_emmc.log &
tail_pid=$!
sleep 300 &
sleep_pid=$!
wait -n $sleep_pid $qemu_pid || true
kill -9 $tail_pid $sleep_pid $qemu_pid 2>/dev/null || true
wait || true

# Run MTD image in qemu and wait for 300s if zImage kernel from eMMC is correctly booted
rm -f qemu_emmc2.log
./qemu-system-arm -M n900 -mtdblock mtd_emmc.img -sd emmc_emmc2.img -serial /dev/stdout -display none > qemu_emmc2.log &
qemu_pid=$!
tail -F qemu_emmc2.log &
tail_pid=$!
sleep 300 &
sleep_pid=$!
wait -n $sleep_pid $qemu_pid || true
kill -9 $tail_pid $sleep_pid $qemu_pid 2>/dev/null || true
wait || true

# Run MTD image in qemu and wait for 300s if kernel from OneNAND is correctly booted
rm -f qemu_nand.log
./qemu-system-arm -M n900 -mtdblock mtd_nand.img -sd emmc_nand.img -serial /dev/stdout -display none > qemu_nand.log &
qemu_pid=$!
tail -F qemu_nand.log &
tail_pid=$!
sleep 300 &
sleep_pid=$!
wait -n $sleep_pid $qemu_pid || true
kill -9 $tail_pid $sleep_pid $qemu_pid 2>/dev/null || true
wait || true

# Run MTD image in qemu and wait for 300s if kernel from UBI is correctly booted
rm -f qemu_ubi.log
./qemu-system-arm -M n900 -mtdblock mtd_ubi.img -sd emmc_ubi.img -serial /dev/stdout -display none > qemu_ubi.log &
qemu_pid=$!
tail -F qemu_ubi.log &
tail_pid=$!
sleep 300 &
sleep_pid=$!
wait -n $sleep_pid $qemu_pid || true
kill -9 $tail_pid $sleep_pid $qemu_pid 2>/dev/null || true
wait || true

echo
echo "============================="
echo "========== Results =========="
echo "============================="
echo

if grep -q 'Successfully booted' qemu_uboot.log; then echo "U-Boot serial console is working"; else echo "U-Boot serial console test failed"; fi
if grep -q 'Successfully booted' qemu_ram.log; then echo "Kernel (uImage) was successfully booted from RAM"; else echo "Failed to boot kernel (uImage) from RAM"; fi
if grep -q 'Successfully booted' qemu_ram2.log; then echo "Kernel (zImage) was successfully booted from RAM"; else echo "Failed to boot kernel (zImage) from RAM"; fi
if grep -q 'Successfully booted' qemu_emmc.log; then echo "Kernel (uImage) was successfully booted from eMMC"; else echo "Failed to boot kernel (uImage) from eMMC"; fi
if grep -q 'Successfully booted' qemu_emmc2.log; then echo "Kernel (zImage) was successfully booted from eMMC"; else echo "Failed to boot kernel (zImage) from eMMC"; fi
if grep -q 'Successfully booted' qemu_nand.log; then echo "Kernel (uImage) was successfully booted from OneNAND"; else echo "Failed to boot kernel (uImage) from OneNAND"; fi
if grep -q 'Successfully booted' qemu_ubi.log; then echo "Kernel (zImage) was successfully booted from UBI"; else echo "Failed to boot kernel (zImage) from UBI"; fi

echo

if grep -q 'Successfully booted' qemu_uboot.log && grep -q 'Successfully booted' qemu_ram.log && grep -q 'Successfully booted' qemu_ram2.log && grep -q 'Successfully booted' qemu_emmc.log && grep -q 'Successfully booted' qemu_emmc2.log && grep -q 'Successfully booted' qemu_nand.log && grep -q 'Successfully booted' qemu_ubi.log; then
	echo "All tests passed"
	exit 0
else
	echo "Some tests failed"
	exit 1
fi
