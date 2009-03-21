#
# (C) Copyright 2000-2006
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
#

subdirs-$(CONFIG_CMD_CRAMFS) := cramfs
subdirs-$(CONFIG_CMD_EXT2) += ext2
subdirs-$(CONFIG_CMD_FAT) += fat
subdirs-$(CONFIG_CMD_FDOS) += fdos
subdirs-$(CONFIG_CMD_JFFS2) += jffs2
subdirs-$(CONFIG_CMD_REISER) += reiserfs
subdirs-$(CONFIG_YAFFS2) += yaffs2
subdirs-$(CONFIG_CMD_UBIFS) += ubifs

SUBDIRS	:= $(subdirs-y)

$(obj).depend all:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) -C $$dir $@ ; done
