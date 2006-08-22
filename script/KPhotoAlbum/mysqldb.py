"""
Used for accessing KPhotoAlbum database in MySQL server.

Currently supports only storing. (no reading)
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

from db import DatabaseWriter
from datatypes import *

def splitPath(fullname):
	if not '/' in fullname:
		return ('.', fullname)
	return tuple(fullname.rsplit("/", 1))

class MySQLDatabase(DatabaseWriter):
	"""
	Manages MySQL database stuff.

	Used to insert categories, media items (with tags and
	drawings), member groups and block list into database.
	"""

	tableList = [
		('dir',
		 'id INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, '
		 'path VARCHAR(511)'),
		('media',
		 'id SERIAL, '
		 'place BIGINT UNSIGNED, '
		 'dirId INT UNSIGNED NOT NULL, '
		 'filename VARCHAR(255) NOT NULL, md5sum CHAR(32), '
		 'type SMALLINT UNSIGNED, '
		 'label VARCHAR(255), description TEXT, '
		 'startTime DATETIME, endTime DATETIME, '
		 'width INT, height INT, angle SMALLINT'),
		('blockitem',
		 'dirId INT UNSIGNED NOT NULL, '
		 'filename VARCHAR(255) NOT NULL'),
		('category',
		 'id INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, '
		 'name VARCHAR(255), icon VARCHAR(1023), '
		 'visible BOOL, viewtype TINYINT, thumbsize TINYINT'),
		('tag',
		 'id SERIAL, '
		 'place BIGINT UNSIGNED, '
		 'categoryId INT UNSIGNED NOT NULL,'
		 'name VARCHAR(255), isGroup BOOL DEFAULT 0'),
		('media_tag',
		 'mediaId BIGINT UNSIGNED NOT NULL, '
		 'tagId BIGINT UNSIGNED NOT NULL, '
		 'UNIQUE KEY (mediaId, tagId)'),
		('drawing',
		 'mediaId BIGINT UNSIGNED NOT NULL, '
		 'shape INT, x0 INT, y0 INT, x1 INT, y1 INT'),
		('tag_relation',
		 'toTagId BIGINT UNSIGNED NOT NULL, '
		 'fromTagId BIGINT UNSIGNED NOT NULL, '
		 'UNIQUE KEY(toTagId, fromTagId)')]

	mediaTypeMap = {'image': 1, 'video': 2, 'audio': 3}

	def __init__(self, mysqlDb):
		"""
		Initialize self with given MySQL connection.

		Parameter mysqlDb should be a db object returned by
		MySQLdb.connect.
		"""
		super(MySQLDatabase, self).__init__()
		self.db = mysqlDb
		self.db.autocommit(True)
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
		self.c.execute('SELECT id, path FROM dir')
		for (i, p) in self.c:
			self.dirMap[self.__decodeString(p)] = i
		self.c.execute('SELECT id, dirId, filename FROM media')
		for (i, d, f) in self.c:
			self.mediaItemMap[(d, self.__decodeString(f))] = i
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
		self.dirMap = ItemNumMap()
		self.mediaItemMap = ItemNumMap()
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
			       'visible, viewtype, thumbsize) '
			       'values(%s,%s,%s,%s,%s,%s)',
			       (cid, c.name, c.icon,
				int(c.visible), c.viewtype, c.thumbsize))
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
			self.c.execute('INSERT INTO tag(id, place, '
				       'categoryId, name) '
				       'values(%s,%s,%s,%s)',
				       (tid, tid, cid, tag.name))
		return tid

	def __insertDir(self, path):
		"""
		Insert path into dir table, if not already present.
		"""
		dirid = self.dirMap.numFor(path)
		if not self.__tableHasCol('dir', 'id', dirid):
			self.c.execute('INSERT INTO dir(id, path) '
				       'values(%s,%s)',
				       (dirid, path))
		return dirid

	def insertMediaItem(self, i):
		(path, filename) = splitPath(i.filename)
		dirid = self.__insertDir(path)
		miid = self.mediaItemMap.numFor((dirid, filename))
		mtid = self.__getMediaTypeNum(i.mediatype)
		self.c.execute('DELETE FROM media WHERE id=%s',
			       (miid,))
		self.c.execute('INSERT INTO media(id, place, '
			       'dirId, filename, md5sum, type, '
			       'label, description, '
			       'startTime, endTime, '
			       'width, height, angle) '
			       'values(%s,%s,%s,%s,%s,%s,'
			       '%s,%s,%s,%s,%s,%s,%s)',
			       (miid, miid,
				dirid, filename, i.md5sum, mtid,
				i.label, i.description,
				i.startTime, i.endTime,
				i.width, i.height, i.angle))
		for tag in i.tags:
			tid = self.__insertTag(tag)
			self.__insertMediaTag(miid, tid)
		self.__deleteMediaDrawings(miid)
		for drw in i.drawings:
			self.__insertMediaDrawing(miid, drw)
		return miid

	def __getMediaTypeNum(self, mt):
		return self.mediaTypeMap[mt]

	def __insertMediaTag(self, miid, tid):
		if self.c.execute('SELECT * FROM media_tag '
				  'WHERE mediaId=%s AND tagId=%s',
				  (miid, tid)) == 0:
			self.c.execute('INSERT INTO '
				       'media_tag(mediaId, tagId) '
				       'values(%s,%s)', (miid, tid))

	def __deleteMediaDrawings(self, miid):
		self.c.execute('DELETE FROM drawing WHERE mediaId=%s', (miid,))

	def __insertMediaDrawing(self, miid, drw):
		shapeid = {'circle': 0,
			   'line': 1,
			   'rectangle': 2}[drw.shape]
		self.c.execute('INSERT INTO '
			       'drawing(mediaId, shape, '
			       'x0, y0, x1, y1) '
			       'values(%s,%s,%s,%s,%s,%s)',
			       (miid, shapeid,
				drw.point0[0], drw.point0[1],
				drw.point1[0], drw.point1[1]))

	def insertMemberGroup(self, m):
		tid = self.__insertTag(m)
		self.c.execute('UPDATE tag SET isGroup=1 WHERE id=%s', (tid,))
		for member in m.members:
			fid = self.__insertTag(Tag(m.category, member))
			if self.c.execute('SELECT * FROM tag_relation '
					  'WHERE toTagId=%s AND fromTagId=%s',
					  (tid, fid)) != 0:
				continue
			self.c.execute('INSERT INTO tag_relation'
				       '(toTagId, fromTagId) '
				       'values(%s,%s)', (tid, fid))

	def insertBlockItem(self, b):
		#self.c.execute('DELETE FROM media WHERE filename=%s',
		#	       (b.filename,))
		(path, filename) = splitPath(b.filename)
		dirid = self.__insertDir(path)
		self.c.execute('INSERT INTO blockitem(dirId, filename) '
			       'values(%s,%s)', (dirid, filename))

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
