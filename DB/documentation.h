/**
  \namespace DB
  \brief The core database classes.

  <h2>The Database</h2>
  The database is abstracted using the class \ref ImageDB. This interface
  is subclasses by \ref XMLDB::Database and \ref SQLDB::Database. You can
  get to the global instance of the running database using \ref
  ImageDB::instance.


  <h2>Image information</h2>
  All information about an image is stored in the class \ref ImageInfo
  (file name, category items for the image, its size etc etc). This class
  is rather heavy, and in the early days of KPA lots of methods was given
  information about an image or a set of images. To avoid copying the
  ImageInfo all the time, or to avoid handing pointers around, the type
  \ref ImageInfoPtr was introduced. This is a smart pointer, which results
  in \ref ImageInfo's are send around as pointers behind the
  screen, without the drawback of pointers.

  Originally KPhotoAlbum was implemented without the SQL backend in mind,
  all the information from the database was loaded into memory. All the
  methods in the \ref ImageDB interface that returned information about an
  image would return a \ref ImageInfoPtr. This, however, is not very SQL
  friendly - SQL should not fetch the info from it database before it really is
  needed.

  To fix this problem, the class \ref IdList was introduced. It abstracts the result of
  queries on the ImageDB. A result consist of a list of \ref Id,
  which is the abstraction of a given item in the result. When you need to
  traverse the items of a result, you must iterate them using the iterator
  \ref IdList::begin returns. Doing it that way allows the SQL backend to
  fetch items in chunks. To get to the actual image information use \ref ImageDB::info.

  <h2>Searching/Browsing</h2>
  When the user searches or browses, a \ref ImageSearchInfo instance is
  build with information about the current search. This class contains
  information about all the parts of the search, like date range, category
  items to match etc.

  When the search is to happen, the information about categories are
  compiled into a structure that can take care of searching for the
  sub component of the category search (A category search may look like
  "friends &! Joe".

  The structure consist of these classes:
  \li \ref CategoryMatcher - This is the base class for the hierachy of
  items.
  \li  \ref SimpleCategoryMatcher - Base class for \ref
  ValueCategoryMatcher and \ref NoOtherItemsCategoryMatcher
  \li \ref ValueCategoryMatcher - this represent an item from the search
  ("Joe" or "friend" in the above example)
  \li \ref NoOtherItemsCategoryMatcher - This represent a "No other" item in
  the search
  \li \ref ContainerCategoryMatcher - Base class for \ref
  AndCategoryMatcher and \ref OrCategoryMatcher
  \li \ref AndCategoryMatcher - represent the components of an "and" expression (Jesper &
  Jan & No Other)
  \li \ref OrCategoryMatcher - represent the components of an "or" expression.


  <h2>Image Dates</h2>
  KPhotoAlbum has support for image dates, which are not know with an exact precission -
  this might be the case for a images taken with a non-digital
  camera. Therefore all dates in KPhotoAlbum are represented using the
  class \ref ImageDate, which has a lower and an upper date for the range.

  The two classes \ref ImageDateCollection and \ref ImageCount work together to support the date bar in its counting of images. See \ref XMLImageDateCollection::count for a detailed description of this.

  <h2>Undocumented classes</h2>

  \li CategoryCollection
  \li Category
  \li CategoryItem
  \li FileInfo
  \li GpsCoordinates
  \li GroupCounter
  \li ImageInfoList
  \li MD5
  \li MD5Map
  \li MediaCount
  \li MemberMap
  \li NewImageFinder
**/
