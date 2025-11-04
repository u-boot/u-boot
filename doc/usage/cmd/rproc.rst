.. SPDX-License-Identifier: GPL-2.0+
   Copyright 2025 NXP
   Written by Peng Fan <peng.fan@nxp.com>

remoteproc command
==================

Synopsis
--------

::

    rproc init <id>
    rproc list
    rproc load <id> [addr] [size]
    rproc start <id>
    rproc stop <id>
    rproc reset <id>
    rproc is_running <id>
    rproc ping <id>

Description
-----------

The rproc command provides a generic U-Boot mechanism to manage the Remote
Processors inside a SoC .

The 'rproc init' command enumerate and initialize the remote processor.

    id
        remote processor id. if id is not passed, initialize all the
        remote prcessors

The 'rproc list' list available remote processors.

The 'rproc load' load the remote processor with binary.

    id
        remote processor id.
    addr
        address that image is loaded at.
    size
        image size

The 'rproc start' start the remote processor(must be loaded).

    id
        remote processor id.

The 'rproc stop' stop the remote processor.

    id
        remote processor id.

The 'rproc reset' reset the remote processor.

    id
        remote processor id.

The 'rproc is_running' reports if the remote processor is running.

    id
        remote processor id.

The 'rproc ping' ping the remote processor for communication.

    id
        remote processor id.

Configuration
-------------

The rproc command is only available if CONFIG_CMD_REMOTEPROC=y.

.. toctree::
   :maxdepth: 2

   ../../board/nxp/rproc.rst
