class DatabaseReader(object):
	"""
	Interface for reading from KPhotoAlbum database.
	"""
	def getCategories(self):
		raise NotImplementedError

	def getMediaItems(self):
		raise NotImplementedError

	def getMemberGroups(self):
		raise NotImplementedError

	def getBlockItems(self):
		raise NotImplementedError

	categories = property(getCategories)
	mediaItems = property(getMediaItems)
	memberGroups = property(getMemberGroups)
	blockItems = property(getBlockItems)

	def isEmpty(self):
		return (len([1 for i in self.categories]) +
			len([1 for i in self.mediaItems]) +
			len([1 for i in self.memberGroups]) +
			len([1 for i in blockItems])) == 0


class DatabaseWriter(object):
	"""
	Interface for writing to KPhotoAlbum database.
	"""
	def insertCategory(self, category):
		"""
		Insert category and its items.

		Return category id number.
		"""
		raise NotImplementedError

	def insertMediaItem(self, mediaItem):
		"""
		Insert media item attributes and its tags and
		drawings.

		All tag categories used by mediaItem should be
		inserted first with insertCategory method.

		Return media item id number.
		"""
		raise NotImplementedError

	def insertMemberGroup(self, memberGroup):
		"""
		Insert member group definition.

		Categoriy of the memberGroup should be inserted first
		with insertCategory method.
		"""
		raise NotImplementedError

	def insertBlockItem(self, blockItem):
		"""
		Make blockItem blocked.
		"""
		raise NotImplementedError

	def deleteCategory(self, categoryId):
		raise NotImplementedError

	def deleteMediaItem(self, mediaItemId):
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

		Parameter other should support DatabaseReader
		interface.

		If progressFunction is given, it is called after each
		media item copy operation.
		"""
		pf = progressFunction
		if pf is None:
			pf = lambda: None
		for c in other.categories:
			self.insertCategory(c)
		for i in other.mediaItems:
			self.insertMediaItem(i)
			pf()
		for m in other.memberGroups:
			self.insertMemberGroup(m)
		for b in other.blockItems:
			self.insertBlockItem(b)


class Database(DatabaseReader, DatabaseWriter):
	pass
