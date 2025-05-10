# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2020 Google LLC
#
"""Talks to the patchwork service to figure out what patches have been reviewed
and commented on. Provides a way to display review tags and comments.
Allows creation of a new branch based on the old but with the review tags
collected from patchwork.
"""

import asyncio
from collections import defaultdict
import concurrent.futures
from itertools import repeat

import aiohttp
import pygit2

from u_boot_pylib import terminal
from u_boot_pylib import tout
from patman import patchstream
from patman import patchwork


def process_reviews(content, comment_data, base_rtags):
    """Process and return review data

    Args:
        content (str): Content text of the patch itself - see pwork.get_patch()
        comment_data (list of dict): Comments for the patch - see
            pwork._get_patch_comments()
        base_rtags (dict): base review tags (before any comments)
            key: Response tag (e.g. 'Reviewed-by')
            value: Set of people who gave that response, each a name/email
                string

    Return: tuple:
        dict: new review tags (noticed since the base_rtags)
            key: Response tag (e.g. 'Reviewed-by')
            value: Set of people who gave that response, each a name/email
                string
        list of patchwork.Review: reviews received on the patch
    """
    pstrm = patchstream.PatchStream.process_text(content, True)
    rtags = defaultdict(set)
    for response, people in pstrm.commit.rtags.items():
        rtags[response].update(people)

    reviews = []
    for comment in comment_data:
        pstrm = patchstream.PatchStream.process_text(comment['content'], True)
        if pstrm.snippets:
            submitter = comment['submitter']
            person = f"{submitter['name']} <{submitter['email']}>"
            reviews.append(patchwork.Review(person, pstrm.snippets))
        for response, people in pstrm.commit.rtags.items():
            rtags[response].update(people)

    # Find the tags that are not in the commit
    new_rtags = defaultdict(set)
    for tag, people in rtags.items():
        for who in people:
            is_new = (tag not in base_rtags or
                      who not in base_rtags[tag])
            if is_new:
                new_rtags[tag].add(who)
    return new_rtags, reviews


def compare_with_series(series, patches):
    """Compare a list of patches with a series it came from

    This prints any problems as warnings

    Args:
        series (Series): Series to compare against
        patches (list of Patch): list of Patch objects to compare with

    Returns:
        tuple
            dict:
                key: Commit number (0...n-1)
                value: Patch object for that commit
            dict:
                key: Patch number  (0...n-1)
                value: Commit object for that patch
    """
    # Check the names match
    warnings = []
    patch_for_commit = {}
    all_patches = set(patches)
    for seq, cmt in enumerate(series.commits):
        pmatch = [p for p in all_patches if p.subject == cmt.subject]
        if len(pmatch) == 1:
            patch_for_commit[seq] = pmatch[0]
            all_patches.remove(pmatch[0])
        elif len(pmatch) > 1:
            warnings.append("Multiple patches match commit %d ('%s'):\n   %s" %
                            (seq + 1, cmt.subject,
                             '\n   '.join([p.subject for p in pmatch])))
        else:
            warnings.append("Cannot find patch for commit %d ('%s')" %
                            (seq + 1, cmt.subject))

    # Check the names match
    commit_for_patch = {}
    all_commits = set(series.commits)
    for seq, patch in enumerate(patches):
        cmatch = [c for c in all_commits if c.subject == patch.subject]
        if len(cmatch) == 1:
            commit_for_patch[seq] = cmatch[0]
            all_commits.remove(cmatch[0])
        elif len(cmatch) > 1:
            warnings.append("Multiple commits match patch %d ('%s'):\n   %s" %
                            (seq + 1, patch.subject,
                             '\n   '.join([c.subject for c in cmatch])))
        else:
            warnings.append("Cannot find commit for patch %d ('%s')" %
                            (seq + 1, patch.subject))

    return patch_for_commit, commit_for_patch, warnings


def show_responses(col, rtags, indent, is_new):
    """Show rtags collected

    Args:
        col (terminal.Colour): Colour object to use
        rtags (dict): review tags to show
            key: Response tag (e.g. 'Reviewed-by')
            value: Set of people who gave that response, each a name/email string
        indent (str): Indentation string to write before each line
        is_new (bool): True if this output should be highlighted

    Returns:
        int: Number of review tags displayed
    """
    count = 0
    for tag in sorted(rtags.keys()):
        people = rtags[tag]
        for who in sorted(people):
            terminal.tprint(indent + '%s %s: ' % ('+' if is_new else ' ', tag),
                           newline=False, colour=col.GREEN, bright=is_new,
                           col=col)
            terminal.tprint(who, colour=col.WHITE, bright=is_new, col=col)
            count += 1
    return count

