.. SPDX-License-Identifier: GPL-2.0+

sbi command
===========

Synopsis
--------

::

    sbi

Description
-----------

The sbi command is used to display information about the SBI (Supervisor Binary
Interface) implementation on RISC-V systems.

The output may look like:

::

    => sbi
    SBI 0.2
    OpenSBI
    Extensions:
      sbi_set_timer
      sbi_console_putchar
      sbi_console_getchar
      sbi_clear_ipi
      sbi_send_ipi
      sbi_remote_fence_i
      sbi_remote_sfence_vma
      sbi_remote_sfence_vma_asid
      sbi_shutdown
      SBI Base Functionality
      Timer Extension
      IPI Extension
      RFENCE Extension
      Hart State Management Extension

The first line indicates the version of the RISC-V SBI specification.
The second line indicates the implementation.
The further lines enumerate the implemented extensions.

Configuration
-------------

To use the sbi command you must specify CONFIG_CMD_SBI=y.
