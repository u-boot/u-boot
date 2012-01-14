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

import os
import subprocess

"""Shell command ease-ups for Python."""

def RunPipe(pipeline, infile=None, outfile=None,
            capture=False, oneline=False, hide_stderr=False):
    """
    Perform a command pipeline, with optional input/output filenames.

    hide_stderr     Don't allow output of stderr (default False)
    """
    last_pipe = None
    while pipeline:
        cmd = pipeline.pop(0)
        kwargs = {}
        if last_pipe is not None:
            kwargs['stdin'] = last_pipe.stdout
        elif infile:
            kwargs['stdin'] = open(infile, 'rb')
        if pipeline or capture:
            kwargs['stdout'] = subprocess.PIPE
        elif outfile:
            kwargs['stdout'] = open(outfile, 'wb')
        if hide_stderr:
            kwargs['stderr'] = open('/dev/null', 'wb')

        last_pipe = subprocess.Popen(cmd, **kwargs)

    if capture:
        ret = last_pipe.communicate()[0]
        if not ret:
            return None
        elif oneline:
            return ret.rstrip('\r\n')
        else:
            return ret
    else:
        return os.waitpid(last_pipe.pid, 0)[1] == 0

def Output(*cmd):
    return RunPipe([cmd], capture=True)

def OutputOneLine(*cmd):
    return RunPipe([cmd], capture=True, oneline=True)

def Run(*cmd, **kwargs):
    return RunPipe([cmd], **kwargs)

def RunList(cmd):
    return RunPipe([cmd], capture=True)
