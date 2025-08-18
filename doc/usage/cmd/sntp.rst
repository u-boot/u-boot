.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: sntp (command)

sntp command
============

Synopsis
--------

::

    sntp [serverip]
    sntp [servername]  # NET_LWIP=y && DNS=y only


Description
-----------

The sntp command gets the current time from an NTP time server and
syncronizes the Real Time Clock (RTC) of the board. This command needs
the server's IP address to be given on the command line or via the
`ntpserverip` environment variable.

The address of the NTP server does not need to be given if the DHCP server
provides one. The legacy network stack (`CONFIG_NET=y`) can only use the
first NTP server provided in the `ntp-servers` DHCP option.

When the network stack is lwIP (`CONFIG_NET_LWIP=y`) and DNS resolution
is enabled (`CONFIG_DNS=y`), then the sntp command accepts a server
name as an argument.

The network time is sent as UTC. So, if you want to set the RTC to any local
time different from UTC, you need to set the `timeoffset` environment variable.

Round-trip delay compensation is not implemented/not enabled. In practice
this should not matter much given that the RTC API does not have sub-second
resolution, and round-trip times are typically 10 to 100 ms at most.

Examples
--------

::

    => setenv ntpserverip 109.190.177.205
    => date
    Date: 2025-06-16 (Monday)    Time: 15:19:35
    => date reset
    Reset RTC...
    Date: 2000-01-01 (Saturday)    Time:  0:00:00
    => date
    Date: 2000-01-01 (Saturday)    Time:  0:00:03
    => sntp
    Date: 2025-06-16 Time: 15:19:43
    => date
    Date: 2025-06-16 (Monday)    Time: 15:19:47
    => setenv timeoffset 7200
    => sntp
    Date: 2025-06-16 Time: 17:19:55
    => date
    Date: 2025-06-16 (Monday)    Time: 17:19:57

With `CONFIG_NET_LWIP=y` and `CONFIG_DNS=y`:

::

    => date reset
    Reset RTC...
    Date: 2000-01-01 (Saturday)    Time:  0:00:00
    => sntp 0.us.pool.ntp.org
    Date: 2025-06-16 Time: 15:10:59
