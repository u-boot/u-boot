.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Patrick Delaunay <patrick.delaunay@st.com>

STM32MP15x boards
=================

This is a quick instruction for setup STM32MP15x boards.

Supported devices
-----------------

U-Boot supports STMP32MP15x SoCs:

 - STM32MP157
 - STM32MP153
 - STM32MP151

The STM32MP15x is a Cortex-A MPU aimed at various applications.

It features:

 - Dual core Cortex-A7 application core (Single on STM32MP151)
 - 2D/3D image composition with GPU (only on STM32MP157)
 - Standard memories interface support
 - Standard connectivity, widely inherited from the STM32 MCU family
 - Comprehensive security support

Each line comes with a security option (cryptography & secure boot) and
a Cortex-A frequency option:

 - A : Cortex-A7 @ 650 MHz
 - C : Secure Boot + HW Crypto + Cortex-A7 @ 650 MHz
 - D : Cortex-A7 @ 800 MHz
 - F : Secure Boot + HW Crypto + Cortex-A7 @ 800 MHz

Everything is supported in Linux but U-Boot is limited to:

 1. UART
 2. SD card/MMC controller (SDMMC)
 3. NAND controller (FMC)
 4. NOR controller (QSPI)
 5. USB controller (OTG DWC2)
 6. Ethernet controller

And the necessary drivers

 1. I2C
 2. STPMIC1 (PMIC and regulator)
 3. Clock, Reset, Sysreset
 4. Fuse

Currently the following boards are supported:

 + stm32mp157a-dk1.dts
 + stm32mp157c-dk2.dts
 + stm32mp157c-ed1.dts
 + stm32mp157c-ev1.dts
 + stm32mp15xx-dhcor-avenger96.dts

Boot Sequences
--------------

3 boot configurations are supported with:

+----------+------------------------+-------------------------+--------------+
| **ROM**  | **FSBL**               | **SSBL**                | **OS**       |
+ **code** +------------------------+-------------------------+--------------+
|          | First Stage Bootloader | Second Stage Bootloader | Linux Kernel |
+          +------------------------+-------------------------+--------------+
|          | embedded RAM           | DDR                                    |
+----------+------------------------+-------------------------+--------------+

The **Trusted** boot chain
``````````````````````````

defconfig_file : stm32mp15_trusted_defconfig

    +-------------+-------------------------+------------+-------+
    |  ROM code   | FSBL                    | SSBL       | OS    |
    +             +-------------------------+------------+-------+
    |             |Trusted Firmware-A (TF-A)| U-Boot     | Linux |
    +-------------+-------------------------+------------+-------+
    | TrustZone   |secure monitor                                |
    +-------------+-------------------------+------------+-------+

TF-A performs a full initialization of Secure peripherals and installs a
secure monitor, BL32:

  * SPMin provided by TF-A or
  * OP-TEE from specific partitions (teeh, teed, teex).

U-Boot is running in normal world and uses the secure monitor to access
to secure resources.

The **Basic** boot chain
````````````````````````

defconfig_file : stm32mp15_basic_defconfig

    +-------------+------------+------------+-------+
    |  ROM code   | FSBL       | SSBL       | OS    |
    +             +------------+------------+-------+
    |             |U-Boot SPL  | U-Boot     | Linux |
    +-------------+------------+------------+-------+
    | TrustZone   |            | PSCI from U-Boot   |
    +-------------+------------+------------+-------+

SPL has limited security initialization

U-Boot is running in secure mode and provide a secure monitor to the kernel
with only PSCI support (Power State Coordination Interface defined by ARM).

All the STM32MP15x boards supported by U-Boot use the same generic board
stm32mp1 which support all the bootable devices.

Each board is configured only with the associated device tree.

Device Tree Selection
---------------------

You need to select the appropriate device tree for your board,
the supported device trees for STM32MP15x are:

