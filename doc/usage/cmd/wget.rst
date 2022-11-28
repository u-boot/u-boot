.. SPDX-License-Identifier: GPL-2.0+:

wget command
============

Synopsis
--------

::
    wget address [[hostIPaddr:]path]

Description
-----------

The wget command is used to download a file from an HTTP server.

wget command will use HTTP over TCP to download files from an HTTP server.
Currently it can only download image from an HTTP server hosted on port 80.

address
    memory address for the data downloaded

hostIPaddr
    IP address of the HTTP server, defaults to the value of environment
    variable *serverip*

path
    path of the file to be downloaded.

Example
-------

In the example the following steps are executed:

* setup client network address
* download a file from the HTTP server

::

    => setenv autoload no
    => dhcp
    BOOTP broadcast 1
    *** Unhandled DHCP Option in OFFER/ACK: 23
    *** Unhandled DHCP Option in OFFER/ACK: 23
    DHCP client bound to address 192.168.1.105 (210 ms)
    => wget ${loadaddr} 192.168.1.254:/index.html
    HTTP/1.0 302 Found
    Packets received 4, Transfer Successful

Configuration
-------------

The command is only available if CONFIG_CMD_WGET=y.

CONFIG_PROT_TCP_SACK can be turned on for the TCP SACK options. This will
help increasing the downloading speed.

Return value
------------

The return value $? is 0 (true) on success and 1 (false) otherwise.
