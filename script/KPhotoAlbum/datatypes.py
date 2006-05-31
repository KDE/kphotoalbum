class Category(object):
	"""
	Stores category information.
	"""
	def __init__(self, name, icon,
		     visible, viewtype, viewsize,
		     items=None):
		self.name = name
		self.icon = icon
		self.visible = visible
		self.viewtype = viewtype
		self.viewsize = viewsize
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
		     repr(self.viewsize))
		if len(self.items) > 0:
			s += ', ' + repr(self.items)
		s += ')'
		return s


class Image(object):
	"""
	Stores image information.
	"""
	def __init__(self, label, description,
		     filename, md5sum,
		     startDate, endDate,
		     width, height, angle,
		     tags=None):
		self.label = label
		self.description = description
		self.filename = filename
		self.md5sum = md5sum
		self.startDate = startDate
		self.endDate = endDate
		self.width = width
		self.height = height
		self.angle = angle
		self.tags = tags
		if self.tags is None:
			self.tags = set()

	def addTag(self, tag):
		self.tags.add(tag)

	def __repr__(self):
		s = (self.__class__.__name__ + '(' +
		     repr(self.label) + ', ' +
		     repr(self.description) + ', ' +
		     repr(self.filename) + ', ' +
		     repr(self.md5sum) + ', ' +
		     repr(self.startDate) + ', ' +
		     repr(self.endDate) + ', ' +
		     repr(self.width) + ', ' +
		     repr(self.height) + ', ' +
		     repr(self.angle))
		if len(self.tags) > 0:
			s += ', ' + repr(self.tags)
		s += ')'
		return s