+ ev1: eval board with pmic stpmic1 (ev1 = mother board + daughter ed1)

   + stm32mp157c-ev1

+ ed1: daughter board with pmic stpmic1

   + stm32mp157c-ed1

+ dk1: Discovery board

   + stm32mp157a-dk1

+ dk2: Discovery board = dk1 with a BT/WiFI combo and a DSI panel

   + stm32mp157c-dk2

+ avenger96: Avenger96 board from Arrow Electronics based on DH Elec. DHCOR SoM

   + stm32mp15xx-dhcor-avenger96

Build Procedure
---------------

1. Install the required tools for U-Boot

   * install package needed in U-Boot makefile
     (libssl-dev, swig, libpython-dev...)

   * install ARMv7 toolchain for 32bit Cortex-A (from Linaro,
     from SDK for STM32MP15x, or any crosstoolchains from your distribution)
     (you can use any gcc cross compiler compatible with U-Boot)

2. Set the cross compiler::

    # export CROSS_COMPILE=/path/to/toolchain/arm-linux-gnueabi-

3. Select the output directory (optional)::

   # export KBUILD_OUTPUT=/path/to/output

   for example: use one output directory for each configuration::

   # export KBUILD_OUTPUT=stm32mp15_trusted
   # export KBUILD_OUTPUT=stm32mp15_basic

   you can build outside of code directory::

   # export KBUILD_OUTPUT=../build/stm32mp15_trusted

4. Configure U-Boot::

   # make <defconfig_file>

   with <defconfig_file>:

   - For **trusted** boot mode : **stm32mp15_trusted_defconfig**
   - For basic boot mode: stm32mp15_basic_defconfig

5. Configure the device-tree and build the U-Boot image::

   # make DEVICE_TREE=<name> all

   Examples:

  a) trusted boot on ev1::

     # export KBUILD_OUTPUT=stm32mp15_trusted
     # make stm32mp15_trusted_defconfig
     # make DEVICE_TREE=stm32mp157c-ev1 all

  b) trusted with OP-TEE boot on dk2::

      # export KBUILD_OUTPUT=stm32mp15_trusted
      # make stm32mp15_trusted_defconfig
      # make DEVICE_TREE=stm32mp157c-dk2 all

  c) basic boot on ev1::

      # export KBUILD_OUTPUT=stm32mp15_basic
      # make stm32mp15_basic_defconfig
      # make DEVICE_TREE=stm32mp157c-ev1 all

  d) basic boot on ed1::

      # export KBUILD_OUTPUT=stm32mp15_basic
      # make stm32mp15_basic_defconfig
      # make DEVICE_TREE=stm32mp157c-ed1 all

  e) basic boot on dk1::

     # export KBUILD_OUTPUT=stm32mp15_basic
     # make stm32mp15_basic_defconfig
     # make DEVICE_TREE=stm32mp157a-dk1 all

  f) basic boot on avenger96::

     # export KBUILD_OUTPUT=stm32mp15_basic
     # make stm32mp15_basic_defconfig
     # make DEVICE_TREE=stm32mp15xx-dhcor-avenger96 all

6. Output files

   BootRom and TF-A expect binaries with STM32 image header
   SPL expects file with U-Boot uImage header

   So in the output directory (selected by KBUILD_OUTPUT),
   you can found the needed files:

  - For **Trusted** boot (with or without OP-TEE)

     - FSBL = **tf-a.stm32** (provided by TF-A compilation)
     - SSBL = **u-boot.stm32**

  - For Basic boot

     - FSBL = spl/u-boot-spl.stm32
     - SSBL = u-boot.img

Switch Setting for Boot Mode
----------------------------

