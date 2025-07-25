# SPDX-License-Identifier: GPL-2.0+
# This Dockerfile is used to build an image containing basic stuff to be used
# to build U-Boot and run our test suites.

FROM ubuntu:jammy-20250714
LABEL org.opencontainers.image.authors="Tom Rini <trini@konsulko.com>"
LABEL org.opencontainers.image.description=" This image is for building U-Boot inside a container"

# Used by docker to set the target platform: valid values are linux/arm64/v8
# and linux/amd64
ARG TARGETPLATFORM

# Used by docker to set the build platform: the only valid value is linux/amd64
ARG BUILDPLATFORM

# Make sure apt is happy
ENV DEBIAN_FRONTEND=noninteractive

# Set architectures to build for (leaving out ARM which is an exception)
ENV ARCHS="aarch64 arc i386 m68k mips microblaze nios2 powerpc riscv64 riscv32 sh2 x86_64"

# Mirror containing the toolchains
ENV MIRROR=https://mirrors.edge.kernel.org/pub/tools/crosstool/files/bin

# Toolchain version
ENV TCVER=14.2.0

RUN echo "Building on $BUILDPLATFORM, for target $TARGETPLATFORM"

# Add LLVM repository
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y gnupg2 wget xz-utils
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN echo deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-18 main | tee /etc/apt/sources.list.d/llvm.list

# Create a list of URLs to process, then pass them into a 'while read' loop
RUN if [ "$TARGETPLATFORM" = "linux/amd64" ]; then HOSTARCH=x86_64; else HOSTARCH=arm64; fi; ( \
	# Manually install the kernel.org "Crosstool"-based toolchains
	for arch in $ARCHS; do \
		echo $MIRROR/$HOSTARCH/$TCVER/${HOSTARCH}-gcc-$TCVER-nolibc-${arch}-linux.tar.xz; \
	done; \
	\
	# Deal with ARM, which has a 'gnueabi' suffix
	echo $MIRROR/${HOSTARCH}/$TCVER/${HOSTARCH}-gcc-$TCVER-nolibc-arm-linux-gnueabi.tar.xz; \
	\
	) | while read url; do \
		# Read the URL and unpack it into /opt
		wget -O - $url | tar -C /opt -xJ; \
	done

# Manually install other toolchains
RUN if [ "$TARGETPLATFORM" = "linux/amd64" ]; then \
		wget -O - https://github.com/foss-xtensa/toolchain/releases/download/2020.07/x86_64-2020.07-xtensa-dc233c-elf.tar.gz | tar -C /opt -xz; \
	fi

# Update and install things from apt now
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get install -y \
	automake \
	autopoint \
	bc \
	binutils-dev \
	bison \
	build-essential \
	byacc \
	cgpt \
	clang-18 \
	coreutils \
	cpio \
	curl \
	device-tree-compiler \
	dosfstools \
	e2fsprogs \
	efitools \
	erofs-utils \
	exfatprogs \
	expect \
	fakeroot \
	fdisk \
	flex \
	gawk \
	gdisk \
	gettext \
	git \
	gnu-efi \
	gnutls-dev \
	graphviz \
	help2man \
	iasl \
	imagemagick \
	inetutils-telnet \
	iputils-ping \
	libconfuse-dev \
	libgit2-dev \
	libjson-glib-dev \
	libgnutls28-dev \
	libgnutls30 \
	libpixman-1-dev \
	libpython3-dev \
	libsdl1.2-dev \
	libsdl2-dev \
	libseccomp-dev \
	libslirp-dev \
	libssl-dev \
	libtool \
	libudev-dev \
	libusb-1.0-0-dev \
	lz4 \
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
	python3-tomli \
	python3-venv \
	rpm2cpio \
	sbsigntool \
	socat \
	softhsm2 \
	sparse \
	srecord \
	sudo \
	swig \
	texinfo \
	util-linux \
	uuid-dev \
	vboot-kernel-utils \
	vboot-utils \
	xilinx-bootgen \
	xxd \
	zip

