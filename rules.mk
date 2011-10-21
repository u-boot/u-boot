#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

#########################################################################

_depend:	$(obj).depend

# Split the source files into two camps: those in the current directory, and
# those somewhere else. For the first camp we want to support CPPFLAGS_<fname>
# and for the second we don't / can't.
PWD_SRCS := $(filter $(notdir $(SRCS)),$(SRCS))
OTHER_SRCS := $(filter-out $(notdir $(SRCS)),$(SRCS))

# This is a list of dependency files to generate
DEPS := $(basename $(patsubst %,$(obj).depend.%,$(PWD_SRCS)))

# Join all the dependencies into a single file, in three parts
#	1 .Concatenate all the generated depend files together
#	2. Add in the deps from OTHER_SRCS which we couldn't process
#	3. Add in the HOSTSRCS
$(obj).depend:	$(src)Makefile $(TOPDIR)/config.mk $(DEPS) $(OTHER_SRCS) \
		$(HOSTSRCS)
	cat /dev/null $(DEPS) >$@
	@for f in $(OTHER_SRCS); do \
		g=`basename $$f | sed -e 's/\(.*\)\.[[:alnum:]_]/\1.o/'`; \
		$(CC) -M $(CPPFLAGS) -MQ $(obj)$$g $$f >> $@ ; \
	done
	@for f in $(HOSTSRCS); do \
		g=`basename $$f | sed -e 's/\(.*\)\.[[:alnum:]_]/\1.o/'`; \
		$(HOSTCC) -M $(HOSTCPPFLAGS) -MQ $(obj)$$g $$f >> $@ ; \
	done

MAKE_DEPEND = $(CC) -M $(CPPFLAGS) $(EXTRA_CPPFLAGS_DEP) \
		-MQ $(addsuffix .o,$(obj)$(basename $<)) $< >$@


$(obj).depend.%:	%.c
	$(MAKE_DEPEND)

$(obj).depend.%:	%.S
	$(MAKE_DEPEND)

$(HOSTOBJS): $(obj)%.o: %.c
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTCFLAGS_$(@F)) $(HOSTCFLAGS_$(BCURDIR)) -o $@ $< -c
$(NOPEDOBJS): $(obj)%.o: %.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTCFLAGS_$(@F)) $(HOSTCFLAGS_$(BCURDIR)) -o $@ $< -c

#########################################################################
