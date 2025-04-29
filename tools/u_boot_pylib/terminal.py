# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
#

"""Terminal utilities

This module handles terminal interaction including ANSI color codes.
"""

from contextlib import contextmanager
from io import StringIO
import os
import re
import shutil
import subprocess
import sys

# Selection of when we want our output to be colored
COLOR_IF_TERMINAL, COLOR_ALWAYS, COLOR_NEVER = range(3)

# Initially, we are set up to print to the terminal
print_test_mode = False
print_test_list = []

# The length of the last line printed without a newline
last_print_len = None

# credit:
# stackoverflow.com/questions/14693701/how-can-i-remove-the-ansi-escape-sequences-from-a-string-in-python
ansi_escape = re.compile(r'\x1b(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')

# True if we are capturing console output
CAPTURING = False

# Set this to False to disable output-capturing globally
USE_CAPTURE = True


class PrintLine:
    """A line of text output

    Members:
        text: Text line that was printed
        newline: True to output a newline after the text
        colour: Text colour to use
    """
    def __init__(self, text, colour, newline=True, bright=True):
        self.text = text
        self.newline = newline
        self.colour = colour
        self.bright = bright

    def __eq__(self, other):
        return (self.text == other.text and
                self.newline == other.newline and
                self.colour == other.colour and
                self.bright == other.bright)

    def __str__(self):
        return ("newline=%s, colour=%s, bright=%d, text='%s'" %
                (self.newline, self.colour, self.bright, self.text))


def calc_ascii_len(text):
    """Calculate the length of a string, ignoring any ANSI sequences

    When displayed on a terminal, ANSI sequences don't take any space, so we
    need to ignore them when calculating the length of a string.

    Args:
        text: Text to check

    Returns:
        Length of text, after skipping ANSI sequences

    >>> col = Color(COLOR_ALWAYS)
    >>> text = col.build(Color.RED, 'abc')
    >>> len(text)
    14
    >>> calc_ascii_len(text)
    3
    >>>
    >>> text += 'def'
    >>> calc_ascii_len(text)
    6
    >>> text += col.build(Color.RED, 'abc')
    >>> calc_ascii_len(text)
    9
    """
    result = ansi_escape.sub('', text)
    return len(result)

def trim_ascii_len(text, size):
    """Trim a string containing ANSI sequences to the given ASCII length

    The string is trimmed with ANSI sequences being ignored for the length
    calculation.

    >>> col = Color(COLOR_ALWAYS)
    >>> text = col.build(Color.RED, 'abc')
    >>> len(text)
    14
    >>> calc_ascii_len(trim_ascii_len(text, 4))
    3
    >>> calc_ascii_len(trim_ascii_len(text, 2))
    2
    >>> text += 'def'
    >>> calc_ascii_len(trim_ascii_len(text, 4))
    4
    >>> text += col.build(Color.RED, 'ghi')
    >>> calc_ascii_len(trim_ascii_len(text, 7))
    7
    """
    if calc_ascii_len(text) < size:
        return text
    pos = 0
    out = ''
    left = size

    # Work through each ANSI sequence in turn
    for m in ansi_escape.finditer(text):
        # Find the text before the sequence and add it to our string, making
        # sure it doesn't overflow
        before = text[pos:m.start()]
        toadd = before[:left]
        out += toadd

        # Figure out how much non-ANSI space we have left
        left -= len(toadd)

        # Add the ANSI sequence and move to the position immediately after it
        out += m.group()
        pos = m.start() + len(m.group())

    # Deal with text after the last ANSI sequence
    after = text[pos:]
    toadd = after[:left]
    out += toadd

    return out


def tprint(text='', newline=True, colour=None, limit_to_line=False,
           bright=True, back=None, col=None):
    """Handle a line of output to the terminal.

    In test mode this is recorded in a list. Otherwise it is output to the
    terminal.

    Args:
        text: Text to print
        newline: True to add a new line at the end of the text
        colour: Colour to use for the text
    """
    global last_print_len

    if print_test_mode:
        print_test_list.append(PrintLine(text, colour, newline, bright))
    else:
        if colour is not None:
            if not col:
                col = Color()
            text = col.build(colour, text, bright=bright, back=back)
        if newline:
            print(text)
            last_print_len = None
        else:
            if limit_to_line:
                cols = shutil.get_terminal_size().columns
                text = trim_ascii_len(text, cols)
            print(text, end='', flush=True)
            last_print_len = calc_ascii_len(text)

