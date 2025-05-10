# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
#

"""Basic utilities for running the git command-line tool from Python"""

import os
import sys

from u_boot_pylib import command
from u_boot_pylib import terminal

# True to use --no-decorate - we check this in setup()
USE_NO_DECORATE = True


def log_cmd(commit_range, git_dir=None, oneline=False, reverse=False,
            count=None, decorate=False):
    """Create a command to perform a 'git log'

    Args:
        commit_range (str): Range expression to use for log, None for none
        git_dir (str): Path to git repository (None to use default)
        oneline (bool): True to use --oneline, else False
        reverse (bool): True to reverse the log (--reverse)
        count (int or None): Number of commits to list, or None for no limit
        decorate (bool): True to use --decorate

    Return:
        List containing command and arguments to run
    """
    cmd = ['git']
    if git_dir:
        cmd += ['--git-dir', git_dir]
    cmd += ['--no-pager', 'log', '--no-color']
    if oneline:
        cmd.append('--oneline')
    if USE_NO_DECORATE and not decorate:
        cmd.append('--no-decorate')
    if decorate:
        cmd.append('--decorate')
    if reverse:
        cmd.append('--reverse')
    if count is not None:
        cmd.append(f'-n{count}')
    if commit_range:
        cmd.append(commit_range)

    # Add this in case we have a branch with the same name as a directory.
    # This avoids messages like this, for example:
    #   fatal: ambiguous argument 'test': both revision and filename
    cmd.append('--')
    return cmd


def count_commits_to_branch(branch, git_dir=None, end=None):
    """Returns number of commits between HEAD and the tracking branch.

    This looks back to the tracking branch and works out the number of commits
    since then.

    Args:
        branch (str or None): Branch to count from (None for current branch)
        git_dir (str): Path to git repository (None to use default)
        end (str): End commit to stop before

    Return:
        Number of patches that exist on top of the branch
    """
    if end:
        rev_range = f'{end}..{branch}'
    elif branch:
        us, msg = get_upstream(git_dir or '.git', branch)
        if not us:
            raise ValueError(msg)
        rev_range = f'{us}..{branch}'
    else:
        rev_range = '@{upstream}..'
    cmd = log_cmd(rev_range, git_dir=git_dir, oneline=True)
    result = command.run_one(*cmd, capture=True, capture_stderr=True,
                             oneline=True, raise_on_error=False)
    if result.return_code:
        raise ValueError(
            f'Failed to determine upstream: {result.stderr.strip()}')
    patch_count = len(result.stdout.splitlines())
    return patch_count


def name_revision(commit_hash):
    """Gets the revision name for a commit

    Args:
        commit_hash (str): Commit hash to look up

    Return:
        Name of revision, if any, else None
    """
    stdout = command.output_one_line('git', 'name-rev', commit_hash)
    if not stdout:
        return None

    # We expect a commit, a space, then a revision name
    name = stdout.split()[1].strip()
    return name


def guess_upstream(git_dir, branch):
    """Tries to guess the upstream for a branch

    This lists out top commits on a branch and tries to find a suitable
    upstream. It does this by looking for the first commit where
    'git name-rev' returns a plain branch name, with no ! or ^ modifiers.

    Args:
        git_dir (str): Git directory containing repo
        branch (str): Name of branch

    Returns:
        Tuple:
            Name of upstream branch (e.g. 'upstream/master') or None if none
            Warning/error message, or None if none
    """
    cmd = log_cmd(branch, git_dir=git_dir, oneline=True, count=100,
                  decorate=True)
    result = command.run_one(*cmd, capture=True, capture_stderr=True,
                             raise_on_error=False)
    if result.return_code:
        return None, f"Branch '{branch}' not found"
    for line in result.stdout.splitlines()[1:]:
        parts = line.split(maxsplit=1)
        if len(parts) >= 2 and parts[1].startswith('('):
            commit_hash = parts[0]
            name = name_revision(commit_hash)
            if '~' not in name and '^' not in name:
                if name.startswith('remotes/'):
                    name = name[8:]
                return name, f"Guessing upstream as '{name}'"
    return None, f"Cannot find a suitable upstream for branch '{branch}'"