You can select the boot mode, on the board with one switch, to select
the boot pin values = BOOT0, BOOT1, BOOT2

  +-------------+---------+---------+---------+
  |*Boot Mode*  | *BOOT2* | *BOOT1* | *BOOT0* |
  +=============+=========+=========+=========+
  | Recovery    |  0      |  0      |  0      |
  +-------------+---------+---------+---------+
  | NOR         |  0      |  0      |  1      |
  +-------------+---------+---------+---------+
  | eMMC        |  0      |  1      |  0      |
  +-------------+---------+---------+---------+
  | NAND        |  0      |  1      |  1      |
  +-------------+---------+---------+---------+
  | Reserved    |  1      |  0      |  0      |
  +-------------+---------+---------+---------+
  | SD-Card     |  1      |  0      |  1      |
  +-------------+---------+---------+---------+
  | Recovery    |  1      |  1      |  0      |
  +-------------+---------+---------+---------+
  | SPI-NAND    |  1      |  1      |  1      |
  +-------------+---------+---------+---------+

- on the **daugther board ed1 = MB1263** with the switch SW1
- on **Avenger96** with switch S3 (NOR and SPI-NAND are not applicable)
- on board **DK1/DK2** with the switch SW1 = BOOT0, BOOT2
  with only 2 pins available (BOOT1 is forced to 0 and NOR not supported),
  the possible value becomes:

    +-------------+---------+---------+
    |*Boot Mode*  | *BOOT2* | *BOOT0* |
    +=============+=========+=========+
    | Recovery    |  0      |  0      |
    +-------------+---------+---------+
    | NOR     (NA)|  0      |  1      |
    +-------------+---------+---------+
    | Reserved    |  1      |  0      |
    +-------------+---------+---------+
    | SD-Card     |  1      |  1      |
    +-------------+---------+---------+

Recovery is a boot from serial link (UART/USB) and it is used with
STM32CubeProgrammer tool to load executable in RAM and to update the flash
devices available on the board (NOR/NAND/eMMC/SD card).

The communication between HOST and board is based on

  - for UARTs : the uart protocol used with all MCU STM32
  - for USB : based on USB DFU 1.1 (without the ST extensions used on MCU STM32)

Prepare an SD card
------------------

The minimal requirements for STMP32MP15x boot up to U-Boot are:

- GPT partitioning (with gdisk or with sgdisk)
- 2 fsbl partitions, named fsbl1 and fsbl2, size at least 256KiB
- one ssbl partition for U-Boot

Then the minimal GPT partition is:

  +-------+--------+---------+-------------+
  | *Num* | *Name* | *Size*  | *Content*   |
  +=======+========+=========+=============+
  | 1     | fsbl1  | 256 KiB | TF-A or SPL |
  +-------+--------+---------+-------------+
  | 2     | fsbl2  | 256 KiB | TF-A or SPL |
  +-------+--------+---------+-------------+
  | 3     | ssbl   | enought | U-Boot      |
  +-------+--------+---------+-------------+
  | 4     | <any>  | <any>   | Rootfs      |
  +-------+--------+---------+-------------+

Add a 4th partition (Rootfs) marked bootable with a file extlinux.conf
following the Generic Distribution feature (doc/README.distro for use).

According the used card reader select the correct block device
(for example /dev/sdx or /dev/mmcblk0).

In the next example, it is /dev/mmcblk0

For example: with gpt table with 128 entries

a) remove previous formatting::

     # sgdisk -o /dev/<SD card dev>

b) create minimal image::

    # sgdisk --resize-table=128 -a 1 \
    -n 1:34:545		-c 1:fsbl1 \
    -n 2:546:1057		-c 2:fsbl2 \
    -n 3:1058:5153		-c 3:ssbl \
    -n 4:5154:		    -c 4:rootfs \
    -p /dev/<SD card dev>

  With other partition for kernel one partition rootfs for kernel.

