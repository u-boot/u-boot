.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: reset (command)

reset command
=============

Synopsis
--------

::

    reset [-c|-w]

Description
-----------

Perform reset of the CPU. By default does COLD reset unless overridden via
Kconfig CONFIG_SYSRESET_CMD_RESET_DEFAULT_{COLD,WARM,POWER}.

-c
    Do COLD reset: reset CPU and peripheral/DDR; on some boards also resets
    external PMIC.

-w
    Do WARM reset: reset CPU but keep peripheral/DDR/PMIC active.


Return value
------------

The return value $? is always set to 0 (true).
