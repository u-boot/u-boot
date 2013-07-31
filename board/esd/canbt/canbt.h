/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/****************************************************************************
 * FLASH Memory Map as used by TQ Monitor:
 *
 *                          Start Address    Length
 * +-----------------------+ 0x4000_0000     Start of Flash -----------------
 * | MON8xx code           | 0x4000_0100     Reset Vector
 * +-----------------------+ 0x400?_????
 * | (unused)              |
 * +-----------------------+ 0x4001_FF00
 * | Ethernet Addresses    |                 0x78
 * +-----------------------+ 0x4001_FF78
 * | (Reserved for MON8xx) |                 0x44
 * +-----------------------+ 0x4001_FFBC
 * | Lock Address          |                 0x04
 * +-----------------------+ 0x4001_FFC0                     ^
 * | Hardware Information  |                 0x40            | MON8xx
 * +=======================+ 0x4002_0000 (sector border)    -----------------
 * | Autostart Header      |                                 | Applications
 * | ...                   |                                 v
 *
 *****************************************************************************/
