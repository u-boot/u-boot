.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: cpuid (command)

cpuid command
=============

Synopsis
--------

::

    cpuid <op>

Description
-----------

The cpuid command requests CPU-identification information on x86 CPUs. The
operation <op> selects what information is returned. Up to four 32-bit registers
can be update (eax-edx) depending on the operation.

Configuration
-------------

The cpuid command is only available on x86.

Return value
------------

The return value $? is 0 (true).

Example
-------

::

    => cpuid 1
    eax 00060fb1
    ebx 00040800
    ecx 80002001
    edx 178bfbfd

This shows checking for 64-bit 'long' mode::

    => cpuid 80000000
    eax 8000000a
    ebx 68747541
    ecx 444d4163
    edx 69746e65
    => cpuid 80000001
    eax 00060fb1
    ebx 00000000
    ecx 00000007
    edx 2193fbfd   # Bit 29 is set in edx, so long mode is available

On a 32-bit-only CPU::

    => cpuid 80000000
    eax 80000004
    ebx 756e6547
    ecx 6c65746e
    edx 49656e69
    => cpuid 80000001
    eax 00000663
    ebx 00000000
    ecx 00000000
    edx 00000000   # Bit 29 is not set in edx, so long mode is not available
