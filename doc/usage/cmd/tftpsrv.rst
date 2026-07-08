.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: tftpsrv (command)

tftpsrv command
===============

Synopsis
--------

::

    tftpsrv [loadAddress]

Description
-----------

The tftpsrv command listens for an incoming TFTP write request and receives
the first transferred file into memory.

loadAddress
    memory address where the received file is stored. If not provided, the
    address is taken from the *loadaddr* environment variable or the default
    image load address.

After a successful transfer, the *fileaddr* and *filesize* environment
variables describe the received file. The command returns successfully after
the transfer has completed. It does not boot the file automatically; boot
scripts can use commands such as bootm, booti or bootefi to boot from the
load address.

The transfer is aborted if no transfer has started after about 50 seconds or
if Ctrl-C is pressed.

Example
-------

In the example the following steps are executed:

* setup the board network address
* receive a FIT image from a host
* boot the received FIT image

::

    => setenv autoload no
    => dhcp
    BOOTP broadcast 1
    DHCP client bound to address 192.168.1.40 (7 ms)
    => tftpsrv $loadaddr
    Using ethernet@1c30000 device
    Listening for TFTP transfer on 192.168.1.40
    Load address: 0x42000000
    Loading: #################################################################
             6.5 MiB/s
    done
    Bytes transferred = 1048576 (100000 hex)
    => bootm $fileaddr

On the host, send the file to the board while U-Boot is listening:

::

    $ curl --upload-file image.fit tftp://192.168.1.40/image.fit

Configuration
-------------

The command is only available if CONFIG_CMD_TFTPSRV=y.

The command is supported by both the legacy network stack and the lwIP network
stack.