def create_branch(series, new_rtag_list, branch, dest_branch, overwrite,
                  repo=None):
    """Create a new branch with review tags added

    Args:
        series (Series): Series object for the existing branch
        new_rtag_list (list): List of review tags to add, one for each commit,
                each a dict:
            key: Response tag (e.g. 'Reviewed-by')
            value: Set of people who gave that response, each a name/email
                string
        branch (str): Existing branch to update
        dest_branch (str): Name of new branch to create
        overwrite (bool): True to force overwriting dest_branch if it exists
        repo (pygit2.Repository): Repo to use (use None unless testing)

    Returns:
        int: Total number of review tags added across all commits

    Raises:
        ValueError: if the destination branch name is the same as the original
            branch, or it already exists and @overwrite is False
    """
    if branch == dest_branch:
        raise ValueError(
            'Destination branch must not be the same as the original branch')
    if not repo:
        repo = pygit2.Repository('.')
    count = len(series.commits)
    new_br = repo.branches.get(dest_branch)
    if new_br:
        if not overwrite:
            raise ValueError("Branch '%s' already exists (-f to overwrite)" %
                             dest_branch)
        new_br.delete()
    if not branch:
        branch = 'HEAD'
    target = repo.revparse_single('%s~%d' % (branch, count))
    repo.branches.local.create(dest_branch, target)

    num_added = 0
    for seq in range(count):
        parent = repo.branches.get(dest_branch)
        cherry = repo.revparse_single('%s~%d' % (branch, count - seq - 1))

        repo.merge_base(cherry.oid, parent.target)
        base_tree = cherry.parents[0].tree

        index = repo.merge_trees(base_tree, parent, cherry)
        tree_id = index.write_tree(repo)

        lines = []
        if new_rtag_list[seq]:
            for tag, people in new_rtag_list[seq].items():
                for who in people:
                    lines.append('%s: %s' % (tag, who))
                    num_added += 1
        message = patchstream.insert_tags(cherry.message.rstrip(),
                                          sorted(lines))

        repo.create_commit(
            parent.name, cherry.author, cherry.committer, message, tree_id,
            [parent.target])
    return num_added


def check_patch_count(num_commits, num_patches):
    """Check the number of commits and patches agree

    Args:
        num_commits (int): Number of commits
        num_patches (int): Number of patches
    """
    if num_patches != num_commits:
        tout.warning(f'Warning: Patchwork reports {num_patches} patches, '
                     f'series has {num_commits}')


def do_show_status(series, cover, patches, show_comments, show_cover_comments,
                   col, warnings_on_stderr=True):
    """Check the status of a series on Patchwork

    This finds review tags and comments for a series in Patchwork, displaying
    them to show what is new compared to the local series.

    Args:
        series (Series): Series object for the existing branch
        cover (COVER): Cover letter info, or None if none
        patches (list of Patch): Patches sorted by sequence number
        show_comments (bool): True to show the comments on each patch
        show_cover_comments (bool): True to show the comments on the
            letter
        col (terminal.Colour): Colour object

    Return: tuple:
        int: Number of new review tags to add
        list: List of review tags to add, one item for each commit, each a
                dict:
            key: Response tag (e.g. 'Reviewed-by')
            value: Set of people who gave that response, each a name/email
                string
    """
    compare = []
    for pw_patch in patches:
        patch = patchwork.Patch(pw_patch.id)
        patch.parse_subject(pw_patch.series_data['name'])
        compare.append(patch)

    count = len(series.commits)
    new_rtag_list = [None] * count
    review_list = [None] * count

    with terminal.pager():
        patch_for_commit, _, warnings = compare_with_series(series, compare)
        for warn in warnings:
            tout.do_output(tout.WARNING if warnings_on_stderr else tout.INFO,
                           warn)

        for seq, pw_patch in enumerate(patches):
            compare[seq].patch = pw_patch

        for i in range(count):
            pat = patch_for_commit.get(i)
            if pat:
                patch_data = pat.patch.data
                comment_data = pat.patch.comments
                new_rtag_list[i], review_list[i] = process_reviews(
                    patch_data['content'], comment_data,
                    series.commits[i].rtags)
        num_to_add = _do_show_status(
            series, cover, patch_for_commit, show_comments,
            show_cover_comments, new_rtag_list, review_list, col)

    return num_to_add, new_rtag_list


