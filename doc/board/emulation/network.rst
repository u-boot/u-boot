.. SPDX-License-Identifier: GPL-2.0-or-later

Emulation of network devices
============================

Networking types
----------------

QEMU can emulate different types of networks:

User networking
'''''''''''''''

User networking is the easiest to use. QEMU provides NAT translation and a DHCP
server.

.. code-block:: bash

    -netdev user,id=eth0 \
    -device virtio-net,netdev=eth0

Port forwarding
~~~~~~~~~~~~~~~

The emulated device can access the outside network but cannot be reached from
outside by default. Forwarding rules can be added.

In the example the SSH port 22 of the default interface of the emulated device
is forwarded to port 2222 of the loopback interface of the host.

.. code-block:: bash

    -netdev user,id=eth0,hostfwd=tcp:127.0.0.1:2222-:22 \
    -device virtio-net,netdev=eth0

TFTP
~~~~

A TFTP server can be added by specifying a file path.

.. code-block:: bash

    -netdev user,id=eth0,tftp=/path/to/tftpdir \
    -device virtio-net,netdev=eth0

Bridge networking
'''''''''''''''''

The emulated NIC is connected to an existing bridge device.

.. code-block:: bash

    -netdev bridge,id=eth0,br=virbr0 \
    -device virtio-net,netdev=eth0

The emulated device becomes part of the same local network as the bridge.

Tap networking
''''''''''''''

With tap networking a tap device is created on the host.

.. code-block:: bash

    -netdev tap,id=eth0 \
    -device virtio-net,netdev=eth0

Emulated network interface controllers
--------------------------------------

QEMU can emulate different NICs. For best performance choose ``virtio-net``.

Intel E1000
'''''''''''

U-Boot's E1000 driver supports a number of Intel PCI NICs. This includes the
QEMU devices ``e1000``, ``e1000-82544gc``, and ``e1000-82545em``.

.. code-block:: bash

    -netdev user,id=eth0 \
    -device e1000,netdev=eth0

Configuration:

* CONFIG_PCI=y
* CONFIG_E1000=y

Realtek RTL8139
'''''''''''''''

The RTL8139 is a PCI network card.
The U-Boot driver only supports the i386 archtitecture.

.. code-block:: bash

    -netdev user,id=eth0 \
    -device rtl8139,netdev=eth0

Configuration:

* CONFIG_PCI=y
* CONFIG_NET_RANDOM_ETHADDR=y
* CONFIG_RTL8139

Virtio
''''''

U-Boot's virtio network driver supports ``virtio-net``, ``virtio-net-device``,
and ``virtio-net-pci`` devices. ``virtio-net-device`` uses MMIO, while
``virtio-net-pci`` uses PCI.

For best performance use ``virtio-net``.

.. code-block:: bash

    -netdev user,id=eth0 \
    -device virtio-net,netdev=eth0

Configuration:

* CONFIG_PCI=y (only for ``virtio-net-pci``)
* CONFIG_NET_VIRTIO=y

Network device options
----------------------

The network adapters provide different configuration parameters.
Here are some common ones.

mac
    set MAC address

    .. code-block:: bash

        -netdev user,id=eth0 \
        -device virtio-net-pci,netdev=eth0,mac=00:00:00:00:01:01

romfile
    provide the ROM file of the NIC

    .. code-block:: bash

        -netdev user,id=eth0 \
        -device virtio-net-pci,netdev=eth0,romfile=pxe-virtio.rom

    or provide none

    .. code-block:: bash

        -netdev user,id=eth0 \
        -device virtio-net-pci,netdev=eth0,romfile=
