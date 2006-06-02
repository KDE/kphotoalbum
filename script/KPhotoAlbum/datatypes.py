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
		     tags=None, drawings=None):
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
			s += ', tags=' + repr(self.tags)
		if len(self.drawings) > 0:
			s += ', drawings=' + repr(self.drawings)
		s += ')'
		return s


class Drawing(object):
	def __init__(self, shape, point1, point2):
		assert shape in ['circle', 'line', 'rectangle']
		self.shape = shape
		self.point1 = point1
		self.point2 = point2

	def __repr__(self):
		return (self.__class__.__name__ + '(' +
			repr(self.shape) + ', ' +
			repr(self.point1) + ', ' +
			repr(self.point2) + ')')
