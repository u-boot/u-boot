#!/bin/bash
#https://opensource.rock-chips.com/wiki_U-Boot
sudo apt-get install bison flex python3 python3-distutils swig libpython3-dev\
    libssl-dev openssl bc  python3-pip python-pyelftools 
pip3 install pyelftools==0.24

ARM_DEV_DIR=/workspace/rockpi/armdev

#export CROSS_COMPILE=/workspace/rockpi/cross_tool_chain/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
export CROSS_COMPILE=/workspace/rockpi/cross_tool_chain/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu-
#export CROSS_COMPILE=/workspace/rockpi/cross_tool_chain/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
# use cross compiler from apt-get install
# export CROSS_COMPILE=aarch64-linux-gnu-
#config u-boot
# make rock-pi-4b-rk3399_defconfig 
#mv .config .4b_config
#make rock-pi-4-rk3399_defconfig
#mv .config .4_config
#build upstream u-book
export ARCH=arm64
# Uboot mainline
#make

cp ${ARM_DEV_DIR}/rkbin/bin/rk33/rk3399_bl31_v1.35.elf  ./bl31.elf
cp ${ARM_DEV_DIR}/rkbin/bin/rk33/rk3399_bl32_v2.10.bin  ./bl32.bin

make rock-pi-4-rk3399_defconfig all
# make u-boot.itb needs bl31.elf
#Note: please copy the trust binary(optee.bin or bl31.elf from rkbin project) to u-boot root directory and rename it to tee.bin(armv7) or bl31.elf(armv8).
# please use the right version you need
#cp ${ARM_DEV_DIR}/rkbin/bin/rk33/rk3399_bl31_v1.35.elf  ./bl31.elf
#cp ${ARM_DEV_DIR}/rkbin/bin/rk33/rk3399_bl32_v2.10.bin  ./bl32.bin
make u-boot.itb

OUTDIR=${ARM_DEV_DIR}/rockpi4b/build_uboot
mkdir ${OUTDIR}
mkdir ${OUTDIR}/spl
mkdir ${OUTDIR}/tpl

cp u-boot.bin ${OUTDIR}/
cp u-boot.itb ${OUTDIR}/
cp u-boot-nodtb.bin ${OUTDIR}/
cp spl/u-boot-spl.bin ${OUTDIR}/spl/
cp spl/u-boot-spl-dtb.bin ${OUTDIR}/spl/
cp spl/u-boot-spl-nodtb.bin ${OUTDIR}/spl/
cp tpl/u-boot-tpl.bin ${OUTDIR}/tpl/
cp tpl/u-boot-tpl-dtb.bin ${OUTDIR}/tpl/
cp tpl/u-boot-tpl-nodtb.bin ${OUTDIR}/tpl/
cp u-boot-dtb.bin ${OUTDIR}
# rockchip uboot
#./make.sh

#./make.sh rock-pi-4b-rk3399
