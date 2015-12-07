/* PCI CRS (current resources) definition. */
Scope(\_SB.PCI0) {

    Name(CRES, ResourceTemplate() {
        WordBusNumber(ResourceProducer, MinFixed, MaxFixed, PosDecode,
            0x0000,             /* Address Space Granularity */
            0x0000,             /* Address Range Minimum */
            0x00ff,             /* Address Range Maximum */
            0x0000,             /* Address Translation Offset */
            0x0100,             /* Address Length */
            ,, )
        IO(Decode16,
            0x0cf8,             /* Address Range Minimum */
            0x0cf8,             /* Address Range Maximum */
            0x01,               /* Address Alignment */
            0x08,               /* Address Length */
            )
        WordIO(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
            0x0000,             /* Address Space Granularity */
            0x0000,             /* Address Range Minimum */
            0x0cf7,             /* Address Range Maximum */
            0x0000,             /* Address Translation Offset */
            0x0cf8,             /* Address Length */
            ,, , TypeStatic)
        WordIO(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
            0x0000,             /* Address Space Granularity */
            0x0d00,             /* Address Range Minimum */
            0xffff,             /* Address Range Maximum */
            0x0000,             /* Address Translation Offset */
            0xf300,             /* Address Length */
            ,, , TypeStatic)
        DWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
            0x00000000,         /* Address Space Granularity */
            0x000a0000,         /* Address Range Minimum */
            0x000bffff,         /* Address Range Maximum */
            0x00000000,         /* Address Translation Offset */
            0x00020000,         /* Address Length */
            ,, , AddressRangeMemory, TypeStatic)
        DWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
            0x00000000,         /* Address Space Granularity */
            0xe0000000,         /* Address Range Minimum */
            0xfebfffff,         /* Address Range Maximum */
            0x00000000,         /* Address Translation Offset */
            0x1ec00000,         /* Address Length */
            ,, PW32, AddressRangeMemory, TypeStatic)
    })

    Name(CR64, ResourceTemplate() {
        QWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
            0x00000000,          /* Address Space Granularity */
            0x80000000,        /* Address Range Minimum */
            0xffffffff,        /* Address Range Maximum */
            0x00000000,          /* Address Translation Offset */
            0x80000000,        /* Address Length */
            ,, PW64, AddressRangeMemory, TypeStatic)
    })

    Method(_CRS, 0) {
        Return (CRES)
    }
}
