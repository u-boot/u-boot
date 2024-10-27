/** @file
 *
 *  Copyright (c) 2021, ARM Limited. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <asm/arch/acpi/bcm2836_sdhost.h>
#include <asm/arch/acpi/bcm2836_sdio.h>
#include <asm/arch/acpi/bcm2711.h>

Device (GDV1) {
  Name (_HID, "ACPI0004")
  Name (_UID, 0x2)
  Name (_CCA, 0x0)

  Name (RBUF, ResourceTemplate ()
  {
    MEMORY32FIXED (ReadWrite, 0, MMCHS2_LENGTH, RMEM)
  })
  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RMEM, RBAS, MMCHS2_OFFSET)
    Return (^RBUF)
  }

  // Translated DMA region for BCM2711 silicon revisions older than C0
  Name (DMTR, ResourceTemplate() {
    QWordMemory (ResourceProducer,
      ,
      MinFixed,
      MaxFixed,
      NonCacheable,
      ReadWrite,
      0x0,
      0x00000000C0000000, // MIN
      0x00000000FFFFFFFF, // MAX
      0xFFFFFFFF40000000, // TRA
      0x0000000040000000, // LEN
      ,
      ,
    )
  })

  // Non translated DMA region for BCM2711 revisions C0 and newer
  Name (DMNT, ResourceTemplate() {
    QWordMemory (ResourceProducer,
      ,
      MinFixed,
      MaxFixed,
      NonCacheable,
      ReadWrite,
      0x0,
      0x0000000000000000, // MIN
      0x000000FFFFFFFFFF, // MAX
      0x0000000000000000, // TRA
      0x0000010000000000, // LEN
      ,
      ,
    )
  })

  // emmc2 Host Controller. (brcm,bcm2711-emmc2)
  Device (SDC3)
  {
    Name (_HID, "BRCME88C")
    Name (_UID, 0x1)
    Name (_CCA, 0x0)
    Name (_S1D, 0x1)
    Name (_S2D, 0x1)
    Name (_S3D, 0x1)
    Name (_S4D, 0x1)
    Name (SDMA, 0x2)

    Name (RBUF, ResourceTemplate ()
    {
      MEMORY32FIXED (ReadWrite, 0, MMCHS2_LENGTH, RMEM)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { BCM2836_MMCHS1_INTERRUPT }
    })
    Method (_CRS, 0x0, Serialized)
    {
      MEMORY32SETBASE (RBUF, RMEM, RBAS, MMCHS2_OFFSET)
      Return (^RBUF)
    }

    // Unfortunately this controller doesn't honor the
    // standard SDHCI voltage control registers
    // (or at least Linux's standard code can't
    // lower the voltage) So, UHS mode is disabled with caps
    Name (DSD1, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () { "sdhci-caps-mask", 0x0000000500080000 },
        }
    })
    // Along with disabling UHS, here both SDMA and ADMA2
    // are also disabled until the linux _DMA() mask/translate
    // works properly.
    Name (DSD2, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package () { "sdhci-caps-mask", 0x0000000504480000 },
        }
    })
    Method (_DSD, 0x0, Serialized)
    {
      // Select one of the sdhci-caps-mask definitions
      // depending on whether we also want to disable DMA
      if (SDMA == 0)
      {
        return (^DSD2)
      }
      else
      {
        return (^DSD1)
      }
    }

    //
    // A child device that represents the
    // sd card, which is marked as non-removable.
    //
    Device (SDMM)
    {
      Method (_ADR)
      {
        Return (0)
      }
      Method (_RMV) // Is removable
      {
        Return (0) // 0 - fixed
      }
    }
  } //SDC3
} //GDV1
