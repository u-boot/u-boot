.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: wget (command)

wget command
============

Synopsis
--------

::

    wget [address] [host:]path
    wget [address] url          # lwIP only


Description
-----------

The wget command is used to download a file from an HTTP(S) server.
In order to use HTTPS you will need to compile wget with lwIP support.

Legacy syntax
~~~~~~~~~~~~~

The legacy syntax is supported by the legacy network stack (CONFIG_NET=y)
as well as by the lwIP base network stack (CONFIG_NET_LWIP=y). It supports HTTP
only.

By default the destination port is 80 and the source port is pseudo-random.
On the legacy nework stack the environment variable *httpdstp* can be used to
set the destination port

address
    memory address for the data downloaded

host
    IP address (or host name if `CONFIG_CMD_DNS` is enabled) of the HTTP
    server, defaults to the value of environment variable *serverip*.

path
    path of the file to be downloaded.

New syntax (lwIP only)
~~~~~~~~~~~~~~~~~~~~~~

In addition to the syntax described above, wget accepts URLs if the network
stack is lwIP.

address
    memory address for the data downloaded

url
    HTTP or HTTPS URL, that is: http[s]://<host>[:<port>]/<path>.

Examples
--------

Example with the legacy network stack
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Example with lwIP
~~~~~~~~~~~~~~~~~

In the example the following steps are executed:

* setup client network address
* download a file from the HTTPS server

::

   => dhcp
   DHCP client bound to address 10.0.2.15 (3 ms)
   => wget https://download.rockylinux.org/pub/rocky/9/isos/aarch64/Rocky-9.4-aarch64-minimal.iso
   ##########################################################################
   ##########################################################################
   ##########################################################################
   [...]
   1694892032 bytes transferred in 492181 ms (3.3 MiB/s)
   Bytes transferred = 1694892032 (65060000 hex)

Configuration
-------------

The command is only available if CONFIG_CMD_WGET=y.
To enable lwIP support set CONFIG_NET_LWIP=y.

TCP Selective Acknowledgments in the legacy network stack can be enabled via
CONFIG_PROT_TCP_SACK=y. This will improve the download speed. Selective
Acknowledgments are enabled by default with lwIP.

.. note::

    U-Boot currently has no way to verify certificates for HTTPS.
    A place to store the root CA certificates is needed, and then MBed TLS would
    need to walk the entire chain. Therefore, man-in-the middle attacks are
    possible and HTTPS should not be relied upon for payload authentication.

Return value
------------

The return value $? is 0 (true) on success and 1 (false) otherwise.
