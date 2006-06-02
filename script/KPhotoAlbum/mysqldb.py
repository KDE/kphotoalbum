"""
Used for accessing KPhotoAlbum database in MySQL server.

Currently supports only storing. (no reading)
"""

from db import DatabaseWriter

class MySQLDatabase(DatabaseWriter):
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
		Initialize self with given MySQL connection.

		Parameter mysqlDb should be a db object returned by
		MySQLdb.connect.
		"""
		super(MySQLDatabase, self).__init__()
		self.db = mysqlDb
		self.c = self.db.cursor()
		self.__createTables()

	def __createTables(self):
		for t in self.tableList:
			self.c.execute('CREATE TABLE IF NOT EXISTS ' +
				       t[0] + '(' + t[1] + ')')
		self.__getIds()

	def __decodeString(self, str):
		return unicode(str, self.db.charset)

	def __getIds(self):
		self.__clearIds()
		self.c.execute('SELECT id, filename, md5sum FROM image')
		for (i, f, m) in self.c:
			self.imageMap[self.__decodeString(f), m)] = i
		self.c.execute('SELECT id, name FROM category')
		for (i, n) in self.c:
			self.categoryMap[self.__decodeString(n)] = i
		self.c.execute('SELECT tag.id, tag.name, category.name '
			       'FROM tag JOIN category '
			       'ON category.id=tag.categoryId')
		for (i, tn, cn) in self.c:
			self.tagMap[(self.__decodeString(cn),
				     self.__decodeString(tn))] = i

	def __clearIds(self):
		self.imageMap = ItemNumMap()
		self.categoryMap = ItemNumMap()
		self.tagMap = ItemNumMap()

	def __tableHasCol(self, table, col, colValue):
		"""
		Return true, iff table has a row which has column col
		set to colValue.
		"""
		return 0 != self.c.execute('SELECT ' + col +
					   ' FROM ' + table +
					   ' WHERE ' + col + '=%s',
					   (colValue,))

	def insertCategory(self, c):
		cid = self.categoryMap.numFor(c.name)
		self.c.execute('DELETE FROM category WHERE id=%s', (cid,))
		self.c.execute('INSERT INTO category(id, name, icon, '
			       'visible, viewtype, viewsize) '
			       'values(%s,%s,%s,%s,%s,%s)',
			       (cid, c.name, c.icon,
				int(c.visible), c.viewtype, c.viewsize))
		for item in c.items.itervalues():
			self.__insertTag((c.name, item))

	def __insertTag(self, tag):
		"""
		Insert tag into tag table, if not already present.

		Assumes that category of tag is already created.
		"""
		tid = self.tagMap.numFor(tag)
		if not self.__tableHasCol('tag', 'id', tid):
			cid = self.categoryMap.numFor(tag[0])
			if not self.__tableHasCol('category', 'id', cid):
				self.c.execute('INSERT INTO '
					       'category(id, name) '
					       'values(%s,%s)',
					       (cid, tag[0]))
			self.c.execute('INSERT INTO tag(id, categoryId, name) '
				       'values(%s,%s,%s)', (tid, cid, tag[1]))

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
			if not self.__tableHasCol('tag', 'id', tid):
				self.__insertTag(tag)
			self.__insertImageTag(iid, tid)

	def __insertImageTag(self, iid, tid):
		if self.c.execute('SELECT * FROM image_tag '
				  'WHERE imageId=%s AND tagId=%s',
				  (iid, tid)) == 0:
			self.c.execute('INSERT INTO '
				       'image_tag(imageId, tagId) '
				       'values(%s,%s)', (iid, tid))

	def insertMemberGroup(self, m):
		pass

	def insertBlockItem(self, b):
		pass

	def clear(self):
		for t in self.tableList:
			self.c.execute('DELETE FROM ' + t[0])
		self.__clearIds()


class ItemNumMap(dict):
	"""
	Simple extension to dict that can assign numbers to items
	implicitly.
	"""
	def numFor(self, item):
		"""
		Get unique number for item.

		Returns different number for different items and same
		number, if it is asked twice for same item.
		"""
		if not self.has_key(item):
			self[item] = len(self) + 1
		return self[item]
