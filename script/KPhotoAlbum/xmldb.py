"""
Module for accessing KPhotoAlbum index.xml.
"""

from xml.dom import minidom
from time import mktime, strptime
from datetime import datetime
from db import Database
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
#	blocklist
#		block
#	member-groups
#		member

class XMLDatabase(Database):
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
		if rootElem.getAttribute('compressed') == '1':
			raise UnsupportedFormat('Compressed format '
						'is not yet supported.')
		self.ctgs = rootElem.getElementsByTagName('Categories')
		self.imgs = rootElem.getElementsByTagName('images')

	def getCategories(self):
		return CategoryIterator(self.ctgs)

	def getImages(self):
		return ImageIterator(self.imgs)

	def getMemberGroups(self):
		return []

	def getBlockItems(self):
		return []

	categories = property(getCategories)
	images = property(getImages)
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
	return datetime.fromtimestamp(mktime(strptime(s, kpaTimeFormatStr)))
#	return MySQLdb.TimestampFromTicks(
#		mktime(strptime(s, kpaTimeFormatStr)))

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
		a[4] = stringToDatetime(a[4]) # startDate
		a[5] = stringToDatetime(a[5]) # endDate
		a[6] = int(a[6]) # width
		if a[6] == -1:
			a[6] = None
		a[7] = int(a[7]) # height
		if a[7] == -1:
			a[7] = None
		a[8] = int(a[8])
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
