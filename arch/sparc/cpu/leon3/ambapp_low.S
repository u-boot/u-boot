/* GRLIB AMBA Plug&Play information scanning implemented without
 * using memory (stack) and one register window. The code scan
 * the PnP info and inserts the AHB bridges/buses into register
 * i0-i5.
 * The code support
 *  - up to 6 AHB buses
 *  - multiple APB buses
 *  - support for AHB2AHB & L2CACHE bridges
 *
 * (C) Copyright 2010, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <ambapp.h>

	.seg	"text"
	.globl	_nomem_amba_init
	.globl	_nomem_ambapp_find_buses
	.globl	_nomem_find_apb
	.globl	_nomem_find_ahb

/* Overview
 * ========
 *
 * _nomem_amba_init         - Init AMBA bus and calls _nomem_ambapp_find_buses
 * _nomem_ambapp_find_buses - Scan AMBA PnP info for AHB buses/bridges and
 *                            place them in i0-i5, see below
 * _nomem_find_apb          - Find one APB device identified by VENDOR:DEVICE
 *                            ID and an index.
 * _nomem_find_ahb          - Find one AHB Master or Slave device identified
 *                            by VENDOR:DEVICE ID and an index.
 * init_ahb_bridges         - Local function. Clears i0-i5
 * insert_ahb_bridge        - Local function. Insert a new AHB bus into first
 *                            free register in i0-i5. It also checks that the
 *                            bus has not already been added.
 * get_ahb_bridge           - Local function. Get AHB bus from registers,
 *                            return register iN, where N is defined by o0.
 *
 * The _nomem_find_apb and _nomem_find_ahb function requires that i0-i5
 * are populated with the AHB buses of the system. The registers are
 * initialized by _nomem_ambapp_find_buses.
 *
 * AHB Bus result and requirements of i0-i5
 * ========================================
 *
 * i0: AHB BUS0 IOAREA, no parent bus
 * i1: AHB BUS1 IOAREA, parent bus is always i0 (AHB BUS0) and bridge address
 * i2: AHB BUS2 IOAREA, 3-bit parent bus number and bridge address
 * i3: AHB BUS3 IOAREA, 3-bit parent bus number and bridge address
 * i4: AHB BUS4 IOAREA, 3-bit parent bus number and bridge address
 * i5: AHB BUS5 IOAREA, 3-bit parent bus number and bridge address
 *
 * AHB BUS
 * -------
 * Bits 31-20 (0xfff00000) contain the found bus I/O Area (AHB PnP area).
 *
 * 3-bit Parent bus
 * ----------------
 * Bits 2-0 (0x00000007) contain parent bus number. Zero if no parent
 * bus, 1 = parent is AHB BUS 0 (i0), 2 = parent is AHB BUS 1 (i1)..
 *
 * Bridge Address
 * --------------
 * Bits 10-5 (0x000007e0) contain the index of the Bridge's PnP
 * information on the parent. Since all bridges are found in the
 * PnP information they all have a PnP entry. Together with the
 * parent bus number the PnP entry can be found:
 *  PnPEntry = (BRIDGE_ADDRESS + (iN & 0xfff00000)) | 0x000ff800
 *  where N is the parent bus minus one.
 *
 */

/* Function initializes the AHB Bridge I/O AREA storage. (Clears i0-i5)
 *
 * Arguments
 *  none
 *
 * Results
 *  none
 *
 * Clobbered
 *  none
 */

init_ahb_bridges:
	mov	%g0, %i0
	mov	%g0, %i1
	mov	%g0, %i2
	mov	%g0, %i3
	mov	%g0, %i4
	retl
	 mov	%g0, %i5

/* Function returns AHB Bridge I/O AREA for specified bus.
 *
 * Arguments
 *  - o0 = bus number
 *
 * Results
 *  - o0 = I/O AREA
 *
 * Clobbered
 *  none
 */
