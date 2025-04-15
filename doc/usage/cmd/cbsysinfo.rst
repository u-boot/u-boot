.. SPDX-License-Identifier: GPL-2.0+

cbsysinfo
=========

Synopsis
--------

::

    cbsysinfo


Description
-----------

This displays information obtained from the coreboot sysinfo table. It is only
useful when booting U-Boot from coreboot.

Example
-------

::

    => cbsysinfo
    Coreboot table at 500, size 5c4, records 1d (dec 29), decoded to 000000007dce4520, forwarded to 000000007ff9a000

    CPU KHz     : 0
    Serial I/O port: 00000000
       base        : 00000000
       pointer     : 000000007ff9a370
       type        : 1
       base        : 000003f8
       baud        : 0d115200
       regwidth    : 1
       input_hz    : 0d1843200
       PCI addr    : 00000010
    Mem ranges  : 7
              id: type               ||   base        ||   size
               0: 10:table    0000000000000000 0000000000001000
               1: 01:ram      0000000000001000 000000000009f000
               2: 02:reserved 00000000000a0000 0000000000060000
               3: 01:ram      0000000000100000 000000007fe6d000
               4: 10:table    000000007ff6d000 0000000000093000
               5: 02:reserved 00000000fec00000 0000000000001000
               6: 02:reserved 00000000ff800000 0000000000800000
    option_table: 000000007ff9a018
     Bit  Len  Cfg  ID  Name
       0  180    r   0  reserved_memory
     180    1    e   4  boot_option            0:Fallback 1:Normal
     184    4    h   0  reboot_counter
     190    8    r   0  reserved_century
     1b8    8    r   0  reserved_ibm_ps2_century
     1c0    1    e   1  power_on_after_fail    0:Disable 1:Enable
     1c4    4    e   6  debug_level            5:Notice 6:Info 7:Debug 8:Spew
     1d0   80    r   0  vbnv
     3f0   10    h   0  check_sum
    CMOS start  : 1c0
       CMOS end    : 1cf
       CMOS csum loc: 3f0
    VBNV start  : ffffffff
    VBNV size   : ffffffff
    CB version  : 4.21-5-g7e6eae9679e3-dirty
       Extra       :
       Build       : Thu Sep 07 14:52:41 UTC 2023
       Time        : 14:52:41
    Framebuffer : 000000007ff9a410
       Phys addr   :         fd000000
       X res       : 0d800
       X res       : 0d600
       Bytes / line: c80
       Bpp         : 0d32
       pos/size      red 16/8, green 8/8, blue 0/8, reserved 24/8
    GPIOs       : 0
              id: port     polarity val name
    MACs        : 0d10
               0: 12:00:00:00:28:00
               1: 00:00:00:fd:00:00
               2: 20:03:00:00:58:02
               3: 80:0c:00:00:20:10
               4: 08:00:08:18:08:00
               5: 16:00:00:00:10:00
               6: 00:d0:fd:7f:00:00
               7: 17:00:00:00:10:00
               8: 00:e0:fd:7f:00:00
               9: 37:00:00:00:10:00
    Multiboot tab: 0000000000000000
    CB header   : 000000007ff9a000
    CB mainboard: 000000007ff9a344
       vendor      : 0: Emulation
       part_number : 10: QEMU x86 i440fx/piix4
    vboot handoff: 0000000000000000
       size        : 0
       vdat addr   : 0000000000000000
       size        : 0
    SMBIOS      :         7ff6d000
       size        : 8000
    ROM MTRR    : 0
    Tstamp table: 000000007ffdd000
    CBmem cons  : 000000007ffde000
    Size        : 1fff8
    Cursor      : 3332
    MRC cache   : 0000000000000000
    ACPI GNVS   : 0000000000000000
    Board ID    : ffffffff
    RAM code    : ffffffff
    WiFi calib  : 0000000000000000
    Ramoops buff:                0
       size        : 0
    SF size     : 0
    SF sector   : 0
    SF erase cmd: 0
    FMAP offset :                0
    CBFS offset :              200
    CBFS size   :           3ffe00
    Boot media size:           400000
    MTC start   :                0
    MTC size    : 0
    Chrome OS VPD: 0000000000000000
    RSDP        : 000000007ff75000
    Unimpl.     : 10 37 40
    =>

Note that "Unimpl." shows tags which U-Boot does not currently implement.
