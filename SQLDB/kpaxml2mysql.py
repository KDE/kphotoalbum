#!/usr/bin/env python
"""
Script for copying data from KPhotoAlbum index.xml to MySQL database.
"""

import sys
import time
import getpass
import MySQLdb
from xml.dom import minidom

# Structure of KPhotoAlbum index.xml:
#
# KPhotoAlbum
# 	Categories
# 		Category
# 			value
# 	images
#		image
#			options
#				option
#					value
#	blocklist
#		block
#	member-groups
#		member

# I use term tag for (category, item) pair.


class ItemNumMap(dict):
	def numFor(self, item):
		"""
		Get unique number for item.

		Returns different number for different items and same
		number, if it is asked twice for same item.
		"""
		if not self.has_key(item):
			self[item] = len(self) + 1
		return self[item]


def stringToTimestamp(s):
	return MySQLdb.TimestampFromTicks(
		time.mktime(time.strptime(s, '%Y-%m-%dT%H:%M:%S')))

def timestampToString(ts):
	return ts.isoformat()


class Category(object):
	"""
	Stores category information.
	"""
	def __init__(self, name, icon,
		     visible, viewtype, viewsize,
		     items=None):
		self.name = name
		self.icon = icon
		self.visible = bool(int(visible))
		self.viewtype = int(viewtype)
		self.viewsize = int(viewsize)
		self.items = items
		if not self.items:
			self.items = set()

	def addItem(self, name, tid):
		self.items.add((name, tid))

	def __repr__(self):
		s = ('Category(' +
		     repr(self.name) + ', ' +
		     repr(self.icon) + ', ' +
		     repr(self.visible) + ', ' +
		     repr(self.viewtype) + ', ' +
		     repr(self.viewsize))
		if len(self.items) > 0:
			s += ', ' + repr(self.items)
		s += ')'
		return s


class Image(object):
	"""
	Stores image information.
	"""
	def __init__(self, label, description, filename, md5sum,
		     startDate, endDate,
		     width, height, angle,
		     tags=None):
		self.label = label
		self.description = description
		self.filename = filename
		self.md5sum = md5sum
		self.startDate = stringToTimestamp(startDate)
		self.endDate = stringToTimestamp(endDate)
		self.width = int(width)
		if self.width == -1:
			self.width = None
		self.height = int(height)
		if self.height == -1:
			self.height = None
		self.angle = int(angle)
		self.tags = tags
		if not self.tags:
			self.tags = set()

	def addTag(self, tag):
		self.tags.add(tag)

	def __repr__(self):
		s = ('Image(' +
		     repr(self.label) + ', ' +
		     repr(self.description) + ', ' +
		     repr(self.filename) + ', ' +
		     repr(self.md5sum) + ', ' +
		     repr(timestampToString(self.startDate)) + ', ' +
		     repr(timestampToString(self.endDate)) + ', ' +
		     repr(self.width) + ', ' +
		     repr(self.height) + ', ' +
		     repr(self.angle))
		if len(self.tags) > 0:
			s += ', ' + repr(self.tags)
		s += ')'
		return s