get_ahb_bridge:
	cmp	%o0, 1
	be,a	L1
	 mov	%i0, %o0

	cmp	%o0, 2
	be,a	L1
	 mov	%i1, %o0

	cmp	%o0, 3
	be,a	L1
	 mov	%i2, %o0

	cmp	%o0, 4
	be,a	L1
	 mov	%i3, %o0

	cmp	%o0, 5
	be,a	L1
	 mov	%i4, %o0

	cmp	%o0, 6
	be,a	L1
	 mov	%i5, %o0

	/* o0 > 6: only 6 buses supported */
	mov	%g0, %o0
L1:
	retl
	 nop

/* Function adds a AHB Bridge I/O AREA to the i0-i5 registers if
 * not already added. It stores the bus PnP start information.
 *
 * Arguments
 *  - o0 = AHB Bridge I/O area
 *
 * Results
 *  none
 *
 * Clobbered
 *  o2, o3
 */
insert_ahb_bridge:
	/* Check that bridge hasn't already been added */
	andn	%o0, 0x7ff, %o2
	andn	%i0, 0x7ff, %o3
	cmp	%o3, %o2
	be	L2
	 andn	%i1, 0x7ff, %o3
	cmp	%o3, %o2
	be	L2
	 andn	%i2, 0x7ff, %o3
	cmp	%o3, %o2
	be	L2
	 andn	%i3, 0x7ff, %o3
	cmp	%o3, %o2
	be	L2
	 andn	%i4, 0x7ff, %o3
	cmp	%o3, %o2
	be	L2
	 andn	%i5, 0x7ff, %o3
	cmp	%o3, %o2
	be	L2

	/* Insert into first free posistion */
	 cmp	%i0, %g0
	be,a	L2
	 mov	%o0, %i0

	cmp	%i1, %g0
	be,a	L2
	 mov	%o0, %i1

	cmp	%i2, %g0
	be,a	L2
	 mov	%o0, %i2

	cmp	%i3, %g0
	be,a	L2
	 mov	%o0, %i3

	cmp	%i4, %g0
	be,a	L2
	 mov	%o0, %i4

	cmp	%i5, %g0
	be,a	L2
	 mov	%o0, %i5
L2:
	retl
	 nop

/* FUNCTION int _nomem_find_ahb_bus(
 *	unsigned int bridge,
 *	int vendor_device,
 *	int index,
 *	void **pconf,
 *	int not_used,
 *	int option
 *	)
 *
 * Scans the AHB Master or Slave area for a matching VENDOR:DEVICE, the
 * index is decremented when a matching device is found but index is
 * greater than zero. When index is zero and a matching DEVICE:VENDOR
 * is found the AHB configuration address and AHB I/O area is returned.
 *
 * i0-i7,l0,l1,l2,l3,l4,g2,o6 is not available for use.
 * o1,o5 Must be left untouched
 *
 * Results
 *  - o0 Number of found devices (1 or 0)
 *  - o2 is decremented for each matching VENDOR:DEVICE found, zero if found
 *  - o3 Address of the AHB PnP configuration entry (Only valid if o0=1)
 *
 * Clobbered
 *  - o3 (Clobbered when no device was found)
 *  - o4 (Number of Devices left to search)
 *  - o0 (Bus ID, PnP ID, Device)
 */
_nomem_find_ahb_bus:

	/* Get the number of Slaves/Masters.
	 * Only AHB Bus 0 has 64 AHB Masters/Slaves the
	 * other AHB buses has 16 slaves and 16 masters.
	 */
	add	%g0, 16, %o4		/* Defaulting to 16 */
	andcc	%o0, 0x7, %g0		/* 3-bit bus id */
	be,a	.L_maxloops_detected
	 add	%g0, 64, %o4		/* AHB Bus 0 has 64 AHB Masters/Slaves */
.L_maxloops_detected:

	/* Get start address of AHB Slave or AHB Master area depending on what
	 * we are searching for.
	 */
	andn	%o0, 0x7ff, %o0		/* Remove Bus ID and 5-bit AHB/AHB
					 * Bridge PnP Address to get I/O Area */
	set	AMBA_CONF_AREA,	%o3
	or	%o3, %o0, %o3		/* Master area address */

	cmp	%o5, DEV_AHB_SLV
	be,a	.L_conf_area_calculated
	 or	%o3, AMBA_AHB_SLAVE_CONF_AREA, %o3	/* Add 0x800 to get to slave area */
