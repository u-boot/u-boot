# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2020 Google LLC
#
"""Talks to the patchwork service to figure out what patches have been reviewed
and commented on. Provides a way to display review tags and comments.
Allows creation of a new branch based on the old but with the review tags
collected from patchwork.
"""

import collections
import concurrent.futures
from itertools import repeat

import pygit2
import requests

from u_boot_pylib import terminal
from u_boot_pylib import tout
from patman import patchstream
from patman import patchwork
from patman.patchstream import PatchStream


def to_int(vals):
    """Convert a list of strings into integers, using 0 if not an integer

    Args:
        vals (list): List of strings

    Returns:
        list: List of integers, one for each input string
    """
    out = [int(val) if val.isdigit() else 0 for val in vals]
    return out


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
    rtags = collections.defaultdict(set)
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
    new_rtags = collections.defaultdict(set)
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
        patches (:type: list of Patch): list of Patch objects to compare with

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

def collect_patches(series_id, pwork):
    """Collect patch information about a series from patchwork

    Uses the Patchwork REST API to collect information provided by patchwork
    about the status of each patch.

    Args:
        series_id (str): Patch series ID number
        pwork (Patchwork): Patchwork object to use for reading

    Returns:
        list of Patch: List of patches sorted by sequence number

    Raises:
        ValueError: if the URL could not be read or the web page does not follow
            the expected structure
    """
    data = pwork.request('series/%s/' % series_id)

    # Get all the rows, which are patches
    patch_dict = data['patches']
    count = len(patch_dict)

    patches = []

    # Work through each row (patch) one at a time, collecting the information
    warn_count = 0
    for pw_patch in patch_dict:
        patch = patchwork.Patch(pw_patch['id'])
        patch.parse_subject(pw_patch['name'])
        patches.append(patch)
    if warn_count > 1:
        tout.warning('   (total of %d warnings)' % warn_count)

    # Sort patches by patch number
    patches = sorted(patches, key=lambda x: x.seq)
    return patches

def find_new_responses(new_rtag_list, review_list, seq, cmt, patch, pwork):
    """Find new rtags collected by patchwork that we don't know about

    This is designed to be run in parallel, once for each commit/patch

    Args:
        new_rtag_list (list): New rtags are written to new_rtag_list[seq]
            list, each a dict:
                key: Response tag (e.g. 'Reviewed-by')
                value: Set of people who gave that response, each a name/email
                    string
        review_list (list): New reviews are written to review_list[seq]
            list, each a
                List of reviews for the patch, each a Review
        seq (int): Position in new_rtag_list to update
        cmt (Commit): Commit object for this commit
        patch (Patch): Corresponding Patch object for this patch
        pwork (Patchwork): Patchwork object to use for reading
    """
    if not patch:
        return

    # Get the content for the patch email itself as well as all comments
    data = pwork.request('patches/%s/' % patch.id)
    comment_data = pwork.request('patches/%s/comments/' % patch.id)

    new_rtags, reviews = process_reviews(data['content'], comment_data,
                                         cmt.rtags)
    new_rtag_list[seq] = new_rtags
    review_list[seq] = reviews

def show_responses(rtags, indent, is_new):
    """Show rtags collected

    Args:
        rtags (dict): review tags to show
            key: Response tag (e.g. 'Reviewed-by')
            value: Set of people who gave that response, each a name/email string
        indent (str): Indentation string to write before each line
        is_new (bool): True if this output should be highlighted

    Returns:
        int: Number of review tags displayed
    """
    col = terminal.Color()
    count = 0
    for tag in sorted(rtags.keys()):
        people = rtags[tag]
        for who in sorted(people):
            terminal.tprint(indent + '%s %s: ' % ('+' if is_new else ' ', tag),
                           newline=False, colour=col.GREEN, bright=is_new)
            terminal.tprint(who, colour=col.WHITE, bright=is_new)
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

