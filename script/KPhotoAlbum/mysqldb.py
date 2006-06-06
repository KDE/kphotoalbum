"""
Used for accessing KPhotoAlbum database in MySQL server.

Currently supports only storing. (no reading)
"""

from db import DatabaseWriter
from datatypes import *

class MySQLDatabase(DatabaseWriter):
	"""
	Manages MySQL database stuff.

	Used to insert categories, media items (with tags and
	drawings), member groups and block list into database.
	"""

	tableList = [
		('media',
		 'id SERIAL, '
		 'filename VARCHAR(1024), md5sum CHAR(32), '
		 'typeId SMALLINT UNSIGNED, '
		 'place BIGINT UNSIGNED, '
		 'label VARCHAR(255), description TEXT, '
		 'startTime DATETIME, endTime DATETIME, '
		 'width INT, height INT, angle SMALLINT'),
		('mediaType',
		 'id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, '
		 'name VARCHAR(255)'),
		('category',
		 'id SERIAL, name VARCHAR(255), icon VARCHAR(255), '
		 'visible BOOL, viewtype TINYINT, viewsize TINYINT'),
		('tag',
		 'id SERIAL, '
		 'categoryId BIGINT UNSIGNED NOT NULL,'
		 'name VARCHAR(255), isGroup BOOL DEFAULT 0'),
		('media_tag',
		 'mediaId BIGINT UNSIGNED NOT NULL, '
		 'tagId BIGINT UNSIGNED NOT NULL, '
		 'UNIQUE KEY (mediaId, tagId)'),
		('drawing',
		 'id SERIAL, mediaId BIGINT UNSIGNED NOT NULL, '
		 'shape INT, x0 INT, y0 INT, x1 INT, y1 INT'),
		('relationType',
		 'id SERIAL, name VARCHAR(255)'),
		('tag_relation',
		 'typeId BIGINT UNSIGNED NOT NULL, '
		 'toTagId BIGINT UNSIGNED NOT NULL, '
		 'fromTagId BIGINT UNSIGNED NOT NULL, '
		 'UNIQUE KEY(typeId, toTagId, fromTagId)')]

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
		self.c.execute('SELECT id, filename FROM media')
		for (i, f) in self.c:
			self.mediaItemMap[self.__decodeString(f)] = i
		self.c.execute('SELECT id, name FROM mediaType')
		for (i, n) in self.c:
			self.mediaTypeMap[self.__decodeString(n)] = i
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
		self.mediaItemMap = ItemNumMap()
		self.mediaTypeMap = ItemNumMap()
		self.drawingMap = ItemNumMap()
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
			self.__insertTag(Tag(c.name, item))
		return cid

	def __insertTag(self, tag):
		"""
		Insert tag into tag table, if not already present.

		Assumes that category of tag is already created.
		"""
		tid = self.tagMap.numFor(tuple(tag))
		if not self.__tableHasCol('tag', 'id', tid):
			cid = self.categoryMap.numFor(tag.category)
			if not self.__tableHasCol('category', 'id', cid):
				self.c.execute('INSERT INTO '
					       'category(id, name) '
					       'values(%s,%s)',
					       (cid, tag.category))
			self.c.execute('INSERT INTO tag(id, categoryId, name) '
				       'values(%s,%s,%s)',
				       (tid, cid, tag.name))
		return tid

	def insertMediaItem(self, i):
		miid = self.mediaItemMap.numFor(i.filename)
		mtid = self.__insertMediaType(i.mediatype)
		self.c.execute('DELETE FROM media WHERE id=%s',
			       (miid,))
		self.c.execute('INSERT INTO media(id, filename, md5sum, '
			       'typeId, place, label, description, '
			       'startTime, endTime, '
			       'width, height, angle) '
			       'values(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)',
			       (miid, i.filename, i.md5sum, mtid,
				miid, i.label, i.description,
				i.startTime, i.endTime,
				i.width, i.height, i.angle))
		for tag in i.tags:
			tid = self.__insertTag(tag)
			self.__insertMediaTag(miid, tid)
		for drw in i.drawings:
			self.__insertMediaDrawing(miid, drw)
		return miid

	def __insertMediaType(self, mt):
		mtid = self.mediaTypeMap.numFor(mt)
		if not self.__tableHasCol('mediaType', 'id', mtid):
			self.c.execute('INSERT INTO mediaType(id, name) '
				       'values(%s,%s)', (mtid, mt))
		return mtid

	def __insertMediaTag(self, miid, tid):
		if self.c.execute('SELECT * FROM media_tag '
				  'WHERE mediaId=%s AND tagId=%s',
				  (miid, tid)) == 0:
			self.c.execute('INSERT INTO '
				       'media_tag(mediaId, tagId) '
				       'values(%s,%s)', (miid, tid))

	def __insertMediaDrawing(self, miid, drw):
		did = self.drawingMap.numFor((drw, miid))
		if not self.__tableHasCol('drawing', 'id', did):
			shapeid = {'circle': 0,
				   'line': 1,
				   'rectangle': 2}[drw.shape]
			self.c.execute('INSERT INTO '
				       'drawing(id, mediaId, shape, '
				       'x0, y0, x1, y1) '
				       'values(%s,%s,%s,%s,%s,%s,%s)',
				       (did, miid, shapeid,
					drw.point0[0], drw.point0[1],
					drw.point1[0], drw.point1[1]))

	def insertMemberGroup(self, m):
		tid = self.__insertTag(m)
		self.c.execute('UPDATE tag SET isGroup=1 WHERE id=%s', (tid,))
		for member in m.members:
			fid = self.__insertTag(Tag(m.category, member))
			if self.c.execute('SELECT * FROM tag_relation '
					  'WHERE typeId=%s AND '
					  'toTagId=%s AND fromTagId=%s',
					  (0, tid, fid)) != 0:
				continue
			self.c.execute('INSERT INTO tag_relation'
				       '(typeId, toTagId, fromTagId) '
				       'values(0,%s,%s)', (tid, fid))

	def insertBlockItem(self, b):
		miid = self.mediaItemMap.numFor(b.filename)
		self.c.execute('DELETE FROM media WHERE id=%s', (miid,))
		self.c.execute('INSERT INTO media(id, place, filename) '
			       'values(%s,%s,%s)', (miid, None, b.filename))

	def clear(self):
		for t in self.tableList:
			self.c.execute('DELETE FROM ' + t[0])
		self.__clearIds()


class ItemNumMap(dict):
	"""
	Simple extension to dict that can assign numbers to items
	implicitly.
	"""
	def __init__(self, items={}):
		super(ItemNumMap, self).__init__(items)
		self.maxNumber = None
		for i in self.itervalues():
			if (self.maxNumber is None or
			    i > self.maxNumber):
				self.maxNumber = i

	def __setitem__(self, key, value):
		#assert value not in self.values()
		if (self.maxNumber is None or
		    value > self.maxNumber):
			self.maxNumber = value
		super(ItemNumMap, self).__setitem__(key, value)

	def numFor(self, item):
		"""
		Get unique number for item.

		Returns different number for different items and same
		number, if it is asked twice for same item.
		"""
		if not self.has_key(item):
			if self.maxNumber is None:
				self.maxNumber = 1
			else:
				self.maxNumber += 1
			self[item] = self.maxNumber
		return self[item]