.L_conf_area_calculated:

	/* Iterate over all AHB device and try to find matching DEVICE:VENDOR
	 * o1 - VENDOR|DEVICE
	 * o2 - Index
	 * o3 - Current AHB Device Configuration address
	 * o5 - Type (leave untouched)
	 *
	 * o4 - Number of AHB device left to process
	 * o0 - tmp
	 */
.L_process_one_conf:
	ld	[%o3], %o0
	andn	%o0, 0xfff, %o0
	cmp	%o0, 0			/* No device if zero */
	beq	.L_next_conf
	 cmp	%o1, 0			/* If VENDOR:DEVICE==0, consider all matching */
	beq	.L_process_ahb_dev_found
	 cmp	%o0, %o1		/* Does VENDOR and DEVICE Match? */
	bne	.L_next_conf
	 nop
.L_process_ahb_dev_found:
	/* Found a Matching VENDOR:DEVICE, index must also match */
	cmp	%o2, %g0
	bne	.L_next_conf
	 dec	%o2
	/* Index matches also, return happy with o3 set to AHB Conf Address */
	mov	%g0, %o2
	retl
	 add	%g0, 1, %o0

.L_next_conf:
	subcc	%o4, 1, %o4		/* One device has been processed,
					 * Are there more devices to process? */
	bne	.L_process_one_conf
	 add	%o3, AMBA_AHB_CONF_LENGH, %o3	/* Next Configuration entry */
	/* No Matching device found */
	retl
	 mov	%g0, %o0

/* FUNCTION int _nomem_find_ahb(
 *      int unused,
 *	int vendor_device,
 *	int index,
 *	void **pconf,
 *	int *ahb_bus_index,
 *	int option,
 *	)
 *
 * Find a AHB Master or AHB Slave device, it puts the address of the AHB PnP
 * configuration in o3 (pconf), the I/O Area base address in o4 (pioarea).
 *
 * Calls _nomem_find_ahb_bus for every AHB bus.
 *
 * i0-i7, l0, l1, o6, g1, g4-g7 is not available for use.
 *
 * Arguments
 *  - o0 Unused
 *
 * Results
 *  - o0 Number of found devices (1 or 0)
 *  - o2 Decremented Index (Zero if found)
 *  - o3 Address of the AHB PnP configuration entry
 *  - o4 AHB Bus index the device was found on (if o0=1)
 *  - o5 Left untouched
 *
 * Clobbered
 *  - o0 (AHB Bridge and used by _nomem_find_ahb_bus)
 *  - o2 (index is decremented)
 *  - l2 (Current AHB Bus index)
 *  - g2 (return address)
 */
_nomem_find_ahb:
	mov	%o7, %g2		/* Save return address */
	/* Scan all AHB Buses found for the AHB Master/Slave matching VENDOR:DEVICE */
	clr	%l2
.L_search_next_ahb_bus:
	add	%l2, 1, %l2
	call	get_ahb_bridge			/* Get bus %l0 I/O Area */
	 mov	%l2, %o0
	cmp	%o0, %g0
	be	.L_no_device_found		/* If no more AHB bus is left to be scanned, proceed */
	 nop
	call	_nomem_find_ahb_bus		/* Scan AHB bus %o0 for VENDOR:DEVICE. Index in o3 is decremented  */
	 nop
	cmp	%o0, %g0			/* If VENDOR:DEVICE was not found scan next AHB Bus */
	be	.L_search_next_ahb_bus		/* Do next bus is o0=0 (not found) */
	 nop
	/* The device was found, o0 is 1 */
	mov	%g2, %o7		/* Restore return address */
	retl
	 mov	%l2, %o4		/* The AHB bus index the device was found on */

	/* No device found matching */
.L_no_device_found:
	mov	%g2, %o7		/* Restore return address */
	retl
	 mov	%g0, %o0


