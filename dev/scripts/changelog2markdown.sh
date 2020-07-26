#!/bin/sh
# Author: Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# I do not consider this shell snippet to be copyrightable.
# If you need to put a license on it, please go ahead. For questions, please do contact me via email.

BUGZILLA=https://bugs.kde.org/

echo "--Paste Changelog entries here--" >&2
sed \
"
 s/^\W*\* \([^:]*:\)/ - **\1**/  ### Replace asterisk with dash, make classifier (Bug|Change|Deprecateion|etc.) **bold**
 s_\#\([0-9]*\)_[#\1]($BUGZILLA\1)_g ### Replace #1234 with link to bugzilla
"
