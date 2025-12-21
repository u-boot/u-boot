.. SPDX-License-Identifier: GPL-2.0-or-later
.. |arrowin| unicode:: U+2190
.. |arrowout| unicode:: U+2192
.. |degreecelsius| unicode:: U+2103
.. _U74-MC Core Complex: https://www.starfivetech.com/uploads/u74mc_core_complex_manual_21G1.pdf
.. _JH-7110 Technical Reference Manual: https://doc-en.rvspace.org/JH7110/TRM/
.. _JH-7110 Boot User Guide BootROM: https://doc-en.rvspace.org/VisionFive2/Boot_UG/JH7110_SDK/bootrom.html
.. _JH-7110 Datasheet: https://doc-en.rvspace.org/JH7110/PDF/JH7110_DS.pdf
.. _JH-7110I Datasheet: https://doc-en.rvspace.org/JH7110/PDF/JH7110I_DS.pdf
.. _Description of StarFive loader: https://lore.kernel.org/u-boot/ZQ2PR01MB1307E9F46803F18B2B9D5394E6C22@ZQ2PR01MB1307.CHNPR01.prod.partner.outlook.cn/
.. _SYS SYSCON: https://doc-en.rvspace.org/JH7110/TRM/JH7110_TRM/sys_syscon.html

StarFive JH-7110 RISC-V SoC
---------------------------

* JH-7110 working frequency 1.5GHz
  ambient operating temperature range -20 |degreecelsius| to +85 |degreecelsius|
  (`JH-7110 Datasheet`_)

* JH-7110I working frequency 1.5GHz
  ambient operating temperature range -40 |degreecelsius| to +85 |degreecelsius|
  (`JH-7110I Datasheet`_)

* JH-7110S working frequency 1.25GHz

JH-7110 is a 4+1 core RISC-V System on Chip:

* `U74-MC Core Complex`_

  * S7 monitor core RV64IMAC

    * Physical Memory Protection
    * 16KB L1 Instruction cache
    * 8KB Data Tightly-Integrated Memory

  * U74 application core RV64GC (x4)

    * Sv39 Memory Management Unit
    * Floating Point Unit
    * 32KB L1 Instruction cache
    * 32KB L1 Data cache
    * Physical Memory Protection

  * Core-Local INTerruptor
  * Platform-Level Interrupt Controller
  * Debug
  * Bus Matrix

    * 2MB L2 cache

      * Memory Port |arrowout| 128-bit AXI4

    * System Port |arrowout| 64-bit AXI4
    * Peripheral Port |arrowout| 64-bit AXI4
    * Front Port |arrowin| 64-bit AXI4

* Block

  * RV32IMAFCB E24 co-processor

    * 16KB Instruction cache
    * 32KB Tightly-Integrated Memory "A"
    * 32KB Tightly-Integrated Memory "B"

  * Mailbox
  * SGDMA

* Network-on-Chip/AXI bus
* Memory

  * SRAM 256KB/BootROM 32KB
  * LPDDR4/DDR4/LPDDR3/DDR3 32-bit 2800 Mbps (2133 Mbps supported working speed)
  * QSPI Flash Controller

* Interfaces

  * PCIe2.0 1-lane x2
  * Ethernet MAC 10/100/1000 Mbps x2
  * USB 2.0 Host/Device
  * SDIO3.0 x2
  * CAN2.0B x2

* Audio

  * Cadence Tensilica HiFi-4 Audio DSP defined by U74MC or E24

    * 4x 32x32-bit MACs
    * some 72-bit accumulators
    * limited support for 8x 32x16-bit MACs
    * fourth VLIW slot
    * two 64-bit loads per cycle
    * optional floating point unit for four single-precision MACs per cycle

  * I2S/PCM-TDM
  * I2S/PCM
  * PDM x4
  * SPDIF

* Graphics

  * Epicsemi ISP
  * IMG BXE-4-32 Integrated GPU with 3D Acceleration
  * 12-bit DVP
  * MIPI-CSI

* Cryptographic function

  * TRNG
  * OTP
  * Security HW Engine AES/DES/3DES/HASH/PKA

* Block

  * PAD_SHARE used for reset
  * Power Management Unit Clock Reset Generator
  * PLL x3
  * JTAG
  * RTC
  * Temperature Sensor

Supported SoC drivers:

