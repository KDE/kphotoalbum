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

class Category(object):
	"""
	Stores category information.
	"""
	def __init__(self, name, icon, visible,
		     viewtype, thumbsize,
		     items=None):
		self.name = name
		self.icon = icon
		self.visible = visible
		self.viewtype = viewtype
		self.thumbsize = thumbsize
		self.items = items
		if self.items is None:
			self.items = {}

	def addItem(self, name, id):
		assert not self.items.has_key(id)
		self.items[id] = name

	def __repr__(self):
		s = (self.__class__.__name__ + '(' +
		     repr(self.name) + ', ' +
		     repr(self.icon) + ', ' +
		     repr(self.visible) + ', ' +
		     repr(self.viewtype) + ', ' +
		     repr(self.thumbsize))
		if len(self.items) > 0:
			s += ', ' + repr(self.items)
		s += ')'
		return s


class MediaItem(object):
	"""
	Stores media item information.
	"""
	def __init__(self, filename, md5sum, mediatype,
		     label, description,
		     startTime, endTime,
		     width, height, angle,
		     tags=None, drawings=None):
		self.filename = filename
		self.md5sum = md5sum
		self.mediatype = mediatype
		self.label = label
		self.description = description
		self.startTime = startTime
		self.endTime = endTime
		self.width = width
		self.height = height
		self.angle = angle
		self.tags = tags
		self.drawings = drawings
		if self.tags is None:
			self.tags = set()
		if self.drawings is None:
			self.drawings = []

	def addTag(self, tag):
		self.tags.add(tag)

	def addDrawing(self, drawing):
		self.drawings += [drawing]

	def __repr__(self):
		s = (self.__class__.__name__ + '(' +
		     repr(self.filename) + ', ' +
		     repr(self.md5sum) + ', ' +
		     repr(self.mediatype) + ', ' +
		     repr(self.label) + ', ' +
		     repr(self.description) + ', ' +
		     repr(self.startTime) + ', ' +
		     repr(self.endTime) + ', ' +
		     repr(self.width) + ', ' +
		     repr(self.height) + ', ' +
		     repr(self.angle))
		if len(self.tags) > 0:
			s += ', tags=' + repr(self.tags)
		if len(self.drawings) > 0:
			s += ', drawings=' + repr(self.drawings)
		s += ')'
		return s


class Tag(object):
	def __init__(self, category, name):
		self.category = category
		self.name = name

	def __eq__(self, other):
		return (self.category == other.category and
			self.name == other.name)

	def __hash__(self):
		return (((hash(self.category) & ((1 << 15) - 1)) << 16) |
			(hash(self.name) & ((1 << 16) - 1)))

	def __getitem__(self, i):
		if i == 0:
			return self.category
		elif i == 1:
			return self.name
		else:
			raise IndexError('index should be 0 or 1')

	def __repr__(self):
		return (self.__class__.__name__ + '(' +
			repr(self.category) + ', ' +
			repr(self.name) + ')')


class Drawing(object):
	def __init__(self, shape, point0, point1):
		assert shape in ['circle', 'line', 'rectangle']
		self.shape = shape
		self.point0 = point0
		self.point1 = point1

	def __eq__(self, other):
		return (self.shape == other.shape and
			self.point0 == other.point0 and
			self.point1 == other.point1)

	def __hash__(self):
		return (((ord(self.shape[0]) & 7) << 28) |
			((hash(self.point0) & ((1 << 14) - 1)) << 14) |
			(hash(self.point1) & ((1 << 14) - 1)))

	def __repr__(self):
		return (self.__class__.__name__ + '(' +
			repr(self.shape) + ', ' +
			repr(self.point0) + ', ' +
			repr(self.point1) + ')')


class MemberGroup(Tag):
	def __init__(self, category, name, members=None):
		super(MemberGroup, self).__init__(category, name)
		self.members = members
		if self.members is None:
			self.members = []

	def addMember(self, member):
		self.members += [member]

	def __repr__(self):
		s = (self.__class__.__name__ + '(' +
		     repr(self.category) + ', ' +
		     repr(self.name))
		if len(self.members) > 0:
			s += ', ' + repr(self.members)
		return s + ')'

class BlockItem(object):
	def __init__(self, filename):
		self.filename = filename

	def __repr__(self):
		return (self.__class__.__name__ + '(' +
			repr(self.filename) + ')')
