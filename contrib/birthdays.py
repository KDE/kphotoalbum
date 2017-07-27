#!/usr/bin/env python
"""
Script for displaying people and their birthdays in KPhotoAlbum index.xml.
"""
# Copyright (C) 2015 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program (see the file COPYING); if not, write to the
# Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
# MA 02110-1301 USA.

import sys
from locale import getpreferredencoding
from KPhotoAlbum import xmldb
from KPhotoAlbum.datatypes import Tag

def printn(msg, stream=sys.stdout):
    """
    Print to stdout without new line.
    """
    stream.write(msg)
    stream.flush()

def main(argv):
    try:
        me = argv[0]
    except:
        me = 'birthdays.py'
        argv = [me]

    # Parse command line parameters
    if len(argv) != 3:
        printn('Usage: ' + me +
                ' xml_file (all|missing|available)\n', sys.stderr)
        return 1
    xml_file = argv[1]
    output = unicode(argv[2], getpreferredencoding())
    output_missing = False
    output_available = False
    if output == 'all':
        output_missing = True
        output_available = True
    elif output == 'missing':
        output_missing = True
    elif output == 'available':
        output_available = True
    else:
        print('Invalid output specifier - chose between all, missing, and available.',sys.stderr)
        return 1

    printn('Parsing the XML file...', sys.stderr)
    try:
        db = xmldb.XMLDatabase(xml_file)
    except xmldb.Error, (e):
        printn('failed.\n', sys.stderr)
        printn(str(e) + '\n', sys.stderr)
        return 2
    printn('parsed.\n', sys.stderr)

    for c in db.categories:
        if c.birthdates:
            for tagid in c.items:
                if tagid not in c.birthdates:
                    if output_missing:
                        print('"'+c.items[tagid] + '"')
                elif output_available:
                    print('"'+c.items[tagid] + '";' + str(c.birthdates[tagid]))

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
