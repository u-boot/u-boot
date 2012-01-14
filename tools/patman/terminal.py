# Copyright (c) 2011 The Chromium OS Authors.
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

"""Terminal utilities

This module handles terminal interaction including ANSI color codes.
"""

class Color(object):
  """Conditionally wraps text in ANSI color escape sequences."""
  BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE = range(8)
  BOLD = -1
  COLOR_START = '\033[1;%dm'
  BOLD_START = '\033[1m'
  RESET = '\033[0m'

  def __init__(self, enabled=True):
    """Create a new Color object, optionally disabling color output.

    Args:
      enabled: True if color output should be enabled. If False then this
        class will not add color codes at all.
    """
    self._enabled = enabled

  def Start(self, color):
    """Returns a start color code.

    Args:
      color: Color to use, .e.g BLACK, RED, etc.

    Returns:
      If color is enabled, returns an ANSI sequence to start the given color,
      otherwise returns empty string
    """
    if self._enabled:
      return self.COLOR_START % (color + 30)
    return ''

  def Stop(self):
    """Retruns a stop color code.

    Returns:
      If color is enabled, returns an ANSI color reset sequence, otherwise
      returns empty string
    """
    if self._enabled:
      return self.RESET
    return ''

  def Color(self, color, text):
    """Returns text with conditionally added color escape sequences.

    Keyword arguments:
      color: Text color -- one of the color constants defined in this class.
      text: The text to color.

    Returns:
      If self._enabled is False, returns the original text. If it's True,
      returns text with color escape sequences based on the value of color.
    """
    if not self._enabled:
      return text
    if color == self.BOLD:
      start = self.BOLD_START
    else:
      start = self.COLOR_START % (color + 30)
    return start + text + self.RESET
