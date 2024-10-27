/** @file
*  Differentiated System Description Table Fields (DSDT).
*
*  Copyright (c) 2020, Linaro Ltd. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <configs/qemu-sbsa.h>

#define LINK_DEVICE(Uid, LinkName, Irq)                                        \
        Device (LinkName) {                                                    \
            Name (_HID, EISAID("PNP0C0F"))                                     \
            Name (_UID, Uid)                                                   \
            Name (_PRS, ResourceTemplate() {                                   \
                Interrupt (ResourceProducer, Level, ActiveHigh, Exclusive) { Irq } \
            })                                                                 \
            Method (_STA) {                                                    \
              Return (0xF)                                                     \
            }                                                                  \
            Method (_CRS, 0) { Return (_PRS) }                                 \
            Method (_SRS, 1) { }                                               \
            Method (_DIS) { }                                                  \
        }

#define PRT_ENTRY(Address, Pin, Link)                                          \
        Package (4) {                                                          \
            Address, Pin, Link, Zero                                           \
          }

DefinitionBlock ("Dsdt.aml", "DSDT", 2, "U-Boot", "SBSAQEMU", 2) {
  Scope (_SB) {
    // UART PL011
    Device (COM0) {
      Name (_HID, "ARMH0011")
      Name (_UID, Zero)
      Name (_CRS, ResourceTemplate () {
        Memory32Fixed (ReadWrite,
                       SBSA_UART_BASE_ADDR,
                       SBSA_UART_LENGTH)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 33 }
      })
      Method (_STA) {
        Return (0xF)
      }
    }

    // AHCI Host Controller
    Device (AHC0) {
      Name (_HID, "LNRO001E")
      Name (_CLS, Package (3) {
        0x01,
        0x06,
        0x01,
      })
      Name (_CCA, 1)
      Name (_CRS, ResourceTemplate() {
        Memory32Fixed (ReadWrite,
                       SBSA_AHCI_BASE_ADDR,
                       SBSA_AHCI_LENGTH)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 42 }
      })
      Method (_STA) {
        Return (0xF)
      }
    }


    // USB XHCI Host Controller
    Device (USB0) {
        Name (_HID, "PNP0D10")      // _HID: Hardware ID
        Name (_UID, 0x00)            // _UID: Unique ID
        Name (_CCA, 0x01)            // _CCA: Cache Coherency Attribute
        Name (XHCI, 0xF)            // will be set using AcpiLib
        Method (_STA) {
          Return (XHCI)
        }
        Name (_CRS, ResourceTemplate() {
            Memory32Fixed (ReadWrite,
                           SBSA_XHCI_BASE_ADDR,
                           SBSA_XHCI_LENGTH)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 43 }
        })

        // Root Hub
        Device (RHUB) {
            Name (_ADR, 0x00000000)  // Address of Root Hub should be 0 as per ACPI 5.0 spec
            Method (_STA) {
              Return (0xF)
            }

            // Ports connected to Root Hub
            Device (HUB1) {
                Name (_ADR, 0x00000001)
                Name (_UPC, Package() {
                    0x00,       // Port is NOT connectable
                    0xFF,       // Don't care
                    0x00000000, // Reserved 0 must be zero
                    0x00000000  // Reserved 1 must be zero
                })
                Method (_STA) {
                  Return (0xF)
                }

                Device (PRT1) {
                    Name (_ADR, 0x00000001)
                    Name (_UPC, Package() {
                        0xFF,        // Port is connectable
                        0x00,        // Port connector is A
                        0x00000000,
                        0x00000000
                    })
                    Name (_PLD, Package() {
                        Buffer(0x10) {
                            0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                        }
                    })
                    Method (_STA) {
                      Return (0xF)
                    }
                } // USB0_RHUB_HUB1_PRT1
                Device (PRT2) {
                    Name (_ADR, 0x00000002)
                    Name (_UPC, Package() {
                        0xFF,        // Port is connectable
                        0x00,        // Port connector is A
                        0x00000000,
                        0x00000000
                    })
                    Name (_PLD, Package() {
                        Buffer(0x10) {
                            0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                        }
                    })
                    Method (_STA) {
                      Return (0xF)
                    }
                } // USB0_RHUB_HUB1_PRT2

                Device (PRT3) {
                    Name (_ADR, 0x00000003)
                    Name (_UPC, Package() {
                        0xFF,        // Port is connectable
                        0x09,        // Type C connector - USB2 and SS with Switch
                        0x00000000,
                        0x00000000
                    })
                    Name (_PLD, Package() {
                        Buffer (0x10) {
                            0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                        }
                    })
                    Method (_STA) {
                      Return (0xF)
                    }
                } // USB0_RHUB_HUB1_PRT3

                Device (PRT4) {
                    Name (_ADR, 0x00000004)
                    Name (_UPC, Package() {
                        0xFF,        // Port is connectable
                        0x09,        // Type C connector - USB2 and SS with Switch
                        0x00000000,
                        0x00000000
                    })
                    Name (_PLD, Package() {
                        Buffer (0x10){
                            0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                        }
                    })
                    Method (_STA) {
                      Return (0xF)
                    }
                } // USB0_RHUB_HUB1_PRT4
            } // USB0_RHUB_HUB1
        } // USB0_RHUB
    } // USB0

    Device (PCI0)
    {
      Name (_HID, EISAID ("PNP0A08")) // PCI Express Root Bridge
      Name (_CID, EISAID ("PNP0A03")) // Compatible PCI Root Bridge
      Name (_SEG, Zero) // PCI Segment Group number
      Name (_BBN, Zero) // PCI Base Bus Number
      Name (_UID, "PCI0")
      Name (_CCA, One)    // Initially mark the PCI coherent (for JunoR1)

      Method (_STA) {
        Return (0xF)
      }

      Method (_CBA, 0, NotSerialized) {
          return (SBSA_PCIE_ECAM_BASE_ADDR)
      }

      LINK_DEVICE(0, GSI0, 0x23)
      LINK_DEVICE(1, GSI1, 0x24)
      LINK_DEVICE(2, GSI2, 0x25)
      LINK_DEVICE(3, GSI3, 0x26)

      Name (_PRT, Package ()  // _PRT: PCI Routing Table
      {
        PRT_ENTRY(0x0000FFFF, 0, GSI0),
        PRT_ENTRY(0x0000FFFF, 0, GSI1),
        PRT_ENTRY(0x0000FFFF, 0, GSI2),
        PRT_ENTRY(0x0000FFFF, 0, GSI3),

        PRT_ENTRY(0x0001FFFF, 0, GSI1),
        PRT_ENTRY(0x0001FFFF, 1, GSI2),
        PRT_ENTRY(0x0001FFFF, 2, GSI3),
        PRT_ENTRY(0x0001FFFF, 3, GSI0),

        PRT_ENTRY(0x0002FFFF, 0, GSI2),
        PRT_ENTRY(0x0002FFFF, 1, GSI3),
        PRT_ENTRY(0x0002FFFF, 2, GSI0),
        PRT_ENTRY(0x0002FFFF, 3, GSI1),

        PRT_ENTRY(0x0003FFFF, 0, GSI3),
        PRT_ENTRY(0x0003FFFF, 1, GSI0),
        PRT_ENTRY(0x0003FFFF, 2, GSI1),
        PRT_ENTRY(0x0003FFFF, 3, GSI2),

        PRT_ENTRY(0x0004FFFF, 0, GSI0),
        PRT_ENTRY(0x0004FFFF, 1, GSI1),
        PRT_ENTRY(0x0004FFFF, 2, GSI2),
        PRT_ENTRY(0x0004FFFF, 3, GSI3),

        PRT_ENTRY(0x0005FFFF, 0, GSI1),
        PRT_ENTRY(0x0005FFFF, 1, GSI2),
        PRT_ENTRY(0x0005FFFF, 2, GSI3),
        PRT_ENTRY(0x0005FFFF, 3, GSI0),

        PRT_ENTRY(0x0006FFFF, 0, GSI2),
        PRT_ENTRY(0x0006FFFF, 1, GSI3),
        PRT_ENTRY(0x0006FFFF, 2, GSI0),
        PRT_ENTRY(0x0006FFFF, 3, GSI1),

        PRT_ENTRY(0x0007FFFF, 0, GSI3),
        PRT_ENTRY(0x0007FFFF, 1, GSI0),
        PRT_ENTRY(0x0007FFFF, 2, GSI1),
        PRT_ENTRY(0x0007FFFF, 3, GSI2),

        PRT_ENTRY(0x0008FFFF, 0, GSI0),
        PRT_ENTRY(0x0008FFFF, 1, GSI1),
        PRT_ENTRY(0x0008FFFF, 2, GSI2),
        PRT_ENTRY(0x0008FFFF, 3, GSI3),

        PRT_ENTRY(0x0009FFFF, 0, GSI1),
        PRT_ENTRY(0x0009FFFF, 1, GSI2),
        PRT_ENTRY(0x0009FFFF, 2, GSI3),
        PRT_ENTRY(0x0009FFFF, 3, GSI0),

        PRT_ENTRY(0x000AFFFF, 0, GSI2),
        PRT_ENTRY(0x000AFFFF, 1, GSI3),
        PRT_ENTRY(0x000AFFFF, 2, GSI0),
        PRT_ENTRY(0x000AFFFF, 3, GSI1),

        PRT_ENTRY(0x000BFFFF, 0, GSI3),
        PRT_ENTRY(0x000BFFFF, 1, GSI0),
        PRT_ENTRY(0x000BFFFF, 2, GSI1),
        PRT_ENTRY(0x000BFFFF, 3, GSI2),

        PRT_ENTRY(0x000CFFFF, 0, GSI0),
        PRT_ENTRY(0x000CFFFF, 1, GSI1),
        PRT_ENTRY(0x000CFFFF, 2, GSI2),
        PRT_ENTRY(0x000CFFFF, 3, GSI3),

        PRT_ENTRY(0x000DFFFF, 0, GSI1),
        PRT_ENTRY(0x000DFFFF, 1, GSI2),
        PRT_ENTRY(0x000DFFFF, 2, GSI3),
        PRT_ENTRY(0x000DFFFF, 3, GSI0),

        PRT_ENTRY(0x000EFFFF, 0, GSI2),
        PRT_ENTRY(0x000EFFFF, 1, GSI3),
        PRT_ENTRY(0x000EFFFF, 2, GSI0),
        PRT_ENTRY(0x000EFFFF, 3, GSI1),

        PRT_ENTRY(0x000FFFFF, 0, GSI3),
        PRT_ENTRY(0x000FFFFF, 1, GSI0),
        PRT_ENTRY(0x000FFFFF, 2, GSI1),
        PRT_ENTRY(0x000FFFFF, 3, GSI2),

        PRT_ENTRY(0x0010FFFF, 0, GSI0),
        PRT_ENTRY(0x0010FFFF, 1, GSI1),
        PRT_ENTRY(0x0010FFFF, 2, GSI2),
        PRT_ENTRY(0x0010FFFF, 3, GSI3),

        PRT_ENTRY(0x0011FFFF, 0, GSI1),
        PRT_ENTRY(0x0011FFFF, 1, GSI2),
        PRT_ENTRY(0x0011FFFF, 2, GSI3),
        PRT_ENTRY(0x0011FFFF, 3, GSI0),

        PRT_ENTRY(0x0012FFFF, 0, GSI2),
        PRT_ENTRY(0x0012FFFF, 1, GSI3),
        PRT_ENTRY(0x0012FFFF, 2, GSI0),
        PRT_ENTRY(0x0012FFFF, 3, GSI1),

        PRT_ENTRY(0x0013FFFF, 0, GSI3),
        PRT_ENTRY(0x0013FFFF, 1, GSI0),
        PRT_ENTRY(0x0013FFFF, 2, GSI1),
        PRT_ENTRY(0x0013FFFF, 3, GSI2),

        PRT_ENTRY(0x0014FFFF, 0, GSI0),
        PRT_ENTRY(0x0014FFFF, 1, GSI1),
        PRT_ENTRY(0x0014FFFF, 2, GSI2),
        PRT_ENTRY(0x0014FFFF, 3, GSI3),

        PRT_ENTRY(0x0015FFFF, 0, GSI1),
        PRT_ENTRY(0x0015FFFF, 1, GSI2),
        PRT_ENTRY(0x0015FFFF, 2, GSI3),
        PRT_ENTRY(0x0015FFFF, 3, GSI0),

        PRT_ENTRY(0x0016FFFF, 0, GSI2),
        PRT_ENTRY(0x0016FFFF, 1, GSI3),
        PRT_ENTRY(0x0016FFFF, 2, GSI0),
        PRT_ENTRY(0x0016FFFF, 3, GSI1),

        PRT_ENTRY(0x0017FFFF, 0, GSI3),
        PRT_ENTRY(0x0017FFFF, 1, GSI0),
        PRT_ENTRY(0x0017FFFF, 2, GSI1),
        PRT_ENTRY(0x0017FFFF, 3, GSI2),

        PRT_ENTRY(0x0018FFFF, 0, GSI0),
        PRT_ENTRY(0x0018FFFF, 1, GSI1),
        PRT_ENTRY(0x0018FFFF, 2, GSI2),
        PRT_ENTRY(0x0018FFFF, 3, GSI3),

        PRT_ENTRY(0x0019FFFF, 0, GSI1),
        PRT_ENTRY(0x0019FFFF, 1, GSI2),
        PRT_ENTRY(0x0019FFFF, 2, GSI3),
        PRT_ENTRY(0x0019FFFF, 3, GSI0),

        PRT_ENTRY(0x001AFFFF, 0, GSI2),
        PRT_ENTRY(0x001AFFFF, 1, GSI3),
        PRT_ENTRY(0x001AFFFF, 2, GSI0),
        PRT_ENTRY(0x001AFFFF, 3, GSI1),

        PRT_ENTRY(0x001BFFFF, 0, GSI3),
        PRT_ENTRY(0x001BFFFF, 1, GSI0),
        PRT_ENTRY(0x001BFFFF, 2, GSI1),
        PRT_ENTRY(0x001BFFFF, 3, GSI2),

        PRT_ENTRY(0x001CFFFF, 0, GSI0),
        PRT_ENTRY(0x001CFFFF, 1, GSI1),
        PRT_ENTRY(0x001CFFFF, 2, GSI2),
        PRT_ENTRY(0x001CFFFF, 3, GSI3),

        PRT_ENTRY(0x001DFFFF, 0, GSI1),
        PRT_ENTRY(0x001DFFFF, 1, GSI2),
        PRT_ENTRY(0x001DFFFF, 2, GSI3),
        PRT_ENTRY(0x001DFFFF, 3, GSI0),

        PRT_ENTRY(0x001EFFFF, 0, GSI2),
        PRT_ENTRY(0x001EFFFF, 1, GSI3),
        PRT_ENTRY(0x001EFFFF, 2, GSI0),
        PRT_ENTRY(0x001EFFFF, 3, GSI1),

        PRT_ENTRY(0x001FFFFF, 0, GSI3),
        PRT_ENTRY(0x001FFFFF, 1, GSI0),
        PRT_ENTRY(0x001FFFFF, 2, GSI1),
        PRT_ENTRY(0x001FFFFF, 3, GSI2),
      })

      // Root complex resources
      Name (_CRS, ResourceTemplate () {
        WordBusNumber ( // Bus numbers assigned to this root
        ResourceProducer,
        MinFixed, MaxFixed, PosDecode,
        0,   // AddressGranularity
        0,   // AddressMinimum - Minimum Bus Number
        0xff,// AddressMaximum - Maximum Bus Number
        0,   // AddressTranslation - Set to 0
        256  // RangeLength - Number of Busses
        )

        // IO to mmio window
        QWordIO (
          ResourceProducer, MinFixed,
          MaxFixed, PosDecode,
          EntireRange,
          0x00000000,                              // Granularity
          0x0000,                                  // Min Base Address
          0xffff,                                  // Max Base Address
          SBSA_PIO_BASE_ADDR,                      // Translate
          SBSA_PIO_LENGTH                          // Length
        )

        DWordMemory ( // 32-bit BAR Windows
          ResourceProducer, PosDecode,
          MinFixed, MaxFixed,
          Cacheable, ReadWrite,
          0x00000000,                              // Granularity
          SBSA_PCIE_MMIO_BASE_ADDR,                // Min Base Address
          SBSA_PCIE_MMIO_END,                      // Max Base Address
          0,                                       // Translate
          SBSA_PCIE_MMIO_LENGTH                    // Length
          )

        QWordMemory ( // 64-bit BAR Windows
          ResourceProducer, PosDecode,
          MinFixed, MaxFixed,
          Cacheable, ReadWrite,
          0x00000000,                              // Granularity
          SBSA_PCIE_MMIO_HIGH_BASE_ADDR,           // Min Base Address
          SBSA_PCIE_MMIO_HIGH_END,                 // Max Base Address
          0,                                       // Translate
          SBSA_PCIE_MMIO_HIGH_LENGTH               // Length
          )
      }) // Name(_CRS)

      Device (RES0)
      {
        Name (_HID, "PNP0C02" /* PNP Motherboard Resources */)  // _HID: Hardware ID
        Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
        {
           QWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
           0x0000000000000000,                       // Granularity
           SBSA_PCIE_ECAM_BASE_ADDR,                 // Range Minimum
           SBSA_PCIE_ECAM_END,                       // Range Maximum
           0x0000000000000000,                       // Translation Offset
           SBSA_PCIE_ECAM_LENGTH,                    // Length
           ,, , AddressRangeMemory, TypeStatic)
        })
        Method (_STA) {
          Return (0xF)
        }
      }

      // OS Control Handoff
      Name (SUPP, Zero) // PCI _OSC Support Field value
      Name (CTRL, Zero) // PCI _OSC Control Field value

      /*
       * See [1] 6.2.10, [2] 4.5
       */
      Method (_OSC,4) {
        // Check for proper UUID
        If (Arg0 == ToUUID("33DB4D5B-1FF7-401C-9657-7441C03DD766")) {
          // Create DWord-adressable fields from the Capabilities Buffer
          CreateDWordField (Arg3,0,CDW1)
          CreateDWordField (Arg3,4,CDW2)
          CreateDWordField (Arg3,8,CDW3)

          // Save Capabilities DWord2 & 3
          Store (CDW2,SUPP)
          Store (CDW3,CTRL)

          // Only allow native hot plug control if OS supports:
          // * ASPM
          // * Clock PM
          // * MSI/MSI-X
          If ((SUPP & 0x16) != 0x16) {
            CTRL &= 0x1E // Mask bit 0 (and undefined bits)
          }

          // Always allow native PME, AER (no dependencies)

          // Never allow SHPC (no SHPC controller in this system)
          CTRL &= 0x1D

          If (Arg1 != One) {         // Unknown revision
            CDW1 |= 0x08
          }

          If (CDW3 != CTRL) {        // Capabilities bits were masked
            CDW1 |= 0x10
          }

          // Update DWORD3 in the buffer
          Store (CTRL,CDW3)
          Return (Arg3)
        } Else {
          CDW1 |= 4 // Unrecognized UUID
          Return (Arg3)
        }
      } // End _OSC
    }
  } // Scope (_SB)
}
