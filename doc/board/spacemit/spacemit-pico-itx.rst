.. SPDX-License-Identifier: GPL-2.0-or-later

SpacemiT K3 Pico-ITX
====================

Building
~~~~~~~~
1. Add a RISC-V toolchain to your PATH

2. Setup cross compilation environment variable:

   .. code-block:: console

      export CROSS_COMPILE=<riscv64 toolchain prefix>

3. U-Boot for SpacemiT K3 SoC requires OpenSBI in-development generic
platform object fw_dynamic.bin to be included in the Flattened Image
Tree blob. OpenSBI may be first built as below:

   .. code-block:: console

      # Clone OpenSBI fork sources with K3 specific patches
      git clone https://github.com/spacemit-com/opensbi-upstream opensbi
      cd opensbi
      make PLATFORM=generic

4. Then build U-Boot as following:

   .. code-block:: console

      cd <U-Boot-dir>
      make spacemit_k3_defconfig
      make OPENSBI=<OpenSBI-dir>/build/platform/generic/firmware/fw_dynamic.bin

This will generate u-boot.itb

Testing
~~~~~~~
Currently there is no U-Boot SPL build target. It is required to use 'FSBL.bin' vendor
board support package SPL for DDR initialization and testing u-boot.itb. Please test with
fastboot command via USB download.

First, retrieve Bianbu Linux image from archive_ and extract the FSBL.bin firmware.

Second, hold the FEL button of Pico-ITX board, then press reset button and release it .
The board will enter into firmware download mode.

   .. code-block:: none

      sys: 0x10001200
      bm:2
      usb_init : enter 3296,3072
      usb_core_init : enter
      DWC3_GRXTHRCFG:0x4400000
      ROM: usb download handler
       rst
      done H
      setup= 0x1000680 0x400000,
       rst
      done H
      setup= 0x290500 0x0,
      setup= 0x1000680 0x120000,
      setup= 0x2000680 0x90000,
      setup= 0x2000680 0x200000,
      setup= 0x3000680 0xff0000,
      setup= 0x3020680 0xff0409,
      setup= 0x3010680 0xff0409,
      setup= 0x10900 0x0,
      usb_rx_bytes : start len[4096]
      setup= 0x3020680 0xff0409,
      setup= 0x3040680 0xff0409,

Run following command to test:

   .. code-block:: console

      fastboot stage FSBL.bin
      fastboot continue
      sleep 10
      fastboot stage u-boot.itb
      fastboot continue

.. _archive: https://archive.spacemit.com/image/k3/version/bianbu/v4.0.7/
.. _uboot: https://bianbu-linux.spacemit.com/en/device/boot#21-firmware-layout

