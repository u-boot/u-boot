# SPDX-License-Identifier: GPL-2.0+
# This Dockerfile is used to build an image containing basic stuff to be used
# to build U-Boot and run our test suites.

FROM ubuntu:jammy-20220801
MAINTAINER Tom Rini <trini@konsulko.com>
LABEL Description=" This image is for building U-Boot inside a container"

# Make sure apt is happy
ENV DEBIAN_FRONTEND=noninteractive

# Add LLVM repository
RUN apt-get update && apt-get install -y gnupg2 wget xz-utils && rm -rf /var/lib/apt/lists/*
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN echo deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-13 main | tee /etc/apt/sources.list.d/llvm.list

# Manually install the kernel.org "Crosstool" based toolchains for gcc-11.1.0
RUN wget -O - https://mirrors.edge.kernel.org/pub/tools/crosstool/files/bin/x86_64/11.1.0/x86_64-gcc-11.1.0-nolibc-aarch64-linux.tar.xz | tar -C /opt -xJ
RUN wget -O - https://mirrors.edge.kernel.org/pub/tools/crosstool/files/bin/x86_64/11.1.0/x86_64-gcc-11.1.0-nolibc-arm-linux-gnueabi.tar.xz | tar -C /opt -xJ
RUN wget -O - https://mirrors.edge.kernel.org/pub/tools/crosstool/files/bin/x86_64/11.1.0/x86_64-gcc-11.1.0-nolibc-i386-linux.tar.xz | tar -C /opt -xJ
RUN wget -O - https://mirrors.edge.kernel.org/pub/tools/crosstool/files/bin/x86_64/11.1.0/x86_64-gcc-11.1.0-nolibc-m68k-linux.tar.xz | tar -C /opt -xJ
RUN wget -O - https://mirrors.edge.kernel.org/pub/tools/crosstool/files/bin/x86_64/11.1.0/x86_64-gcc-11.1.0-nolibc-mips-linux.tar.xz | tar -C /opt -xJ
RUN wget -O - https://mirrors.edge.kernel.org/pub/tools/crosstool/files/bin/x86_64/11.1.0/x86_64-gcc-11.1.0-nolibc-microblaze-linux.tar.xz | tar -C /opt -xJ
RUN wget -O - https://mirrors.edge.kernel.org/pub/tools/crosstool/files/bin/x86_64/11.1.0/x86_64-gcc-11.1.0-nolibc-nios2-linux.tar.xz | tar -C /opt -xJ
RUN wget -O - https://mirrors.edge.kernel.org/pub/tools/crosstool/files/bin/x86_64/11.1.0/x86_64-gcc-11.1.0-nolibc-powerpc-linux.tar.xz | tar -C /opt -xJ
RUN wget -O - https://mirrors.edge.kernel.org/pub/tools/crosstool/files/bin/x86_64/11.1.0/x86_64-gcc-11.1.0-nolibc-riscv64-linux.tar.xz | tar -C /opt -xJ
RUN wget -O - https://mirrors.edge.kernel.org/pub/tools/crosstool/files/bin/x86_64/11.1.0/x86_64-gcc-11.1.0-nolibc-sh2-linux.tar.xz | tar -C /opt -xJ

# Manually install other toolchains
RUN wget -O - https://github.com/foss-xtensa/toolchain/releases/download/2020.07/x86_64-2020.07-xtensa-dc233c-elf.tar.gz | tar -C /opt -xz
RUN wget -O - https://github.com/foss-for-synopsys-dwc-arc-processors/toolchain/releases/download/arc-2021.03-release/arc_gnu_2021.03_prebuilt_uclibc_le_archs_linux_install.tar.gz | tar --no-same-owner -C /opt -xz

# Update and install things from apt now
RUN apt-get update && apt-get install -y \
	automake \
	autopoint \
	bc \
	binutils-dev \
	bison \
	build-essential \
	clang-13 \
	coreutils \
	cpio \
	cppcheck \
	curl \
	device-tree-compiler \
	dosfstools \
	e2fsprogs \
	efitools \
	erofs-utils \
	expect \
	fakeroot \
	flex \
	gawk \
	gdisk \
	git \
	gnu-efi \
	gnutls-dev \
	graphviz \
	grub-efi-amd64-bin \
	grub-efi-ia32-bin \
	help2man \
	iasl \
	imagemagick \
	iputils-ping \
	libconfuse-dev \
	libgit2-dev \
	libjson-glib-dev \
	libguestfs-tools \
	libgnutls28-dev \
	libgnutls30 \
	liblz4-tool \
	libpixman-1-dev \
	libpython3-dev \
	libsdl1.2-dev \
	libsdl2-dev \
	libseccomp-dev \
	libssl-dev \
	libtool \
	libudev-dev \
	libusb-1.0-0-dev \
	linux-image-kvm \
	lzma-alone \
	lzop \
	mount \
	mtd-utils \
	mtools \
	net-tools \
	ninja-build \
	openssl \
	picocom \
	parted \
	pkg-config \
	python-is-python3 \
	python2.7 \
	python3 \
	python3-dev \
	python3-pip \
	python3-sphinx \
	python3-virtualenv \
	rpm2cpio \
	sbsigntool \
	sloccount \
	socat \
	softhsm2 \
	sparse \
	srecord \
	sudo \
	swig \
	util-linux \
	uuid-dev \
	virtualenv \
	xxd \
	zip \
	&& rm -rf /var/lib/apt/lists/*

# Make kernels readable for libguestfs tools to work correctly
RUN chmod +r /boot/vmlinu*

# Build GRUB UEFI targets for ARM & RISC-V, 32-bit and 64-bit
RUN git clone git://git.savannah.gnu.org/grub.git /tmp/grub && \
	cd /tmp/grub && \
	git checkout grub-2.06 && \
	./bootstrap && \
	mkdir -p /opt/grub && \
	./configure --target=aarch64 --with-platform=efi \
	CC=gcc \
	TARGET_CC=/opt/gcc-11.1.0-nolibc/aarch64-linux/bin/aarch64-linux-gcc \
	TARGET_OBJCOPY=/opt/gcc-11.1.0-nolibc/aarch64-linux/bin/aarch64-linux-objcopy \
	TARGET_STRIP=/opt/gcc-11.1.0-nolibc/aarch64-linux/bin/aarch64-linux-strip \
	TARGET_NM=/opt/gcc-11.1.0-nolibc/aarch64-linux/bin/aarch64-linux-nm \
	TARGET_RANLIB=/opt/gcc-11.1.0-nolibc/aarch64-linux/bin/aarch64-linux-ranlib && \
	make && \
	./grub-mkimage -O arm64-efi -o /opt/grub/grubaa64.efi --prefix= -d \
	grub-core cat chain configfile echo efinet ext2 fat halt help linux \
	lsefisystab loadenv lvm minicmd normal part_msdos part_gpt reboot \
	search search_fs_file search_fs_uuid search_label serial sleep test \
	true && \
	make clean && \
	./configure --target=arm --with-platform=efi \
	CC=gcc \
	TARGET_CC=/opt/gcc-11.1.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc \
	TARGET_OBJCOPY=/opt/gcc-11.1.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-objcopy \
	TARGET_STRIP=/opt/gcc-11.1.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-strip \
	TARGET_NM=/opt/gcc-11.1.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-nm \
	TARGET_RANLIB=/opt/gcc-11.1.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-ranlib && \
	make && \
	./grub-mkimage -O arm-efi -o /opt/grub/grubarm.efi --prefix= -d \
	grub-core cat chain configfile echo efinet ext2 fat halt help linux \
	lsefisystab loadenv lvm minicmd normal part_msdos part_gpt reboot \
	search search_fs_file search_fs_uuid search_label serial sleep test \
	true && \
	make clean && \
	./configure --target=riscv64 --with-platform=efi \
	CC=gcc \
	TARGET_CC=/opt/gcc-11.1.0-nolibc/riscv64-linux/bin/riscv64-linux-gcc \
	TARGET_OBJCOPY=/opt/gcc-11.1.0-nolibc/riscv64-linux/bin/riscv64-linux-objcopy \
	TARGET_STRIP=/opt/gcc-11.1.0-nolibc/riscv64-linux/bin/riscv64-linux-strip \
	TARGET_NM=/opt/gcc-11.1.0-nolibc/riscv64-linux/bin/riscv64-linux-nm \
	TARGET_RANLIB=/opt/gcc-11.1.0-nolibc/riscv64-linux/bin/riscv64-linux-ranlib && \
	make && \
	./grub-mkimage -O riscv64-efi -o /opt/grub/grubriscv64.efi --prefix= -d \
	grub-core cat chain configfile echo efinet ext2 fat halt help linux \
	lsefisystab loadenv lvm minicmd normal part_msdos part_gpt reboot \
	search search_fs_file search_fs_uuid search_label serial sleep test \
	true && \
	rm -rf /tmp/grub

RUN git clone git://git.qemu.org/qemu.git /tmp/qemu && \
	cd /tmp/qemu && \
	git submodule update --init dtc && \
	git checkout v6.1.0 && \
	# config user.name and user.email to make 'git am' happy
	git config user.name u-boot && \
	git config user.email u-boot@denx.de && \
	# manually apply the bug fix for QEMU 6.1.0 Xilinx Zynq UART emulation codes
	wget -O - http://patchwork.ozlabs.org/project/qemu-devel/patch/20210823020813.25192-2-bmeng.cn@gmail.com/mbox/ | git am && \
	./configure --prefix=/opt/qemu --target-list="aarch64-softmmu,arm-softmmu,i386-softmmu,mips-softmmu,mips64-softmmu,mips64el-softmmu,mipsel-softmmu,ppc-softmmu,riscv32-softmmu,riscv64-softmmu,sh4-softmmu,x86_64-softmmu,xtensa-softmmu" && \
	make -j$(nproc) all install && \
	rm -rf /tmp/qemu

# Build genimage (required by some targets to generate disk images)
RUN wget -O - https://github.com/pengutronix/genimage/releases/download/v14/genimage-14.tar.xz | tar -C /tmp -xJ && \
	cd /tmp/genimage-14 && \
	./configure && \
	make -j$(nproc) && \
	make install && \
	rm -rf /tmp/genimage-14

# Build libtpms
RUN git clone https://github.com/stefanberger/libtpms /tmp/libtpms && \
	cd /tmp/libtpms && \
	./autogen.sh && \
	./configure && \
	make -j$(nproc) && \
	make install && \
	ldconfig && \
	rm -rf /tmp/libtpms

# Build swtpm
RUN git clone https://github.com/stefanberger/swtpm /tmp/swtpm && \
	cd /tmp/swtpm && \
	./autogen.sh && \
	./configure && \
	make -j$(nproc) && \
	make install && \
	rm -rf /tmp/swtpm

# Create our user/group
RUN echo uboot ALL=NOPASSWD: ALL > /etc/sudoers.d/uboot
RUN useradd -m -U uboot
USER uboot:uboot

# Create the buildman config file
RUN /bin/echo -e "[toolchain]\nroot = /usr" > ~/.buildman
RUN /bin/echo -e "kernelorg = /opt/gcc-11.1.0-nolibc/*" >> ~/.buildman
RUN /bin/echo -e "arc = /opt/arc_gnu_2021.03_prebuilt_uclibc_le_archs_linux_install" >> ~/.buildman
RUN /bin/echo -e "\n[toolchain-prefix]\nxtensa = /opt/2020.07/xtensa-dc233c-elf/bin/xtensa-dc233c-elf-" >> ~/.buildman;
RUN /bin/echo -e "\n[toolchain-alias]\nsh = sh2" >> ~/.buildman
RUN /bin/echo -e "\nriscv = riscv64" >> ~/.buildman
RUN /bin/echo -e "\nsandbox = x86_64" >> ~/.buildman
RUN /bin/echo -e "\nx86 = i386" >> ~/.buildman;
