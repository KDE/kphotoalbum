#!/usr/bin/env python
"""
Script for finding all media items, which have some tag, from
KPhotoAlbum index.xml.
"""
# Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>
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
		me = 'findmediabytag.py'
		argv = [me]

	# Parse command line parameters
	if len(argv) != 4:
		printn('Usage: ' + me +
		      ' xml_file category_name item_name\n', sys.stderr)
		return 1
	xml_file = argv[1]
	category_name = unicode(argv[2], getpreferredencoding())
	item_name = unicode(argv[3], getpreferredencoding())

	printn('Parsing the XML file...', sys.stderr)
	try:
		db = xmldb.XMLDatabase(xml_file)
	except xmldb.Error, (e):
		printn('failed.\n', sys.stderr)
		printn(str(e) + '\n', sys.stderr)
		return 2
	printn('parsed.\n', sys.stderr)

	for i in db.mediaItems:
		if Tag(category_name, item_name) in i.tags:
			print i.filename

	return 0


if __name__ == '__main__':
	sys.exit(main(sys.argv))