/* FUNCTION int _nomem_find_apb_bus(
 *      int apbmst,
 *	int vendor_device,
 *	int index,
 *	void **pconf
 *	)
 *
 * Find a APB Slave device, it puts the address of the APB PnP configuration
 * in o3 (pconf).
 *
 * Calls _nomem_find_ahb_bus for every AHB bus searching for AHB/APB Bridges.
 * The AHB/APB bridges are AHB Slaves with ID GAISLER_APBMST.
 *
 * Results
 *  - o0 Number of found devices (1 or 0)
 *  - o2 Decremented Index
 *  - o3 Address of the found APB device PnP configuration entry
 *
 * Clobbered
 *  - o5 PnP VENDOR:DEVICE ID
 */

_nomem_find_apb_bus:
	set	AMBA_CONF_AREA, %o3
	or	%o0, %o3, %o3		/* Calc start of APB device PnP info */
	add	%g0, 16, %o0		/* o0, number of APB Slaves left to scan */
.L_process_one_apb_conf:
	ld	[%o3], %o5
	andn	%o5, 0xfff, %o5
	cmp	%o5, 0			/* No device if zero */
	beq	.L_process_apb_dev_not_found
	 cmp	%o1, 0			/* If VENDOR:DEVICE == -1, consider all matching */
	beq	.L_process_apb_dev_found
	 cmp	%o1, %o5		/* Found VENDOR:DEVICE */
	bne	.L_process_apb_dev_not_found
	 nop

.L_process_apb_dev_found:
	/* Found matching device, compare index */
	cmp	%o2, %g0
	bne	.L_process_apb_dev_not_found
	 dec	%o2
	/* Matching index and VENDOR:DEVICE */
	retl
	 add	%g0, 1, %o0

.L_process_apb_dev_not_found:
	subcc	%o0, 1, %o0
	bne	.L_process_one_apb_conf
	 add	%o3, 8, %o3
	retl
	 mov	%g0, %o0

/* FUNCTION int _nomem_find_apb(
 *      int unused,
 *	int vendor_device,
 *	int index,
 *	void **pconf,
 *	int *ahb_bus_index
 *	)
 *
 * Find a APB Slave device, it puts the address of the APB PnP configuration
 * in o3 (pconf), the APB Master I/O Area base address in o4 (papbarea).
 *
 * Calls _nomem_find_ahb_bus for every AHB bus searching for AHB/APB Bridges.
 * The AHB/APB bridges are AHB Slaves with ID GAISLER_APBMST.
 *
 * i0-i7, l0, l1, o6 is not available for use.
 *
 * Arguments
 *  - o0 Unused
 *
 * Results
 *  - o0 Number of found devices (1 or 0)
 *  - o2 Decremented Index if not found
 *  - o3 Address of the APB PnP configuration entry
 *  - o4 AHB Bus index of APB Bridge/APB Device
 *
 * Clobbered
 *  - o0 (AHB Bridge)
 *  - o2 (index is decremented)
 *  - l2 (APB DEV Index [7..4] : APBMST AHB Index [3..0])
 *  - l3 (Current AHB Bus index)
 *  - l4 (temporary storage for APB VENDOR:DEVICE)
 *  - o5 (AHB Slave ID)
 *  - o0 (clobbered by _nomem_find_ahb_bus)
 *  - g2 (Return address)
 */
_nomem_find_apb:
	/* Scan all AHB Buses found for AHB/APB Bridges */
	mov	%o7, %g2		/* Save return address */
	mov	%o1, %l4		/* Save APB VENDOR:DEVICE */
	sll	%o2, 4, %l2		/* APB MST index = 0 */
	add	%g0, 1, %l3		/* AHB Bus index = 0 */
