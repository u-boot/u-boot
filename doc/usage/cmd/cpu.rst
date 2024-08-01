.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2024 NXP

.. index::
   single: cpu (command)

cpu command
===========

Synopsis
--------

::

    cpu list
    cpu detail
    cpu release <core ID> <addr>

Description
-----------

The *cpu* command prints information about the CPUs, and release a CPU core
to a given address to run applications.


cpu list
~~~~~~~~

The 'list' subcommand lists and prints brief information of all the CPU cores,
the CPU information is provided by vendors' CPU driver.

cpu detail
~~~~~~~~~~

The 'detail' subcommand prints more details about the CPU cores, including
CPU ID, core frequency and feature list.

cpu release
~~~~~~~~~~~

The 'release' subcommand is used to release a CPU core to run a baremetal or
RTOS applications.
The parameter <core ID> is the sequence number of the CPU core to release.
The parameter <addr> is the address to run of the specified core after release.


Examples
--------

cpu list
~~~~~~~~

This example lists all the CPU cores On i.MX8M Plus EVK:
::

    u-boot=> cpu list
      0: cpu@0      NXP i.MX8MP Rev1.1 A53 at 1200 MHz at 31C
      1: cpu@1      NXP i.MX8MP Rev1.1 A53 at 1200 MHz at 30C
      2: cpu@2      NXP i.MX8MP Rev1.1 A53 at 1200 MHz at 31C
      3: cpu@3      NXP i.MX8MP Rev1.1 A53 at 1200 MHz at 31C

cpu detail
~~~~~~~~~~

This example prints the details of the CPU cores On i.MX8M Plus EVK:
::

    u-boot=> cpu detail
      0: cpu@0      NXP i.MX8MP Rev1.1 A53 at 1200 MHz at 31C
            ID = 0, freq = 1.2 GHz: L1 cache, MMU
      1: cpu@1      NXP i.MX8MP Rev1.1 A53 at 1200 MHz at 30C
            ID = 0, freq = 1.2 GHz: L1 cache, MMU
      2: cpu@2      NXP i.MX8MP Rev1.1 A53 at 1200 MHz at 31C
            ID = 0, freq = 1.2 GHz: L1 cache, MMU
      3: cpu@3      NXP i.MX8MP Rev1.1 A53 at 1200 MHz at 31C
            ID = 0, freq = 1.2 GHz: L1 cache, MMU

cpu release
~~~~~~~~~~~

This example shows release the LAST CPU core to run a RTOS application, on
i.MX8M Plus EVK:
::

     u-boot=> load mmc 1:2 c0000000 /hello_world.bin
     66008 bytes read in 5 ms (12.6 MiB/s)
     u-boot=> dcache flush; icache flush
     u-boot=> cpu release 3 c0000000
     Released CPU core (mpidr: 0x3) to address 0xc0000000


Configuration
-------------

The cpu command is available if CONFIG_CMD_CPU=y.

Return code
-----------

The return value $? is set to 0 (true) if the command is successful,
1 (false) otherwise.