def print_clear():
    """Clear a previously line that was printed with no newline"""
    global last_print_len

    if last_print_len:
        if print_test_mode:
            print_test_list.append(PrintLine(None, None, None, None))
        else:
            print('\r%s\r' % (' '* last_print_len), end='', flush=True)
            last_print_len = None

def set_print_test_mode(enable=True):
    """Go into test mode, where all printing is recorded"""
    global print_test_mode

    print_test_mode = enable
    get_print_test_lines()

def get_print_test_lines():
    """Get a list of all lines output through tprint()

    Returns:
        A list of PrintLine objects
    """
    global print_test_list

    ret = print_test_list
    print_test_list = []
    return ret

def echo_print_test_lines():
    """Print out the text lines collected"""
    for line in print_test_list:
        if line.colour:
            col = Color()
            print(col.build(line.colour, line.text), end='')
        else:
            print(line.text, end='')
        if line.newline:
            print()

def have_terminal():
    """Check if we have an interactive terminal or not

    Returns:
        bool: true if an interactive terminal is attached
    """
    return os.isatty(sys.stdout.fileno())


class Color():
    """Conditionally wraps text in ANSI color escape sequences."""
    BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE = range(8)
    BOLD = -1
    BRIGHT_START = '\033[1;%d%sm'
    NORMAL_START = '\033[22;%d%sm'
    BOLD_START = '\033[1m'
    BACK_EXTRA = ';%d'
    RESET = '\033[0m'

    def __init__(self, colored=COLOR_IF_TERMINAL):
        """Create a new Color object, optionally disabling color output.

        Args:
          enabled: True if color output should be enabled. If False then this
            class will not add color codes at all.
        """
        try:
            self._enabled = (colored == COLOR_ALWAYS or
                    (colored == COLOR_IF_TERMINAL and
                     os.isatty(sys.stdout.fileno())))
        except:
            self._enabled = False

    def enabled(self):
        """Check if colour is enabled

        Return: True if enabled, else False
        """
        return self._enabled

    def start(self, color, bright=True, back=None):
        """Returns a start color code.

        Args:
          color: Color to use, .e.g BLACK, RED, etc.

        Returns:
          If color is enabled, returns an ANSI sequence to start the given
          color, otherwise returns empty string
        """
        if self._enabled:
            if color == self.BOLD:
                return self.BOLD_START
            base = self.BRIGHT_START if bright else self.NORMAL_START
            extra = self.BACK_EXTRA % (back + 40) if back else ''
            return base % (color + 30, extra)
        return ''

    def stop(self):
        """Returns a stop color code.

        Returns:
          If color is enabled, returns an ANSI color reset sequence,
          otherwise returns empty string
        """
        if self._enabled:
            return self.RESET
        return ''

    def build(self, color, text, bright=True, back=None):
        """Returns text with conditionally added color escape sequences.

        Keyword arguments:
          color: Text color -- one of the color constants defined in this
                  class.
          text: The text to color.

        Returns:
          If self._enabled is False, returns the original text. If it's True,
          returns text with color escape sequences based on the value of
          color.
        """
        if not self._enabled:
            return text
        return self.start(color, bright, back) + text + self.RESET


# Use this to suppress stdout/stderr output:
# with terminal.capture() as (stdout, stderr)
#   ...do something...
@contextmanager
def capture():
    global CAPTURING

    capture_out, capture_err = StringIO(), StringIO()
    old_out, old_err = sys.stdout, sys.stderr
    try:
        CAPTURING = True
        sys.stdout, sys.stderr = capture_out, capture_err
        yield capture_out, capture_err
    finally:
        sys.stdout, sys.stderr = old_out, old_err
        CAPTURING = False
        if not USE_CAPTURE:
            sys.stdout.write(capture_out.getvalue())
            sys.stderr.write(capture_err.getvalue())


@contextmanager
def pager():
    """Simple pager for outputting lots of text

    Usage:
        with terminal.pager():
            print(...)
    """
    proc = None
    old_stdout = None
    try:
        less = os.getenv('PAGER')
        if not CAPTURING and less != 'none' and have_terminal():
            if not less:
                less = 'less -R --quit-if-one-screen'
            proc = subprocess.Popen(less, stdin=subprocess.PIPE, text=True,
                                    shell=True)
            old_stdout = sys.stdout
            sys.stdout = proc.stdin
        yield
    finally:
        if proc:
            sys.stdout = old_stdout
            proc.communicate()
