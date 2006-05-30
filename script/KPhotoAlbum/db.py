class Database(object):
	"""
	Interface for KPhotoAlbum database.
	"""
	def getCategories(self):
		raise NotImplementedError

	def getImages(self):
		raise NotImplementedError

	def getMemberGroups(self):
		raise NotImplementedError

	def getBlockItems(self):
		raise NotImplementedError

	categories = property(getCategories)
	images = property(getImages)
	memberGroups = property(getMemberGroups)
	blockItems = property(getBlockItems)

	def insertCategory(self, category):
		"""
		Insert category and its items.

		Return category id number.
		"""
		raise NotImplementedError

	def insertImage(self, image):
		"""
		Insert image attributes and its tags.

		Return image id number.
		"""
		raise NotImplementedError

	def insertMemberGroup(self, memberGroup):
		raise NotImplementedError

	def insertBlockItem(self, blockItem):
		raise NotImplementedError

	def deleteCategory(self, categoryId):
		raise NotImplementedError

	def deleteImage(self, imageId):
		raise NotImplementedError

	def deleteMemberGroup(self, memberGroupId):
		raise NotImplementedError

	def deleteBlockItem(self, blockItemId):
		raise NotImplementedError

	def clear(self):
		raise NotImplementedError

	def feedFrom(self, other, progressFunction=None):
		"""
		Copy data from to other to self.

		If progressFunction is given, it is called after each
		image copy operation.
		"""
		pf = progressFunction
		if pf is None:
			pf = lambda: None
		for c in other.categories:
			self.insertCategory(c)
		for i in other.images:
			self.insertImage(i)
			pf()
		for m in other.memberGroups:
			self.insertMemberGroup(m)
		for b in other.blockItems:
			self.insertBlockItem(b)