* ns16550 UART
* StarFive JH-7110 clock
* StarFive JH-7110 reset
* Cadence QSPI controller
* DesignWare MMC for eMMC/SD support
* PLDA PCIe controller
* Cadence USB2.0/3.0 controller

Supported common peripherals:

* AXP15060 Power Management Unit
* LPDDR4 2GB / 4GB / 8GB DRAM memory
* AT24C04F 4K bits (512 x 8) EEPROM
* QSPI NOR Flash 16M or SoC ROM UART loader for boot (selectable by GPIO)

Extra supported peripherals present on some boards:

* Motorcomm YT8531C Gigabit Ethernet PHY
* On-board VL805 PCIE-USB controller driver
* Status LED RGPIO3

Build U-Boot
------------

1.  Add a RISC-V toolchain to your PATH.
2.  Set cross compilation environment variable if needed:

    .. code-block:: none

       export CROSS_COMPILE=<riscv64 toolchain prefix>

3.  U-Boot for JH-7110 requires OpenSBI v1.5+ generic platform object
    fw_dynamic.bin to be included in the Flattened Image Tree blob. OpenSBI may
    first be built as below:

    .. code-block:: console

       # clone and/or update OpenSBI sources
       git clone https://github.com/riscv/opensbi.git opensbi.git
       git -C opensbi.git checkout v1.7
       # always clean build directory when building OpenSBI due to incomplete
       # dependency tracking
       make -C opensbi.git -O opensbi clean
       make -C opensbi.git -O opensbi PLATFORM=generic

4.  Now build the First Stage BootLoader (U-Boot Secondary Program Loader) and
    Second Boot Loader (OpenSBI + U-Boot Main):

    .. code-block:: console

       git clone https://source.denx.de/u-boot/u-boot.git u-boot.git
       make -C u-boot.git -O u-boot starfive_visionfive2_defconfig
       export OPENSBI=opensbi/build/platform/generic/firmware/fw_dynamic.bin
       make -C u-boot.git -O u-boot

    This will generate the U-Boot SPL image object post-processed with StarFive
    SPL headers (u-boot/spl/u-boot-spl.bin.normal.out) as well as the FIT image
    (u-boot/u-boot.itb) of OpenSBI and U-Boot Main.

    Note: Debug UART is not available from U-Boot SPL when U-Boot Main uses the
    SBI interface for this. Add the following configuration changes above to
    enable early debug UART in U-Boot SPL::

        u-boot.git/scripts/config --file u-boot/.config \
         --set-val DEBUG_UART_BASE 0x10000000 \
         --set-val DEBUG_UART_CLOCK 24000000 \
         --enable DEBUG_UART_NS16550 \
         --disable DEBUG_SBI_CONSOLE \
         --set-val SPL_DEBUG_UART_BASE 0x10000000 \
         --set-val DEBUG_UART_SHIFT 2

        make -C u-boot.git -O u-boot olddefconfig

Boot description
----------------

JH-7110 reset vectors (one 36-bit address per each of four U7 cores) are located
split into four pairs of `SYS SYSCON`_ registers. The default value for all four
reset vectors is 2a000000.

Execute-in-place BootROM code located at 2a000000 is not published by StarFive
however may be generally described as deciding based on [RGPIO2:RGPIO0] state
where to transfer SPL data from, verify headers and CRC, and then jump to code
execution in L2 LIM.

Zero Stage Program Loader
^^^^^^^^^^^^^^^^^^^^^^^^^

====== =========== =============
RGPIO2 Boot Vector ZSPL function
====== =========== =============
0      0x2A00_0000 On-chip 32KB BootROM
1      0x2100_0000 QSPI XIP Flash (256mb)
====== =========== =============

JH-7110 ZSPL functionally consists of the selection of reset vector register
defaults at chip-design time in concert with BootROM code at 2a000000. `JH-7110
Technical Reference Manual`_ says "if XIP flash is disabled in OTP
configuration, system cannot boot from XIP flash". Presumably there is some OTP
configuration involved but there is no documentation available with which to
expand on that topic.

Zero Stage BootLoader
^^^^^^^^^^^^^^^^^^^^^

JH-7110 ZSBL is typically StarFive loader code in BootROM selected by RGPIO2
pull-down.

====== ====== ======================================
RGPIO1 RGPIO0 StarFive loader function @ 0x2A00_0000
====== ====== ======================================
0      0      1-bit QSPI Flash offset 0x0
0      1      SDIO3.0 *(deprecated)*
1      0      eMMC5.0 or eMMC5.1 *(deprecated)*
1      1      UART Serial XMODEM loader
====== ====== ======================================

