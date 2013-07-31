/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Memory map:
 *
 *   ff100000 -> ff13ffff : FPGA        CS1
 *   ff030000 -> ff03ffff : EXPANSION   CS7
 *   ff020000 -> ff02ffff : DATA FLASH  CS4
 *   ff018000 -> ff01ffff : UART B      CS6/UPMB
 *   ff010000 -> ff017fff : UART A      CS5/UPMB
 *   ff000000 -> ff00ffff : IMAP                   internal to the MPC855T
 *   f8000000 -> fbffffff : FLASH       CS0        up to 64MB
 *   f4000000 -> f7ffffff : NVSRAM      CS2        up to 64MB
 *   00000000 -> 0fffffff : SDRAM       CS3/UPMA   up to 256MB
 */
