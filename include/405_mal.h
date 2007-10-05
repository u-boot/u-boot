/* include/mal.h, openbios_walnut, walnut_bios 8/6/99 08:48:40 */
/*----------------------------------------------------------------------------+
|
|	This source code has been made available to you by IBM on an AS-IS
|	basis.	Anyone receiving this source is licensed under IBM
|	copyrights to use it in any way he or she deems fit, including
|	copying it, modifying it, compiling it, and redistributing it either
|	with or without modifications.	No license under IBM patents or
|	patent applications is to be implied by the copyright license.
|
|	Any user of this software should understand that IBM cannot provide
|	technical support for this software and will not be responsible for
|	any consequences resulting from the use of this software.
|
|	Any person who transfers this source code or any derivative work
|	must include the IBM copyright notice, this paragraph, and the
|	preceding two paragraphs in the transferred software.
|
|	COPYRIGHT   I B M   CORPORATION 1999
|	LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
+----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
|
|  File Name:	mal.h
|
|  Function:	Header file for the MAL (MADMAL) macro on the 405GP.
|
|  Author:	Mark Wisner
|
|  Change Activity-
|
|  Date	       Description of Change					   BY
|  ---------   ---------------------					   ---
|  29-Apr-99   Created							   MKW
|
+----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
|  17-Nov-03  Travis B. Sawyer, Sandburst Corporation, tsawyer@sandburst.com
|	      Added register bit definitions to support multiple channels
+----------------------------------------------------------------------------*/
#ifndef _mal_h_
#define _mal_h_
/* MADMAL transmit and receive status/control bits  */
/* for COMMAC bits, refer to the COMMAC header file */

#define MAL_TX_CTRL_READY 0x8000
#define MAL_TX_CTRL_WRAP  0x4000
#define MAL_TX_CTRL_CM	  0x2000
#define MAL_TX_CTRL_LAST  0x1000
#define MAL_TX_CTRL_INTR  0x0400

#define MAL_RX_CTRL_EMPTY 0x8000
#define MAL_RX_CTRL_WRAP  0x4000
#define MAL_RX_CTRL_CM	  0x2000
#define MAL_RX_CTRL_LAST  0x1000
#define MAL_RX_CTRL_FIRST 0x0800
#define MAL_RX_CTRL_INTR  0x0400

      /* Configuration Reg  */
#define MAL_CR_MMSR	  0x80000000
#define MAL_CR_PLBP_1	  0x00400000   /* lowsest is 00 */
#define MAL_CR_PLBP_2	  0x00800000
#define MAL_CR_PLBP_3	  0x00C00000   /* highest	*/
#define MAL_CR_GA	  0x00200000
#define MAL_CR_OA	  0x00100000
#define MAL_CR_PLBLE	  0x00080000
#define MAL_CR_PLBLT_1	0x00040000
#define MAL_CR_PLBLT_2	0x00020000
#define MAL_CR_PLBLT_3	0x00010000
#define MAL_CR_PLBLT_4	0x00008000
#define MAL_CR_PLBLT_DEFAULT 0x00078000 /* ????? */
#define MAL_CR_PLBB	  0x00004000
#define MAL_CR_OPBBL	  0x00000080
#define MAL_CR_EOPIE	  0x00000004
#define MAL_CR_LEA	  0x00000002
#define MAL_CR_MSD	  0x00000001

    /* Error Status Reg	   */
#define MAL_ESR_EVB	  0x80000000
#define MAL_ESR_CID	  0x40000000
#define MAL_ESR_DE	  0x00100000
#define MAL_ESR_ONE	  0x00080000
#define MAL_ESR_OTE	  0x00040000
#define MAL_ESR_OSE	  0x00020000
#define MAL_ESR_PEIN	  0x00010000
      /* same bit position as the IER */
      /* VV			 VV   */
#define MAL_ESR_DEI	  0x00000010
#define MAL_ESR_ONEI	  0x00000008
#define MAL_ESR_OTEI	  0x00000004
#define MAL_ESR_OSEI	  0x00000002
#define MAL_ESR_PBEI	  0x00000001
      /* ^^			 ^^   */
      /* Mal IER		      */
#if defined(CONFIG_440SPE) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_405EX)
#define MAL_IER_PT	  0x00000080
#define MAL_IER_PRE	  0x00000040
#define MAL_IER_PWE	  0x00000020
#define MAL_IER_DE	  0x00000010
#define MAL_IER_OTE	  0x00000004
#define MAL_IER_OE	  0x00000002
#define MAL_IER_PE	  0x00000001
#else
#define MAL_IER_DE	  0x00000010
#define MAL_IER_NE	  0x00000008
#define MAL_IER_TE	  0x00000004
#define MAL_IER_OPBE	  0x00000002
#define MAL_IER_PLBE	  0x00000001
#endif

/* MAL Channel Active Set and Reset Registers */
#define MAL_TXRX_CASR	(0x80000000)

#define MAL_TXRX_CASR_V(__x)  (__x)  /* Channel 0 shifts 0, channel 1 shifts 1, etc */


/* MAL Buffer Descriptor structure */
typedef struct {
  short	 ctrl;		    /* MAL / Commac status control bits */
  short	 data_len;	    /* Max length is 4K-1 (12 bits)	*/
  char	*data_ptr;	    /* pointer to actual data buffer	*/
} mal_desc_t;

#endif