c) copy the FSBL (2 times) and SSBL file on the correct partition.
   in this example in partition 1 to 3

   for basic boot mode : <SD card dev> = /dev/mmcblk0::

    # dd if=u-boot-spl.stm32 of=/dev/mmcblk0p1
    # dd if=u-boot-spl.stm32 of=/dev/mmcblk0p2
    # dd if=u-boot.img of=/dev/mmcblk0p3

   for trusted boot mode: ::

    # dd if=tf-a.stm32 of=/dev/mmcblk0p1
    # dd if=tf-a.stm32 of=/dev/mmcblk0p2
    # dd if=u-boot.stm32 of=/dev/mmcblk0p3

To boot from SD card, select BootPinMode = 1 0 1 and reset.

Prepare eMMC
------------

You can use U-Boot to copy binary in eMMC.

In the next example, you need to boot from SD card and the images
(u-boot-spl.stm32, u-boot.img) are presents on SD card (mmc 0)
in ext4 partition 4 (bootfs).

To boot from SD card, select BootPinMode = 1 0 1 and reset.

Then you update the eMMC with the next U-Boot command :

a) prepare GPT on eMMC,
   example with 2 partitions, bootfs and roots::

    # setenv emmc_part "name=ssbl,size=2MiB;name=bootfs,type=linux,bootable,size=64MiB;name=rootfs,type=linux,size=512"
    # gpt write mmc 1 ${emmc_part}

b) copy SPL on eMMC on firts boot partition
   (SPL max size is 256kB, with LBA 512, 0x200)::

    # ext4load mmc 0:4 0xC0000000 u-boot-spl.stm32
    # mmc dev 1
    # mmc partconf 1 1 1 1
    # mmc write ${fileaddr} 0 200
    # mmc partconf 1 1 1 0

c) copy U-Boot in first GPT partition of eMMC::

    # ext4load mmc 0:4 0xC0000000 u-boo	t.img
    # mmc dev 1
    # part start mmc 1 1 partstart
    # mmc write ${fileaddr} ${partstart} ${filesize}

To boot from eMMC, select BootPinMode = 0 1 0 and reset.

MAC Address
-----------

Please read doc/README.enetaddr for the implementation guidelines for mac id
usage. Basically, environment has precedence over board specific storage.

For STMicroelectonics board, it is retrieved in STM32MP15x OTP :

 - OTP_57[31:0] = MAC_ADDR[31:0]
 - OTP_58[15:0] = MAC_ADDR[47:32]

To program a MAC address on virgin OTP words above, you can use the fuse command
on bank 0 to access to internal OTP and lock them:

Prerequisite: check if a MAC address isn't yet programmed in OTP

1) check OTP: their value must be equal to 0::

    STM32MP> fuse sense 0 57 2
    Sensing bank 0:
    Word 0x00000039: 00000000 00000000

2) check environment variable::

    STM32MP> env print ethaddr
    ## Error: "ethaddr" not defined

3) check lock status of fuse 57 & 58 (at 0x39, 0=unlocked, 1=locked)::

    STM32MP> fuse sense 0 0x10000039 2
    Sensing bank 0:
       Word 0x10000039: 00000000 00000000

Example to set mac address "12:34:56:78:9a:bc"

1) Write OTP::

    STM32MP> fuse prog -y 0 57 0x78563412 0x0000bc9a

2) Read OTP::

    STM32MP> fuse sense 0 57 2
    Sensing bank 0:
    Word 0x00000039: 78563412 0000bc9a

3) Lock OTP::

    STM32MP> fuse prog 0 0x10000039 1 1

    STM32MP> fuse sense 0 0x10000039 2
    Sensing bank 0:
       Word 0x10000039: 00000001 00000001

4) next REBOOT, in the trace::

    ### Setting environment from OTP MAC address = "12:34:56:78:9a:bc"

5) check env update::

    STM32MP> env print ethaddr
    ethaddr=12:34:56:78:9a:bc

.. warning:: This command can't be executed twice on the same board as
             OTP are protected. It is already done for the board
             provided by STMicroelectronics.

Coprocessor firmware
--------------------

