.. SPDX-License-Identifier: GPL-2.0-only
.. sectionauthor:: Emanuele Ghidoli <emanuele.ghidoli@toradex.com>

Aquila AM69 Module
==================

Quick Start
-----------

- Setup environment variables
- Get binary-only TI Linux firmware
- Build the ARM trusted firmware binary
- Build the OPTEE binary
- Build U-Boot for the R5
- Build U-Boot for the A72
- Flash to eMMC
- Boot

Setup environment
-----------------

Suggested current toolchains are ARM 11.3 (https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads):

- https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-linux-gnueabihf.tar.xz
- https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-x86_64-aarch64-none-linux-gnu.tar.xz

.. code-block:: console

    $ export CROSS_COMPILE_32=<path/to/arm/toolchain/bin/>arm-none-linux-gnueabihf-
    $ export CROSS_COMPILE_64=<path/to/arm64/toolchain/bin/>aarch64-none-linux-gnu-

Get the TI Linux Firmware
-------------------------

.. code-block:: console

    $ echo "Downloading TI Linux Firmware..."
    $ git clone -b ti-linux-firmware https://git.ti.com/git/processor-firmware/ti-linux-firmware.git

Get and Build the ARM Trusted Firmware (Trusted Firmware A)
-----------------------------------------------------------

.. code-block:: console

    $ echo "Downloading and building TF-A..."
    $ git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    $ cd trusted-firmware-a

Then build ATF (TF-A):

.. code-block:: console

    $ export CROSS_COMPILE="$CROSS_COMPILE_64"
    $ make PLAT=k3 TARGET_BOARD=j784s4 SPD=opteed K3_USART=0x8

Get and Build OPTEE
-------------------

.. code-block:: console

    $ echo "Downloading and building OPTEE..."
    $ git clone https://github.com/OP-TEE/optee_os.git
    $ cd optee_os

Then build OPTEE:

.. code-block:: console

    $ export CROSS_COMPILE="$CROSS_COMPILE_32"
    $ export CROSS_COMPILE64="$CROSS_COMPILE_64"
    $ export CFG_CONSOLE_UART=0x8
    $ make PLATFORM=k3-j784s4 CFG_ARM64_core=y

Build U-Boot for R5
-------------------

.. code-block:: console

    $ export CROSS_COMPILE="$CROSS_COMPILE_32"
    $ export BINMAN_INDIRS=<path/to/ti-linux-firmware>
    $ make O=/tmp/aquila-r5 aquila-am69_r5_config
    $ make O=/tmp/aquila-r5

Build U-Boot for A72
--------------------

.. code-block:: console

    $ export CROSS_COMPILE=$CROSS_COMPILE_64
    $ export BL31=<path/to/atf>/build/k3/j784s4/release/bl31.bin
    $ export TEE=<path/to/optee>/out/arm-plat-k3/core/tee-pager_v2.bin
    $ export BINMAN_INDIRS="<path/to/ti-linux-firmware> /tmp/aquila-r5"
    $ make O=/tmp/aquila-a72 aquila-am69_a72_config
    $ make O=/tmp/aquila-a72

Flash to eMMC
-------------

.. code-block:: console

    => mmc dev 0 1
    => fatload mmc 1 ${loadaddr} tiboot3.bin
    => mmc write ${loadaddr} 0x0 0x400
    => fatload mmc 1 ${loadaddr} tispl.bin
    => mmc write ${loadaddr} 0x400 0x1000
    => fatload mmc 1 ${loadaddr} u-boot.img
    => mmc write ${loadaddr} 0x1400 0xc00

As a convenience, instead of having to remember all those addresses and sizes,
one may also use the update U-Boot wrappers:

.. code-block:: console

    => tftpboot ${loadaddr} tiboot3.bin
    => run update_tiboot3

    => tftpboot ${loadaddr} tispl.bin
    => run update_tispl

    => tftpboot ${loadaddr} u-boot.img
    => run update_uboot

Boot
----