# Build GRUB UEFI targets for ARM & RISC-V, 32-bit and 64-bit
RUN git clone git://git.savannah.gnu.org/grub.git /tmp/grub && \
	cd /tmp/grub && \
	git checkout grub-2.12 && \
	git config --global user.name "GitLab CI Runner" && \
	git config --global user.email trini@konsulko.com && \
	./bootstrap && \
	mkdir -p /opt/grub && \
	./configure --target=aarch64 --with-platform=efi \
	CC=gcc \
	TARGET_CC=/opt/gcc-${TCVER}-nolibc/aarch64-linux/bin/aarch64-linux-gcc \
	TARGET_OBJCOPY=/opt/gcc-${TCVER}-nolibc/aarch64-linux/bin/aarch64-linux-objcopy \
	TARGET_STRIP=/opt/gcc-${TCVER}-nolibc/aarch64-linux/bin/aarch64-linux-strip \
	TARGET_NM=/opt/gcc-${TCVER}-nolibc/aarch64-linux/bin/aarch64-linux-nm \
	TARGET_RANLIB=/opt/gcc-${TCVER}-nolibc/aarch64-linux/bin/aarch64-linux-ranlib && \
	make -j$(nproc) && \
	./grub-mkimage -O arm64-efi -o /opt/grub/grubaa64.efi --prefix= -d \
	grub-core cat chain configfile echo efinet ext2 fat halt help linux \
	lsefisystab loadenv lvm minicmd normal part_msdos part_gpt reboot \
	search search_fs_file search_fs_uuid search_label serial sleep test \
	true && \
	make clean && \
	./configure --target=arm --with-platform=efi \
	CC=gcc \
	TARGET_CC=/opt/gcc-${TCVER}-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc \
	TARGET_OBJCOPY=/opt/gcc-${TCVER}-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-objcopy \
	TARGET_STRIP=/opt/gcc-${TCVER}-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-strip \
	TARGET_NM=/opt/gcc-${TCVER}-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-nm \
	TARGET_RANLIB=/opt/gcc-${TCVER}-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-ranlib && \
	make -j$(nproc) && \
	./grub-mkimage -O arm-efi -o /opt/grub/grubarm.efi --prefix= -d \
	grub-core cat chain configfile echo efinet ext2 fat halt help linux \
	lsefisystab loadenv lvm minicmd normal part_msdos part_gpt reboot \
	search search_fs_file search_fs_uuid search_label serial sleep test \
	true && \
	make clean && \
	grub_cv_cc_mcmodel=no ./configure --target=riscv64 --with-platform=efi \
	CC=gcc \
	TARGET_CC=/opt/gcc-${TCVER}-nolibc/riscv64-linux/bin/riscv64-linux-gcc \
	TARGET_OBJCOPY=/opt/gcc-${TCVER}-nolibc/riscv64-linux/bin/riscv64-linux-objcopy \
	TARGET_STRIP=/opt/gcc-${TCVER}-nolibc/riscv64-linux/bin/riscv64-linux-strip \
	TARGET_NM=/opt/gcc-${TCVER}-nolibc/riscv64-linux/bin/riscv64-linux-nm \
	TARGET_RANLIB=/opt/gcc-${TCVER}-nolibc/riscv64-linux/bin/riscv64-linux-ranlib && \
	make -j$(nproc) && \
	./grub-mkimage -O riscv64-efi -o /opt/grub/grubriscv64.efi --prefix= -d \
	grub-core cat chain configfile echo efinet ext2 fat halt help linux \
	lsefisystab loadenv lvm minicmd normal part_msdos part_gpt reboot \
	search search_fs_file search_fs_uuid search_label serial sleep test \
	true && \
	make clean && \
	./configure --target=i386 --with-platform=efi \
	CC=gcc \
	TARGET_CC=/opt/gcc-${TCVER}-nolibc/i386-linux/bin/i386-linux-gcc \
	TARGET_OBJCOPY=/opt/gcc-${TCVER}-nolibc/i386-linux/bin/i386-linux-objcopy \
	TARGET_STRIP=/opt/gcc-${TCVER}-nolibc/i386-linux/bin/i386-linux-strip \
	TARGET_NM=/opt/gcc-${TCVER}-nolibc/i386-linux/bin/i386-linux-nm \
	TARGET_RANLIB=/opt/gcc-${TCVER}-nolibc/i386-linux/bin/i386-linux-ranlib && \
	make -j$(nproc) && \
	./grub-mkimage -O i386-efi -o /opt/grub/grub_x86.efi --prefix= -d \
	grub-core normal echo lsefimmap lsefi lsefisystab efinet tftp minicmd && \
	make clean && \
	./configure --target=x86_64 --with-platform=efi \
	CC=gcc \
	TARGET_CC=/opt/gcc-${TCVER}-nolibc/x86_64-linux/bin/x86_64-linux-gcc \
	TARGET_OBJCOPY=/opt/gcc-${TCVER}-nolibc/x86_64-linux/bin/x86_64-linux-objcopy \
	TARGET_STRIP=/opt/gcc-${TCVER}-nolibc/x86_64-linux/bin/x86_64-linux-strip \
	TARGET_NM=/opt/gcc-${TCVER}-nolibc/x86_64-linux/bin/x86_64-linux-nm \
	TARGET_RANLIB=/opt/gcc-${TCVER}-nolibc/x86_64-linux/bin/x86_64-linux-ranlib && \
	make -j$(nproc) && \
	./grub-mkimage -O x86_64-efi -o /opt/grub/grub_x64.efi --prefix= -d \
	grub-core normal echo lsefimmap lsefi lsefisystab efinet tftp minicmd && \
	rm -rf /tmp/grub

