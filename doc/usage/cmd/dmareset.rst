.. SPDX-License-Identifier: GPL-2.0-or-later

.. index::
   single: dmareset (command)

dmareset command
================

Synopsis
--------

::

    Usage: dmareset <channel 0-7> [<channel 0-7> ...]
    dmareset - Release PL330 DMA channel reset(s) for SoCFPGA

    Usage:
    dmareset <channel 0-7> [<channel 0-7> ...]  - release reset for one or more DMA channels

Description
-----------

Release the DMA channel reset *channel*.

Parameters
----------

channel
    DMA channel number

Example
-------

Release DMA channel(s)::

    => dmareset 0
    PL330 DMA channel 0 reset released
    => dmareset 1
    PL330 DMA channel 1 reset released
    => dmareset 0 1
    PL330 DMA channel 0 reset released
    PL330 DMA channel 1 reset released


Configuration
-------------

The dmareset command is only available if CONFIG_CMD_C5_PL330_DMA=y in
"Shell scripting commands".
