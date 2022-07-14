.. SPDX-License-Identifier: GPL-2.0+:

U-Boot Development Process
==========================

Management Summary
------------------

* Development happens in Release Cycles of 3 months.

* The first 2 weeks are called Merge Window, which is followed by a
  Stabilization Period.

* Patches with new code get only accepted while the Merge Window is open.

* A patch that is generally in good shape and that was submitted while the
  Merge Window was open is eligible to go into the upcoming release, even if
  changes and resubmits are needed.

* During the Stabilization Period, only patches that contain bug fixes get
  applied.

Phases of the Development Process
---------------------------------

U-Boot development takes place in `Release Cycles
<https://www.denx.de/wiki/U-Boot/ReleaseCycle>`_.  A Release Cycle lasts
normally for three months.

The first two weeks of each Release Cycle are called *Merge Window*.

It is followed by a *Stabilization Period*.

The end of a Release Cycle is marked by the release of a new U-Boot version.

Merge Window
------------

The Merge Window is the period when new patches get submitted
(and hopefully accepted) for inclusion into U-Boot mainline.

This is the only time when new code (like support for new processors or new
boards, or other new features or reorganization of code) is accepted.

Twilight Time
-------------

Usually patches do not get accepted as they are - the peer review that takes
place will usually require changes and resubmissions of the patches before they
are considered to be ripe for inclusion into mainline.

Also the review often happens not immediately after a patch was submitted,
but only when somebody (usually the responsible custodian) finds time to do
this.

The result is that the final version of such patches gets submitted after the
merge window has been closed.

It is current practice in U-Boot that such patches are eligible to go into the
upcoming release.

The result is that the release of the ``"-rc1"`` version and formal closing of
the Merge Window does not preclude patches that were already posted from being
merged for the upcoming release.

Stabilization Period
--------------------

During the Stabilization Period only patches containing bug fixes get
applied.

Corner Cases
------------

Sometimes it is not clear if a patch contains a bug fix or not.
For example, changes that remove dead code, unused macros etc. or
that contain Coding Style fixes are not strict bug fixes.

In such situations it is up to the responsible custodian to decide if they
apply such patches even when the Merge Window is closed.

Exception: at the end of the Stabilization Period only strict bug
fixes my be applied.

Sometimes patches miss the Merge Window slightly - say by a few
hours or even a day. Patch acceptance is not as critical as a
financial transaction, or such. So if there is such a slight delay,
the custodian is free to turn a blind eye and accept it anyway. The
idea of the development process is to make it foreseeable,
but not to slow down development.

It makes more sense if an engineer spends another day on testing and
cleanup and submits the patch a couple of hours late, instead of
submitting a green patch which will waste efforts from several people
during several rounds of review and reposts.

Differences to the Linux Development Process
--------------------------------------------

* In Linux, top-level maintainers will collect patches in their trees and send
  pull requests to Linus as soon as the merge window opens.
  So far, most U-Boot custodians do not work like that; they send pull requests
  only at (or even after) the end of the merge window.

* In Linux, the closing of the merge window is marked by the release of the
  next ``"-rc1"``
  In U-Boot, ``"-rc1"`` will only be released after all (or at least most of
  the) patches that were submitted during the merge window have been applied.

Custodians
----------

The Custodians take responsibility for some area of the U-Boot code.  The
in-tree ``MAINTAINERS`` files list who is responsible for which areas.

It is their responsibility to pick up patches from the mailing list
that fall into their responsibility, and to process these.

A very important responsibility of each custodian is to provide
feedback to the submitter of a patch about what is going on: if the
patch was accepted, or if it was rejected (which exact list of
reasons), if it needs to be reworked (with respective review
comments). Even a "I have no time now, will look into it later"
message is better than nothing. Also, if there are remarks to a
patch, these should leave no doubt if they were just comments and the
patch will be accepted anyway, or if the patch should be
reworked/resubmitted, or if it was rejected.

Work flow of a Custodian
------------------------

The normal flow of work in the U-Boot development process will look
like this:

#. A developer submits a patch via e-mail to the u-boot-users mailing list.
   U-Boot has adopted the `Linux kernel signoff policy <https://groups.google.com/g/fa.linux.kernel/c/TLJIJVA-I6o?pli=1>`_, so the submitter must
   include a ``Signed-off-by:`` line.

#. Everybody who can is invited to review and test the changes.  Reviews should
   reply on the mailing list with ``Acked-by`` lines.

#. The responsible custodian

   #. inspects this patch, especially for:

   #. :doc:`codingstyle`

   #. Basic logic:

      * The patch fixes a real problem.

      * The patch does not introduce new problems, especially it does not break
        other boards or architectures

   #. U-Boot Philosophy

   #. Applies cleanly to the source tree

   #. Passes :doc:`ci_testing` as this checks for new warnings and other issues.

#. Notes:

  #. In some cases more than one custodian may be affected or feel responsible.
     To avoid duplicated efforts, the custodian who starts processing the
     patch should send a short ACK to the mailing list.

  #. We should create some tool to automatically do this.

  #. This is well documented in :doc:`designprinciples`.

  #. The custodian decides themselves how recent the code must be.  It is
     acceptable to request patches against the last officially released
     version of U-Boot or newer.  Of course a custodian can also accept
     patches against older code.

  #. Commits should show original author in the ``author`` field and include all
      sign off/ack lines.

#. The custodian decides to accept or to reject the patch.

#. If accepted, the custodian adds the patch to their public git repository and
   notifies the mailing list. This note should include:

   * a short description of the changes

   * the list of the affected boards / architectures etc.

   * suggested tests

   Although the custodian is supposed to perform their own tests
   it is a well-known and accepted fact that they needs help from
   other developers who - for example - have access to the required
   hardware or tool chains.
   The custodian request help for tests and feedback from
   specific maintainers and U-Boot users.

#. Once tests are passed, some agreed time limit expires, the custodian
   requests that the changes in their public git repository be merged into the
   main tree. If necessary, the custodian may have to adapt their changes to
   allow for a clean merge.
   Todo: define a reasonable time limit. 3 weeks?