RUN git clone https://gitlab.com/qemu-project/qemu.git /tmp/qemu && \
	cd /tmp/qemu && \
	git checkout v10.0.2 && \
	# config user.name and user.email to make 'git am' happy
	git config user.name u-boot && \
	git config user.email u-boot@denx.de && \
	./configure --prefix=/opt/qemu --target-list="aarch64-softmmu,arm-softmmu,i386-softmmu,m68k-softmmu,mips-softmmu,mips64-softmmu,mips64el-softmmu,mipsel-softmmu,ppc-softmmu,riscv32-softmmu,riscv64-softmmu,sh4-softmmu,x86_64-softmmu,xtensa-softmmu" && \
	make -j$(nproc) all install && \
	rm -rf /tmp/qemu

# Build fiptool
RUN git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git /tmp/tf-a && \
	cd /tmp/tf-a/ && \
	git checkout v2.12.0 && \
	make CROSS_COMPILE=/opt/gcc-${TCVER}-nolibc/aarch64-linux/bin/aarch64-linux- \
		PLAT=fvp BL33=/dev/null -j$(nproc) all fip && \
	mkdir -p /usr/local/bin /opt/tf-a/vexpress_fvp && \
	cp tools/fiptool/fiptool /usr/local/bin && \
	cp build/fvp/release/fip.bin build/fvp/release/bl1.bin \
		/opt/tf-a/vexpress_fvp/ && \
	rm -rf build/fvp && \
	make CROSS_COMPILE=/opt/gcc-${TCVER}-nolibc/aarch64-linux/bin/aarch64-linux- \
		PLAT=fvp BL33=/dev/null TRANSFER_LIST=1 -j$(nproc) all fip && \
	mkdir -p /opt/tf-a/vexpress_fvp_bloblist && \
	cp build/fvp/release/fip.bin build/fvp/release/bl1.bin \
		/opt/tf-a/vexpress_fvp_bloblist/ && \
	rm -rf /tmp/tf-a

# Download the Arm Architecture FVP platform. This file is double compressed.
RUN wget -O - https://developer.arm.com/-/cdn-downloads/permalink/FVPs-Architecture/FM-11.28/FVP_Base_RevC-2xAEMvA_11.28_23_Linux64.tgz | gunzip -dc | tar -C /opt -x

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

# Build trace-cmd
RUN mkdir /tmp/trace && \
    git clone https://git.kernel.org/pub/scm/libs/libtrace/libtraceevent.git /tmp/trace/libtraceevent && \
    cd /tmp/trace/libtraceevent && \
    make -j$(nproc) && \
    sudo make install && \
    git clone https://git.kernel.org/pub/scm/libs/libtrace/libtracefs.git /tmp/trace/libtracefs && \
    cd /tmp/trace/libtracefs && \
    make -j$(nproc) && \
    sudo make install && \
    git clone https://github.com/rostedt/trace-cmd.git /tmp/trace/trace-cmd && \
    cd /tmp/trace/trace-cmd && \
    make -j$(nproc) && \
    sudo make install && \
    rm -rf /tmp/trace