def get_upstream(git_dir, branch):
    """Returns the name of the upstream for a branch

    Args:
        git_dir (str): Git directory containing repo
        branch (str): Name of branch

    Returns:
        Tuple:
            Name of upstream branch (e.g. 'upstream/master') or None if none
            Warning/error message, or None if none
    """
    try:
        remote = command.output_one_line('git', '--git-dir', git_dir, 'config',
                                         f'branch.{branch}.remote')
        merge = command.output_one_line('git', '--git-dir', git_dir, 'config',
                                        f'branch.{branch}.merge')
    except command.CommandExc:
        upstream, msg = guess_upstream(git_dir, branch)
        return upstream, msg

    if remote == '.':
        return merge, None
    if remote and merge:
        # Drop the initial refs/heads from merge
        leaf = merge.split('/', maxsplit=2)[2:]
        return f'{remote}/{"/".join(leaf)}', None
    raise ValueError("Cannot determine upstream branch for branch "
                     f"'{branch}' remote='{remote}', merge='{merge}'")


def get_range_in_branch(git_dir, branch, include_upstream=False):
    """Returns an expression for the commits in the given branch.

    Args:
        git_dir (str): Directory containing git repo
        branch (str): Name of branch
        include_upstream (bool): Include the upstream commit as well
    Return:
        Expression in the form 'upstream..branch' which can be used to
        access the commits. If the branch does not exist, returns None.
    """
    upstream, msg = get_upstream(git_dir, branch)
    if not upstream:
        return None, msg
    rstr = f"{upstream}{'~' if include_upstream else ''}..{branch}"
    return rstr, msg


def count_commits_in_range(git_dir, range_expr):
    """Returns the number of commits in the given range.

    Args:
        git_dir (str): Directory containing git repo
        range_expr (str): Range to check
    Return:
        Number of patches that exist in the supplied range or None if none
        were found
    """
    cmd = log_cmd(range_expr, git_dir=git_dir, oneline=True)
    result = command.run_one(*cmd, capture=True, capture_stderr=True,
                             raise_on_error=False)
    if result.return_code:
        return None, f"Range '{range_expr}' not found or is invalid"
    patch_count = len(result.stdout.splitlines())
    return patch_count, None


def count_commits_in_branch(git_dir, branch, include_upstream=False):
    """Returns the number of commits in the given branch.

    Args:
        git_dir (str): Directory containing git repo
        branch (str): Name of branch
        include_upstream (bool): Include the upstream commit as well
    Return:
        Number of patches that exist on top of the branch, or None if the
        branch does not exist.
    """
    range_expr, msg = get_range_in_branch(git_dir, branch, include_upstream)
    if not range_expr:
        return None, msg
    return count_commits_in_range(git_dir, range_expr)


def count_commits(commit_range):
    """Returns the number of commits in the given range.

    Args:
        commit_range (str): Range of commits to count (e.g. 'HEAD..base')
    Return:
        Number of patches that exist on top of the branch
    """
    pipe = [log_cmd(commit_range, oneline=True),
            ['wc', '-l']]
    stdout = command.run_pipe(pipe, capture=True, oneline=True).stdout
    patch_count = int(stdout)
    return patch_count


