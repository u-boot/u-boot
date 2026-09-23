.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>

.. index::
   single: efi (command)

efi command
===========

Synopsis
--------

::

    efi mem
    efi tables

Description
-----------

The *efi* command provides information about the EFI environment U-Boot is
running in, when it is started from EFI.

When running as an EFI app, this command queries EFI boot services for the
information. When running as an EFI payload, EFI boot services have been
stopped, so it uses the information collected by the boot stub before that
happened.

efi mem
~~~~~~~

This shows the EFI memory map in the same format as the *efidebug memmap*
command. One line is printed per memory region with the following fields:

Type
    Memory type, named as in the UEFI specification without the leading
    'Efi' and the trailing 'Type', e.g. ConventionalMemory for
    EfiConventionalMemory.

Start, End
    Physical start and end address of the region. The virtual addresses of
    the regions are not shown: the memory map is identity mapped when this
    command can be used, i.e. before SetVirtualAddressMap() is called, so
    the virtual address field of the memory map carries no information.

Attributes
    The attributes of the region as a '|'-separated list of mnemonics,
    e.g. UC for EFI_MEMORY_UC and RT for EFI_MEMORY_RUNTIME. When the
    region carries ISA-specific attributes (EFI_MEMORY_ISA_VALID is set),
    the ISA-specific field is shown as ISA=<value>. Attribute bits without
    a mnemonic are shown as a hexadecimal value.

efi tables
~~~~~~~~~~

This shows a list of the EFI tables provided in the system table. These use
GUIDs so it is not possible in general to show the name of a table. But some
effort is made to provide a useful table, where the GUID is known by U-Boot.


Example
-------

