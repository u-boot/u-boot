/*
 * Copyright (C) 2003 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Serial Presence Detect (SPD) EEPROM format according to the
 * Intel's PC SDRAM Serial Presence Detect (SPD) Specification,
 * revision 1.2B, November 1999
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _SPD_H_
#define _SPD_H_

typedef struct spd_eeprom_s {
   unsigned char info_size;   /* # of bytes written into serial memory           */
   unsigned char chip_size;   /* Total # of bytes of SPD memory device           */
   unsigned char mem_type;    /* Fundamental memory type (FPM, EDO, SDRAM...)    */
   unsigned char nrow_addr;   /* # of Row Addresses on this assembly             */
   unsigned char ncol_addr;   /* # of Column Addresses on this assembly          */
   unsigned char nrows;       /* # of Module Rows on this assembly               */
   unsigned char dataw_lsb;   /* Data Width of this assembly                     */
   unsigned char dataw_msb;   /* ... Data Width continuation                     */
   unsigned char voltage;     /* Voltage interface standard of this assembly     */
   unsigned char clk_cycle;   /* SDRAM Cycle time at CL=X                        */
   unsigned char clk_access;  /* SDRAM Access from Clock at CL=X                 */
   unsigned char config;      /* DIMM Configuration type (non-parity, ECC)       */
   unsigned char refresh;     /* Refresh Rate/Type                               */
   unsigned char primw;       /* Primary SDRAM Width                             */
   unsigned char ecw;         /* Error Checking SDRAM width                      */
   unsigned char min_delay;   /* Min Clock Delay for Back to Back Random Address */
   unsigned char burstl;      /* Burst Lengths Supported                         */
   unsigned char nbanks;      /* # of Banks on Each SDRAM Device                 */
   unsigned char cas_lat;     /* CAS# Latencies Supported                        */
   unsigned char cs_lat;      /* CS# Latency                                     */
   unsigned char write_lat;   /* Write Latency (also called Write Recovery time) */
   unsigned char mod_attr;    /* SDRAM Module Attributes                         */
   unsigned char dev_attr;    /* SDRAM Device Attributes                         */
   unsigned char clk_cycle2;  /* Min SDRAM Cycle time at CL=X-1                  */
   unsigned char clk_access2; /* SDRAM Access from Clock at CL=X-1               */
   unsigned char clk_cycle3;  /* Min SDRAM Cycle time at CL=X-2                  */
   unsigned char clk_access3; /* Max SDRAM Access from Clock at CL=X-2           */
   unsigned char trp;         /* Min Row Precharge Time (tRP)                    */
   unsigned char trrd;        /* Min Row Active to Row Active (tRRD)             */
   unsigned char trcd;        /* Min RAS to CAS Delay (tRCD)                     */
   unsigned char tras;        /* Minimum RAS Pulse Width (tRAS)                  */
   unsigned char row_dens;    /* Density of each row on module                   */
   unsigned char ca_setup;    /* Command and Address signal input setup time     */
   unsigned char ca_hold;     /* Command and Address signal input hold time      */
   unsigned char data_setup;  /* Data signal input setup time                    */
   unsigned char data_hold;   /* Data signal input hold time                     */
   unsigned char sset[26];    /* Superset Information (may be used in future)    */
   unsigned char spd_rev;     /* SPD Data Revision Code                          */
   unsigned char cksum;       /* Checksum for bytes 0-62                         */
   unsigned char mid[8];      /* Manufacturer's JEDEC ID code per JEP-108E       */
   unsigned char mloc;        /* Manufacturing Location                          */
   unsigned char mpart[18];   /* Manufacturer's Part Number                      */
   unsigned char rev[2];      /* Revision Code                                   */
   unsigned char mdate[2];    /* Manufacturing Date                              */
   unsigned char sernum[4];   /* Assembly Serial Number                          */
   unsigned char mspec[27];   /* Manufacturer Specific Data                      */
   unsigned char freq;        /* Intel specification frequency                   */
   unsigned char intel_cas;   /* Intel Specification CAS# Latency support        */
} spd_eeprom_t;

#endif /* _SPD_H_ */