class DatabaseManager(object):
	"""
	Manages database stuff.

	Used to insert categories, images (with tags), member groups
	and block list into database.
	"""

	tableList = [
		('image',
		 'id SERIAL, '
		 'label VARCHAR(255), description TEXT, '
		 'filename VARCHAR(1024), md5sum CHAR(32), '
		 'startDate DATETIME, endDate DATETIME, '
		 'width INT, height INT, angle INT(1)'),
		('category',
		 'id SERIAL, name VARCHAR(255), icon VARCHAR(255), '
		 'visible BOOL, viewtype TINYINT, viewsize TINYINT'),
		('tag',
		 'id SERIAL, '
		 'categoryId BIGINT(20) UNSIGNED NOT NULL,'
		 'name VARCHAR(255)'),
		('image_tag',
		 'imageId BIGINT(20) UNSIGNED NOT NULL, '
		 'tagId BIGINT(20) UNSIGNED NOT NULL, '
		 'UNIQUE KEY (imageId, tagId)')]

	def __init__(self, db, progressFunction=None):
		self.db = db
		self.c = db.cursor()
		self.pf = progressFunction
		self.createTables(False)

	def createTables(self, dropFirst):
		for t in self.tableList:
			if dropFirst:
				self.c.execute('DROP TABLE IF EXISTS ' + t[0])
			self.c.execute('CREATE TABLE IF NOT EXISTS ' +
				       t[0] + '(' + t[1] + ')')
		self.getIdsFromDatabase()

	def getIdsFromDatabase(self):
		self.clearIds()
		self.c.execute('SELECT id, filename, md5sum FROM image')
		for (i, f, m) in self.c:
			self.imageMap[(unicode(f, self.db.charset), m)] = i
		self.c.execute('SELECT id, name FROM category')
		for (i, n) in self.c:
			self.categoryMap[unicode(n, self.db.charset)] = i
		self.c.execute('SELECT tag.id, tag.name, category.name '
			       'FROM tag JOIN category '
			       'ON category.id=tag.categoryId')
		for (i, tn, cn) in self.c:
			self.tagMap[(unicode(cn, self.db.charset),
				     unicode(tn, self.db.charset))] = i

	def clearIds(self):
		self.imageMap = ItemNumMap()
		self.categoryMap = ItemNumMap()
		self.tagMap = ItemNumMap()

	def feedDatabase(self, categories, images):
		for c in categories:
			self.insertCategory(c)
		for i in images:
			self.insertImage(i)

	def clearTables(self):
		for t in self.tableList:
			self.c.execute('DELETE FROM ' + t[0])
		self.clearIds()

	def tableHasCol(self, table, col, colValue):
		"""
		Return true, iff table has a row which has column col
		set to colValue.
		"""
		return 0L != self.c.execute('SELECT ' + col +
					    ' FROM ' + table +
					    ' WHERE ' + col + '=%s',
					    (colValue,))

	def insertCategory(self, c):
		"""
		Insert category and its items.
		"""
		cid = self.categoryMap.numFor(c.name)
		self.c.execute('DELETE FROM category WHERE id=%s',
			       (cid,))
		self.c.execute('INSERT INTO category(id, name, icon, '
			       'visible, viewtype, viewsize) '
			       'values(%s,%s,%s,%s,%s,%s)',
			       (cid, c.name, c.icon,
				int(c.visible), c.viewtype, c.viewsize))
		for i in c.items:
			self.insertTag((c.name, i[0]))

	def insertTag(self, tag):
		"""
		Insert tag into tag table, if not already present.

		Assumes that category of tag is already created.
		"""
		tid = self.tagMap.numFor(tag)
		if not self.tableHasCol('tag', 'id', tid):
			cid = self.categoryMap.numFor(tag[0])
			if not self.tableHasCol('category', 'id', cid):
				self.c.execute('INSERT INTO '
					       'category(id, name) '
					       'values(%s,%s)',
					       (cid, tag[0]))
			self.c.execute('INSERT INTO tag'
				       '(id, categoryId, name) '
				       'values(%s,%s,%s)',
				       (tid, cid, tag[1]))

	def insertImage(self, i):
		"""
		Insert image attributes to image table and image tags
		into image_tag table.
		"""
		iid = self.imageMap.numFor((i.filename, i.md5sum))
		self.c.execute('DELETE FROM image WHERE id=%s',
			       (iid,))
		self.c.execute('INSERT INTO image(id, label, description, '
			       'filename, md5sum, '
			       'startDate, endDate, '
			       'width, height, angle) '
			       'values(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)',
			       (iid, i.label, i.description,
				i.filename, i.md5sum,
				i.startDate, i.endDate,
				i.width, i.height, i.angle))
		for tag in i.tags:
			tid = self.tagMap.numFor(tag)
			if not self.tableHasCol('tag', 'id', tid):
				self.insertTag(tag)
			self.insertImageTag(iid, tid)
		if self.pf:
			self.pf(iid)

	def insertImageTag(self, iid, tid):
		if self.c.execute('SELECT * FROM image_tag '
				  'WHERE imageId=%s AND tagId=%s',
				  (iid, tid)) == 0L:
			self.c.execute('INSERT INTO '
				       'image_tag(imageId, tagId) '
				       'values(%s,%s)', (iid, tid))


class CategoryIterator(object):
	"""
	Iterates categories in given DOM element.
	"""
	def __init__(self, categoriesElems):
		self.categoriesElems = categoriesElems

	def __iter__(self):
		for ctgs in self.categoriesElems:
			for c in ctgs.getElementsByTagName('Category'):
				yield self.__getCategory(c)

	def __getCategory(self, ctgNode):
		a = [ctgNode.getAttribute(x)
		     for x in ['name', 'icon',
			       'show', 'viewtype', 'viewsize']]
		ctg = Category(*a)
		for valElem in ctgNode.getElementsByTagName('value'):
			name = valElem.getAttribute('value')
			tid = int(valElem.getAttribute('id'))
			ctg.addItem(name, tid)
		return ctg