Output::

  U-Boot SPL 2026.01-rc1-00200-g39abfe677ff5-dirty (Nov 03 2025 - 18:09:30 +0100)
  SYSFW ABI: 4.0 (firmware rev 0x000b '11.1.8--v11.01.08 (Fancy Rat)')
  HW CFG: 0x00
  Initialized 4 DRAM controllers
  SPL initial stack usage: 13552 bytes
  Trying to boot from MMC1
  Authentication passed
  Authentication passed
  Authentication passed
  Loading Environment from nowhere... OK
  init_env from device 17 not supported!
  Authentication passed
  Authentication passed
  Starting ATF on ARM64 core...

  NOTICE:  BL31: v2.13.0(release):v2.13.0-1063-g7a0a320df
  NOTICE:  BL31: Built : 13:30:07, Oct 28 2025
  I/TC: 
  I/TC: OP-TEE version: 4.8.0 (gcc version 11.3.1 20220712 (Arm GNU Toolchain 11.3.Rel1)) #1 Tue Oct 28 12:32:30 UTC 2025 aarch64
  I/TC: WARNING: This OP-TEE configuration might be insecure!
  I/TC: WARNING: Please check https://optee.readthedocs.io/en/latest/architecture/porting_guidelines.html
  I/TC: Primary CPU initializing
  I/TC: GIC redistributor base address not provided
  I/TC: Assuming default GIC group status and modifier
  I/TC: SYSFW ABI: 4.0 (firmware rev 0x000b '11.1.8--v11.01.08 (Fancy Rat)')
  I/TC: Activated SA2UL device
  I/TC: Enabled firewalls for SA2UL TRNG device
  I/TC: EIP76D TRNG initialized
  I/TC: SA2UL Drivers initialized
  I/TC: HUK Initialized
  I/TC: Disabling output console

  U-Boot SPL 2026.01-rc1-00200-g39abfe677ff5-dirty (Nov 03 2025 - 18:11:30 +0100)
  Unable to shutdown MCU R5 core 1, -22
  SYSFW ABI: 4.0 (firmware rev 0x000b '11.1.8--v11.01.08 (Fancy Rat)')
  DM ABI: 3.0 (firmware ver 0x000b 'PSDK.11.01.00.04--v11.01.08a' patch_ver: 8)
  HW CFG: 0x00
  Trying to boot from MMC1
  Authentication passed
  Authentication passed


  U-Boot 2026.01-rc1-00200-g39abfe677ff5-dirty (Nov 03 2025 - 18:11:30 +0100)

  SoC:   J784S4 SR1.0 HS-FS
  DRAM:  2 GiB (total 32 GiB)
  optee optee: OP-TEE: revision 4.8 (86660925433a8d4d)
  Core:  191 devices, 35 uclasses, devicetree: separate
  MMC:   mmc@4f80000: 0, mmc@4fb0000: 1
  Loading Environment from MMC... Reading from MMC(0)... OK
  MISSING TORADEX CARRIER CONFIG BLOCKS
  In:    serial@2880000
  Out:   serial@2880000
  Err:   serial@2880000
  Model: Toradex 0088 Aquila AM69 Octa 32GB WB IT V1.1A
  Serial#: 12593784
  Net:   am65_cpsw_nuss ethernet@46000000: K3 CPSW: nuss_ver: 0x6BA02102 cpsw_ver: 0x6BA82102 ale_ver: 0x00293904 Ports:1

  Warning: ethernet@46000000port@1 MAC addresses don't match:
  Address in ROM is               c0:d6:0a:de:0e:e6
  Address in environment is       00:14:2d:c0:2a:78
  eth0: ethernet@46000000port@1 [PRIME]
  Hit any key to stop autoboot: 0
  MMC: no card present
  Cannot persist EFI variables without system partition
  ** Booting bootflow '<NULL>' with efi_mgr
  Loading Boot0000 'mmc 0' failed
  EFI boot manager: Cannot load any image
  Boot failed (err=-14)
  MMC: no card present
  MMC: no card present
  MMC: no card present
  MMC: no card present
  ** Booting bootflow 'mmc@4f80000.bootdev.part_1' with script
  Loading DeviceTree: k3-am69-aquila-dev.dtb
  142461 bytes read in 2 ms (67.9 MiB/s)
  87 bytes read in 1 ms (85 KiB/s)
  Working FDT set to 90200000
  Applying Overlay: aquila-am69_spi1_spidev_overlay.dtbo
  560 bytes read in 1 ms (546.9 KiB/s)
  Applying Overlay: aquila-am69_spi2_spidev_overlay.dtbo
  560 bytes read in 1 ms (546.9 KiB/s)
  7937825 bytes read in 28 ms (270.4 MiB/s)
  Bootargs: root=PARTUUID=1d80c51f-02 ro rootwait console=tty1 console=ttyS2,115200
     Uncompressing Kernel Image to 0
  ## Flattened Device Tree blob at 90200000
     Booting using the fdt blob at 0x90200000
  Working FDT set to 90200000
     Loading Device Tree to 00000000fce5c000, end 00000000fcea1fff ... OK
  Working FDT set to fce5c000

  Starting kernel ...
