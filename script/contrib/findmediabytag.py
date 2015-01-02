#!/usr/bin/env python3

# Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>
# Copyright (C) 2015 Tobias Leupold <tobias.leupold@web.de>
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

"""
Script for finding all media items, which have some tag, from
KPhotoAlbum index.xml.
"""

import sys
from KPhotoAlbum import xmldb
from KPhotoAlbum.datatypes import Tag

def main():
    # Parse command line parameters
    if len(sys.argv) != 4:
        print('Usage: {} <XML file> <category name> <item name>'.format(sys.argv[0]),
            file = sys.stderr)
        return 1

    print('Parsing the XML file ... ', end = '', file = sys.stderr)

    try:
        db = xmldb.XMLDatabase(sys.argv[1])
    except:
        print('failed', file = sys.stderr)
        print(sys.exc_info()[1], file = sys.stderr)
        return 2

    print('done.', file = sys.stderr)

    for item in db.mediaItems:
        if Tag(sys.argv[2], sys.argv[3]) in item.tags:
            print(item.filename)

if __name__ == '__main__':
    sys.exit(main())
