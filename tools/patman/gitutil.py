# Copyright (c) 2011 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

import command
import re
import os
import series
import subprocess
import sys
import terminal

import settings


def CountCommitsToBranch():
    """Returns number of commits between HEAD and the tracking branch.

    This looks back to the tracking branch and works out the number of commits
    since then.

    Return:
        Number of patches that exist on top of the branch
    """
    pipe = [['git', 'log', '--no-color', '--oneline', '--no-decorate',
             '@{upstream}..'],
            ['wc', '-l']]
    stdout = command.RunPipe(pipe, capture=True, oneline=True).stdout
    patch_count = int(stdout)
    return patch_count

def GetUpstream(git_dir, branch):
    """Returns the name of the upstream for a branch

    Args:
        git_dir: Git directory containing repo
        branch: Name of branch

    Returns:
        Name of upstream branch (e.g. 'upstream/master') or None if none
    """
    try:
        remote = command.OutputOneLine('git', '--git-dir', git_dir, 'config',
                                       'branch.%s.remote' % branch)
        merge = command.OutputOneLine('git', '--git-dir', git_dir, 'config',
                                      'branch.%s.merge' % branch)
    except:
        return None

    if remote == '.':
        return merge
    elif remote and merge:
        leaf = merge.split('/')[-1]
        return '%s/%s' % (remote, leaf)
    else:
        raise ValueError, ("Cannot determine upstream branch for branch "
                "'%s' remote='%s', merge='%s'" % (branch, remote, merge))


def GetRangeInBranch(git_dir, branch, include_upstream=False):
    """Returns an expression for the commits in the given branch.

    Args:
        git_dir: Directory containing git repo
        branch: Name of branch
    Return:
        Expression in the form 'upstream..branch' which can be used to
        access the commits. If the branch does not exist, returns None.
    """
    upstream = GetUpstream(git_dir, branch)
    if not upstream:
        return None
    return '%s%s..%s' % (upstream, '~' if include_upstream else '', branch)

def CountCommitsInBranch(git_dir, branch, include_upstream=False):
    """Returns the number of commits in the given branch.

    Args:
        git_dir: Directory containing git repo
        branch: Name of branch
    Return:
        Number of patches that exist on top of the branch, or None if the
        branch does not exist.
    """
    range_expr = GetRangeInBranch(git_dir, branch, include_upstream)
    if not range_expr:
        return None
    pipe = [['git', '--git-dir', git_dir, 'log', '--oneline', '--no-decorate',
             range_expr],
            ['wc', '-l']]
    result = command.RunPipe(pipe, capture=True, oneline=True)
    patch_count = int(result.stdout)
    return patch_count

def CountCommits(commit_range):
    """Returns the number of commits in the given range.

    Args:
        commit_range: Range of commits to count (e.g. 'HEAD..base')
    Return:
        Number of patches that exist on top of the branch
    """
    pipe = [['git', 'log', '--oneline', '--no-decorate', commit_range],
            ['wc', '-l']]
    stdout = command.RunPipe(pipe, capture=True, oneline=True).stdout
    patch_count = int(stdout)
    return patch_count

def Checkout(commit_hash, git_dir=None, work_tree=None, force=False):
    """Checkout the selected commit for this build

    Args:
        commit_hash: Commit hash to check out
    """
    pipe = ['git']
    if git_dir:
        pipe.extend(['--git-dir', git_dir])
    if work_tree:
        pipe.extend(['--work-tree', work_tree])
    pipe.append('checkout')
    if force:
        pipe.append('-f')
    pipe.append(commit_hash)
    result = command.RunPipe([pipe], capture=True, raise_on_error=False)
    if result.return_code != 0:
        raise OSError, 'git checkout (%s): %s' % (pipe, result.stderr)

def Clone(git_dir, output_dir):
    """Checkout the selected commit for this build

    Args:
        commit_hash: Commit hash to check out
    """
    pipe = ['git', 'clone', git_dir, '.']
    result = command.RunPipe([pipe], capture=True, cwd=output_dir)
    if result.return_code != 0:
        raise OSError, 'git clone: %s' % result.stderr

def Fetch(git_dir=None, work_tree=None):
    """Fetch from the origin repo

    Args:
        commit_hash: Commit hash to check out
    """
    pipe = ['git']
    if git_dir:
        pipe.extend(['--git-dir', git_dir])
    if work_tree:
        pipe.extend(['--work-tree', work_tree])
    pipe.append('fetch')
    result = command.RunPipe([pipe], capture=True)
    if result.return_code != 0:
        raise OSError, 'git fetch: %s' % result.stderr

