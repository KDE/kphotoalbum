"""
Module for accessing KPhotoAlbum index.xml.
"""

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
		if rootElem.getAttribute('version') != '2':
			raise UnsupportedFormat('Only version 2 is supported')
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
		     for x in ['name', 'icon',
			       'show', 'viewtype', 'viewsize']]
		a[2] = bool(int(a[2])) # show
		a[3] = int(a[3]) # viewtype
		a[4] = int(a[4]) # viewsize
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
		     for x in ['label', 'description', 'file', 'md5sum',
			       'startDate', 'endDate',
			       'width', 'height', 'angle']]
		a[4] = stringToDatetime(a[4]) # startDate
		a[5] = stringToDatetime(a[5]) # endDate
		a[6] = int(a[6]) # width
		if a[6] == -1:
			a[6] = None
		a[7] = int(a[7]) # height
		if a[7] == -1:
			a[7] = None
		a[8] = int(a[8])
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
	def __init__(self, memberGroupsElems):
		self.memberGroupsElems = memberGroupsElems

	def __memberIter(self):
		for mgs in self.memberGroupsElems:
			for m in mgs.getElementsByTagName('member'):
				yield m

	def __iter__(self):
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