def check_status(series, series_id, pwork):
    """Check the status of a series on Patchwork

    This finds review tags and comments for a series in Patchwork, displaying
    them to show what is new compared to the local series.

    Args:
        series (Series): Series object for the existing branch
        series_id (str): Patch series ID number
        pwork (Patchwork): Patchwork object to use for reading

    Return:
        tuple:
            list of Patch: List of patches sorted by sequence number
            dict: Patches for commit
                key: Commit number (0...n-1)
                value: Patch object for that commit
            list of dict: review tags:
                key: Response tag (e.g. 'Reviewed-by')
                value: Set of people who gave that response, each a name/email
                    string
            list for each patch, each a:
                list of Review objects for the patch
    """
    patches = collect_patches(series_id, pwork)
    count = len(series.commits)
    new_rtag_list = [None] * count
    review_list = [None] * count

    patch_for_commit, _, warnings = compare_with_series(series, patches)
    for warn in warnings:
        tout.warning(warn)

    patch_list = [patch_for_commit.get(c) for c in range(len(series.commits))]

    with concurrent.futures.ThreadPoolExecutor(max_workers=16) as executor:
        futures = executor.map(
            find_new_responses, repeat(new_rtag_list), repeat(review_list),
            range(count), series.commits, patch_list, repeat(pwork))
    for fresponse in futures:
        if fresponse:
            raise fresponse.exception()
    return patches, patch_for_commit, new_rtag_list, review_list


def check_patch_count(num_commits, num_patches):
    """Check the number of commits and patches agree

    Args:
        num_commits (int): Number of commits
        num_patches (int): Number of patches
    """
    if num_patches != num_commits:
        tout.warning(f'Warning: Patchwork reports {num_patches} patches, '
                     f'series has {num_commits}')


def do_show_status(series, patch_for_commit, show_comments, new_rtag_list,
                   review_list, col):
    num_to_add = 0
    for seq, cmt in enumerate(series.commits):
        patch = patch_for_commit.get(seq)
        if not patch:
            continue
        terminal.tprint('%3d %s' % (patch.seq, patch.subject[:50]),
                       colour=col.BLUE)
        cmt = series.commits[seq]
        base_rtags = cmt.rtags
        new_rtags = new_rtag_list[seq]

        indent = ' ' * 2
        show_responses(base_rtags, indent, False)
        num_to_add += show_responses(new_rtags, indent, True)
        if show_comments:
            for review in review_list[seq]:
                terminal.tprint('Review: %s' % review.meta, colour=col.RED)
                for snippet in review.snippets:
                    for line in snippet:
                        quoted = line.startswith('>')
                        terminal.tprint('    %s' % line,
                                       colour=col.MAGENTA if quoted else None)
                    terminal.tprint()
    return num_to_add


def show_status(series, branch, dest_branch, force, patches, patch_for_commit,
                show_comments, new_rtag_list, review_list, test_repo=None):
    """Show status to the user and allow a branch to be written

    Args:
        series (Series): Series object for the existing branch
        branch (str): Existing branch to update, or None
        dest_branch (str): Name of new branch to create, or None
        force (bool): True to force overwriting dest_branch if it exists
        patches (list of Patch): Patches sorted by sequence number
        patch_for_commit (dict): Patches for commit
            key: Commit number (0...n-1)
            value: Patch object for that commit
        show_comments (bool): True to show patch comments
        new_rtag_list (list of dict) review tags for each patch:
            key: Response tag (e.g. 'Reviewed-by')
            value: Set of people who gave that response, each a name/email
                string
        review_list (list of list): list for each patch, each a:
            list of Review objects for the patch
        test_repo (pygit2.Repository): Repo to use (use None unless testing)
    """
    col = terminal.Color()
    check_patch_count(len(series.commits), len(patches))
    num_to_add = do_show_status(series, patch_for_commit, show_comments,
                                new_rtag_list, review_list, col)

    terminal.tprint("%d new response%s available in patchwork%s" %
                   (num_to_add, 's' if num_to_add != 1 else '',
                    '' if dest_branch
                    else ' (use -d to write them to a new branch)'))

    if dest_branch:
        num_added = create_branch(series, new_rtag_list, branch, dest_branch,
                                  force, test_repo)
        terminal.tprint(
            "%d response%s added from patchwork into new branch '%s'" %
            (num_added, 's' if num_added != 1 else '', dest_branch))


def check_and_show_status(series, link, branch, dest_branch, force,
                          show_comments, pwork, test_repo=None):
    """Read the series status from patchwork and show it to the user

    Args:
        series (Series): Series object for the existing branch
        link (str): Patch series ID number
        branch (str): Existing branch to update, or None
        dest_branch (str): Name of new branch to create, or None
        force (bool): True to force overwriting dest_branch if it exists
        show_comments (bool): True to show patch comments
        pwork (Patchwork): Patchwork object to use for reading
        test_repo (pygit2.Repository): Repo to use (use None unless testing)
    """
    patches, patch_for_commit, new_rtag_list, review_list = check_status(
        series, link, pwork)
    show_status(series, branch, dest_branch, force, patches, patch_for_commit,
                show_comments, new_rtag_list, review_list, test_repo)
