"""
Used for accessing KPhotoAlbum database in MySQL server.

Currently supports only storing. (no reading)
"""
from db import Database
#from datatypes import *

class MySQLDatabase(Database):
	"""
	Manages MySQL database stuff.

	Used to insert categories, images (with tags), member groups
	and block list into database.
	"""

	tableList = [
		('image',
		 'id SERIAL, '
		 'label VARCHAR(255), description TEXT, '
		 'filename VARCHAR(1024), md5sum CHAR(32), '
		 'startDate DATETIME, endDate DATETIME, '
		 'width INT, height INT, angle SMALLINT'),
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

	def __init__(self, mysqlDb):
		"""
		Parameter mysqlDb should be a db object returned by
		MySQLdb.connect.
		"""
		super(MySQLDatabase, self).__init__()
		self.db = mysqlDb
		self.c = self.db.cursor()
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

	def clear(self):
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
		cid = self.categoryMap.numFor(c.name)
		self.c.execute('DELETE FROM category WHERE id=%s',
			       (cid,))
		self.c.execute('INSERT INTO category(id, name, icon, '
			       'visible, viewtype, viewsize) '
			       'values(%s,%s,%s,%s,%s,%s)',
			       (cid, c.name, c.icon,
				int(c.visible), c.viewtype, c.viewsize))
		for item in c.items.itervalues():
			self.insertTag((c.name, item))

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

	def insertImageTag(self, iid, tid):
		if self.c.execute('SELECT * FROM image_tag '
				  'WHERE imageId=%s AND tagId=%s',
				  (iid, tid)) == 0L:
			self.c.execute('INSERT INTO '
				       'image_tag(imageId, tagId) '
				       'values(%s,%s)', (iid, tid))

	def insertMemberGroup(self, m):
		pass

	def insertBlockItem(self, b):
		pass


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