# Build coreboot
RUN wget -O - https://coreboot.org/releases/coreboot-25.03.tar.xz | tar -C /tmp -xJ && \
    cd /tmp/coreboot-25.03 && \
    make crossgcc-i386 CPUS=$(nproc) && \
    make -C payloads/coreinfo olddefconfig && \
    make -C payloads/coreinfo && \
    make olddefconfig && \
    echo CONFIG_GENERIC_LINEAR_FRAMEBUFFER=y | tee -a .config && \
    echo CONFIG_USE_OPTION_TABLE=y | tee -a .config && \
    make olddefconfig && \
    make -j $(nproc) && \
    sudo mkdir /opt/coreboot && \
    sudo cp build/coreboot.rom build/cbfstool /opt/coreboot/ && \
    rm -rf /tmp/coreboot-25.03

# Create our user/group
RUN echo uboot ALL=NOPASSWD: ALL > /etc/sudoers.d/uboot
RUN useradd -m -U uboot
USER uboot:uboot

# Populate the cache for pip to use. Get these via wget as the
# COPY / ADD directives don't work as we need them to.
RUN wget -O /tmp/pytest-requirements.txt https://source.denx.de/u-boot/u-boot/-/raw/master/test/py/requirements.txt
RUN wget -O /tmp/sphinx-requirements.txt https://source.denx.de/u-boot/u-boot/-/raw/master/doc/sphinx/requirements.txt
RUN wget -O /tmp/binman-requirements.txt https://source.denx.de/u-boot/u-boot/-/raw/master/tools/binman/requirements.txt
RUN wget -O /tmp/buildman-requirements.txt https://source.denx.de/u-boot/u-boot/-/raw/master/tools/buildman/requirements.txt
RUN wget -O /tmp/patman-requirements.txt https://source.denx.de/u-boot/u-boot/-/raw/master/tools/patman/requirements.txt
RUN wget -O /tmp/u_boot_pylib-requirements.txt https://source.denx.de/u-boot/u-boot/-/raw/master/tools/u_boot_pylib/requirements.txt
RUN python3 -m venv /tmp/venv && \
	. /tmp/venv/bin/activate && \
	pip install -r /tmp/pytest-requirements.txt \
		-r /tmp/sphinx-requirements.txt \
		-r /tmp/binman-requirements.txt \
		-r /tmp/buildman-requirements.txt \
		-r /tmp/patman-requirements.txt \
		-r /tmp/u_boot_pylib-requirements.txt && \
	deactivate && \
	rm -rf /tmp/venv /tmp/*-requirements.txt

# Create the buildman config file
RUN /bin/echo -e "[toolchain]\nkernelorg = /opt/gcc-${TCVER}-nolibc/*" > ~/.buildman
RUN /bin/echo -e "root = /usr" >> ~/.buildman
RUN if [ "$TARGETPLATFORM" = "linux/amd64" ]; then \
        /bin/echo -e "\n[toolchain-prefix]\nxtensa = /opt/2020.07/xtensa-dc233c-elf/bin/xtensa-dc233c-elf-" >> ~/.buildman; \
    fi
RUN if [ "$TARGETPLATFORM" = "linux/arm64" ]; then \
        /bin/echo -e "\n[toolchain-prefix]\naarch64 = /opt/gcc-${TCVER}-nolibc/aarch64-linux/bin/aarch64-linux-" >> ~/.buildman; \
    fi
RUN /bin/echo -e "\n[toolchain-alias]\nsh = sh2" >> ~/.buildman
RUN /bin/echo -e "\nx86 = i386" >> ~/.buildman;

# Add mkbootimg tool
RUN git clone https://android.googlesource.com/platform/system/tools/mkbootimg /home/uboot/mkbootimg
ENV PYTHONPATH="${PYTHONPATH}:/home/uboot/mkbootimg"
