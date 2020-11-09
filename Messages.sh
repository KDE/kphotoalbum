#! /bin/sh
# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0
$PREPARETIPS > tips.cc
$EXTRACTRC `find . -name \*.ui -o -name \*.rc` >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.cc -o -name \*.h | grep -v kexi` -o $podir/kphotoalbum.pot
rm -f tips.cc