U-Boot can boot the coprocessor before the kernel (coprocessor early boot).

a) Manuallly by using rproc commands (update the bootcmd)

   Configurations::

	# env set name_copro "rproc-m4-fw.elf"
	# env set dev_copro 0
	# env set loadaddr_copro 0xC1000000

   Load binary from bootfs partition (number 4) on SD card (mmc 0)::

	# ext4load mmc 0:4 ${loadaddr_copro} ${name_copro}

   => ${filesize} variable is updated with the size of the loaded file.

   Start M4 firmware with remote proc command::

	# rproc init
	# rproc load ${dev_copro} ${loadaddr_copro} ${filesize}
	# rproc start ${dev_copro}"00270033

b) Automatically by using FIT feature and generic DISTRO bootcmd

   see examples in the board stm32mp1 directory: fit_copro_kernel_dtb.its

   Generate FIT including kernel + device tree + M4 firmware with cfg with M4 boot::

   $> mkimage -f fit_copro_kernel_dtb.its fit_copro_kernel_dtb.itb

   Then using DISTRO configuration file: see extlinux.conf to select the correct
   configuration:

   - stm32mp157c-ev1-m4
   - stm32mp157c-dk2-m4

DFU support
-----------

The DFU is supported on ST board.

The env variable dfu_alt_info is automatically build, and all
the memory present on the ST boards are exported.

The dfu mode is started by the command::

  STM32MP> dfu 0

On EV1 board, booting from SD card, without OP-TEE::

  STM32MP> dfu 0 list
  DFU alt settings list:
  dev: RAM alt: 0 name: uImage layout: RAM_ADDR
  dev: RAM alt: 1 name: devicetree.dtb layout: RAM_ADDR
  dev: RAM alt: 2 name: uramdisk.image.gz layout: RAM_ADDR
  dev: eMMC alt: 3 name: mmc0_fsbl1 layout: RAW_ADDR
  dev: eMMC alt: 4 name: mmc0_fsbl2 layout: RAW_ADDR
  dev: eMMC alt: 5 name: mmc0_ssbl layout: RAW_ADDR
  dev: eMMC alt: 6 name: mmc0_bootfs layout: RAW_ADDR
  dev: eMMC alt: 7 name: mmc0_vendorfs layout: RAW_ADDR
  dev: eMMC alt: 8 name: mmc0_rootfs layout: RAW_ADDR
  dev: eMMC alt: 9 name: mmc0_userfs layout: RAW_ADDR
  dev: eMMC alt: 10 name: mmc1_boot1 layout: RAW_ADDR
  dev: eMMC alt: 11 name: mmc1_boot2 layout: RAW_ADDR
  dev: eMMC alt: 12 name: mmc1_ssbl layout: RAW_ADDR
  dev: eMMC alt: 13 name: mmc1_bootfs layout: RAW_ADDR
  dev: eMMC alt: 14 name: mmc1_vendorfs layout: RAW_ADDR
  dev: eMMC alt: 15 name: mmc1_rootfs layout: RAW_ADDR
  dev: eMMC alt: 16 name: mmc1_userfs layout: RAW_ADDR
  dev: MTD alt: 17 name: nor0 layout: RAW_ADDR
  dev: MTD alt: 18 name: nand0 layout: RAW_ADDR
  dev: VIRT alt: 19 name: OTP layout: RAW_ADDR
  dev: VIRT alt: 20 name: PMIC layout: RAW_ADDR

