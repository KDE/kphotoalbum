//krazy:skip
/**
  \namespace DB
  \brief The core database classes.

  <h2>The Database</h2>
  The database is abstracted using the class \ref ImageDB. This interface
  is subclasses by \ref XMLDB::Database. You can
  get to the global instance of the running database using \ref
  ImageDB::instance. The reason for this abstractioin was that for the longest time we also had a SQLDB::Database subclass.


  <h2>Image information</h2>
  All information about an image is stored in the class \ref ImageInfo
  (file name, category items for the image, its size etc etc). This class
  is rather heavy, and in the early days of KPA lots of methods was given
  information about an image or a set of images. To avoid copying the
  ImageInfo all the time, or to avoid handing pointers around, the type
  \ref ImageInfoPtr was introduced. This is a smart pointer, which results
  in \ref ImageInfo's are send around as pointers behind the
  screen, without the drawback of pointers.

  <h2>Searching/Browsing</h2>
  When the user searches or browses, a \ref ImageSearchInfo instance is
  built with information about the current search. This class contains
  information about all the parts of the search, like date range, category
  items to match etc.

  When the search is to happen, the information about categories are
  compiled into a structure that can take care of searching for the
  sub component of the category search (A category search may look like
  "friends &! Joe".

  The structure consist of these classes:
  \li \ref CategoryMatcher - This is the base class for the hierachy of
  items.
  \li  \ref SimpleCategoryMatcher - Base class for \ref ValueCategoryMatcher.
  \li \ref ValueCategoryMatcher - represents an item from the search
  ("Joe" or "friend" in the above example)
  \li \ref NoTagCategoryMatcher - represents a "No other" item with no other tags (i.e. "None")
  \li \ref ExactCategoryMatcher - represents a "No other" item on top of other tags
  \li \ref ContainerCategoryMatcher - Base class for \ref
  AndCategoryMatcher, \ref NegationCategoryMatcher and \ref OrCategoryMatcher
  \li \ref AndCategoryMatcher - represents the components of an "and" expression ("Jesper & Jan & No Other")
  \li \ref NegationCategoryMatcher - represents the "!" in "! Joe"
  \li \ref OrCategoryMatcher - represents the components of an "or" expression.


  <h2>Image Dates</h2>
  KPhotoAlbum has support for image dates, which are not know with an exact precission -
  this might be the case for a images taken with a non-digital
  camera. Therefore all dates in KPhotoAlbum are represented using the
  class \ref ImageDate, which has a lower and an upper date for the range.
  In KPhotoAlbum lingo, such an inexact date is called a fuzzy date.

  The two classes \ref ImageDateCollection and \ref ImageCount work together to support the date bar in its counting of images.
  See \ref XMLDB::XMLImageDateCollection::count for a detailed description of this.

  <h2>Undocumented classes</h2>

  \li CategoryCollection
  \li Category
  \li CategoryItem
  \li FileInfo
  \li GroupCounter
  \li ImageInfoList
  \li MD5
  \li MD5Map
  \li MediaCount
  \li MemberMap
  \li NewImageFinder
**/
// vi:expandtab:tabstop=4 shiftwidth=4:
