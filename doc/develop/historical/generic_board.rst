.. SPDX-License-Identifier: GPL-2.0+
.. (C) Copyright 2014 Google, Inc
.. sectionauthor:: Simon Glass <sjg@chromium.org>

Generic board
-------------

U-Boot traditionally had a board.c file for each architecture. This introduced
quite a lot of duplication, with each architecture tending to do
initialisation slightly differently. To address this, a new 'generic board
init' feature was introduced in March 2013 (further motivation is
provided in the cover letter below).

All boards and architectures have moved to this as of mid 2016.


What has changed?
~~~~~~~~~~~~~~~~~

The main change is that the arch/<arch>/lib/board.c file is removed in
favour of common/board_f.c (for pre-relocation init) and common/board_r.c
(for post-relocation init).

Related to this, the global_data and bd_info structures now have a core set of
fields which are common to all architectures. Architecture-specific fields
have been moved to separate structures.


Further Background
~~~~~~~~~~~~~~~~~~

The full text of the original generic board series is reproduced below.

--8<-------------

This series creates a generic board.c implementation which contains
the essential functions of the major arch/xxx/lib/board.c files.

What is the motivation for this change?

1. There is a lot of repeated code in the board.c files. Any change to
things like setting up the baud rate requires a change in 10 separate
places.

2. Since there are 10 separate files, adding a new feature which requires
initialisation is painful since it must be independently added in 10
places.

3. As time goes by the architectures naturally diverge since there is limited
pressure to compare features or even CONFIG options against similar things
in other board.c files.

4. New architectures must implement all the features all over again, and
sometimes in subtle different ways. This places an unfair burden on getting
a new architecture fully functional and running with U-Boot.

5. While it is a bit of a tricky change, I believe it is worthwhile and
achievable. There is no requirement that all code be common, only that
the code that is common should be located in common/board.c rather than
arch/xxx/lib/board.c.

All the functions of board_init_f() and board_init_r() are broken into
separate function calls so that they can easily be included or excluded
for a particular architecture. It also makes it easier to adopt Graeme's
initcall proposal when it is ready.

http://lists.denx.de/pipermail/u-boot/2012-January/114499.html

This series removes the dependency on generic relocation. So relocation
happens as one big chunk and is still completely arch-specific. See the
relocation series for a proposed solution to this for ARM:

http://lists.denx.de/pipermail/u-boot/2011-December/112928.html

or Graeme's recent x86 series v2:

http://lists.denx.de/pipermail/u-boot/2012-January/114467.html

Instead of moving over a whole architecture, this series takes the approach
of simply enabling generic board support for an architecture. It is then up
to each board to opt in by defining CONFIG_SYS_GENERIC_BOARD in the board
config file. If this is not done, then the code will be generated as
before. This allows both sets of code to co-exist until we are comfortable
with the generic approach, and enough boards run.

ARM is a relatively large board.c file and one which I can test, therefore
I think it is a good target for this series. On the other hand, x86 is
relatively small and simple, but different enough that it introduces a
few issues to be solved. So I have chosen both ARM and x86 for this series.
After a suggestion from Wolfgang I have added PPC also. This is the
largest and most feature-full board, so hopefully we have all bases
covered in this RFC.

A generic global_data structure is also required. This might upset a few
people. Here is my basic reasoning: most fields are the same, all
architectures include and need it, most global_data.h files already have
#ifdefs to select fields for a particular SOC, so it is hard to
see why architecures are different in this area. We can perhaps add a
way to put architecture-specific fields into a separate header file, but
for now I have judged that to be counter-productive.

Similarly we need a generic bd_info structure, since generic code will
be accessing it. I have done this in the same way as global_data and the
same comments apply.

There was dicussion on the list about passing gd_t around as a parameter
to pre-relocation init functions. I think this makes sense, but it can
be done as a separate change, and this series does not require it.

While this series needs to stand on its own (as with the link script
cleanup series and the generic relocation series) the goal is the
unification of the board init code. So I hope we can address issues with
this in mind, rather than focusing too narrowly on particular ARM, x86 or
PPC issues.

I have run-tested ARM on Tegra Seaboard only. To try it out, define
CONFIG_SYS_GENERIC_BOARD in your board file and rebuild. Most likely on
x86 and PPC at least it will hang, but if you are lucky it will print
something first :-)

I have run this though MAKEALL with CONFIG_SYS_GENERIC_BOARD on for all
ARM, PPC and x86 boards. There are a few failures due to errors in
the board config, which I have sent patches for. The main issue is
just the difference between __bss_end and __bss_end__.

Note: the first group of commits are required for this series to build,
but could be separated out if required. I have included them here for
convenience.

------------->8--

Simon Glass, sjg@chromium.org
March 2014

Updated after final removal, May 2016