According to `JH-7110 Boot User Guide BootROM`_ the StarFive loader code reads
content to SRAM @ 0x0800_0000 from different media selected by [RGPIO1,RGPIO0].

`Description of StarFive loader`_ by the StarFive VisionFive2 board maintainer:

    The SD card boot mode is supported but the mmc driver of BootROM is not
    compatible with a few SD cards. If you can't boot from a SD card, you can
    change another card for a try.

    The eMMC boot mode loads SPL from sector 0, while the SD card boot mode
    loads GPT header from sector 1 and then finds the partition whose GUID is
    2E54B353-1271-4842-806F-E436D6AF6985 to load SPL. So if we try to use GPT
    partition in eMMC, it will fail to boot and report CRC (stored at 0x290)
    failure. The workaround is using the backup load address. After the CRC
    failure happens, it will try to load the SPL from the backup address
    (stored at 0x4~0x7). That is why we write 0x00100000 to 0x4~0x7. But this
    workaround is not a standard process and may destroy the partition
    information stored in sector 0.

There are additional unexplained GUID references in the JH-7110 MaskROM so the
description given is not complete. Attribution and modifications to the GPL2.0+
source code used in StarFive loader have not been published as of this writing.
Due to the lack of verifiable documentation the upstream Linux devicetree does
not contain hints for mmc0 and mmc1 interfaces to be included in U-Boot SPL. As
of U-Boot release v2025.10 and newer the U-Boot specific devicetree override of
hints for mmc0 and mmc1 interfaces that are required for the deprecated modes
have been dropped marking the deprecation of these boot modes in U-Boot.

Supported modes in U-Boot SPL are QSPI Flash and UART Serial XMODEM loader as
accepted by upstream Linux Kernel for StarFive JH-7110 common devicetree.

First Stage BootLoader
^^^^^^^^^^^^^^^^^^^^^^

JH-7110 FSBL is typically U-Boot SPL or any vendor flash programming tool.

U-Boot SPL initializes DRAM and configures PLLs needed by CPU and peripherals.

