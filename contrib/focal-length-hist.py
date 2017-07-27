#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
A simple script showing ASCII-art histogram of lens focal lengths
"""

# Copyright (C) 2009 Jan Kundrát <jkt@flaska.net>
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
import sqlite3

def load_data( fname, condition=None ):
    conn = sqlite3.connect( fname )
    cur = conn.cursor()
    query = 'select distinct Exif_Photo_FocalLength, count(*) from exif '
    args = []
    if condition is not None:
        query += 'where filename like ? '
        args += [ condition ]
    query += 'group by Exif_Photo_FocalLength;'

    cur.execute( query, args )
    return [ [ row[0], row[1] ] for row in cur ]

def print_histogram( data ):
    boundary = max( x[1] for x in data )
    for row in data:
        stars = int( 80.0 * row[1]/boundary ) * '*'
        print "%s\t%s" % ( row[0], stars )

if __name__ == '__main__':
    if len(sys.argv) == 2:
        data = load_data( sys.argv[1] )
    elif len(sys.argv) == 3:
        data = load_data( sys.argv[1], sys.argv[2] )
    else:
        print "Usage: %s path/to/your/images/exif-info.db [filter]" % sys.argv[0]
        sys.exit(1)

    print_histogram( data )