def checkout(commit_hash, git_dir=None, work_tree=None, force=False):
    """Checkout the selected commit for this build

    Args:
        commit_hash (str): Commit hash to check out
        git_dir (str): Directory containing git repo, or None for current dir
        work_tree (str): Git worktree to use, or None if none
        force (bool): True to force the checkout (git checkout -f)
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
    result = command.run_pipe([pipe], capture=True, raise_on_error=False,
                              capture_stderr=True)
    if result.return_code != 0:
        raise OSError(f'git checkout ({pipe}): {result.stderr}')


def clone(repo, output_dir):
    """Clone a repo

    Args:
        repo (str): Repo to clone (e.g. web address)
        output_dir (str): Directory to close into
    """
    result = command.run_one('git', 'clone', repo, '.', capture=True,
                             cwd=output_dir, capture_stderr=True)
    if result.return_code != 0:
        raise OSError(f'git clone: {result.stderr}')


def fetch(git_dir=None, work_tree=None):
    """Fetch from the origin repo

    Args:
        git_dir (str): Directory containing git repo, or None for current dir
        work_tree (str or None): Git worktree to use, or None if none
    """
    cmd = ['git']
    if git_dir:
        cmd.extend(['--git-dir', git_dir])
    if work_tree:
        cmd.extend(['--work-tree', work_tree])
    cmd.append('fetch')
    result = command.run_one(*cmd, capture=True, capture_stderr=True)
    if result.return_code != 0:
        raise OSError(f'git fetch: {result.stderr}')


def check_worktree_is_available(git_dir):
    """Check if git-worktree functionality is available

    Args:
        git_dir (str): The repository to test in

    Returns:
        True if git-worktree commands will work, False otherwise.
    """
    result = command.run_one('git', '--git-dir', git_dir, 'worktree', 'list',
                             capture=True, capture_stderr=True,
                             raise_on_error=False)
    return result.return_code == 0


def add_worktree(git_dir, output_dir, commit_hash=None):
    """Create and checkout a new git worktree for this build

    Args:
        git_dir (str): The repository to checkout the worktree from
        output_dir (str): Path for the new worktree
        commit_hash (str): Commit hash to checkout
    """
    # We need to pass --detach to avoid creating a new branch
    cmd = ['git', '--git-dir', git_dir, 'worktree', 'add', '.', '--detach']
    if commit_hash:
        cmd.append(commit_hash)
    result = command.run_one(*cmd, capture=True, cwd=output_dir,
                             capture_stderr=True)
    if result.return_code != 0:
        raise OSError(f'git worktree add: {result.stderr}')


def prune_worktrees(git_dir):
    """Remove administrative files for deleted worktrees

    Args:
        git_dir (str): The repository whose deleted worktrees should be pruned
    """
    result = command.run_one('git', '--git-dir', git_dir, 'worktree', 'prune',
                             capture=True, capture_stderr=True)
    if result.return_code != 0:
        raise OSError(f'git worktree prune: {result.stderr}')


def create_patches(branch, start, count, ignore_binary, series, signoff=True,
                   git_dir=None, cwd=None):
    """Create a series of patches from the top of the current branch.

    The patch files are written to the current directory using
    git format-patch.

    Args:
        branch (str): Branch to create patches from (None for current branch)
        start (int): Commit to start from: 0=HEAD, 1=next one, etc.
        count (int): number of commits to include
        ignore_binary (bool): Don't generate patches for binary files
        series (Series): Series object for this series (set of patches)
        signoff (bool): True to add signoff lines automatically
        git_dir (str): Path to git repository (None to use default)
        cwd (str): Path to use for git operations
    Return:
        Filename of cover letter (None if none)
        List of filenames of patch files
    """
    cmd = ['git']
    if git_dir:
        cmd += ['--git-dir', git_dir]
    cmd += ['format-patch', '-M']
    if signoff:
        cmd.append('--signoff')
    if ignore_binary:
        cmd.append('--no-binary')
    if series.get('cover'):
        cmd.append('--cover-letter')
    prefix = series.GetPatchPrefix()
    if prefix:
        cmd += [f'--subject-prefix={prefix}']
    brname = branch or 'HEAD'
    cmd += [f'{brname}~{start + count}..{brname}~{start}']

    stdout = command.run_list(cmd, cwd=cwd)
    files = stdout.splitlines()

    # We have an extra file if there is a cover letter
    if series.get('cover'):
        return files[0], files[1:]
    return None, files


def build_email_list(in_list, alias, tag=None, warn_on_error=True):
    """Build a list of email addresses based on an input list.

    Takes a list of email addresses and aliases, and turns this into a list
    of only email address, by resolving any aliases that are present.

    If the tag is given, then each email address is prepended with this
    tag and a space. If the tag starts with a minus sign (indicating a
    command line parameter) then the email address is quoted.

    Args:
        in_list (list of str): List of aliases/email addresses
        alias (dict): Alias dictionary:
            key: alias
            value: list of aliases or email addresses
        tag (str): Text to put before each address
        warn_on_error (bool): True to raise an error when an alias fails to
            match, False to just print a message.

    Returns:
        List of email addresses

    >>> alias = {}
    >>> alias['fred'] = ['f.bloggs@napier.co.nz']
    >>> alias['john'] = ['j.bloggs@napier.co.nz']
    >>> alias['mary'] = ['Mary Poppins <m.poppins@cloud.net>']
    >>> alias['boys'] = ['fred', ' john']
    >>> alias['all'] = ['fred ', 'john', '   mary   ']
    >>> build_email_list(['john', 'mary'], alias, None)
    ['j.bloggs@napier.co.nz', 'Mary Poppins <m.poppins@cloud.net>']
    >>> build_email_list(['john', 'mary'], alias, '--to')
    ['--to "j.bloggs@napier.co.nz"', \
'--to "Mary Poppins <m.poppins@cloud.net>"']
    >>> build_email_list(['john', 'mary'], alias, 'Cc')
    ['Cc j.bloggs@napier.co.nz', 'Cc Mary Poppins <m.poppins@cloud.net>']
    """
    raw = []
    for item in in_list:
        raw += lookup_email(item, alias, warn_on_error=warn_on_error)
    result = []
    for item in raw:
        if item not in result:
            result.append(item)
    if tag:
        return [x for email in result for x in (tag, email)]
    return result


def check_suppress_cc_config():
    """Check if sendemail.suppresscc is configured correctly.

    Returns:
        bool: True if the option is configured correctly, False otherwise.
    """
    suppresscc = command.output_one_line(
        'git', 'config', 'sendemail.suppresscc', raise_on_error=False)

    # Other settings should be fine.
    if suppresscc in ('all', 'cccmd'):
        col = terminal.Color()

        print(col.build(col.RED, 'error') +
              f': git config sendemail.suppresscc set to {suppresscc}\n' +
              '  patman needs --cc-cmd to be run to set the cc list.\n' +
              '  Please run:\n' +
              '    git config --unset sendemail.suppresscc\n' +
              '  Or read the man page:\n' +
              '    git send-email --help\n' +
              '  and set an option that runs --cc-cmd\n')
        return False

    return True


def email_patches(series, cover_fname, args, dry_run, warn_on_error, cc_fname,
                  alias, self_only=False, in_reply_to=None, thread=False,
                  smtp_server=None, cwd=None):
    """Email a patch series.

    Args:
        series (Series): Series object containing destination info
        cover_fname (str or None): filename of cover letter
        args (list of str): list of filenames of patch files
        dry_run (bool): Just return the command that would be run
        warn_on_error (bool): True to print a warning when an alias fails to
            match, False to ignore it.
        cc_fname (str): Filename of Cc file for per-commit Cc
        alias (dict): Alias dictionary:
            key: alias
            value: list of aliases or email addresses
        self_only (bool): True to just email to yourself as a test
        in_reply_to (str or None): If set we'll pass this to git as
            --in-reply-to - should be a message ID that this is in reply to.
        thread (bool): True to add --thread to git send-email (make
            all patches reply to cover-letter or first patch in series)
        smtp_server (str or None): SMTP server to use to send patches
        cwd (str): Path to use for patch files (None to use current dir)

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
    >>> series = {}
    >>> series['to'] = ['fred']
    >>> series['cc'] = ['mary']
    >>> email_patches(series, 'cover', ['p1', 'p2'], True, True, 'cc-fname', \
            False, alias)
    'git send-email --annotate --to "f.bloggs@napier.co.nz" --cc \
"m.poppins@cloud.net" --cc-cmd "./patman send --cc-cmd cc-fname" cover p1 p2'
    >>> email_patches(series, None, ['p1'], True, True, 'cc-fname', False, \
            alias)
    'git send-email --annotate --to "f.bloggs@napier.co.nz" --cc \
"m.poppins@cloud.net" --cc-cmd "./patman send --cc-cmd cc-fname" p1'
    >>> series['cc'] = ['all']
    >>> email_patches(series, 'cover', ['p1', 'p2'], True, True, 'cc-fname', \
            True, alias)
    'git send-email --annotate --to "this-is-me@me.com" --cc-cmd "./patman \
send --cc-cmd cc-fname" cover p1 p2'
    >>> email_patches(series, 'cover', ['p1', 'p2'], True, True, 'cc-fname', \
            False, alias)
    'git send-email --annotate --to "f.bloggs@napier.co.nz" --cc \
"f.bloggs@napier.co.nz" --cc "j.bloggs@napier.co.nz" --cc \
"m.poppins@cloud.net" --cc-cmd "./patman send --cc-cmd cc-fname" cover p1 p2'

    # Restore argv[0] since we clobbered it.
    >>> sys.argv[0] = _old_argv0
    """
    to = build_email_list(series.get('to'), alias, '--to', warn_on_error)
    if not to:
        if not command.output('git', 'config', 'sendemail.to',
                              raise_on_error=False):
            print("No recipient.\n"
                  "Please add something like this to a commit\n"
                  "Series-to: Fred Bloggs <f.blogs@napier.co.nz>\n"
                  "Or do something like this\n"
                  "git config sendemail.to u-boot@lists.denx.de")
            return None
    cc = build_email_list(list(set(series.get('cc')) - set(series.get('to'))),
                          alias, '--cc', warn_on_error)
    if self_only:
        to = build_email_list([os.getenv('USER')], '--to', alias,
                              warn_on_error)
        cc = []
    cmd = ['git', 'send-email', '--annotate']
    if smtp_server:
        cmd.append(f'--smtp-server={smtp_server}')
    if in_reply_to:
        cmd.append(f'--in-reply-to="{in_reply_to}"')
    if thread:
        cmd.append('--thread')

    cmd += to
    cmd += cc
    cmd += ['--cc-cmd', f'{sys.argv[0]} send --cc-cmd {cc_fname}']
    if cover_fname:
        cmd.append(cover_fname)
    cmd += args
    if not dry_run:
        command.run(*cmd, capture=False, capture_stderr=False, cwd=cwd)
    return' '.join([f'"{x}"' if ' ' in x and '"' not in x else x
                    for x in cmd])


def lookup_email(lookup_name, alias, warn_on_error=True, level=0):
    """If an email address is an alias, look it up and return the full name

    TODO: Why not just use git's own alias feature?

    Args:
        lookup_name (str): Alias or email address to look up
        alias (dict): Alias dictionary
            key: alias
            value: list of aliases or email addresses
        warn_on_error (bool): True to print a warning when an alias fails to
            match, False to ignore it.
        level (int): Depth of alias stack, used to detect recusion/loops

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
    >>> lookup_email('mary', alias)
    ['m.poppins@cloud.net']
    >>> lookup_email('arthur.wellesley@howe.ro.uk', alias)
    ['arthur.wellesley@howe.ro.uk']
    >>> lookup_email('boys', alias)
    ['f.bloggs@napier.co.nz', 'j.bloggs@napier.co.nz']
    >>> lookup_email('all', alias)
    ['f.bloggs@napier.co.nz', 'j.bloggs@napier.co.nz', 'm.poppins@cloud.net']
    >>> lookup_email('odd', alias)
    Alias 'odd' not found
    []
    >>> lookup_email('loop', alias)
    Traceback (most recent call last):
    ...
    OSError: Recursive email alias at 'other'
    >>> lookup_email('odd', alias, warn_on_error=False)
    []
    >>> # In this case the loop part will effectively be ignored.
    >>> lookup_email('loop', alias, warn_on_error=False)
    Recursive email alias at 'other'
    Recursive email alias at 'john'
    Recursive email alias at 'mary'
    ['j.bloggs@napier.co.nz', 'm.poppins@cloud.net']
    """
    lookup_name = lookup_name.strip()
    if '@' in lookup_name:      # Perhaps a real email address
        return [lookup_name]

    lookup_name = lookup_name.lower()
    col = terminal.Color()

    out_list = []
    if level > 10:
        msg = f"Recursive email alias at '{lookup_name}'"
        if warn_on_error:
            raise OSError(msg)
        print(col.build(col.RED, msg))
        return out_list

    if lookup_name:
        if lookup_name not in alias:
            msg = f"Alias '{lookup_name}' not found"
            if warn_on_error:
                print(col.build(col.RED, msg))
            return out_list
        for item in alias[lookup_name]:
            todo = lookup_email(item, alias, warn_on_error, level + 1)
            for new_item in todo:
                if new_item not in out_list:
                    out_list.append(new_item)

    return out_list


def get_top_level():
    """Return name of top-level directory for this git repo.

    Returns:
        str: Full path to git top-level directory, or None if not found

    This test makes sure that we are running tests in the right subdir

    >>> os.path.realpath(os.path.dirname(__file__)) == \
            os.path.join(get_top_level(), 'tools', 'patman')
    True
    """
    result = command.run_one(
        'git', 'rev-parse', '--show-toplevel', oneline=True, capture=True,
        capture_stderr=True, raise_on_error=False)
    if result.return_code:
        return None
    return result.stdout.strip()


def get_alias_file():
    """Gets the name of the git alias file.

    Returns:
        str: Filename of git alias file, or None if none
    """
    fname = command.output_one_line('git', 'config', 'sendemail.aliasesfile',
                                    raise_on_error=False)
    if not fname:
        return None

    fname = os.path.expanduser(fname.strip())
    if os.path.isabs(fname):
        return fname

    return os.path.join(get_top_level() or '', fname)


def get_default_user_name():
    """Gets the user.name from .gitconfig file.

    Returns:
        User name found in .gitconfig file, or None if none
    """
    uname = command.output_one_line('git', 'config', '--global', '--includes',
                                    'user.name')
    return uname


def get_default_user_email():
    """Gets the user.email from the global .gitconfig file.

    Returns:
        User's email found in .gitconfig file, or None if none
    """
    uemail = command.output_one_line('git', 'config', '--global', '--includes',
                                     'user.email')
    return uemail


def get_default_subject_prefix():
    """Gets the format.subjectprefix from local .git/config file.

    Returns:
        Subject prefix found in local .git/config file, or None if none
    """
    sub_prefix = command.output_one_line(
        'git', 'config', 'format.subjectprefix', raise_on_error=False)

    return sub_prefix


def setup():
    """setup() - Set up git utils, by reading the alias files."""
    # Check for a git alias file also
    global USE_NO_DECORATE

    cmd = log_cmd(None, count=0)
    USE_NO_DECORATE = (command.run_one(*cmd, raise_on_error=False)
                       .return_code == 0)


def get_hash(spec, git_dir=None):
    """Get the hash of a commit

    Args:
        spec (str): Git commit to show, e.g. 'my-branch~12'
        git_dir (str): Path to git repository (None to use default)

    Returns:
        str: Hash of commit
    """
    cmd = ['git']
    if git_dir:
        cmd += ['--git-dir', git_dir]
    cmd += ['show', '-s', '--pretty=format:%H', spec]
    return command.output_one_line(*cmd)


def get_head():
    """Get the hash of the current HEAD

    Returns:
        Hash of HEAD
    """
    return get_hash('HEAD')


def get_branch(git_dir=None):
    """Get the branch we are currently on

    Return:
        str: branch name, or None if none
        git_dir (str): Path to git repository (None to use default)
    """
    cmd = ['git']
    if git_dir:
        cmd += ['--git-dir', git_dir]
    cmd += ['rev-parse', '--abbrev-ref', 'HEAD']
    out = command.output_one_line(*cmd, raise_on_error=False)
    if out == 'HEAD':
        return None
    return out


def check_dirty(git_dir=None, work_tree=None):
    """Check if the tree is dirty

    Args:
        git_dir (str): Path to git repository (None to use default)
        work_tree (str): Git worktree to use, or None if none

    Return:
        str: List of dirty filenames and state
    """
    cmd = ['git']
    if git_dir:
        cmd += ['--git-dir', git_dir]
    if work_tree:
        cmd += ['--work-tree', work_tree]
    cmd += ['status', '--porcelain', '--untracked-files=no']
    return command.output(*cmd).splitlines()


def check_branch(name, git_dir=None):
    """Check if a branch exists

    Args:
        name (str): Name of the branch to check
        git_dir (str): Path to git repository (None to use default)
    """
    cmd = ['git']
    if git_dir:
        cmd += ['--git-dir', git_dir]
    cmd += ['branch', '--list', name]

    # This produces '  <name>' or '* <name>'
    out = command.output(*cmd).rstrip()
    return out[2:] == name


def rename_branch(old_name, name, git_dir=None):
    """Check if a branch exists

    Args:
        old_name (str): Name of the branch to rename
        name (str): New name for the branch
        git_dir (str): Path to git repository (None to use default)

    Return:
        str: Output from command
    """
    cmd = ['git']
    if git_dir:
        cmd += ['--git-dir', git_dir]
    cmd += ['branch', '--move', old_name, name]

    # This produces '  <name>' or '* <name>'
    return command.output(*cmd).rstrip()


def get_commit_message(commit, git_dir=None):
    """Gets the commit message for a commit

    Args:
        commit (str): commit to check
        git_dir (str): Path to git repository (None to use default)

    Return:
        list of str: Lines from the commit message
    """
    cmd = ['git']
    if git_dir:
        cmd += ['--git-dir', git_dir]
    cmd += ['show', '--quiet', commit]

    out = command.output(*cmd)
    # the header is followed by a blank line
    lines = out.splitlines()
    empty = lines.index('')
    msg = lines[empty + 1:]
    unindented = [line[4:] for line in msg]

    return unindented


def show_commit(commit, msg=True, diffstat=False, patch=False, colour=True,
                git_dir=None):
    """Runs 'git show' and returns the output

    Args:
        commit (str): commit to check
        msg (bool): Show the commit message
        diffstat (bool): True to include the diffstat
        patch (bool): True to include the patch
        colour (bool): True to force use of colour
        git_dir (str): Path to git repository (None to use default)

    Return:
        list of str: Lines from the commit message
    """
    cmd = ['git']
    if git_dir:
        cmd += ['--git-dir', git_dir]
    cmd += ['show']
    if colour:
        cmd.append('--color')
    if not msg:
        cmd.append('--oneline')
    if diffstat:
        cmd.append('--stat')
    else:
        cmd.append('--quiet')
    if patch:
        cmd.append('--patch')
    cmd.append(commit)

    return command.output(*cmd)


if __name__ == "__main__":
    import doctest

    doctest.testmod()
