/** @file
 *
 *  Copyright (c) 2019 Linaro, Limited. All rights reserved.
 *  Copyright (c) 2021 Arm
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

Device(PCI0)
{
  Name(_HID, EISAID("PNP0A08")) // PCI Express Root Bridge
  Name(_CID, EISAID("PNP0A03")) // Compatible PCI Root Bridge
  Name(_SEG, Zero) // PCI Segment Group number
  Name(_BBN, Zero) // PCI Base Bus Number
  Name(_CCA, 0)    // Mark the PCI noncoherent

  // PCIe can only DMA to first 3GB with early SOC's
  // But we keep the restriction on the later ones
  // To avoid DMA translation problems.
  Name (_DMA, ResourceTemplate() {
    QWordMemory (ResourceProducer,
      ,
      MinFixed,
      MaxFixed,
      NonCacheable,
      ReadWrite,
      0x0,
      0x0,        // MIN
      0xbfffffff, // MAX
      0x0,        // TRA
      0xc0000000, // LEN
      ,
      ,
      )
  })

  // PCI Routing Table
  Name(_PRT, Package() {
    Package (4) { 0x0000FFFF, 0, zero, 175 },
    Package (4) { 0x0000FFFF, 1, zero, 176 },
    Package (4) { 0x0000FFFF, 2, zero, 177 },
    Package (4) { 0x0000FFFF, 3, zero, 178 }
  })

  Name (_DSD, Package () {
    ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () { "linux-ecam-quirk-id", "bcm2711" },
      }
  })

  // Root complex resources
  Method (_CRS, 0, Serialized) {
    Name (RBUF, ResourceTemplate () {

      // bus numbers assigned to this root
      WordBusNumber (
        ResourceProducer,
        MinFixed, MaxFixed, PosDecode,
        0,   // AddressGranularity
        0,   // AddressMinimum - Minimum Bus Number
        255, // AddressMaximum - Maximum Bus Number
        0,   // AddressTranslation - Set to 0
        256  // RangeLength - Number of Busses
      )

      // 32-bit mmio window in 64-bit addr
      QWordMemory (
        ResourceProducer, PosDecode,
        MinFixed, MaxFixed,
        NonCacheable, ReadWrite,        // cacheable
        0x00000000,                     // Granularity
        0,                              // PCIE_PCI_MMIO_BEGIN
        1,                              // PCIE_MMIO_LEN + PCIE_PCI_MMIO_BEGIN
        PCIE_CPU_MMIO_WINDOW,           // PCIE_PCI_MMIO_BEGIN - PCIE_CPU_MMIO_WINDOW
        2                               // PCIE_MMIO_LEN + 1
        ,,,MMI1
      )

      // root port registers, not to be used if SMCCC is utilized
      QWordMemory (
        ResourceConsumer, ,
        MinFixed, MaxFixed,
        NonCacheable, ReadWrite,        // cacheable
        0x00000000,                     // Granularity
        0xFD500000,                     // Root port begin
        0xFD509FFF,                     // Root port end
        0x00000000,                     // no translation
        0x0000A000,                     // size
        ,,
      )
    }) // end Name(RBUF)

    // Work around ASL's inability to add in a resource definition
    // or for that matter compute the min,max,len properly
    CreateQwordField (RBUF, MMI1._MIN, MMIB)
    CreateQwordField (RBUF, MMI1._MAX, MMIE)
    CreateQwordField (RBUF, MMI1._TRA, MMIT)
    CreateQwordField (RBUF, MMI1._LEN, MMIL)
    Add (MMIB, PCIE_TOP_OF_MEM_WIN, MMIB)
    Add (PCIE_BRIDGE_MMIO_LEN, PCIE_TOP_OF_MEM_WIN, MMIE)
    Subtract (MMIT, PCIE_TOP_OF_MEM_WIN, MMIT)
    Add (PCIE_BRIDGE_MMIO_LEN, 1 , MMIL)

    Return (RBUF)
  } // end Method(_CRS)

  // OS Control Handoff
  Name(SUPP, Zero) // PCI _OSC Support Field value
  Name(CTRL, Zero) // PCI _OSC Control Field value

  // See [1] 6.2.10, [2] 4.5
  Method(_OSC,4) {
    // Note, This code is very similar to the code in the PCIe firmware
    // specification which can be used as a reference
    // Check for proper UUID
    If(LEqual(Arg0,ToUUID("33DB4D5B-1FF7-401C-9657-7441C03DD766"))) {
      // Create DWord-adressable fields from the Capabilities Buffer
      CreateDWordField(Arg3,0,CDW1)
      CreateDWordField(Arg3,4,CDW2)
      CreateDWordField(Arg3,8,CDW3)
      // Save Capabilities DWord2 & 3
      Store(CDW2,SUPP)
      Store(CDW3,CTRL)
      // Mask out Native HotPlug
      And(CTRL,0x1E,CTRL)
      // Always allow native PME, AER (no dependencies)
      // Never allow SHPC (no SHPC controller in this system)
      And(CTRL,0x1D,CTRL)

      If(LNotEqual(Arg1,One)) { // Unknown revision
        Or(CDW1,0x08,CDW1)
      }

      If(LNotEqual(CDW3,CTRL)) {  // Capabilities bits were masked
        Or(CDW1,0x10,CDW1)
      }
      // Update DWORD3 in the buffer
      Store(CTRL,CDW3)
      Return(Arg3)
    } Else {
      Or(CDW1,4,CDW1) // Unrecognized UUID
      Return(Arg3)
    }
  } // End _OSC

  Device (XHC0)
  {
    Name (_ADR, 0x00010000)
    Name (_CID, "PNP0D10")
    Name (_UID, 0x0)            // _UID: Unique ID
    Name (_CCA, 0x0)            // _CCA: Cache Coherency Attribute

    /*
     * Microsoft's USB Device-Specific Methods. See:
     * https://docs.microsoft.com/en-us/windows-hardware/drivers/bringup/usb-device-specific-method---dsm-
     */
    Name (DSMU, ToUUID ("ce2ee385-00e6-48cb-9f05-2edb927c4899"))

    Method (_DSM, 4, Serialized) {
        If (LEqual (Arg0, DSMU)) {              // USB capabilities UUID
            Switch (ToInteger (Arg2)) {
            Case (0) {                          // Function 0: List of supported functions
                Return (Buffer () { 0x41 })     // 0x41 - Functions 0 and 6 supported
            }
            Case (6) {                          // Function 6: RegisterAccessType
                Return (Buffer () { 0x01 })     // 0x01 - Must use 32bit register access
            }
            Default { }                         // Unsupported
            }
        }
        return (Buffer () { 0x00 })             // Return 0x00 for anything unsupported
    }
  } // end XHC0

} // PCI0