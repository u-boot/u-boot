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

The official U-Boot repository is located at https://git.u-boot-project.org/u-boot/u-boot

Further more detailed documentation can be found at the following link: :doc:`/index`.

Contributions
-------------

Contributions to the project are welcome. The U-Boot project uses a fairly
traditional Linux style development work-flow using git and `a mailing list
<https://lists.u-boot-project.org/mailman/listinfo/u-boot>`_.

Patches should be sent to the mailing list using ``git send-email`` or the
equivalent commands using ``b4`` or ``patman`` with appropriate sign-off and
attributions for the code in question. Maintainers should be copied on mails
and they can be found with the ``./scripts/get_maintainer.pl 0001-fix.patch``
script. Please don't send patches as attachments, and ensure corporate mail
systems don't reformat patches, append disclaimers or other unnecessary notes.
The b4 tool automates a number of components mentioned above.

Code is not the only thing you can contribute to the project. Like most
open-source projects, the U-Boot project suffers from a lack of reviewers.
Consider spending some time reading patches on `the mailing list archive
<https://lists.u-boot-project.org/pipermail/u-boot/>`_ and providing feedback to
contributors when something could be improved or if you have questions. Contrary
to what's most often believed, you do not need to be an expert to review patches
and the project will benefit from people with different skillsets and experience
looking at the same patches and each catch different bugs.

The project would also benefit from more :ref:`develop/index:testing`.

New sections or updates to the documentation are most welcome, e.g.
:ref:`usage/index:shell commands`. See :doc:`/develop/docstyle`.

Thank you for your help!

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