====== ====== ===================
RGPIO1 RGPIO0 U-Boot SPL function
====== ====== ===================
0      0      BOOT_DEVICE_SPI @ offset 0x100000 (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
0      1      BOOT_DEVICE_MMC2 @ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
1      0      BOOT_DEVICE_MMC1 @ CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
1      1      BOOT_DEVICE_UART @ YMODEM
====== ====== ===================

U-Boot SPL function is selected by configuration of [RGPIO1:RGPIO0] then copies
data to the start of DRAM and executes.

=========== ===========
Address     Description
=========== ===========
0x040000000 start of DRAM
0x240000000 uncached alias of DRAM
=========== ===========

Note: The largest DRAM size with JH7110 is 8GB because the uncached alias of
DRAM begins at +8GB following the start of DRAM.

Second Stage BootLoader (OpenSBI fw_dynamic.bin + U-Boot Main)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

U-Boot Main is supported in S-mode and depends on prior stage M-mode SBI runtime
services provided by OpenSBI FW_DYNAMIC firmware.

Loading U-Boot
--------------

Vendored versions of U-Boot (as pre-installed on supported boards) are generally
capable in U-Boot console of UART data transfer and updating QSPI Flash.
Additionally there may be a vendor Board Support Package using the deprecated
SDIO3.0 / eMMC5.0 boot modes. It is not documented here how to update U-Boot in
vendor BSP data images, nor use of the deprecated boot modes. The recommended
upgrade path is to update QSPI Flash in U-Boot console or from GNU/Linux OS.

============ ============================ ===========
Flash offset Length                       Data source
============ ============================ ===========
0x000000     0x0f0000 (CONFIG_ENV_OFFSET) u-boot/spl/u-boot-spl.bin.normal.out
0x0f0000     0x010000 (CONFIG_ENV_SIZE)   runtime generated defaults if bad CRC
0x100000     0xf00000                     u-boot/u-boot.itb
============ ============================ ===========

Recovery U-Boot console
^^^^^^^^^^^^^^^^^^^^^^^

With UART serial USB adapter and tio serial terminal::

    tio /dev/ttyUSB0 -o 1
        tio 3.9
        Press ctrl-t q to quit
        Connected to /dev/ttyUSB0

    # Power on the board with [RGPIO1:RGPI0]=3
        (C)StarFive
        CC
        (C)StarFive
        CCC

    (Control-t-x)
        Please enter which X modem protocol to use:
         (0) XMODEM-1K send
         (1) XMODEM-CRC send
         (2) XMODEM-CRC receive

    0
        Send file with XMODEM-1K
        Enter file name:

    u-boot/spl/u-boot-spl.bin.normal.out
        Sending file 'u-boot/spl/u-boot-spl.bin.normal.out'
        Press any key to abort transfer
        ...|
        Done

    # U-Boot SPL on the board with [RGPIO1:RGPI0]=3
        U-Boot SPL 2025.10 (Oct 23 2025 - 17:01:49 -0700)
        DDR version: dc2e84f0.
        Trying to boot from UART
        CCC

    (Control-t-y)
        Send file with YMODEM
        Enter file name:

    u-boot/u-boot.itb
        Sending file 'u-boot/u-boot.itb'
        Press any key to abort transfer
        ...|
        Done
        Loaded 3122637 bytes

        OpenSBI v1.7
           ____                    _____ ____ _____
          / __ \                  / ____|  _ \_   _|
         | |  | |_ __   ___ _ __ | (___ | |_) || |
         | |  | | '_ \ / _ \ '_ \ \___ \|  _ < | |
         | |__| | |_) |  __/ | | |____) | |_) || |_
          \____/| .__/ \___|_| |_|_____/|____/_____|
                | |
                |_|

        Platform Name               : Pine64 Star64
        Platform Features           : medeleg
        Platform HART Count         : 4
        Platform IPI Device         : aclint-mswi
        Platform Timer Device       : aclint-mtimer @ 4000000Hz
        Platform Console Device     : uart8250
        Platform HSM Device         : ---
        Platform PMU Device         : ---
        Platform Reboot Device      : pm-reset
        Platform Shutdown Device    : pm-reset
        Platform Suspend Device     : ---
        Platform CPPC Device        : ---
        Firmware Base               : 0x40000000
        Firmware Size               : 353 KB
        Firmware RW Offset          : 0x40000
        Firmware RW Size            : 97 KB
        Firmware Heap Offset        : 0x4c000
        Firmware Heap Size          : 49 KB (total), 3 KB (reserved), 12 KB (used), 33 KB (free)
        Firmware Scratch Size       : 4096 B (total), 400 B (used), 3696 B (free)
        Runtime SBI Version         : 3.0
        Standard SBI Extensions     : time,rfnc,ipi,base,hsm,srst,pmu,dbcn,fwft,legacy,dbtr,sse
        Experimental SBI Extensions : none

        Domain0 Name                : root
        Domain0 Boot HART           : 4
        Domain0 HARTs               : 1*,2*,3*,4*
        Domain0 Region00            : 0x0000000010000000-0x0000000010000fff M: (I,R,W) S/U: (R,W)
        Domain0 Region01            : 0x0000000002000000-0x000000000200ffff M: (I,R,W) S/U: ()
        Domain0 Region02            : 0x0000000040040000-0x000000004005ffff M: (R,W) S/U: ()
        Domain0 Region03            : 0x0000000040000000-0x000000004003ffff M: (R,X) S/U: ()
        Domain0 Region04            : 0x000000000c000000-0x000000000fffffff M: (I,R,W) S/U: (R,W)
        Domain0 Region05            : 0x0000000000000000-0xffffffffffffffff M: () S/U: (R,W,X)
        Domain0 Next Address        : 0x0000000040200000
        Domain0 Next Arg1           : 0x0000000042200000
        Domain0 Next Mode           : S-mode
        Domain0 SysReset            : yes
        Domain0 SysSuspend          : yes

        Boot HART ID                : 4
        Boot HART Domain            : root
        Boot HART Priv Version      : v1.11
        Boot HART Base ISA          : rv64imafdcbx
        Boot HART ISA Extensions    : zihpm,sdtrig
        Boot HART PMP Count         : 8
        Boot HART PMP Granularity   : 12 bits
        Boot HART PMP Address Bits  : 34
        Boot HART MHPM Info         : 2 (0x00000018)
        Boot HART Debug Triggers    : 8 triggers
        Boot HART MIDELEG           : 0x0000000000000222
        Boot HART MEDELEG           : 0x000000000000b109


        U-Boot 2025.10 (Oct 23 2025 - 17:01:49 -0700)

        CPU:   sifive,u74-mc
        Model: Pine64 Star64
        DRAM:  4 GiB
        Core:  160 devices, 29 uclasses, devicetree: board
        WDT:   Not starting watchdog@13070000
        MMC:   mmc@16010000: 0, mmc@16020000: 1
        Loading Environment from SPIFlash...
        SF: Detected gd25lq128 with page size 256 Bytes, erase size 4 KiB, total 16 MiB
        *** Warning - bad CRC, using default environment

        StarFive EEPROM format v2

        --------EEPROM INFO--------
        Vendor : PINE64
        Product full SN: STAR64V1-2310-D004E000-0000xxxx
        data version: 0x2
        PCB revision: 0xc1
        BOM revision: A
        Ethernet MAC0 address: 6c:cf:39:00:xx:xx
        Ethernet MAC1 address: 6c:cf:39:00:xx:xx
        --------EEPROM INFO--------

        starfive_7110_pcie pcie@9c0000000: Starfive PCIe bus probed.
        In:    serial@10000000
        Out:   serial@10000000
        Err:   serial@10000000
        Net:   eth0: ethernet@16030000, eth1: ethernet@16040000
        starting USB...
        Register 2000820 NbrPorts 2
        Starting the controller
        USB XHCI 1.00
        Bus usb@0: 4 USB Device(s) found
               scanning usb for storage devices... 0 Storage Device(s) found
        Working FDT set to ff717fe0
        Hit any key to stop autoboot: 2 1 0

    (enter)
        StarFive #