def CreatePatches(start, count, series):
    """Create a series of patches from the top of the current branch.

    The patch files are written to the current directory using
    git format-patch.

    Args:
        start: Commit to start from: 0=HEAD, 1=next one, etc.
        count: number of commits to include
    Return:
        Filename of cover letter
        List of filenames of patch files
    """
    if series.get('version'):
        version = '%s ' % series['version']
    cmd = ['git', 'format-patch', '-M', '--signoff']
    if series.get('cover'):
        cmd.append('--cover-letter')
    prefix = series.GetPatchPrefix()
    if prefix:
        cmd += ['--subject-prefix=%s' % prefix]
    cmd += ['HEAD~%d..HEAD~%d' % (start + count, start)]

    stdout = command.RunList(cmd)
    files = stdout.splitlines()

    # We have an extra file if there is a cover letter
    if series.get('cover'):
       return files[0], files[1:]
    else:
       return None, files

def ApplyPatch(verbose, fname):
    """Apply a patch with git am to test it

    TODO: Convert these to use command, with stderr option

    Args:
        fname: filename of patch file to apply
    """
    cmd = ['git', 'am', fname]
    pipe = subprocess.Popen(cmd, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
    stdout, stderr = pipe.communicate()
    re_error = re.compile('^error: patch failed: (.+):(\d+)')
    for line in stderr.splitlines():
        if verbose:
            print line
        match = re_error.match(line)
        if match:
            print GetWarningMsg('warning', match.group(1), int(match.group(2)),
                    'Patch failed')
    return pipe.returncode == 0, stdout

def ApplyPatches(verbose, args, start_point):
    """Apply the patches with git am to make sure all is well

    Args:
        verbose: Print out 'git am' output verbatim
        args: List of patch files to apply
        start_point: Number of commits back from HEAD to start applying.
            Normally this is len(args), but it can be larger if a start
            offset was given.
    """
    error_count = 0
    col = terminal.Color()

    # Figure out our current position
    cmd = ['git', 'name-rev', 'HEAD', '--name-only']
    pipe = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    stdout, stderr = pipe.communicate()
    if pipe.returncode:
        str = 'Could not find current commit name'
        print col.Color(col.RED, str)
        print stdout
        return False
    old_head = stdout.splitlines()[0]

    # Checkout the required start point
    cmd = ['git', 'checkout', 'HEAD~%d' % start_point]
    pipe = subprocess.Popen(cmd, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
    stdout, stderr = pipe.communicate()
    if pipe.returncode:
        str = 'Could not move to commit before patch series'
        print col.Color(col.RED, str)
        print stdout, stderr
        return False

    # Apply all the patches
    for fname in args:
        ok, stdout = ApplyPatch(verbose, fname)
        if not ok:
            print col.Color(col.RED, 'git am returned errors for %s: will '
                    'skip this patch' % fname)
            if verbose:
                print stdout
            error_count += 1
            cmd = ['git', 'am', '--skip']
            pipe = subprocess.Popen(cmd, stdout=subprocess.PIPE)
            stdout, stderr = pipe.communicate()
            if pipe.returncode != 0:
                print col.Color(col.RED, 'Unable to skip patch! Aborting...')
                print stdout
                break

    # Return to our previous position
    cmd = ['git', 'checkout', old_head]
    pipe = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = pipe.communicate()
    if pipe.returncode:
        print col.Color(col.RED, 'Could not move back to head commit')
        print stdout, stderr
    return error_count == 0

def BuildEmailList(in_list, tag=None, alias=None, raise_on_error=True):
    """Build a list of email addresses based on an input list.

    Takes a list of email addresses and aliases, and turns this into a list
    of only email address, by resolving any aliases that are present.

    If the tag is given, then each email address is prepended with this
    tag and a space. If the tag starts with a minus sign (indicating a
    command line parameter) then the email address is quoted.

    Args:
        in_list:        List of aliases/email addresses
        tag:            Text to put before each address
        alias:          Alias dictionary
        raise_on_error: True to raise an error when an alias fails to match,
                False to just print a message.

    Returns:
        List of email addresses

    >>> alias = {}
    >>> alias['fred'] = ['f.bloggs@napier.co.nz']
    >>> alias['john'] = ['j.bloggs@napier.co.nz']
    >>> alias['mary'] = ['Mary Poppins <m.poppins@cloud.net>']
    >>> alias['boys'] = ['fred', ' john']
    >>> alias['all'] = ['fred ', 'john', '   mary   ']
    >>> BuildEmailList(['john', 'mary'], None, alias)
    ['j.bloggs@napier.co.nz', 'Mary Poppins <m.poppins@cloud.net>']
    >>> BuildEmailList(['john', 'mary'], '--to', alias)
    ['--to "j.bloggs@napier.co.nz"', \
'--to "Mary Poppins <m.poppins@cloud.net>"']
    >>> BuildEmailList(['john', 'mary'], 'Cc', alias)
    ['Cc j.bloggs@napier.co.nz', 'Cc Mary Poppins <m.poppins@cloud.net>']
    """
    quote = '"' if tag and tag[0] == '-' else ''
    raw = []
    for item in in_list:
        raw += LookupEmail(item, alias, raise_on_error=raise_on_error)
    result = []
    for item in raw:
        if not item in result:
            result.append(item)
    if tag:
        return ['%s %s%s%s' % (tag, quote, email, quote) for email in result]
    return result

def EmailPatches(series, cover_fname, args, dry_run, raise_on_error, cc_fname,
        self_only=False, alias=None, in_reply_to=None):
    """Email a patch series.

    Args:
        series: Series object containing destination info
        cover_fname: filename of cover letter
        args: list of filenames of patch files
        dry_run: Just return the command that would be run
        raise_on_error: True to raise an error when an alias fails to match,
                False to just print a message.
        cc_fname: Filename of Cc file for per-commit Cc
        self_only: True to just email to yourself as a test
        in_reply_to: If set we'll pass this to git as --in-reply-to.
            Should be a message ID that this is in reply to.

    Returns:
        Git command that was/would be run

    # For the duration of this doctest pretend that we ran patman with ./patman
    >>> _old_argv0 = sys.argv[0]
    >>> sys.argv[0] = './patman'

    >>> alias = {}
    >>> alias['fred'] = ['f.bloggs@napier.co.nz']
    >>> alias['john'] = ['j.bloggs@napier.co.nz']
    >>> alias['mary'] = ['m.poppins@cloud.net']
    >>> alias['boys'] = ['fred', ' john']
    >>> alias['all'] = ['fred ', 'john', '   mary   ']
    >>> alias[os.getenv('USER')] = ['this-is-me@me.com']
    >>> series = series.Series()
    >>> series.to = ['fred']
    >>> series.cc = ['mary']
    >>> EmailPatches(series, 'cover', ['p1', 'p2'], True, True, 'cc-fname', \
            False, alias)
    'git send-email --annotate --to "f.bloggs@napier.co.nz" --cc \
"m.poppins@cloud.net" --cc-cmd "./patman --cc-cmd cc-fname" cover p1 p2'
    >>> EmailPatches(series, None, ['p1'], True, True, 'cc-fname', False, \
            alias)
    'git send-email --annotate --to "f.bloggs@napier.co.nz" --cc \
"m.poppins@cloud.net" --cc-cmd "./patman --cc-cmd cc-fname" p1'
    >>> series.cc = ['all']
    >>> EmailPatches(series, 'cover', ['p1', 'p2'], True, True, 'cc-fname', \
            True, alias)
    'git send-email --annotate --to "this-is-me@me.com" --cc-cmd "./patman \
--cc-cmd cc-fname" cover p1 p2'
    >>> EmailPatches(series, 'cover', ['p1', 'p2'], True, True, 'cc-fname', \
            False, alias)
    'git send-email --annotate --to "f.bloggs@napier.co.nz" --cc \
"f.bloggs@napier.co.nz" --cc "j.bloggs@napier.co.nz" --cc \
"m.poppins@cloud.net" --cc-cmd "./patman --cc-cmd cc-fname" cover p1 p2'

    # Restore argv[0] since we clobbered it.
    >>> sys.argv[0] = _old_argv0
    """
    to = BuildEmailList(series.get('to'), '--to', alias, raise_on_error)
    if not to:
        print ("No recipient, please add something like this to a commit\n"
            "Series-to: Fred Bloggs <f.blogs@napier.co.nz>")
        return
    cc = BuildEmailList(series.get('cc'), '--cc', alias, raise_on_error)
    if self_only:
        to = BuildEmailList([os.getenv('USER')], '--to', alias, raise_on_error)
        cc = []
    cmd = ['git', 'send-email', '--annotate']
    if in_reply_to:
        cmd.append('--in-reply-to="%s"' % in_reply_to)

    cmd += to
    cmd += cc
    cmd += ['--cc-cmd', '"%s --cc-cmd %s"' % (sys.argv[0], cc_fname)]
    if cover_fname:
        cmd.append(cover_fname)
    cmd += args
    str = ' '.join(cmd)
    if not dry_run:
        os.system(str)
    return str


def LookupEmail(lookup_name, alias=None, raise_on_error=True, level=0):
    """If an email address is an alias, look it up and return the full name

    TODO: Why not just use git's own alias feature?

    Args:
        lookup_name: Alias or email address to look up
        alias: Dictionary containing aliases (None to use settings default)
        raise_on_error: True to raise an error when an alias fails to match,
                False to just print a message.

    Returns:
        tuple:
            list containing a list of email addresses

    Raises:
        OSError if a recursive alias reference was found
        ValueError if an alias was not found

    >>> alias = {}
    >>> alias['fred'] = ['f.bloggs@napier.co.nz']
    >>> alias['john'] = ['j.bloggs@napier.co.nz']
    >>> alias['mary'] = ['m.poppins@cloud.net']
    >>> alias['boys'] = ['fred', ' john', 'f.bloggs@napier.co.nz']
    >>> alias['all'] = ['fred ', 'john', '   mary   ']
    >>> alias['loop'] = ['other', 'john', '   mary   ']
    >>> alias['other'] = ['loop', 'john', '   mary   ']
    >>> LookupEmail('mary', alias)
    ['m.poppins@cloud.net']
    >>> LookupEmail('arthur.wellesley@howe.ro.uk', alias)
    ['arthur.wellesley@howe.ro.uk']
    >>> LookupEmail('boys', alias)
    ['f.bloggs@napier.co.nz', 'j.bloggs@napier.co.nz']
    >>> LookupEmail('all', alias)
    ['f.bloggs@napier.co.nz', 'j.bloggs@napier.co.nz', 'm.poppins@cloud.net']
    >>> LookupEmail('odd', alias)
    Traceback (most recent call last):
    ...
    ValueError: Alias 'odd' not found
    >>> LookupEmail('loop', alias)
    Traceback (most recent call last):
    ...
    OSError: Recursive email alias at 'other'
    >>> LookupEmail('odd', alias, raise_on_error=False)
    \033[1;31mAlias 'odd' not found\033[0m
    []
    >>> # In this case the loop part will effectively be ignored.
    >>> LookupEmail('loop', alias, raise_on_error=False)
    \033[1;31mRecursive email alias at 'other'\033[0m
    \033[1;31mRecursive email alias at 'john'\033[0m
    \033[1;31mRecursive email alias at 'mary'\033[0m
    ['j.bloggs@napier.co.nz', 'm.poppins@cloud.net']
    """
    if not alias:
        alias = settings.alias
    lookup_name = lookup_name.strip()
    if '@' in lookup_name: # Perhaps a real email address
        return [lookup_name]

    lookup_name = lookup_name.lower()
    col = terminal.Color()

    out_list = []
    if level > 10:
        msg = "Recursive email alias at '%s'" % lookup_name
        if raise_on_error:
            raise OSError, msg
        else:
            print col.Color(col.RED, msg)
            return out_list

    if lookup_name:
        if not lookup_name in alias:
            msg = "Alias '%s' not found" % lookup_name
            if raise_on_error:
                raise ValueError, msg
            else:
                print col.Color(col.RED, msg)
                return out_list
        for item in alias[lookup_name]:
            todo = LookupEmail(item, alias, raise_on_error, level + 1)
            for new_item in todo:
                if not new_item in out_list:
                    out_list.append(new_item)

    #print "No match for alias '%s'" % lookup_name
    return out_list

def GetTopLevel():
    """Return name of top-level directory for this git repo.

    Returns:
        Full path to git top-level directory

    This test makes sure that we are running tests in the right subdir

    >>> os.path.realpath(os.path.dirname(__file__)) == \
            os.path.join(GetTopLevel(), 'tools', 'patman')
    True
    """
    return command.OutputOneLine('git', 'rev-parse', '--show-toplevel')

def GetAliasFile():
    """Gets the name of the git alias file.

    Returns:
        Filename of git alias file, or None if none
    """
    fname = command.OutputOneLine('git', 'config', 'sendemail.aliasesfile',
            raise_on_error=False)
    if fname:
        fname = os.path.join(GetTopLevel(), fname.strip())
    return fname

def GetDefaultUserName():
    """Gets the user.name from .gitconfig file.

    Returns:
        User name found in .gitconfig file, or None if none
    """
    uname = command.OutputOneLine('git', 'config', '--global', 'user.name')
    return uname

def GetDefaultUserEmail():
    """Gets the user.email from the global .gitconfig file.

    Returns:
        User's email found in .gitconfig file, or None if none
    """
    uemail = command.OutputOneLine('git', 'config', '--global', 'user.email')
    return uemail

def Setup():
    """Set up git utils, by reading the alias files."""
    # Check for a git alias file also
    alias_fname = GetAliasFile()
    if alias_fname:
        settings.ReadGitAliases(alias_fname)

def GetHead():
    """Get the hash of the current HEAD

    Returns:
        Hash of HEAD
    """
    return command.OutputOneLine('git', 'show', '-s', '--pretty=format:%H')

if __name__ == "__main__":
    import doctest

    doctest.testmod()
