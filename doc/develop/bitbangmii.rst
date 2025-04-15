.. SPDX-License-Identifier: GPL-2.0-or-later
.. Luigi 'Comio' Mantellini <luigi.mantellini@idf-hit.com>, Industrie Dial Face S.p.A., 2009

Bit-banged MII bus support
==========================

The miiphybb ( Bit-banged MII bus driver ) supports an arbitrary number of
MII buses. This feature is useful when a driver uses different MII buses for
different PHYs and all (or a part) of these buses are implemented via
bit-banging mode.

The driver requires that the following macro is defined in the board
configuration file:

* CONFIG_BITBANGMII - Enable the miiphybb driver

The driver code needs to allocate a regular MDIO device using mdio_alloc()
and assign .read and .write accessors which wrap bb_miiphy_read() and
bb_miiphy_write() functions respectively. The bb_miiphy_read() and
bb_miiphy_write() functions take a pointer to a callback structure,
struct bb_miiphy_bus_ops. The struct bb_miiphy_bus_ops has the following
fields/callbacks (see miiphy.h for details):

.. code-block:: c

    int (*mdio_active)()   // Activate the MDIO pin as output
    int (*mdio_tristate)() // Activate the MDIO pin as input/tristate pin
    int (*set_mdio)()      // Write the MDIO pin
    int (*get_mdio)()      // Read the MDIO pin
    int (*set_mdc)()       // Write the MDC pin
    int (*delay)()         // Delay function

The driver code will look like:

.. code-block:: c

    static const struct bb_miiphy_bus_ops ravb_bb_miiphy_bus_ops = {
        .mdio_active      = ravb_bb_mdio_active,
        .mdio_tristate    = ravb_bb_mdio_tristate,
        .set_mdio         = ravb_bb_set_mdio,
        .get_mdio         = ravb_bb_get_mdio,
        .set_mdc          = ravb_bb_set_mdc,
        .delay            = ravb_bb_delay,
    };

    static int ravb_bb_miiphy_read(struct mii_dev *miidev, int addr,
                                   int devad, int reg)
    {
        return bb_miiphy_read(miidev, &ravb_bb_miiphy_bus_ops,
                              addr, devad, reg);
    }

    static int ravb_bb_miiphy_write(struct mii_dev *miidev, int addr,
                                    int devad, int reg, u16 value)
    {
        return bb_miiphy_write(miidev, &ravb_bb_miiphy_bus_ops,
                               addr, devad, reg, value);
    }

    static int ravb_probe(struct udevice *dev)
    {
        struct mii_dev *mdiodev;
    ...
        mdiodev = mdio_alloc();
        if (!mdiodev)
            return -ENOMEM;

        mdiodev->read = ravb_bb_miiphy_read;
        mdiodev->write = ravb_bb_miiphy_write;
        mdiodev->priv = eth;
        snprintf(mdiodev->name, sizeof(mdiodev->name), dev->name);

        ret = mdio_register(mdiodev);
    ...
    }