.L2_search_next_ahb_bus:
	call	get_ahb_bridge		/* Get bus %l3 I/O Area */
	 mov	%l3, %o0
	cmp	%o0, %g0
	be	.L2_no_device_found	/* If no more AHB bus is left to be scanned, proceed */
	 add	%g0, DEV_AHB_SLV, %o5	/* Search for AHB Slave */
	sethi	%hi(AMBA_PNP_ID(VENDOR_GAISLER, GAISLER_APBMST)), %o1
	call	_nomem_find_ahb_bus	/* Scan AHB bus %o0 for VENDOR:DEVICE. Index in o3 is decremented */
	 and	%l2, 0xf, %o2		/* Set APBMST index */
	cmp	%o0, %g0		/* If no AHB/APB Bridge was not found, scan next AHB Bus */
	be	.L_no_apb_bridge_found	/* Do next bus */
	 nop

	/* The AHB/APB Bridge was found.
         * Search for the requested APB Device on the APB bus using
	 * find_apb_bus, it will decrement the index.
         */
	ld	[%o3 + AMBA_AHB_MBAR0_OFS], %o3
	sll	%o3, 16, %o0
	and	%o0, %o3, %o0		/* Address AND Address Mask */
	sethi	%hi(0xfff00000), %o3
	and	%o0, %o3, %o0		/* AHB/APB Bridge address */

	srl	%l2, 4, %o2		/* APB DEV Index */
	call	_nomem_find_apb_bus
	 mov	%l4, %o1		/* APB VENDOR:DEVICE */
	cmp	%o0, %g0
	be	.L_apb_dev_not_found
	 mov	%g2, %o7		/* Restore return address */
	/* APB Device found
	 * o0 1
	 * o2 Index is decremented to zero
	 * o3 APB configuration address,
	 * o4 APB Bridge Configuration address.
	 */
	mov	%g0, %o2
	retl
	 mov	%l3, %o4

.L_apb_dev_not_found:
	/* Update APB DEV Index by saving output from find_apb_bus
	 * (index parameter) into bits [31..4] in L2.
	 */
	sll	%o2, 4, %o2
	and	%l2, 0xf, %l2
	or	%o2, %l2, %l2
	/* Try finding the next AHB/APB Bridge on the same AHB bus
	 * to find more APB devices
	 */
	ba	.L2_search_next_ahb_bus	/* Find next AHB/APB bridge */
	 inc	%l2

.L_no_apb_bridge_found:
	inc	%l3			/* Next AHB Bus */
	ba	.L2_search_next_ahb_bus	/* Process next AHB bus */
	 andn	%l2, 0xf, %l2		/* Start at APB Bridge index 0 at every AHB Bus */
	/* No device found matching */
.L2_no_device_found:
	mov	%g2, %o7		/* Restore return address */
	srl	%l2, 4, %o2		/* APB DEV Index */
	retl
	 mov	%g0, %o0



/* FUNCTION _nomem_amba_scan_gaisler_ahb2ahb_bridge(unsigned int bridge, int bus)
 *
 * Constraints:
 *   - o1 may not be used
 *   - o0, o2, o3 may be used.
 *
 * Arguments
 *  - o0 PnP Address of Bridge AHB device
 *  - o2 PnP ID of AHB device
 *
 * Results
 *  - o0 Address of new bus PnP area or a 1 if AHB device is no bridge
 *
 * Clobbered
 *   - o0, o2
 *
 */
_nomem_amba_scan_gaisler_ahb2ahb_bridge:
	andn	%o2, 0xfff, %o2
	sethi	%hi(AMBA_PNP_ID(VENDOR_GAISLER,GAISLER_AHB2AHB)), %o3
	cmp	%o2, %o3
	beq	.L_is_ahb2ahb_bridge
	 nop

	retl
	 add	%g0, 1, %o0

.L_is_ahb2ahb_bridge:
	/* Found a GAISLER AHB2AHB bridge */
	retl
	 ld	[%o0 + AMBA_AHB_CUSTOM1_OFS], %o0 /* Get address of bridge PnP area */


/* FUNCTION _nomem_amba_scan_gaisler_l2cache_bridge(unsigned int bridge, int bus)
 *
 * Constraints:
 *   - o1 may not be used
 *   - o0, o2, o3 may be used.
 *
 * Arguments
 *  - o0 PnP Address of Bridge AHB device
 *  - o2 PnP ID of AHB device
 *
 * Results
 *  - o0 Address of new bus PnP area or a 1 if AHB device is no bridge
 *
 * Clobbered
 *   - o0, o2
 *
 */
