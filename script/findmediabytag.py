#!/usr/bin/env python
"""
Script for finding all media items, which have some tag, from
KPhotoAlbum index.xml.
"""

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