Update QSPI Flash using U-Boot console
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

With UART serial USB adapter and tio serial terminal::

    tio /dev/ttyUSB0 -o 1
        tio 3.9
        Press ctrl-t q to quit
        Connected to /dev/ttyUSB0

    (enter)
        StarFive #

    sf probe
        StarFive # sf probe
        SF: Detected gd25lq128 with page size 256 Bytes, erase size 4 KiB, total 16 MiB

    loady && sf update $loadaddr 0 $filesize
        StarFive # loady && sf update $loadaddr 0 $filesize
        ## Ready for binary (ymodem) download to 0x82000000 at 115200 bps...
        CCC

    (Control-t-y)
        Send file with YMODEM
        Enter file name:

    u-boot/spl/u-boot-spl.bin.normal.out
        Sending file 'u-boot/spl/u-boot-spl.bin.normal.out'
        Press any key to abort transfer
        ...|
        Done
        ## Total Size      = 0x00024eb7 = 151223 Bytes
        ## Start Addr      = 0x82000000
        device 0 offset 0x0, size 0x24eb7
        151223 bytes written, 0 bytes skipped in 0.634s, speed 243096 B/s
        StarFive #

    env erase
        StarFive # env erase
        Erasing Environment on SPIFlash... OK
        StarFive #

    loady && sf update $loadaddr 100000 $filesize
        StarFive # loady $loadaddr && sf update $loadaddr 100000 $filesize
        ## Ready for binary (ymodem) download to 0x82000000 at 115200 bps...
        CCC

    (Control-t-y)
        Send file with YMODEM
        Enter file name:

    u-boot/u-boot.itb
        Sending file 'u-boot/u-boot.itb'
        Press any key to abort transfer
        ...|
        Done
        ## Total Size      = 0x002fa5cd = 3122637 Bytes
        ## Start Addr      = 0x82000000
        device 0 offset 0x100000, size 0x2fa5cd
        3122637 bytes written, 0 bytes skipped in 18.137s, speed 176272 B/s
        StarFive #

Update QSPI Flash from GNU/Linux OS
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

With mtd-utils::

    cat /proc/mtd
        dev:    size   erasesize  name
        mtd0: 000f0000 00001000 "spl"
        mtd1: 00010000 00001000 "uboot-env"
        mtd2: 00f00000 00001000 "uboot"

    flashcp --verbose u-boot/spl/u-boot-spl.bin.normal.out /dev/mtd0
        Erasing blocks: 37/37 (100%)
        Writing data: 147k/147k (100%)
        Verifying data: 147k/147k (100%)

    flashcp --verbose --erase-all /dev/zero /dev/mtd1
        Erasing blocks: 16/16 (100%)
        Writing data: 0k/0k (100%)
        Verifying data: 0k/0k (100%)

    flashcp --verbose u-boot/u-boot.itb /dev/mtd2
        Erasing blocks: 763/763 (100%)
        Writing data: 3049k/3049k (100%)
        Verifying data: 3049k/3049k (100%)
