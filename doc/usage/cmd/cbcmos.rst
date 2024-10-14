.. SPDX-License-Identifier: GPL-2.0+

cbcmos
======

Synopis
-------

::

    cbcmos check [<dev>]
    cbcmos update [<dev>]


Description
-----------

This checks or updates the CMOS-RAM checksum value against the CMOS-RAM
contents. It is used with coreboot, which provides information about where to
find the checksum and what part of the CMOS RAM it covers.

If `<dev>` is provided then the named real-time clock (RTC) device is used.
Otherwise the default RTC is used.

Example
-------

This shows checking and updating a checksum across bytes 38 and 39 of the
CMOS RAM::

    => rtc read 38 2
    00000038: 71 00                                            q.
    => cbc check
    => rtc write 38 66
    => rtc read 38 2
    00000038: 66 00                                            f.
    => cbc check
    Checksum 7100 error: calculated 6600
    => cbc update
    Checksum 6600 written
    => cbc check
    =>

See also :ref:`cedit_cb_load` which shows an example that includes the
configuration editor.
