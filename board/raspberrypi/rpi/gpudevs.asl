/** @file
 *
 *  [DSDT] Devices behind the GPU.
 *
 *  Copyright (c) 2018-2020, Andrey Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

// DWC OTG Controller
Device (USB0)
{
  Name (_HID, "BCM2848")
#if defined(CONFIG_TARGET_RPI_3)
  Name (_CID, "DWC_OTG")
#elif defined(CONFIG_TARGET_RPI_4)
  Name (_CID, "BCM2848")
#endif
  Name (_UID, 0x0)
  Name (_CCA, 0x0)
  Method (_STA)
  {
    Return (0xf)
  }
  Name (RBUF, ResourceTemplate ()
  {
    MEMORY32FIXED (ReadWrite, 0, BCM2836_USB_LENGTH, RMEM)
    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_USB_INTERRUPT }
  })
  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RMEM, RBAS, BCM2836_USB_OFFSET)
    Return (^RBUF)
  }
}

// Video Core 4 GPU
Device (GPU0)
{
  Name (_HID, "BCM2850")
  Name (_CID, "BCM2850")
  Name (_UID, 0x0)
  Name (_CCA, 0x0)
  Method (_STA)
  {
    Return (0xf)
  }
  Name (RBUF, ResourceTemplate ()
  {
    // Memory and interrupt for the GPU
    MEMORY32FIXED (ReadWrite, 0, BCM2836_V3D_BUS_LENGTH, RM01)
    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_V3D_BUS_INTERRUPT }

    // HVS - Hardware Video Scalar
    MEMORY32FIXED (ReadWrite, 0, BCM2836_HVS_LENGTH, RM02)
    // The HVS interrupt is reserved by the VPU
    // Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_HVS_INTERRUPT }

    // PixelValve0 - DSI0 or DPI
    // MEMORY32FIXED (ReadWrite, BCM2836_PV0_BASE_ADDRESS, BCM2836_PV0_LENGTH, RM03)
    // Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_PV0_INTERRUPT }

    // PixelValve1 - DS1 or SMI
    // MEMORY32FIXED (ReadWrite, BCM2836_PV1_BASE_ADDRESS, BCM2836_PV1_LENGTH, RM04)
    // Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_PV1_INTERRUPT }

    // PixelValve2 - HDMI output - connected to HVS display FIFO 1
    MEMORY32FIXED (ReadWrite, 0, BCM2836_PV2_LENGTH, RM05)
    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_PV2_INTERRUPT }

    // HDMI registers
    MEMORY32FIXED (ReadWrite, 0, BCM2836_HDMI0_LENGTH, RM06)
    MEMORY32FIXED (ReadWrite, 0, BCM2836_HDMI1_LENGTH, RM07)
    // hdmi_int[0]
    // Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_HDMI0_INTERRUPT }
    // hdmi_int[1]
    // Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_HDMI1_INTERRUPT }

    // HDMI DDC connection
    I2CSerialBus (0x50,, 100000,, "\\_SB.GDV0.I2C2",,,,)  // EDID
    I2CSerialBus (0x30,, 100000,, "\\_SB.GDV0.I2C2",,,,)  // E-DDC Segment Pointer
  })
  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RM01, RB01, BCM2836_V3D_BUS_OFFSET)
    MEMORY32SETBASE (RBUF, RM02, RB02, BCM2836_HVS_OFFSET)
    MEMORY32SETBASE (RBUF, RM05, RB05, BCM2836_PV2_OFFSET)
    MEMORY32SETBASE (RBUF, RM06, RB06, BCM2836_HDMI0_OFFSET)
    MEMORY32SETBASE (RBUF, RM07, RB07, BCM2836_HDMI1_OFFSET)
    Return (^RBUF)
  }

  // GPU Power Management Component Data
  // Reference : https://github.com/Microsoft/graphics-driver-samples/wiki/Install-Driver-in-a-Windows-VM
  Method (PMCD, 0, Serialized)
  {
    Name (RBUF, Package ()
    {
      1,                  // Version
      1,                  // Number of graphics power components
      Package ()          // Power components package
      {
        Package ()        // GPU component package
        {
          0,              // Component Index
          0,              // DXGK_POWER_COMPONENT_MAPPING.ComponentType (0 = DXGK_POWER_COMPONENT_ENGINE)
          0,              // DXGK_POWER_COMPONENT_MAPPING.NodeIndex

          Buffer ()       // DXGK_POWER_RUNTIME_COMPONENT.ComponentGuid
          {               // 9B2D1E26-1575-4747-8FC0-B9EB4BAA2D2B
            0x26, 0x1E, 0x2D, 0x9B, 0x75, 0x15, 0x47, 0x47,
            0x8f, 0xc0, 0xb9, 0xeb, 0x4b, 0xaa, 0x2d, 0x2b
          },

          "VC4_Engine_00",// DXGK_POWER_RUNTIME_COMPONENT.ComponentName
          2,              // DXGK_POWER_RUNTIME_COMPONENT.StateCount

          Package ()      // DXGK_POWER_RUNTIME_COMPONENT.States[] package
          {
            Package ()   // F0
            {
              0,         // DXGK_POWER_RUNTIME_STATE.TransitionLatency
              0,         // DXGK_POWER_RUNTIME_STATE.ResidencyRequirement
              1210000,   // DXGK_POWER_RUNTIME_STATE.NominalPower (microwatt)
            },

            Package ()   // F1 - Placeholder
            {
              10000,     // DXGK_POWER_RUNTIME_STATE.TransitionLatency
              10000,     // DXGK_POWER_RUNTIME_STATE.ResidencyRequirement
              4,         // DXGK_POWER_RUNTIME_STATE.NominalPower
            },
          }
        }
      }
    })
    Return (RBUF)
  }
}

// PiQ Mailbox Driver
Device (RPIQ)
{
  Name (_HID, "BCM2849")
  Name (_CID, "BCM2849")
  Name (_UID, 0)
  Name (_CCA, 0x0)

  Name (RBUF, ResourceTemplate ()
  {
    MEMORY32FIXED (ReadWrite, 0, BCM2836_MBOX_LENGTH, RMEM)
    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_MBOX_INTERRUPT }
  })

  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RMEM, RBAS, BCM2836_MBOX_OFFSET)
    Return (^RBUF)
  }
}

// VCHIQ Driver
Device (VCIQ)
{
  Name (_HID, "BCM2835")
  Name (_CID, "BCM2835")
  Name (_UID, 0)
  Name (_CCA, 0x0)
  Name (_DEP, Package() { \_SB.GDV0.RPIQ })
  Method (_STA)
  {
    Return (0xf)
  }
  Name (RBUF, ResourceTemplate ()
  {
    MEMORY32FIXED (ReadWrite, 0, BCM2836_VCHIQ_LENGTH, RMEM)
    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_VCHIQ_INTERRUPT }
  })

  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RMEM, RBAS, BCM2836_VCHIQ_OFFSET)
    Return (^RBUF)
  }
}

// VC Shared Memory Driver
Device (VCSM)
{
  Name (_HID, "BCM2856")
  Name (_CID, "BCM2856")
  Name (_UID, 0)
  Name (_CCA, 0x0)
  Name (_DEP, Package() { \_SB.GDV0.VCIQ })
  Method (_STA)
  {
    Return (0xf)
  }
}

// Description: GPIO
Device (GPI0)
{
  Name (_HID, "BCM2845")
  Name (_CID, "BCM2845")
  Name (_UID, 0x0)
  Name (_CCA, 0x0)

  Name (RBUF, ResourceTemplate ()
  {
    MEMORY32FIXED (ReadWrite, 0, GPIO_LENGTH, RMEM)
    Interrupt (ResourceConsumer, Level, ActiveHigh, Shared)
    {
      BCM2386_GPIO_INTERRUPT0, BCM2386_GPIO_INTERRUPT1,
      BCM2386_GPIO_INTERRUPT2, BCM2386_GPIO_INTERRUPT3
    }
  })
  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RMEM, RBAS, GPIO_OFFSET)
    Return (^RBUF)
  }
}

// Description: I2C
Device (I2C1)
{
  Name (_HID, "BCM2841")
  Name (_CID, "BCM2841")
  Name (_UID, 0x1)
  Name (_CCA, 0x0)

  Name (RBUF, ResourceTemplate ()
  {
    MEMORY32FIXED (ReadWrite, 0, BCM2836_I2C1_LENGTH, RMEM)
    Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { BCM2836_I2C1_INTERRUPT }
    PinFunction (Exclusive, PullUp, BCM_ALT0, "\\_SB.GDV0.GPI0", 0, ResourceConsumer, , ) { 2, 3 }
  })
  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RMEM, RBAS, BCM2836_I2C1_OFFSET)
    Return (^RBUF)
  }
}

// I2C2 is the HDMI DDC connection
Device (I2C2)
{
  Name (_HID, "BCM2841")
  Name (_CID, "BCM2841")
  Name (_UID, 0x2)
  Name (_CCA, 0x0)

  Name (RBUF, ResourceTemplate()
  {
    MEMORY32FIXED (ReadWrite, 0, BCM2836_I2C2_LENGTH, RMEM)
    Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { BCM2836_I2C2_INTERRUPT }
  })

  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RMEM, RBAS, BCM2836_I2C2_OFFSET)
    Return (^RBUF)
  }
}

// SPI
Device (SPI0)
{
  Name (_HID, "BCM2838")
  Name (_CID, "BCM2838")
  Name (_UID, 0x0)
  Name (_CCA, 0x0)

  Name (RBUF, ResourceTemplate ()
  {
    MEMORY32FIXED (ReadWrite, 0, BCM2836_SPI0_LENGTH, RMEM)
    Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { BCM2836_SPI0_INTERRUPT }
    PinFunction (Exclusive, PullDown, BCM_ALT0, "\\_SB.GDV0.GPI0", 0, ResourceConsumer, , ) { 9, 10, 11 } // MISO, MOSI, SCLK
    PinFunction (Exclusive, PullUp, BCM_ALT0, "\\_SB.GDV0.GPI0", 0, ResourceConsumer, , ) { 8 } // CE0
    PinFunction (Exclusive, PullUp, BCM_ALT0, "\\_SB.GDV0.GPI0", 0, ResourceConsumer, , ) { 7 } // CE1
  })

  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RMEM, RBAS, BCM2836_SPI0_OFFSET)
    Return (^RBUF)
  }
}

Device (SPI1)
{
  Name (_HID, "BCM2839")
  Name (_CID, "BCM2839")
  Name (_UID, 0x1)
  Name (_CCA, 0x0)
  Name (_DEP, Package() { \_SB.GDV0.RPIQ })

  Name (RBUF, ResourceTemplate ()
  {
    MEMORY32FIXED (ReadWrite, 0, BCM2836_SPI1_LENGTH, RMEM)
    Interrupt (ResourceConsumer, Level, ActiveHigh, Shared,) { BCM2836_SPI1_INTERRUPT }
    PinFunction (Exclusive, PullDown, BCM_ALT4, "\\_SB.GDV0.GPI0", 0, ResourceConsumer, , ) { 19, 20, 21 } // MISO, MOSI, SCLK
    PinFunction (Exclusive, PullDown, BCM_ALT4, "\\_SB.GDV0.GPI0", 0, ResourceConsumer, , ) { 16 } // CE2
  })

  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RMEM, RBAS, BCM2836_SPI1_OFFSET)
    Return (^RBUF)
  }
}

// SPI2 has no pins on GPIO header
// Device (SPI2)
// {
//   Name (_HID, "BCM2839")
//   Name (_CID, "BCM2839")
//   Name (_UID, 0x2)
//   Name (_CCA, 0x0)
//   Name (_DEP, Package() { \_SB.GDV0.RPIQ })
//   Method (_STA)
//   {
//     Return (0xf)     // Disabled
//   }
//   Method (_CRS, 0x0, Serialized)
//   {
//     Name (RBUF, ResourceTemplate ()
//     {
//       MEMORY32FIXED (ReadWrite, BCM2836_SPI2_BASE_ADDRESS, BCM2836_SPI2_LENGTH, RMEM)
//       Interrupt (ResourceConsumer, Level, ActiveHigh, Shared,) { BCM2836_SPI2_INTERRUPT }
//     })
//     Return (RBUF)
//   }
// }

// PWM Driver
Device (PWM0)
{
  Name (_HID, "BCM2844")
  Name (_CID, "BCM2844")
  Name (_UID, 0)
  Name (_CCA, 0x0)

  Name (RBUF, ResourceTemplate ()
  {
    // DMA channel 11 control
    MEMORY32FIXED (ReadWrite, 0, BCM2836_PWM_DMA_LENGTH, RM01)
    // PWM control
    MEMORY32FIXED (ReadWrite, 0, BCM2836_PWM_CTRL_LENGTH, RM02)
    // PWM control bus
    MEMORY32FIXED (ReadWrite, BCM2836_PWM_BUS_BASE_ADDRESS, BCM2836_PWM_BUS_LENGTH, )
    // PWM control uncached
    MEMORY32FIXED (ReadWrite, BCM2836_PWM_CTRL_UNCACHED_BASE_ADDRESS, BCM2836_PWM_CTRL_UNCACHED_LENGTH, )
    // PWM clock control
    MEMORY32FIXED (ReadWrite, 0, BCM2836_PWM_CLK_LENGTH, RM03)
    // Interrupt DMA channel 11
    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_DMA_INTERRUPT }
    // DMA channel 11, DREQ 5 for PWM
    FixedDMA (5, 11, Width32Bit, )
  })

  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RM01, RB01, BCM2836_PWM_DMA_OFFSET)
    MEMORY32SETBASE (RBUF, RM02, RB02, BCM2836_PWM_CTRL_OFFSET)
    MEMORY32SETBASE (RBUF, RM03, RB03, BCM2836_PWM_CLK_OFFSET)
    Return (^RBUF)
  }
}