Image database overview for KPhotoAlbum
=======================================

Concepts
--------

 1) Fuzzy Dates

KPhotoAlbum has the concept of fuzzy dates (or date intervals), which are defined by a start date
and an end date (both include a timestamp). This helps for photos which have been digitized from an
analog medium.

When the exact timestamp is known, startDate equals endDate.


 2) Directory structure

All images are expected to be located below a common root folder. The root folder is the one
containing the index.xml database file.

All file names in the index.xml file are relative to the root folder.


 3) Tags

Tags (sometimes called Categories in KPhotoAlbum) are arranged in multiple independent
hierarchies, i.e. there is no common root for all tags.

Tag hierarchies are organized as DAGs (directed acyclic graphs).


 4) Additional metadata

Exif information is stored in an sqlite database called `exif-info.db` in the image root folder.


index.xml
---------

Below is a visualization of the DOM-Tree of the index.xml file. Attributes are
within parenthesis, comments in square brackets.

- - - - - - - -
	KPhotoAlbum
	| (version=3,compressed=1)
	|
	+-Categories
	| +-Category (name,icon,show,viewtype,thumbnailsize)
	|   +-value (value, id)
	|
	+-images
	| +-image
	|   (file, label, description, startDate, endDate, angle, md5sum, width, height)
	|   (stackId, stackOrder, rating) [optional]
	|   (#Categories.Category.name#=#Categories.Category.value.id#) [optional]
	|
	+-blocklist
	| +-block (file)
	|
	+-member-groups
	  +-member (category,group-name,members)
- - - - - - - -
	KPhotoAlbum
	| (version=3,compressed=0)
	|
	+-Categories
	| +-Category (name,icon,show,viewtype,thumbnailsize)
	|   +-value (value, id)
	|
	+-images
	| +-image
	|   (file, label, description, startDate, endDate, angle, md5sum, width, height)
	|   (stackId, stackOrder, rating) [optional]
	|   +-options
   |     +-option(name=#Categories.Category.name#)
   |       +-value(value=#Categories.Category.value.value#)
	|
	+-blocklist
	| +-block (file)
	|
	+-member-groups
	  +-member (category,group-name,member)
