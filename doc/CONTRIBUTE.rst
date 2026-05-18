.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Peter Robinson <pbrobinson@gmail.com>

Contributing
============

This document is a high level contributors overview setting overall expectations,
so people can get started quickly, the rest of the documentation goes into the
details.

Code of Conduct
---------------

The U-Boot project doesn't currently have an explicit code of conduct, but all
contributors are expected to act cordially to, and be respectful of, each other's
contributions and opinions. There are many code of conducts for open source
projects available to review if you are unsure of expectations.

Repository
----------

The official U-Boot repository is located at https://source.denx.de/u-boot/u-boot

Further more detailed documentation can be found at the following link:
https://docs.u-boot.org/en/latest/index.html

Contributions
-------------

Contributions to the project are welcome. The U-Boot project uses a fairly
traditional Linux style development work-flow using git and `a mailing list
<https://lists.denx.de/listinfo/u-boot>`_.

Patches should be sent to the mailing list using ``git send-email`` or the
equivalent commands using ``b4`` or ``patman`` with appropriate sign-off and
attributions for the code in question. Maintainers should be copied on mails
and they can be found with the ``./scripts/get_maintainer.pl 0001-fix.patch``
script. Please don't send patches as attachments, and ensure corporate mail
systems don't reformat patches, append disclaimers or other unnecessary notes.
The b4 tool automates a number of components mentioned above.

Patch Series
------------

Patch series for a specific subject are welcome but they should be constrained
to a single topic with a cover letter outlining the intention of the series.
Each patch within the series should cover a single change, be self contained,
not break the build or cause a regression.

Generally bug fixes for existing bugs should be at the beginning of the
series before any enhancements to allow those patches to be picked up early.

Each iteration of a patch set should be versioned, allow enough time for people
to review previous versions of the series and incorporate all the review
feedback before sending a new version. A week between larger patch sets is
considered as reasonable amount of time.

Development Branches
--------------------

The U-Boot developers use two main branches for developing the code. The master
branch is used for the current development cycle, while there is also a next
branch intended to land changes for the next release early to enable wider
testing of larger code changes. The next branch is merged to master shortly
after the tagging of a new major release.

Similar to Linux there is a three week merge window post release after which a
release candidate is tagged. There's typically a new release candidate every
two weeks post merge window until the stable generally available release.

Release Schedule
----------------

There is currently four major releases a year in January (.01), April (.04),
July (.07) and October (.10). These typically happen on the first Monday of
that month. There is currently no release branches or long term releases.