All the supported device are exported for dfu-util tool::

  $> dfu-util -l
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=20, name="PMIC", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=19, name="OTP", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=18, name="nand0", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=17, name="nor0", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=16, name="mmc1_userfs", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=15, name="mmc1_rootfs", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=14, name="mmc1_vendorfs", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=13, name="mmc1_bootfs", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=12, name="mmc1_ssbl", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=11, name="mmc1_boot2", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=10, name="mmc1_boot1", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=9, name="mmc0_userfs", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=8, name="mmc0_rootfs", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=7, name="mmc0_vendorfs", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=6, name="mmc0_bootfs", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=5, name="mmc0_ssbl", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=4, name="mmc0_fsbl2", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=3, name="mmc0_fsbl1", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=2, name="uramdisk.image.gz", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=1, name="devicetree.dtb", serial="002700333338511934383330"
  Found DFU: [0483:df11] ver=9999, devnum=99, cfg=1, intf=0, alt=0, name="uImage", serial="002700333338511934383330"

You can update the boot device:

- SD card (mmc0) ::

  $> dfu-util -d 0483:5720 -a 3 -D tf-a-stm32mp157c-ev1-trusted.stm32
  $> dfu-util -d 0483:5720 -a 4 -D tf-a-stm32mp157c-ev1-trusted.stm32
  $> dfu-util -d 0483:5720 -a 5 -D u-boot-stm32mp157c-ev1-trusted.img
  $> dfu-util -d 0483:5720 -a 6 -D st-image-bootfs-openstlinux-weston-stm32mp1.ext4
  $> dfu-util -d 0483:5720 -a 7 -D st-image-vendorfs-openstlinux-weston-stm32mp1.ext4
  $> dfu-util -d 0483:5720 -a 8 -D st-image-weston-openstlinux-weston-stm32mp1.ext4
  $> dfu-util -d 0483:5720 -a 9 -D st-image-userfs-openstlinux-weston-stm32mp1.ext4

- EMMC (mmc1)::

  $> dfu-util -d 0483:5720 -a 10 -D tf-a-stm32mp157c-ev1-trusted.stm32
  $> dfu-util -d 0483:5720 -a 11 -D tf-a-stm32mp157c-ev1-trusted.stm32
  $> dfu-util -d 0483:5720 -a 12 -D u-boot-stm32mp157c-ev1-trusted.img
  $> dfu-util -d 0483:5720 -a 13 -D st-image-bootfs-openstlinux-weston-stm32mp1.ext4
  $> dfu-util -d 0483:5720 -a 14 -D st-image-vendorfs-openstlinux-weston-stm32mp1.ext4
  $> dfu-util -d 0483:5720 -a 15 -D st-image-weston-openstlinux-weston-stm32mp1.ext4
  $> dfu-util -d 0483:5720 -a 16 -D st-image-userfs-openstlinux-weston-stm32mp1.ext4

- you can also dump the OTP and the PMIC NVM with::

  $> dfu-util -d 0483:5720 -a 19 -U otp.bin
  $> dfu-util -d 0483:5720 -a 20 -U pmic.bin


When the board is booting for nor0 or nand0,
only the MTD partition on the boot devices are available, for example:

- NOR (nor0 = alt 20) & NAND (nand0 = alt 26) ::

  $> dfu-util -d 0483:5720 -a 21 -D tf-a-stm32mp157c-ev1-trusted.stm32
  $> dfu-util -d 0483:5720 -a 22 -D tf-a-stm32mp157c-ev1-trusted.stm32
  $> dfu-util -d 0483:5720 -a 23 -D u-boot-stm32mp157c-ev1-trusted.img
  $> dfu-util -d 0483:5720 -a 27 -D st-image-weston-openstlinux-weston-stm32mp1_nand_4_256_multivolume.ubi

- NAND (nand0 = alt 21)::

  $> dfu-util -d 0483:5720 -a 22 -D tf-a-stm32mp157c-ev1-trusted.stm32
  $> dfu-util -d 0483:5720 -a 23 -D u-boot-stm32mp157c-ev1-trusted.img
  $> dfu-util -d 0483:5720 -a 24 -D u-boot-stm32mp157c-ev1-trusted.img
  $> dfu-util -d 0483:5720 -a 25 -D st-image-weston-openstlinux-weston-stm32mp1_nand_4_256_multivolume.ubi
