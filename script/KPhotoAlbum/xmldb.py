"""
Module for accessing KPhotoAlbum index.xml.
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

from xml.dom import minidom
from time import mktime, strptime
from datetime import datetime
from db import DatabaseReader
from datatypes import *

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
#			drawings
#				Circle
#				Rectangle
#				Line
#	member-groups
#		member
#	blocklist
#		block

class XMLDatabase(DatabaseReader):
	"""
	Class for reading KPhotoAlbum index.xml.
	"""
	def __init__(self, filename):
		"""
		Initialize self with given XML file.

		Pre:
		 - ``filename`` is a KPhotoAlbum XML database (index.xml)
		"""
		super(XMLDatabase, self).__init__()
		try:
			self.dom = minidom.parse(filename)
		except Exception, (e):
			raise InvalidFile('Parsing XML failed: ' + str(e))
		rootElem = self.dom.documentElement
		if rootElem.tagName != u'KPhotoAlbum':
			raise InvalidFile('File should be in '
					  'KPhotoAlbum index.xml format.')
		if not rootElem.getAttribute('version') in ['2', '3']:
			raise UnsupportedFormat('Only versions 2 and 3 are supported')
		self.isCompressed = False
		if rootElem.getAttribute('compressed') == '1':
			self.isCompressed = True
		self.ctgs = rootElem.getElementsByTagName('Categories')
		self.imgs = rootElem.getElementsByTagName('images')
		self.mgs = rootElem.getElementsByTagName('member-groups')
		self.blks = rootElem.getElementsByTagName('blocklist')

	def getCategories(self):
		return CategoryIterator(self.ctgs)

	def getMediaItems(self):
		if self.isCompressed:
			return MediaItemIterator(self.imgs, self.categories)
		else:
			return MediaItemIterator(self.imgs)

	def getMemberGroups(self):
		if self.isCompressed:
			return MemberGroupIterator(self.mgs, self.categories)
		else:
			return MemberGroupIterator(self.mgs)

	def getBlockItems(self):
		return BlockItemIterator(self.blks)

	categories = property(getCategories)
	mediaItems = property(getMediaItems)
	memberGroups = property(getMemberGroups)
	blockItems = property(getBlockItems)


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


kpaTimeFormatStr = '%Y-%m-%dT%H:%M:%S'

def stringToDatetime(s):
	if s == '':
		return None
	return datetime.fromtimestamp(mktime(strptime(s, kpaTimeFormatStr)))

def datetimeToString(ts):
	return ts.strftime(kpaTimeFormatStr)


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
		     for x in ['name', 'icon', 'show',
			       'viewtype', 'thumbnailsize']]
		a[2] = bool(int(a[2])) # show
		a[3] = int(a[3]) # viewtype
		a[4] = int(a[4]) # thumbsize
		ctg = Category(*a)
		for valElem in ctgNode.getElementsByTagName('value'):
			name = valElem.getAttribute('value')
			idNum = int(valElem.getAttribute('id'))
			ctg.addItem(name, idNum)
		return ctg


class MediaItemIterator(object):
	"""
	Iterates media items in given DOM element.
	"""
	def __init__(self, imagesElems, categories=None):
		"""
		Initialize with list of images elements.

		If categories is None, will not parse compressed
		format.
		"""
		self.imagesElems = imagesElems
		self.categories = categories
		if self.categories is None:
			self.categories = []

	def __iter__(self):
		for imgs in self.imagesElems:
			for i in imgs.getElementsByTagName('image'):
				yield self.__getMediaItem(i)

	def __getMediaItem(self, imgElem):
		a = [imgElem.getAttribute(x)
		     for x in ['file', 'md5sum', 'mediatype',
			       'label', 'description',
			       'startDate', 'endDate',
			       'width', 'height', 'angle']]
		if a[2] == '': # mediatype
			a[2] = 'image'
		elif a[2] == 'movie':
			a[2] = 'video'
		a[5] = stringToDatetime(a[5]) # startDate
		a[6] = stringToDatetime(a[6]) # endDate
		a[7] = int(a[7]) # width
		if a[7] == -1:
			a[7] = None
		a[8] = int(a[8]) # height
		if a[8] == -1:
			a[8] = None
		a[9] = int(a[9]) # angle
		img = MediaItem(*a)

		# Parse uncompressed category items
		for opts in imgElem.getElementsByTagName('options'):
			for opt in opts.getElementsByTagName('option'):
				category = opt.getAttribute('name')
				if category == 'Folder':
					continue
				for ov in opt.getElementsByTagName('value'):
					item = ov.getAttribute('value')
					img.addTag(Tag(category, item))

		# Parse compressed category items
		for category in self.categories:
			if category.name == 'Folder':
				continue
			idList = imgElem.getAttribute(category.name)
			for s in idList.split(','):
				try:
					n = int(s)
				except:
					continue
				item = category.items[n]
				img.addTag(Tag(category.name, item))
		# Parse drawings
		for drws in imgElem.getElementsByTagName('drawings'):
			for shape in ['Circle', 'Line', 'Rectangle']:
				for s in drws.getElementsByTagName(shape):
					x1 = int(s.getAttribute('_startPos.x'))
					y1 = int(s.getAttribute('_startPos.y'))
					x2 = int(s.getAttribute('_lastPos.x'))
					y2 = int(s.getAttribute('_lastPos.y'))
					img.addDrawing(Drawing(shape.lower(),
							       (x1, y1),
							       (x2, y2)))
		return img


class MemberGroupIterator(object):
	"""
	Iterates member groups in given DOM element.
	"""
	def __init__(self, memberGroupsElems, categories=None):
		"""
		Initialize with a list of member-groups elements.

		If categories is None, will not parse compressed
		format.
		"""
		self.memberGroupsElems = memberGroupsElems
		self.categories = {}
		if not categories is None:
			for c in categories:
				self.categories[c.name] = c.items

	def __memberIter(self):
		for mgs in self.memberGroupsElems:
			for m in mgs.getElementsByTagName('member'):
				yield m

	def __compressedIter(self):
		for m in self.__memberIter():
			mg = MemberGroup(*self.__getLabel(m))
			items = self.categories[mg.category]
			for s in m.getAttribute('members').split(','):
				try:
					n = int(s)
				except:
					continue
				mg.addMember(items[n])
			yield mg

	def __normalIter(self):
		collected = []
		while True:
			memberIter = self.__memberIter()
			try:
				while True:
					x = memberIter.next()
					label = self.__getLabel(x)
					if not label in collected:
						break
			except StopIteration:
				return
			mg = MemberGroup(*label)
			mg.addMember(x.getAttribute('member'))
			for x in memberIter:
				if self.__getLabel(x) == label:
					mg.addMember(x.getAttribute('member'))
			collected += [label]
			yield mg

	def __iter__(self):
		mi = self.__memberIter()
		try:
			m = mi.next()
		except StopIteration:
			return
		if m.hasAttribute('members'):
			return self.__compressedIter()
		else:
			return self.__normalIter()

	def __getLabel(self, elem):
		return (elem.getAttribute('category'),
			elem.getAttribute('group-name'))


class BlockItemIterator(object):
	"""
	Iterates block items in given DOM element.
	"""
	def __init__(self, blocklistElems):
		self.blocklistElems = blocklistElems

	def __iter__(self):
		for blklst in self.blocklistElems:
			for b in blklst.getElementsByTagName('block'):
				yield self.__getBlockItem(b)

	def __getBlockItem(self, blkNode):
		return BlockItem(blkNode.getAttribute('file'))
