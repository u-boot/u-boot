/** @file
 *
 *  [DSDT] Serial devices (UART).
 *
 *  Copyright (c) 2021, ARM Limited. All rights reserved.
 *  Copyright (c) 2020, Pete Batard <pete@akeo.ie>
 *  Copyright (c) 2018, Andrey Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <asm/arch/acpi/bcm2836.h>

#include "acpitables.h"

// PL011 based UART.
Device (URT0)
{
  Name (_HID, "BCM2837")
  Name (_CID, "ARMH0011")
  Name (_UID, 0x4)
  Name (_CCA, 0x0)

  Name (RBUF, ResourceTemplate ()
  {
    MEMORY32FIXED (ReadWrite, 0, BCM2836_PL011_UART_LENGTH, RMEM)
    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { BCM2836_PL011_UART_INTERRUPT }
  })
  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RMEM, RBAS, BCM2836_PL011_UART_OFFSET)
    Return (^RBUF)
  }

  Name (CLCK, 48000000)

  Name (_DSD, Package ()
  {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"), Package ()
    {
      Package (2) { "clock-frequency", CLCK },
    }
  })
}

//
// UART Mini.
//
// This device is referenced in the DBG2 table, which will cause the system to
// not start the driver when the debugger is enabled and to mark the device
// with problem code 53 (CM_PROB_USED_BY_DEBUGGER).
//

Device (URTM)
{
  Name (_HID, "BCM2836")
  Name (_CID, "BCM2836")
  Name (_UID, 0x0)
  Name (_CCA, 0x0)

  Name (RBUF, ResourceTemplate ()
  {
    MEMORY32FIXED (ReadWrite, 0, BCM2836_MINI_UART_LENGTH, RMEM)
    Interrupt(ResourceConsumer, Level, ActiveHigh, Shared) { BCM2836_MINI_UART_INTERRUPT }

  })
  Method (_CRS, 0x0, Serialized)
  {
    MEMORY32SETBASE (RBUF, RMEM, RBAS, BCM2836_MINI_UART_OFFSET)
    Return (^RBUF)
  }

  //
  // Mini Uart Clock Rate will be dynamically updated during boot
  //
  External (\_SB.URTM.MUCR, IntObj)

  Name (_DSD, Package ()
  {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"), Package ()
    {
      Package (2) { "clock-frequency", MUCR },
    }
  })
}

//
// Multifunction serial bus device to support Bluetooth function.
//
Device(BTH0)
{
  Name (_HID, "BCM2EA6")
  Name (_CID, "BCM2EA6")

  //
  // UART In Use will be dynamically updated during boot
  //
  External (\_SB.BTH0.URIU, IntObj)

  Method (_STA)
  {
    Return (0xf)
  }

  //
  // Resource for URT0 (PL011)
  //
  Name (BTPL, ResourceTemplate ()
  {
    UARTSerialBus(
      115200,        // InitialBaudRate: in BPS
      ,              // BitsPerByte: default to 8 bits
      ,              // StopBits: Defaults to one bit
      0x00,          // LinesInUse: 8 1-bit flags to
                    //   declare enabled control lines.
                    //   Raspberry Pi does not exposed
                    //   HW control signals -> not supported.
                    //   Optional bits:
                    //   - Bit 7 (0x80) Request To Send (RTS)
                    //   - Bit 6 (0x40) Clear To Send (CTS)
                    //   - Bit 5 (0x20) Data Terminal Ready (DTR)
                    //   - Bit 4 (0x10) Data Set Ready (DSR)
                    //   - Bit 3 (0x08) Ring Indicator (RI)
                    //   - Bit 2 (0x04) Data Carrier Detect (DTD)
                    //   - Bit 1 (0x02) Reserved. Must be 0.
                    //   - Bit 0 (0x01) Reserved. Must be 0.
      ,              // IsBigEndian:
                    //   default to LittleEndian.
      ,              // Parity: Defaults to no parity
      ,              // FlowControl: Defaults to
                    //   no flow control.
      16,            // ReceiveBufferSize
      16,            // TransmitBufferSize
      "\\_SB.GDV0.URT0",  // ResourceSource:
                    //   UART bus controller name
      ,              // ResourceSourceIndex: assumed to be 0
      ,              // ResourceUsage: assumed to be
                    //   ResourceConsumer
      UAR0,          // DescriptorName: creates name
                    //   for offset of resource descriptor
    )                // Vendor data
  })

  //
  // Resource for URTM (miniUART)
  //
  Name (BTMN, ResourceTemplate ()
  {
    //
    // BT UART: ResourceSource will be dynamically updated to
    // either URT0 (PL011) or URTM (miniUART) during boot
    //
    UARTSerialBus(
      115200,        // InitialBaudRate: in BPS
      ,              // BitsPerByte: default to 8 bits
      ,              // StopBits: Defaults to one bit
      0x00,          // LinesInUse: 8 1-bit flags to
                    //   declare enabled control lines.
                    //   Raspberry Pi does not exposed
                    //   HW control signals -> not supported.
                    //   Optional bits:
                    //   - Bit 7 (0x80) Request To Send (RTS)
                    //   - Bit 6 (0x40) Clear To Send (CTS)
                    //   - Bit 5 (0x20) Data Terminal Ready (DTR)
                    //   - Bit 4 (0x10) Data Set Ready (DSR)
                    //   - Bit 3 (0x08) Ring Indicator (RI)
                    //   - Bit 2 (0x04) Data Carrier Detect (DTD)
                    //   - Bit 1 (0x02) Reserved. Must be 0.
                    //   - Bit 0 (0x01) Reserved. Must be 0.
      ,              // IsBigEndian:
                    //   default to LittleEndian.
      ,              // Parity: Defaults to no parity
      ,              // FlowControl: Defaults to
                    //   no flow control.
      16,            // ReceiveBufferSize
      16,            // TransmitBufferSize
      "\\_SB.GDV0.URTM",  // ResourceSource:
                    //   UART bus controller name
      ,              // ResourceSourceIndex: assumed to be 0
      ,              // ResourceUsage: assumed to be
                    //   ResourceConsumer
      UARM,          // DescriptorName: creates name
                    //   for offset of resource descriptor
    )                // Vendor data
  })

  Method (_CRS, 0x0, Serialized)
  {
    if (URIU == 0)
    {
      //
      // PL011 UART is configured for console output
      // Return Mini UART for Bluetooth
      //
      return (^BTMN)
    }
    else
    {
      //
      // Mini UART is configured for console output
      // Return PL011 UART for Bluetooth
      //
      return (^BTPL)
    }
  }
}
