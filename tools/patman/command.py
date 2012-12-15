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
import cros_subprocess

"""Shell command ease-ups for Python."""

class CommandResult:
    """A class which captures the result of executing a command.

    Members:
        stdout: stdout obtained from command, as a string
        stderr: stderr obtained from command, as a string
        return_code: Return code from command
        exception: Exception received, or None if all ok
    """
    def __init__(self):
        self.stdout = None
        self.stderr = None
        self.return_code = None
        self.exception = None


def RunPipe(pipe_list, infile=None, outfile=None,
            capture=False, capture_stderr=False, oneline=False,
            raise_on_error=True, cwd=None, **kwargs):
    """
    Perform a command pipeline, with optional input/output filenames.

    Args:
        pipe_list: List of command lines to execute. Each command line is
            piped into the next, and is itself a list of strings. For
            example [ ['ls', '.git'] ['wc'] ] will pipe the output of
            'ls .git' into 'wc'.
        infile: File to provide stdin to the pipeline
        outfile: File to store stdout
        capture: True to capture output
        capture_stderr: True to capture stderr
        oneline: True to strip newline chars from output
        kwargs: Additional keyword arguments to cros_subprocess.Popen()
    Returns:
        CommandResult object
    """
    result = CommandResult()
    last_pipe = None
    pipeline = list(pipe_list)
    user_pipestr =  '|'.join([' '.join(pipe) for pipe in pipe_list])
    while pipeline:
        cmd = pipeline.pop(0)
        if last_pipe is not None:
            kwargs['stdin'] = last_pipe.stdout
        elif infile:
            kwargs['stdin'] = open(infile, 'rb')
        if pipeline or capture:
            kwargs['stdout'] = cros_subprocess.PIPE
        elif outfile:
            kwargs['stdout'] = open(outfile, 'wb')
        if capture_stderr:
            kwargs['stderr'] = cros_subprocess.PIPE

        try:
            last_pipe = cros_subprocess.Popen(cmd, cwd=cwd, **kwargs)
        except Exception, err:
            result.exception = err
            if raise_on_error:
                raise Exception("Error running '%s': %s" % (user_pipestr, str))
            result.return_code = 255
            return result

    if capture:
        result.stdout, result.stderr, result.combined = (
                last_pipe.CommunicateFilter(None))
        if result.stdout and oneline:
            result.output = result.stdout.rstrip('\r\n')
        result.return_code = last_pipe.wait()
    else:
        result.return_code = os.waitpid(last_pipe.pid, 0)[1]
    if raise_on_error and result.return_code:
        raise Exception("Error running '%s'" % user_pipestr)
    return result

def Output(*cmd):
    return RunPipe([cmd], capture=True, raise_on_error=False).stdout

def OutputOneLine(*cmd, **kwargs):
    raise_on_error = kwargs.pop('raise_on_error', True)
    return (RunPipe([cmd], capture=True, oneline=True,
            raise_on_error=raise_on_error,
            **kwargs).stdout.strip())

def Run(*cmd, **kwargs):
    return RunPipe([cmd], **kwargs).stdout

def RunList(cmd):
    return RunPipe([cmd], capture=True).stdout

def StopAll():
    cros_subprocess.stay_alive = False