::

    => efi mem
    EFI table at 0, memory map 000000001a977d90, size 1860, key 9e7, version 1, descr. size 0x30
    Type                    Start            End              Attributes
    ======================= ================ ================ ==========
    BootServicesCode        0000000000000000-0000000000001000 UC|WC|WT|WB
    ConventionalMemory      0000000000001000-00000000000a0000 UC|WC|WT|WB
    ConventionalMemory      0000000000100000-0000000000800000 UC|WC|WT|WB
    ACPIMemoryNVS           0000000000800000-0000000000808000 UC|WC|WT|WB
    ConventionalMemory      0000000000808000-000000000080b000 UC|WC|WT|WB
    ACPIMemoryNVS           000000000080b000-000000000080c000 UC|WC|WT|WB
    ConventionalMemory      000000000080c000-0000000000810000 UC|WC|WT|WB
    ACPIMemoryNVS           0000000000810000-0000000000900000 UC|WC|WT|WB
    BootServicesData        0000000000900000-0000000001780000 UC|WC|WT|WB
    ConventionalMemory      0000000001780000-000000000bb75000 UC|WC|WT|WB
    LoaderData              000000000bb75000-000000001bb75000 UC|WC|WT|WB
    BootServicesData        000000001bb75000-000000001bb95000 UC|WC|WT|WB
    ConventionalMemory      000000001bb95000-000000001dd24000 UC|WC|WT|WB
    LoaderCode              000000001dd24000-000000001ddfe000 UC|WC|WT|WB
    ConventionalMemory      000000001ddfe000-000000001de99000 UC|WC|WT|WB
    BootServicesData        000000001de99000-000000001dea9000 UC|WC|WT|WB
    LoaderData              000000001dea9000-000000001deac000 UC|WC|WT|WB
    BootServicesData        000000001deac000-000000001e58e000 UC|WC|WT|WB
    BootServicesCode        000000001e58e000-000000001e642000 UC|WC|WT|WB
    BootServicesData        000000001e642000-000000001e672000 UC|WC|WT|WB
    BootServicesCode        000000001e672000-000000001e753000 UC|WC|WT|WB
    BootServicesData        000000001e753000-000000001e7b8000 UC|WC|WT|WB
    BootServicesCode        000000001e7b8000-000000001e7c1000 UC|WC|WT|WB
    BootServicesData        000000001e7c1000-000000001e7c6000 UC|WC|WT|WB
    BootServicesCode        000000001e7c6000-000000001e7e6000 UC|WC|WT|WB
    BootServicesData        000000001e7e6000-000000001e7e8000 UC|WC|WT|WB
    BootServicesCode        000000001e7e8000-000000001e7f2000 UC|WC|WT|WB
    BootServicesData        000000001e7f2000-000000001e7f3000 UC|WC|WT|WB
    BootServicesCode        000000001e7f3000-000000001e7fb000 UC|WC|WT|WB
    BootServicesData        000000001e7fb000-000000001e7fc000 UC|WC|WT|WB
    BootServicesCode        000000001e7fc000-000000001e80c000 UC|WC|WT|WB
    BootServicesData        000000001e80c000-000000001e80f000 UC|WC|WT|WB
    BootServicesCode        000000001e80f000-000000001e812000 UC|WC|WT|WB
    BootServicesData        000000001e812000-000000001e819000 UC|WC|WT|WB
    BootServicesCode        000000001e819000-000000001e82b000 UC|WC|WT|WB
    BootServicesData        000000001e82b000-000000001e834000 UC|WC|WT|WB
    BootServicesCode        000000001e834000-000000001e842000 UC|WC|WT|WB
    BootServicesData        000000001e842000-000000001e851000 UC|WC|WT|WB
    BootServicesCode        000000001e851000-000000001e85c000 UC|WC|WT|WB
    BootServicesData        000000001e85c000-000000001e867000 UC|WC|WT|WB
    BootServicesCode        000000001e867000-000000001e87d000 UC|WC|WT|WB
    BootServicesData        000000001e87d000-000000001e886000 UC|WC|WT|WB
    BootServicesCode        000000001e886000-000000001e8aa000 UC|WC|WT|WB
    BootServicesData        000000001e8aa000-000000001e8ad000 UC|WC|WT|WB
    BootServicesCode        000000001e8ad000-000000001e8c1000 UC|WC|WT|WB
    BootServicesData        000000001e8c1000-000000001e8c8000 UC|WC|WT|WB
    BootServicesCode        000000001e8c8000-000000001e8e1000 UC|WC|WT|WB
    BootServicesData        000000001e8e1000-000000001e8e4000 UC|WC|WT|WB
    BootServicesCode        000000001e8e4000-000000001e8ea000 UC|WC|WT|WB
    BootServicesData        000000001e8ea000-000000001e8ec000 UC|WC|WT|WB
    BootServicesCode        000000001e8ec000-000000001e8ff000 UC|WC|WT|WB
    BootServicesData        000000001e8ff000-000000001e904000 UC|WC|WT|WB
    BootServicesCode        000000001e904000-000000001e91a000 UC|WC|WT|WB
    BootServicesData        000000001e91a000-000000001e91c000 UC|WC|WT|WB
    BootServicesCode        000000001e91c000-000000001e929000 UC|WC|WT|WB
    BootServicesData        000000001e929000-000000001e92c000 UC|WC|WT|WB
    BootServicesCode        000000001e92c000-000000001e932000 UC|WC|WT|WB
    BootServicesData        000000001e932000-000000001e934000 UC|WC|WT|WB
    BootServicesCode        000000001e934000-000000001e935000 UC|WC|WT|WB
    BootServicesData        000000001e935000-000000001e936000 UC|WC|WT|WB
    BootServicesCode        000000001e936000-000000001e93b000 UC|WC|WT|WB
    BootServicesData        000000001e93b000-000000001e940000 UC|WC|WT|WB
    BootServicesCode        000000001e940000-000000001e94c000 UC|WC|WT|WB
    BootServicesData        000000001e94c000-000000001e94e000 UC|WC|WT|WB
    BootServicesCode        000000001e94e000-000000001e96b000 UC|WC|WT|WB
    BootServicesData        000000001e96b000-000000001e96c000 UC|WC|WT|WB
    BootServicesCode        000000001e96c000-000000001e976000 UC|WC|WT|WB
    BootServicesData        000000001e976000-000000001e977000 UC|WC|WT|WB
    BootServicesCode        000000001e977000-000000001e978000 UC|WC|WT|WB
    BootServicesData        000000001e978000-000000001e97a000 UC|WC|WT|WB
    BootServicesCode        000000001e97a000-000000001e98f000 UC|WC|WT|WB
    BootServicesData        000000001e98f000-000000001e992000 UC|WC|WT|WB
    BootServicesCode        000000001e992000-000000001e994000 UC|WC|WT|WB
    BootServicesData        000000001e994000-000000001e996000 UC|WC|WT|WB
    BootServicesCode        000000001e996000-000000001e99d000 UC|WC|WT|WB
    BootServicesData        000000001e99d000-000000001e9a4000 UC|WC|WT|WB
    BootServicesCode        000000001e9a4000-000000001e9a8000 UC|WC|WT|WB
    BootServicesData        000000001e9a8000-000000001e9ae000 UC|WC|WT|WB
    BootServicesCode        000000001e9ae000-000000001e9b2000 UC|WC|WT|WB
    BootServicesData        000000001e9b2000-000000001e9b4000 UC|WC|WT|WB
    BootServicesCode        000000001e9b4000-000000001e9ef000 UC|WC|WT|WB
    BootServicesData        000000001e9ef000-000000001e9f1000 UC|WC|WT|WB
    BootServicesCode        000000001e9f1000-000000001e9f5000 UC|WC|WT|WB
    BootServicesData        000000001e9f5000-000000001e9fd000 UC|WC|WT|WB
    BootServicesCode        000000001e9fd000-000000001ea00000 UC|WC|WT|WB
    BootServicesData        000000001ea00000-000000001ec02000 UC|WC|WT|WB
    BootServicesCode        000000001ec02000-000000001ec0a000 UC|WC|WT|WB
    BootServicesData        000000001ec0a000-000000001ec0f000 UC|WC|WT|WB
    RuntimeServicesData     000000001ec0f000-000000001ecd0000 UC|WC|WT|WB|RT
    BootServicesCode        000000001ecd0000-000000001ece5000 UC|WC|WT|WB
    BootServicesData        000000001ece5000-000000001ece6000 UC|WC|WT|WB
    BootServicesCode        000000001ece6000-000000001ecea000 UC|WC|WT|WB
    BootServicesData        000000001ecea000-000000001eced000 UC|WC|WT|WB
    BootServicesCode        000000001eced000-000000001ecf6000 UC|WC|WT|WB
    BootServicesData        000000001ecf6000-000000001ecf7000 UC|WC|WT|WB
    BootServicesCode        000000001ecf7000-000000001ecf8000 UC|WC|WT|WB
    BootServicesData        000000001ecf8000-000000001ecfa000 UC|WC|WT|WB
    BootServicesCode        000000001ecfa000-000000001ecfd000 UC|WC|WT|WB
    BootServicesData        000000001ecfd000-000000001ecfe000 UC|WC|WT|WB
    BootServicesCode        000000001ecfe000-000000001ecff000 UC|WC|WT|WB
    BootServicesData        000000001ecff000-000000001ed00000 UC|WC|WT|WB
    BootServicesCode        000000001ed00000-000000001ed16000 UC|WC|WT|WB
    BootServicesData        000000001ed16000-000000001ed17000 UC|WC|WT|WB
    BootServicesCode        000000001ed17000-000000001ed19000 UC|WC|WT|WB
    BootServicesData        000000001ed19000-000000001ed2c000 UC|WC|WT|WB
    BootServicesCode        000000001ed2c000-000000001ed2e000 UC|WC|WT|WB
    BootServicesData        000000001ed2e000-000000001f12e000 UC|WC|WT|WB
    BootServicesCode        000000001f12e000-000000001f132000 UC|WC|WT|WB
    BootServicesData        000000001f132000-000000001f133000 UC|WC|WT|WB
    BootServicesCode        000000001f133000-000000001f137000 UC|WC|WT|WB
    BootServicesData        000000001f137000-000000001f13d000 UC|WC|WT|WB
    BootServicesCode        000000001f13d000-000000001f147000 UC|WC|WT|WB
    BootServicesData        000000001f147000-000000001f148000 UC|WC|WT|WB
    BootServicesCode        000000001f148000-000000001f14d000 UC|WC|WT|WB
    BootServicesData        000000001f14d000-000000001f4ed000 UC|WC|WT|WB
    RuntimeServicesData     000000001f4ed000-000000001f5ed000 UC|WC|WT|WB|RT
    RuntimeServicesCode     000000001f5ed000-000000001f6ed000 UC|WC|WT|WB|RT
    ReservedMemory          000000001f6ed000-000000001f76d000 UC|WC|WT|WB
    ACPIReclaimMemory       000000001f76d000-000000001f77f000 UC|WC|WT|WB
    ACPIMemoryNVS           000000001f77f000-000000001f7ff000 UC|WC|WT|WB
    BootServicesData        000000001f7ff000-000000001fe00000 UC|WC|WT|WB
    ConventionalMemory      000000001fe00000-000000001fe77000 UC|WC|WT|WB
    BootServicesData        000000001fe77000-000000001fe97000 UC|WC|WT|WB
    BootServicesCode        000000001fe97000-000000001feca000 UC|WC|WT|WB
    BootServicesData        000000001feca000-000000001fedb000 UC|WC|WT|WB
    BootServicesCode        000000001fedb000-000000001fef4000 UC|WC|WT|WB
    RuntimeServicesData     000000001fef4000-000000001ff78000 UC|WC|WT|WB|RT
    ACPIMemoryNVS           000000001ff78000-0000000020000000 UC|WC|WT|WB
    MemoryMappedIO          00000000ffc00000-0000000100000000 UC|RT
    ReservedMemory          000000fd00000000-0000010000000000

    => efi tables
    000000001f8edf98  ee4e5898-3914-4259-9d6e-dc7bd79403cf  EFI_LZMA_COMPRESSED
    000000001ff2ace0  05ad34ba-6f02-4214-952e-4da0398e2bb9  EFI_DXE_SERVICES
    000000001f8ea018  7739f24c-93d7-11d4-9a3a-0090273fc14d  EFI_HOB_LIST
    000000001ff2bac0  4c19049f-4137-4dd3-9c10-8b97a83ffdfa  EFI_MEMORY_TYPE
    000000001ff2cb10  49152e77-1ada-4764-b7a2-7afefed95e8b  (unknown)
    000000001f9ac018  060cc026-4c0d-4dda-8f41-595fef00a502  EFI_MEM_STATUS_CODE_REC
    000000001f9ab000  eb9d2d31-2d88-11d3-9a16-0090273fc14d  SMBIOS table
    000000001fb7e000  eb9d2d30-2d88-11d3-9a16-0090273fc14d  EFI_GUID_EFI_ACPI1
    000000001fb7e014  8868e871-e4f1-11d3-bc22-0080c73c8881  ACPI table
    000000001e654018  dcfa911d-26eb-469f-a220-38b7dc461220  (unknown)
