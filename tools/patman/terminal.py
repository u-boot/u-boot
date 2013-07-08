# Copyright (c) 2011 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

"""Terminal utilities

This module handles terminal interaction including ANSI color codes.
"""

import os
import sys

# Selection of when we want our output to be colored
COLOR_IF_TERMINAL, COLOR_ALWAYS, COLOR_NEVER = range(3)

class Color(object):
  """Conditionally wraps text in ANSI color escape sequences."""
  BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE = range(8)
  BOLD = -1
  BRIGHT_START = '\033[1;%dm'
  NORMAL_START = '\033[22;%dm'
  BOLD_START = '\033[1m'
  RESET = '\033[0m'

  def __init__(self, colored=COLOR_IF_TERMINAL):
    """Create a new Color object, optionally disabling color output.

    Args:
      enabled: True if color output should be enabled. If False then this
        class will not add color codes at all.
    """
    self._enabled = (colored == COLOR_ALWAYS or
        (colored == COLOR_IF_TERMINAL and os.isatty(sys.stdout.fileno())))

  def Start(self, color, bright=True):
    """Returns a start color code.

    Args:
      color: Color to use, .e.g BLACK, RED, etc.

    Returns:
      If color is enabled, returns an ANSI sequence to start the given color,
      otherwise returns empty string
    """
    if self._enabled:
        base = self.BRIGHT_START if bright else self.NORMAL_START
        return base % (color + 30)
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

  def Color(self, color, text, bright=True):
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
        base = self.BRIGHT_START if bright else self.NORMAL_START
        start = base % (color + 30)
    return start + text + self.RESET