_nomem_amba_scan_gaisler_l2cache_bridge:
	andn	%o2, 0xfff, %o2
	sethi	%hi(AMBA_PNP_ID(VENDOR_GAISLER,GAISLER_L2CACHE)), %o3
	cmp	%o2, %o3
	beq	.L_is_l2cache_bridge
	 nop

	retl
	 add	%g0, 1, %o0

.L_is_l2cache_bridge:
	/* Found a GAISLER l2cache bridge */
	retl
	 ld	[%o0 + AMBA_AHB_CUSTOM1_OFS], %o0 /* Get address of bridge PnP area */


/* FUNCTION _nomem_amba_scan(unsigned int bridge, int bus)
 *
 * Constraints:
 *  i0-i7, l0 is used by caller
 *  o5-o7 may not be used.
 *
 * Arguments
 *  - o0 Bridge Information: I/O AREA and parent bus
 *  - o1 Bus
 *
 * Results
 *  - o0 Number of AHB bridges found
 *
 * Clobbered
 *  - o0 (Current AHB slave conf address)
 *  - o2 (Used by insert_bridge)
 *  - o3 (Used by insert_bridge)
 *  - l1 (Number of AHB Slaves left to process)
 *  - l2 (Current AHB slave conf address)
 *  - g2 (Return address)
 */
_nomem_amba_scan:
	mov	%o7, %g2	/* Save return address */
	set	16, %l1
	cmp	%o1, 1
	be,a	.L2_maxloops_detected
	 add	%g0, 64, %l1
.L2_maxloops_detected:

	/* Clear 3-bit parent bus from bridge to get I/O AREA, then or
	 * (AMBA_CONF_AREA | AMBA_AHB_SLAVE_CONF_AREA) to get first AHB slave
	 * conf address.
	 */
	andn	%o0, 0x7ff, %o0
	set	(AMBA_CONF_AREA | AMBA_AHB_SLAVE_CONF_AREA), %l2
	or	%o0, %l2, %l2

	/* Scan AHB Slave area for AHB<->AHB bridges. For each AHB device
	 * all "bridge drivers" are called, the driver function interface:
	 *
	 * Input:
	 *   - o0 PnP Address of Bridge AHB device
	 *   - o2 PnP ID of AHB device
	 * Return values:
	 *   - o0 Address of new bus PnP area, returning a 1 in o2 means not found
	 *
	 * Constraints:
	 *   - o1 may not be used
	 *   - o0, o2, o3 may be used.
	 *
	 */
.L_scan_one_ahb_slave:
	ld	[%l2], %o2

	cmp	%o2, %g0
	beq	.L_scan_next_ahb_slave
	 nop

	/* Call the GAISLER AHB2AHB bridge driver */
	call	_nomem_amba_scan_gaisler_ahb2ahb_bridge
	 mov	%l2, %o0
	cmp	%o0, 1
	bne	.L_found_bridge
	 ld	[%l2], %o2

	/* Call the GAISLER L2CACHE bridge driver */
	call	_nomem_amba_scan_gaisler_l2cache_bridge
	 mov	%l2, %o0
	cmp	%o0, 1
	bne	.L_found_bridge
	 ld	[%l2], %o2

	/* Insert next bridge "driver" function here */


	/* The PnP ID did not match a bridge - a new bus was not found ==>
	 * step to next AHB device */
	ba	.L_scan_next_ahb_slave
	 nop

	/* Add Found bus */
.L_found_bridge:
	and	%l2, 0x7e0, %o2
	or	%o2, %o0, %o0		/* Add AHB/AHB Bridge PnP address */
	call	insert_ahb_bridge	/* Insert Bridge into found buses storage */
	 or	%o1, %o0, %o0		/* Add parent bus LSB 3-bits */

.L_scan_next_ahb_slave:
	/* More Slaves to process? */
	subcc	%l1, 1, %l1
	bne	.L_scan_one_ahb_slave
	 add	%l2, AMBA_AHB_CONF_LENGH, %l2

	/* No more AHB devices to process */
	mov	%g2, %o7	/* Restore return address */
	retl
	 nop

