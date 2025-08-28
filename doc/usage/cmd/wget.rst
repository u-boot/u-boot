.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: wget (command)

wget command
============

Synopsis
--------

::

    wget [address] [host:]path
    wget [address] url                  # lwIP only
    wget cacert none|optional|required  # lwIP only
    wget cacert <address> <size>        # lwIP only


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
    IP address (or host name if `CONFIG_DNS` is enabled) of the HTTP
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

The cacert (stands for 'Certification Authority certificates') subcommand is
used to provide root certificates for the purpose of HTTPS authentication. It
also allows to enable or disable authentication.

wget cacert <address> <size>

address
    memory address of the root certificates in X509 DER format

size
    the size of the root certificates

wget cacert none|optional|required

none
    certificate verification is disabled. HTTPS is used without any server
    authentication (unsafe)
optional
    certificate verification is enabled provided root certificates have been
    provided via wget cacert <addr> <size> or wget cacert builtin. Otherwise
    HTTPS is used without any server authentication (unsafe).
required
    certificate verification is mandatory. If no root certificates have been
    configured, HTTPS transfers will fail.


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

Here is an example showing how to configure built-in root certificates as
well as providing some at run time. In this example it is assumed that
CONFIG_WGET_BUILTIN_CACERT_PATH=DigiCertTLSRSA4096RootG5.crt downloaded from
https://cacerts.digicert.com/DigiCertTLSRSA4096RootG5.crt.

::

   # Make sure IP is configured
   => dhcp
   # When built-in certificates are configured, authentication is mandatory
   # (i.e., "wget cacert required"). Use a test server...
   => wget https://digicert-tls-rsa4096-root-g5.chain-demos.digicert.com/
   1864 bytes transferred in 1 ms (1.8 MiB/s)
   Bytes transferred = 1864 (748 hex)
   # Another server not signed against Digicert will fail
   => wget https://www.google.com/

   HTTP client error 4
   Certificate verification failed
   # Disable authentication to allow the command to proceed anyways
   => wget cacert none
   => wget https://www.google.com/
   WARNING: no CA certificates, HTTPS connections not authenticated
   16683 bytes transferred in 15 ms (1.1 MiB/s)
   Bytes transferred = 16683 (412b hex)
   # Force verification but unregister the CA certificates
   => wget cacert required
   => wget cacert 0 0
   # Unsurprisingly, download fails
   => wget https://digicert-tls-rsa4096-root-g5.chain-demos.digicert.com/
   Error: cacert authentication mode is 'required' but no CA certificates given
   # Get the same certificates as above from the network
   => wget cacert none
   => wget https://cacerts.digicert.com/DigiCertTLSRSA4096RootG5.crt
   WARNING: no CA certificates, HTTPS connections not authenticated
   1386 bytes transferred in 1 ms (1.3 MiB/s)
   Bytes transferred = 1386 (56a hex)
   # Register them and force authentication
   => wget cacert $fileaddr $filesize
   => wget cacert required
   # Authentication is operational again
   => wget https://digicert-tls-rsa4096-root-g5.chain-demos.digicert.com/
   1864 bytes transferred in 1 ms (1.8 MiB/s)
   Bytes transferred = 1864 (748 hex)
   # The builtin certificates can be restored at any time
   => wget cacert builtin

Configuration
-------------

The command is only available if CONFIG_CMD_WGET=y.
To enable lwIP support set CONFIG_NET_LWIP=y. In this case, root certificates
support can be enabled via CONFIG_WGET_BUILTIN_CACERT=y
CONFIG_WGET_BUILTIN_CACERT_PATH=<some path> (for built-in certificates) and/or
CONFIG_WGET_CACERT=y (for the wget cacert command).

TCP Selective Acknowledgments in the legacy network stack can be enabled via
CONFIG_PROT_TCP_SACK=y. This will improve the download speed. Selective
Acknowledgments are enabled by default with lwIP.

Return value
------------

The return value $? is 0 (true) on success and 1 (false) otherwise.