Booting
~~~~~~~
Sample boot log
~~~~~~~~~~~~~~~
   .. code-block:: none

      sys: 0x10001200
      bm:2
      usb_init : enter 3296,3072
      usb_core_init : enter
      DWC3_GRXTHRCFG:0x4400000
      ROM: usb download handler
       rst
      done H
      setup= 0x1000680 0x400000,
       rst
      done H
      setup= 0x2f0500 0x0,
      setup= 0x1000680 0x120000,
      setup= 0x2000680 0x90000,
      setup= 0x2000680 0x200000,
      setup= 0x3000680 0xff0000,
      setup= 0x3020680 0xff0409,
      setup= 0x3010680 0xff0409,
      setup= 0x10900 0x0,
      usb_rx_bytes : start len[4096]
      setup= 0x3020680 0xff0409,
      setup= 0x3040680 0xff0409,
      fastboot_handle_command: max-download-size
      usb_tx_bytes : start len[14]
      usb_rx_bytes : start len[4096]
      fastboot_handle_command: 00072b20
      Starting download of 469792 bytes
      usb_tx_bytes : start len[12]
      usb_rx_bytes : start len[469792]
      usb_tx_bytes : start len[4]
      usb_rx_bytes : start len[4096]
      fastboot_handle_command: continue
      usb_tx_bytes : start len[4]
      j...
      U-Boot SPL 2022.10 (Sep 07 2026 - 11:31:13 +0000)
      LPDDR5 total size: 16 GB, data rate: 6400 MT/s
      DDR training consume 5625ms
      ..
      OpenSBI v1.9-72-g13a401f8
         ____                    _____ ____ _____
        / __ \                  / ____|  _ \_   _|
       | |  | |_ __   ___ _ __ | (___ | |_) || |
       | |  | | '_ \ / _ \ '_ \ \___ \|  _ < | |
       | |__| | |_) |  __/ | | |____) | |_) || |_
        \____/| .__/ \___|_| |_|_____/|____/_____|
              | |
              |_|
      Platform Name               : SpacemiT K3 Pico-ITX
      Platform Features           : medeleg
      Platform HART Count         : 8
      Platform HART Protection    : pmp
      Platform IPI Device         : aclint-mswi
      Platform Timer Device       : aclint-mtimer @ 24000000Hz
      Platform Console Device     : uart8250
      Platform HSM Device         : spacemit-k3-hsm
      Platform PMU Device         : ---
      Platform Reboot Device      : spacemit-p1-reset
      Platform Shutdown Device    : spacemit-p1-reset
      Platform Suspend Device     : ---
      Platform CPPC Device        : ---
      Firmware Base               : 0x100000000
      Firmware Size               : 416 KB
      Firmware RW Offset          : 0x40000
      Firmware RW Size            : 160 KB
      Firmware Heap Offset        : 0x57000
      Firmware Heap Size          : 68 KB (total), 1 KB (reserved), 14 KB (used), 52 KB (free)
      Firmware Scratch Size       : 4096 B (total), 1472 B (used), 2624 B (free)
      Runtime SBI Version         : 3.0
      Standard SBI Extensions     : time,rfnc,ipi,base,hsm,srst,pmu,dbcn,fwft,legacy,dbtr,sse
      Experimental SBI Extensions : none
      Domain0 Name                : root
      Domain0 Init Order          : 0xffffffff
      Domain0 Boot HART           : 0
      Domain0 HARTs               : 0x0*,0x1*,0x2*,0x3*,0x4*,0x5*,0x6*,0x7*
      Domain0 Region00            : 0x0000000100000000-0x000000010003ffff M: (F,R,X) S/U: ()
      Domain0 Region01            : 0x0000000100040000-0x000000010007ffff M: (F,R,W) S/U: ()
      Domain0 Region02            : 0x00000000d4017000-0x00000000d4017fff M: (I,R,W) S/U: (R,W)
      Domain0 Region03            : 0x00000000e081c000-0x00000000e081ffff M: (I,R,W) S/U: ()
      Domain0 Region04            : 0x0000000000000000-0xffffffffffffffff M: () S/U: (R,W,X)
      Domain0 Next Address        : 0x0000000102000000
      Domain0 Next Arg1           : 0x0000000102091398
      Domain0 Next Mode           : S-mode
      Domain0 SysReset            : yes
      Domain0 SysSuspend          : yes
      Boot HART ID                : 0
      Boot HART Domain            : root
      Boot HART Priv Version      : v1.12
      Boot HART Base ISA          : rv64imafdcbvhx
      Boot HART ISA Extensions    : smaia,smstateen,sscofpmf,sstc,zicntr,zihpm,smcntrpmf,zicboz,zicbom,svpbmt,sdtrig,svade,ssstateen,v,f,d
      Boot HART PMP Count         : 16
      Boot HART PMP Granularity   : 12 bits
      Boot HART PMP Address Bits  : 38
      Boot HART MHPM Info         : 16 (0x0007fff8)
      Boot HART Debug Triggers    : 4 triggers
      Boot HART MIDELEG           : 0x0000000000003666
      Boot HART MEDELEG           : 0x0000000000f0b509

      <debug_uart>

      U-Boot 2026.10-rc4-00501-g9fba7b59df42 (Sep 22 2026 - 13:08:33 +0000)

      DRAM:  16 GiB
      Core:  433 devices, 15 uclasses, devicetree: separate
      Loading Environment from nowhere...Loading Environment from nowhere... OK
      In:    serial@d4017000
      Out:   serial@d4017000
      Err:   serial@d4017000
      Net:   No ethernet found.
      =>
