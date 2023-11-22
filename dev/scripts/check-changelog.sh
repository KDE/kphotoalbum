#!/bin/bash
# SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-License-Identifier: BSD-2-Clause
##
# A script that does some rudimentary checks on the Changelog.
# The script only works for the latest version, really, but it's a start...
# Usage: check-changelog.sh VERSION
# E.g. check-changelog.sh VERSION
##

guess_version()
# guess_version CHANGELOG_FILE
{
    local changelog_file="$1"
    if ! sed -n  '/^KPhotoAlbum [.0-9]* / { s/KPhotoAlbum \([.0-9]*\) .*/\1/p ; q }' "$changelog_file"
    then
        echo "! Failed to extract latest release version from $changelog_file!" >&2
        exit 1
    fi
}

parse_git_issues()
# parse_git_issues GITREF
# Output:
# ISSUE_NUMBER COMMIT_HASH
# ...
{
    local gitref="$1"
    # transform log into lines with commit hashes and lines with issue numbers
    # e.g.:
    # H:deadbeef
    # I:123456
    git log --pretty='#:%h%n%b%n' "$gitref" -- | sed -n '/^\#:/ s/\#/H/p ; /^\(BUG\)\|\(FEATURE\):/ s/[^0-9]*\([0-9]*\).*/I:\1/p' | {
        # append previous hash to issue number
        hash=
        while read -r line
        do
            case "${line}" in
                H:*) hash="${line:2}"
                    ;;
                I:*) echo "${line:2}" "$hash"
                    ;;
                *)
                    echo "! Unexpected line format! This is a bug!" >&2
                    exit 1
            esac
        done
    }
}

check_issues()
# check_issues CHANGELOG_FILE issues-commits.txt
{
    local changelog_file="$1"
    local icfile="$2"
    local num_issues
    num_issues=$(wc -l < "$icfile")
    echo "# Checking mentions for resolved issues..." >&2
    {
        local num_ok=0
        local num_check=0
        while read -r issue hash
        do
            if grep -q "#${issue}\W" "${changelog_file}"
            then
                echo "[OK] $issue"
                (( num_ok++ ))
            else
                echo "[CHECK] $issue - Check commit $hash and bug report https://bugs.kde.org/$issue for details."
                (( num_check++ ))
            fi
        done
        echo "# $num_issues issues, $num_ok ok, $num_check suspicious." >&2
        if [[ "$num_check" -gt 0 ]]
        then
            echo "**" >&2
            echo "* Please check the mentioned issues for false positives. Here are some examples for false positives that should not be part of the changelog:" >&2
            echo "*  - any bugs that have not yet been part of a release" >&2
            echo "*  - any bugs that were reopened and thus are not actually fixed" >&2
            echo "**" >&2
        fi
    } < "$icfile"
}

parse_changelog_issues()
# parse_changelog_issues CHANGELOG VERSION
{
    local changelog_file="$1"
    local version="$2"
    # check lines up to the $version announcement,
    # ... place strings looking like a issue number (e.g. #12345) on their own line,
    # ... remove '#' characters that are not part of an issue number,
    # and then only print the issue numbers from the output, striping the '#' character
    sed -n "1,/^KPhotoAlbum ${version} / { s/[^#]*\#\([0-9]\+\)[^#]*/\n#\1/g ; s/#[^0-9]//g; p }" "${changelog_file}" | sed -n  '/^#/ s/\#//p'
}

check_issue_plausibility()
# check_issue_plausibility ISSUES-COMMITS-FILE issues.txt
{
    local icfile="$1"
    local issuefile="$2"
    local num_issues
    num_issues=$(wc -l < "$issuefile")
    echo "# Checking issue numbers found in Changelog file:" >&2
    {
        local num_ok=0
        local num_check=0
        while read -r issue
        do
            local hash
            if hash="$(sed -n "/^$issue / s/[^ ]* //p" "$icfile" | head -n1)"
            then
                echo "[OK] $issue - commit $hash"
                (( num_ok++ ))
            else
                echo "[CHECK] $issue - Check bug report https://bugs.kde.org/$issue"
                (( num_check++ ))
            fi
        done
        echo "# $num_issues issues, $num_ok ok, $num_check uncertain." >&2
        if [[ "$num_check" -gt 0 ]]
        then
            echo "**" >&2
            echo "* Please check manually, if the mentioned issue reports exist." >&2
            echo "**" >&2
        fi
    } < "$issuefile"
}


BASEDIR="$(git rev-parse --show-toplevel)"

CHANGELOG="$BASEDIR/CHANGELOG.md"
VERSION="$(guess_version "$CHANGELOG")"

TEMPDIR=$(mktemp -d)
cleanup()
{
    rm -r "$TEMPDIR"
    unset TEMPDIR
}
trap cleanup EXOT

echo "# Checking all changes since version $VERSION..." >&2

parse_git_issues "v${VERSION}.." > "$TEMPDIR/git-issues-commits.txt"
check_issues "$CHANGELOG" "$TEMPDIR/git-issues-commits.txt"

parse_changelog_issues "$CHANGELOG" "$VERSION" >"$TEMPDIR/changelog-issues.txt"
check_issue_plausibility "$TEMPDIR/git-issues-commits.txt" "$TEMPDIR/changelog-issues.txt"