/* FUNCTION _nomem_ambapp_find_buses(unsigned int ioarea)
 *
 * Find AMBA AHB buses.
 *
 * Constraints:
 *  i6-i7, l7 is used by caller
 *
 * Arguments
 *  - o0 Bridge Information: I/O AREA and parent bus
 *
 * Results
 *  - o0 Number of AHB bridges found
 *  - i0-i5 initialized
 *
 * Clobbered
 *  - o0 (Current AHB slave conf address)
 *  - o2 (Used by insert_bridge)
 *  - o3 (Used by insert_bridge)
 *  - l0 (Current AHB Bus)
 *  - l1 (Used by nomem_amba_scan)
 *  - l2 (Used by nomem_amba_scan)
 *  - l3 (Used by nomem_amba_scan)
 *  - l4 (Used by nomem_amba_scan)
 *
 *  - g1 (level 1 return address)
 *  - g2 (Used by nomem_amba_scan)
 */
_nomem_ambapp_find_buses:
	mov	%o7, %g1	/* Save return address */

	/* Initialize AHB Bus storage */
	call	init_ahb_bridges
	 nop

	/* Insert AHB Bus 0 */
	call	insert_ahb_bridge
	 nop			/* Argument already prepared by caller */

	/* Scan AHB Bus 0 for AHB Bridges */
	call	_nomem_amba_scan
	 add	%g0, 1, %o1

	/* Scan all AHB Buses found for more AHB Bridges */
	add	%g0, 2, %l0
.L100_search_next_ahb_bus:
	call	get_ahb_bridge			/* Get bus %l0 I/O Area */
	 mov	%l0, %o0
	cmp	%o0, %g0
	be	.L100_return			/* If no more AHB bus is left to be scanned, proceed */
	 nop
	call	_nomem_amba_scan		/* Scan bus %l0 for AHB Bridges. i0-i7,l0 is used */
	 mov	%l0, %o1			/* I/O AREA untouched in o0 */
	ba	.L100_search_next_ahb_bus	/* Do next bus */
	 add	%l0, 1, %l0

.L100_return:
	mov	%g1, %o7
	retl
	 nop


/* FUNCTION _nomem_amba_init(unsigned int ioarea)
 *
 *  Find all AHB buses
 *
 * Constraints:
 *  i6, i7, o6, o7, l7, l6, g3, g4, g5, g6, g7 is used by caller
 *
 * Arguments
 *  - o0 Bridge Information: I/O AREA and parent bus
 *
 * Results
 *  - o0 Number of AHB bridges found
 *
 * Clobbered
 *  - l0, l1, l2, l3, l4, g1, g2 (used by _nomem_ambapp_find_buses)
 *  - o0, o1, o2, o3 (Used as arguments)
 *  - o5 (return address)
 *  - g1 (level 1 return address)
 *  - g2 (level 2 return address)
 */
_nomem_amba_init:
	mov	%o7, %o5	/* Save return address, o5 not used */

	/* Scan for buses, it will init i0-i5 */
	call	_nomem_ambapp_find_buses
	 nop

	mov	%o5, %o7
	retl
	 nop

/* Call tree and their return address register
 *
 *_nomem_amba_scan           (g1)
 * -> init_ahb_bridges       (o7)
 * -> insert_ahb_bridge      (o7)
 * -> _nomem_amba_scan       (g2)
 *    -> insert_ahb_bridge   (o7)
 * -> get_ahb_bridge         (o7)
 *
 *
 * -> _nomem_find_apb        (g2)
 *    -> get_ahb_bridge      (o7)
 *    -> _nomem_find_ahb_bus (o7)
 *    -> _nomem_find_apb_bus (o7)
 * -> _nomem_find_ahb        (g2)
 *    -> get_ahb_bridge      (o7)
 *    -> _nomem_find_ahb_bus (o7)
 * -> mem_handler.func()     (o7)
 *
 */
