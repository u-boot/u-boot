#
# Copyright (C) 2012 Marek Vasut <marex@denx.de>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
#########################################################################

##
# make_u_boot_list - Generate contents of u_boot_list section
# 1:		The name of the resulting file (usually u-boot.lst)
# 2:		Files to analyze for possible u_boot_list entries
#
# This function generates the contents of the u_boot_list section,
# including all the border symbols for it's subsections. The operation
# of this function is as follows, numbering goes per lines:
#
# 1) Dump the ELF header sections from all files supplied via $(2)
# 2) Filter out all other stuff that does not belong into .u_boot_list
#    section.
# 3) Fix up the lines so that the resulting output is is in format
#    ".u_boot_list.*".
# 4) Remove the last .something$, since that only contains the name
#    of the variable to be put into a subsection. This name is irelevant
#    for generation of border symbols, thus of no interest, remove it.
# 5) Take each line and for every dot "." in that line, print the whole
#    line until that dot "." . This is important so that we have all
#    parent border symbols generated as well.
# 6) Load every line and firstly append "\a" at the end and print the
#    line. Next, append "@" at the end and print the line. Finally,
#    append "~" at the end of line. This will make sense in conjunction
#    with 6) and 7).
# 7) Sort the lines. It is imperative to use LC_COLLATE=C here because
#    with this, the "\a" symbol is first and "~" symbol is last. Any
#    other symbols fall inbetween. Symbols like "@", which marks the
#    end of current line (representing current section) and ".", which
#    means the line continues and thus represents subsection.
# 8) With such ordering, all lines ending with "\a" will float at the
#    begining of all lines with the same prefix. Thus it is easy to
#    replace "\a" with __start and make it the __start border symbol.
#    Very similarly for "~", which will be always at the bottom and so
#    can be replaced by "__end" and made into the __end border symbol.
#    Finally, every line ending with "@" symbol will be transformed
#    into " *(SORT(${line}*)); " format, which in the linker parlance
#    will allow it to trap all symbols relevant to the subsection.
#
define make_u_boot_list
$(1): $(2)
	$(OBJDUMP) -h $(2) | \
	sed -n -e '/.*\.u_boot_list[^ ]\+/ ! {d;n}' \
		-e 's/.*\(\.u_boot_list[^ ]\+\).*$$$$/\1/' \
		-e 's/\.[^\.]\+$$$$//' \
		-e ':s /^.\+$$$$/ { p;s/^\(.*\)\.[^\.]*$$$$/\1/;b s }' | \
	sed -n -e 'h;s/$$$$/\a/p;g;s/$$$$/@/p;g;s/$$$$/~/p;' | \
	LC_COLLATE=C sort -u | \
	sed -n -e '/\a$$$$/ { s/\./_/g;s/\a$$$$/__start = .;/p; }'\
		-e '/~$$$$/ { s/\./_/g;s/~$$$$/__end = .;/p; }'\
		-e '/@$$$$/ { s/\(.*\)@$$$$/*(SORT(\1.*));/p }' > $(1)
endef
