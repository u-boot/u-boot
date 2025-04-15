.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: sb (command)

sb command
==========

Synopsis
--------

::

    sb handoff
    sb map
    sb state

Description
-----------

The *sb* command is used to display information about sandbox's internal
operation. See :doc:`/arch/sandbox/index` for more information.

sb handoff
~~~~~~~~~~

This shows information about any handoff information received from SPL. If
U-Boot is started from an SPL build, it shows a valid magic number.

sb map
~~~~~~

This shows any mappings between sandbox's emulated RAM and the underlying host
address-space.

Fields shown are:

Addr
    Address in emulated RAM

Mapping
    Equivalent address in the host address-space. While sandbox requests address
    ``0x10000000`` from the OS, this is not always available.

Refcnt
    Shows the number of references to this mapping.

sb state
~~~~~~~~

This shows basic information about the sandbox state, currently just the
command-line with which sandbox was started.

Example
-------

This shows checking for the presence of SPL-handoff information. For this to
work, ``u-boot-spl`` must be run, with build that enables ``CONFIG_SPL``, such
as ``sandbox_spl``::

    => sb handoff
    SPL handoff magic 14f93c7b

This shows output from the *sb map* subcommand, with a single mapping::

    Sandbox memory-mapping
        Addr           Mapping  Refcnt
    ff000000  000056185b46d6d0       2

This shows output from the *sb state* subcommand::

    => sb state
    Arguments:
    /tmp/b/sandbox/u-boot -D

Configuration
-------------

The *sb handoff* command is only supported if CONFIG_HANDOFF is enabled.