def _do_show_status(series, cover, patch_for_commit, show_comments,
                    show_cover_comments, new_rtag_list, review_list, col):
    if cover and show_cover_comments:
        terminal.tprint(f'Cov {cover.name}', colour=col.BLACK, col=col,
                        bright=False, back=col.YELLOW)
        for seq, comment in enumerate(cover.comments):
            submitter = comment['submitter']
            person = '%s <%s>' % (submitter['name'], submitter['email'])
            terminal.tprint(f"From: {person}: {comment['date']}",
                            colour=col.RED, col=col)
            print(comment['content'])
            print()

    num_to_add = 0
    for seq, cmt in enumerate(series.commits):
        patch = patch_for_commit.get(seq)
        if not patch:
            continue
        terminal.tprint('%3d %s' % (patch.seq, patch.subject[:50]),
                       colour=col.YELLOW, col=col)
        cmt = series.commits[seq]
        base_rtags = cmt.rtags
        new_rtags = new_rtag_list[seq]

        indent = ' ' * 2
        show_responses(col, base_rtags, indent, False)
        num_to_add += show_responses(col, new_rtags, indent, True)
        if show_comments:
            for review in review_list[seq]:
                terminal.tprint('Review: %s' % review.meta, colour=col.RED,
                                col=col)
                for snippet in review.snippets:
                    for line in snippet:
                        quoted = line.startswith('>')
                        terminal.tprint(
                            f'    {line}',
                            colour=col.MAGENTA if quoted else None, col=col)
                    terminal.tprint()
    return num_to_add


def show_status(series, branch, dest_branch, force, cover, patches,
                show_comments, show_cover_comments, test_repo=None):
    """Check the status of a series on Patchwork

    This finds review tags and comments for a series in Patchwork, displaying
    them to show what is new compared to the local series.

    Args:
        client (aiohttp.ClientSession): Session to use
        series (Series): Series object for the existing branch
        branch (str): Existing branch to update, or None
        dest_branch (str): Name of new branch to create, or None
        force (bool): True to force overwriting dest_branch if it exists
        cover (COVER): Cover letter info, or None if none
        patches (list of Patch): Patches sorted by sequence number
        show_comments (bool): True to show the comments on each patch
        show_cover_comments (bool): True to show the comments on the letter
        test_repo (pygit2.Repository): Repo to use (use None unless testing)
    """
    col = terminal.Color()
    check_patch_count(len(series.commits), len(patches))
    num_to_add, new_rtag_list = do_show_status(
        series, cover, patches, show_comments, show_cover_comments, col)

    if not dest_branch and num_to_add:
        msg = ' (use -d to write them to a new branch)'
    else:
        msg = ''
    terminal.tprint(
        f"{num_to_add} new response{'s' if num_to_add != 1 else ''} "
        f'available in patchwork{msg}')

    if dest_branch:
        num_added = create_branch(series, new_rtag_list, branch,
                                  dest_branch, force, test_repo)
        terminal.tprint(
            f"{num_added} response{'s' if num_added != 1 else ''} added "
            f"from patchwork into new branch '{dest_branch}'")


async def check_status(link, pwork, read_comments=False,
                       read_cover_comments=False):
    """Set up an HTTP session and get the required state

    Args:
        link (str): Patch series ID number
        pwork (Patchwork): Patchwork object to use for reading
        read_comments (bool): True to read comments and state for each patch

        Return: tuple:
            COVER object, or None if none or not read_cover_comments
            list of PATCH objects
    """
    async with aiohttp.ClientSession() as client:
        return await pwork.series_get_state(client, link, read_comments,
                                             read_cover_comments)


def check_and_show_status(series, link, branch, dest_branch, force,
                          show_comments, show_cover_comments, pwork,
                          test_repo=None):
    """Read the series status from patchwork and show it to the user

    Args:
        series (Series): Series object for the existing branch
        link (str): Patch series ID number
        branch (str): Existing branch to update, or None
        dest_branch (str): Name of new branch to create, or None
        force (bool): True to force overwriting dest_branch if it exists
        show_comments (bool): True to show the comments on each patch
        show_cover_comments (bool): True to show the comments on the letter
        pwork (Patchwork): Patchwork object to use for reading
        test_repo (pygit2.Repository): Repo to use (use None unless testing)
    """
    loop = asyncio.get_event_loop()
    cover, patches = loop.run_until_complete(check_status(
        link, pwork, True, show_cover_comments))

    show_status(series, branch, dest_branch, force, cover, patches,
                show_comments, show_cover_comments, test_repo=test_repo)
