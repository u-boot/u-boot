.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: rsu (command)

rsu command
===========

Synopsis
--------

::

    rsu list
    rsu slot_count
    rsu slot_by_name <name>
    rsu slot_get_info <slot>
    rsu slot_size <slot>
    rsu slot_priority <slot>
    rsu slot_enable <slot>
    rsu slot_disable <slot>
    rsu slot_load <slot>
    rsu slot_load_factory
    rsu slot_rename <slot> <name>
    rsu slot_delete <slot>
    rsu slot_create <name> <address> <size>
    rsu slot_erase <slot>
    rsu slot_program_buf <slot> <buffer> <size>
    rsu slot_program_buf_raw <slot> <buffer> <size>
    rsu slot_program_factory_update_buf <slot> <buffer> <size>
    rsu slot_verify_buf <slot> <buffer> <size>
    rsu slot_verify_buf_raw <slot> <buffer> <size>
    rsu update <flash_offset>
    rsu notify <value>
    rsu clear_error_status
    rsu reset_retry_counter
    rsu display_dcmf_version
    rsu display_dcmf_status
    rsu display_max_retry
    rsu restore_spt <address>
    rsu save_spt <address>
    rsu create_empty_cpb
    rsu restore_cpb <address>
    rsu save_cpb <address>
    rsu check_running_factory
    rsu dtb

Description
-----------

The *rsu* command provides access to the Remote System Update (RSU) feature
of the Altera Stratix 10 and Agilex family of SoCFPGA devices. RSU lets an
application boot from one of several configuration images stored in SPI NOR
flash and recover to a known-good factory image when an update fails.

RSU is implemented in the Secure Device Manager (SDM) firmware and in the
on-board Cadence QSPI flash. U-Boot talks to the SDM through the mailbox
(or, when running on top of Arm Trusted Firmware, through an SMC call) and
accesses the flash through the SPI-NOR framework. The flash layout is
described by a Sub-Partition Table (SPT) and a Configuration Pointer Block
(CPB).

The command is bound to a driver-model anchor device with compatible string
``altr,socfpga-rsu``. The node carries no MMIO; it only holds the active RSU
low-level session while a command runs.

Slot inspection
~~~~~~~~~~~~~~~

list
    List the bitstreams (slots) available in flash.

slot_count
    Display the number of slots defined in the CPB.

slot_by_name <name>
    Find a slot by name and display its slot number.

slot_get_info <slot>
    Display the offset, size, priority and name of *slot*.

slot_size <slot>
    Display the size of *slot* in bytes.

slot_priority <slot>
    Display the load priority of *slot*.

Slot management
~~~~~~~~~~~~~~~

slot_enable <slot>
    Make *slot* the highest-priority slot.

slot_disable <slot>
    Remove *slot* from the CPB so it is no longer a load candidate.

slot_load <slot>
    Request the SDM to load *slot* immediately (triggers a reconfiguration).

slot_load_factory
    Request the SDM to load the factory image immediately.

slot_rename <slot> <name>
    Rename *slot* to *name*.

slot_delete <slot>
    Delete *slot*.

slot_create <name> <address> <size>
    Create a new slot *name* at flash *address* with *size* bytes.

slot_erase <slot>
    Erase the flash content of *slot*.

Programming and verification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

slot_program_buf <slot> <buffer> <size>
    Program *size* bytes from memory *buffer* into *slot* and make the slot
    the highest priority.

slot_program_buf_raw <slot> <buffer> <size>
    Program a raw (non-decompressed) buffer into *slot*.

slot_program_factory_update_buf <slot> <buffer> <size>
    Program a factory-update buffer into *slot* and make it highest priority.

slot_verify_buf <slot> <buffer> <size>
    Verify the contents of *slot* against memory *buffer*.

slot_verify_buf_raw <slot> <buffer> <size>
    Verify the contents of *slot* against a raw memory *buffer*.

Firmware operations
~~~~~~~~~~~~~~~~~~~

update <flash_offset>
    Tell the SDM to load the bitstream located at *flash_offset* on the next
    reboot.

notify <value>
    Notify the SDM of the current state of the HPS software.

clear_error_status
    Clear the RSU error status.

reset_retry_counter
    Reset the RSU retry counter.

display_dcmf_version
    Display the Decision Configuration Management Firmware (DCMF) versions and
    store them for later retrieval by the SMC handler.

display_dcmf_status
    Display the DCMF status and store it for the SMC handler.

display_max_retry
    Display the ``max_retry`` parameter and store it for the SMC handler.

SPT/CPB maintenance
~~~~~~~~~~~~~~~~~~~

restore_spt <address>
    Restore the SPT from the image located at memory *address*.

save_spt <address>
    Save the current SPT to memory *address*.

create_empty_cpb
    Create an empty CPB.

restore_cpb <address>
    Restore the CPB from the image located at memory *address*.

save_cpb <address>
    Save the current CPB to memory *address*.

check_running_factory
    Report whether the factory image is currently running.

dtb
    Update the Linux DTB ``qspi-boot`` partition offset with the spt0 value.

Configuration
-------------

The command is available when ``CONFIG_CMD_SOCFPGA_RSU`` is enabled. It depends
on ``CONFIG_ARCH_SOCFPGA_SOC64`` and ``CONFIG_CADENCE_QSPI``. The driver-model
anchor device is provided by ``CONFIG_SOCFPGA_RSU_DM``.

Example
-------

List the available slots and inspect the first one::

    => rsu list
    => rsu slot_count
    => rsu slot_get_info 0

Program a new bitstream that has been loaded to memory at ``0x2000000``
(size ``0x100000``) into slot 1 and make it the highest priority::

    => rsu slot_program_buf 1 0x2000000 0x100000

Return value
------------

The return value ``$?`` is 0 (true) on success and 1 (false) on failure.
