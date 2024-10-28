.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: sbi (command)

sbi command
===========

Synopsis
--------

::

    sb handoff
    sb state

Description
-----------

The *sb* command is used to display information about sandbox's internal
operation. See :doc:`/arch/sandbox/index` for more information.

sb handoff
~~~~~~~~~~

This shows information about any handoff information received from SPL. If
U-Boot is started from an SPL build, it shows a valid magic number.

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

This shows output from the *sb state* subcommand::

    => sb state
    Arguments:
    /tmp/b/sandbox/u-boot -D

Configuration
-------------

The *sb handoff* command is only supported if CONFIG_HANDOFF is enabled.
