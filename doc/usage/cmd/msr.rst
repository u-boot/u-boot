.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: msr (command)

msr command
===========

Synopsis
--------

::

    msr read <op>
    msr write <op> <hi> <lo>

Description
-----------

The msr command reads and writes machine-status registers (MSRs) on x86 CPUs.
The information is a 64-bit value split into two parts, <hi> for the top 32
bits and <lo> for the bottom 32 bits.

The operation <op> selects what information is read or written.

msr read
~~~~~~~~

This reads an MSR and displays the value obtained.

msr write
~~~~~~~~~

This writes a value to an MSR.

Configuration
-------------

The msr command is only available on x86.

Return value
------------

The return value $? is 0 (true).

Example
-------

This shows reading msr 0x194 which is MSR_FLEX_RATIO on Intel CPUs::

    => msr read 194
    00000000 00011200   # Bits 16 (flex ratio enable) and 20 (lock) are set

This shows adjusting the energy-performance bias on an Intel CPU::

    => msr read 1b0
    00000000 00000006     # 6 means 'normal'

    => msr write 1b0 0 f  # change to power-save
    => msr read 1b0
    00000000 0000000f
