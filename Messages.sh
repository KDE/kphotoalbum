#! /bin/sh
$PREPARETIPS > tips.cc
$EXTRACTRC *.rc >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.cc -o -name \*.h | grep -v kexi` -o $podir/kphotoalbum.pot
rm -f tips.cc