class ImageIterator(object):
	"""
	Iterates images in given DOM element.
	"""
	def __init__(self, imagesElems):
		self.imagesElems = imagesElems

	def __iter__(self):
		for imgs in self.imagesElems:
			for i in imgs.getElementsByTagName('image'):
				yield self.__getImage(i)

	def __getImage(self, imgNode):
		a = [imgNode.getAttribute(x)
		     for x in ['label', 'description', 'file', 'md5sum',
			       'startDate', 'endDate',
			       'width', 'height', 'angle']]
		img = Image(*a)
		for opts in imgNode.getElementsByTagName('options'):
			for opt in opts.getElementsByTagName('option'):
				category = opt.getAttribute('name')
				if category == 'Folder':
					continue
				for ov in opt.getElementsByTagName('value'):
					item = ov.getAttribute('value')
					img.addTag((category, item))
		return img


class Error(Exception):
	"""
	General error.
	"""
	def __init__(self, msg):
		Exception.__init__(self, msg)

class InvalidFile(Error):
	"""
	File is invalid.
	"""

class UnsupportedFormat(InvalidFile):
	"""
	File format is not supported.
	"""


class KPADatabaseReader(object):
	"""
	Class for reading KPhotoAlbum index.xml.

	Use categories and images attributes to iterate data of the
	XML file.
	"""
	def __init__(self, filename):
		"""
		Initialize self with given XML file.

		Pre:
		 - ``filename`` is a KPhotoAlbum XML database (index.xml)
		"""
		try:
			self.dom = minidom.parse(filename)
		except Exception, (e):
			raise InvalidFile('Parsing failed: ' + str(e))
		rootElem = self.dom.documentElement
		if rootElem.tagName != u'KPhotoAlbum':
			raise InvalidFile('File should be in '
					  'KPhotoAlbum index.xml format.')
		if rootElem.getAttribute('compressed') == '1':
			raise UnsupportedFormat('Compressed format '
						'is not yet supported.')
		ctgs = rootElem.getElementsByTagName('Categories')
		imgs = rootElem.getElementsByTagName('images')
		self.categories = CategoryIterator(ctgs)
		self.images = ImageIterator(imgs)


def copyToDatabase(kpaReader, db, clearFirst=False, pf=None):
	"""
	Copy images from kpaReader to db.

	If clearFirst, clear tables from db first.

	If pf is given, it is called with image number as only
	parameter for every image inserted into db.
	"""
	dbmgr = DatabaseManager(db, pf)
	if clearFirst:
		dbmgr.clearTables()

	dbmgr.feedDatabase(kpaReader.categories, kpaReader.images)


def printn(msg):
	"""
	Print to stdout without new line.
	"""
	sys.stdout.write(msg)
	sys.stdout.flush()


def showProgress(n):
	if n % 100 == 0:
		printn('.')


def main(argv):
	try:
		me = argv[0]
	except:
		me = 'kpaxml2mysql.py'
		argv = [me]

	# Parse command line parameters
	if len(argv) < 3 or len(argv) > 5:
		print('Usage: ' + me +
		      ' xml_file db_name [username [password]]')
		return 1
	xml_file = argv[1]
	db_name = argv[2]
	if len(argv) < 4:
		username = getpass.getuser()
		print('Using username: ' + username)
	else:
		username = argv[3]
	if len(argv) < 5:
		password = getpass.getpass()
	else:
		password = argv[4]

	printn('Connecting to database...')
	db = MySQLdb.connect(db=db_name, user=username, passwd=password)
	print('connected.')

	print('WARNING!')
	print('This script will mess up your database (' + db_name + ').')
	printn('Are you sure you want to continue? ')
	a = raw_input()
	if a[0] != 'y' and a[0] != 'Y':
		return 0

	printn('Do you want to clear old tables first? ')
	a = raw_input()
	doClear = (a[0] == 'y' or a[0] == 'Y')

	printn('Parsing the XML file...')
	try:
		reader = KPADatabaseReader(xml_file)
	except Error, (e):
		print('failed.')
		print(e)
		return 2
	print('parsed.')

	printn('Copying data to the MySQL database')
	copyToDatabase(reader, db, doClear, showProgress)
	print('copied.')

	return 0


if __name__ == '__main__':
	sys.exit(main(sys.argv))
