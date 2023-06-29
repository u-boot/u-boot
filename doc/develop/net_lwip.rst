.. SPDX-License-Identifier: GPL-2.0+

lwIP IP stack integration for U-Boot
====================================

Intro
-----

lwIP is a library implementing network protocols, which is commonly used
on embedded devices.

https://savannah.nongnu.org/projects/lwip/
lwIP  license:
lwIP is licensed under a BSD-style license: http://lwip.wikia.com/wiki/License.

Main features include:

* Protocols: IP, IPv6, ICMP, ND, MLD, UDP, TCP, IGMP, ARP, PPPoS, PPPoE

* DHCP client, DNS client (incl. mDNS hostname resolver),
  AutoIP/APIPA (Zeroconf), SNMP agent (v1, v2c, v3, private MIB support
  & MIB compiler)

* APIs: specialized APIs for enhanced performance, optional Berkeley-alike
  socket API

* Extended features: IP forwarding over multiple network interfaces, TCP
  congestion control, RTT estimation and fast recovery/fast retransmit

* Addon applications: HTTP(S) server, SNTP client, SMTP(S) client, ping,
  NetBIOS nameserver, mDNS responder, MQTT client, TFTP server

U-Boot implementation details
-----------------------------

1. In general we can build lwIP as a library and link it against U-Boot or
   compile it in the U-Boot tree in the same way as other U-Boot files. There
   are few reasons why second variant was selected: lwIP is very customizable
   with defines for features, memory size, types of allocation, some internal
   types and platform specific code. It turned out easier to enable/disable
   debug which is also done with defines, and is needed periodically.

2. lwIP has 2 APIs - raw mode and sequential (as lwIP names it, or socket API
   as we name it in Linux). For now only raw API is supported.

In raw IP mode a callback function for RX path is registered and will be called
when packet is passed to the IP stack and is ready for the application.

One example is the unmodified working ping example from lwip sources which
registered the callback:

.. code-block:: c

        ping_pcb = raw_new(IP_PROTO_ICMP);
        raw_recv(ping_pcb, ping_recv, NULL); <- ping_recv is app callback.
        raw_bind(ping_pcb, IP_ADDR_ANY)

3.  Input and output

RX packet path is injected to U-Boot eth_rx() polling loop and TX patch is in
eth_send() accordingly. That way we can leave the driver code unmodified and
consume packets once they are ready. So we do not touch any drivers code and
just eat packets when they are ready.

U-Boot lwIP Applications
========================

.. kernel-doc:: include/net/lwip.h
   :internal:

lwIP API to control polling loop
================================

.. kernel-doc:: include/net/ulwip.h
   :internal:
