Image database overview for KPhotoAlbum
=======================================

Concepts
--------

### Fuzzy Dates ###

KPhotoAlbum has the concept of fuzzy dates (or date intervals), which are defined by a start date
and an end date (both include a timestamp). This helps for photos which have been digitized from an
analog medium.

When the exact timestamp is known, startDate equals endDate.


### Directory structure ###

All images are expected to be located below a common root folder. The root folder is the one
containing the index.xml database file.

All file names in the index.xml file are relative to the root folder.


### Tags ###

Tags (sometimes called Categories in KPhotoAlbum) are arranged in multiple independent
hierarchies, i.e. there is no common root for all tags.

Tag hierarchies are organized as DAGs (directed acyclic graphs).


### Additional metadata ###

Exif information is stored in an sqlite database called `exif-info.db` in the image root folder.
If the exif database is removed, it can be recreated from the image files.


### Thunmbnails ###

Thumbnails are stored in packed form in `.thumbnails` in the image root folder.  Thumbnails can be recreated from the image files.  The packed format is much more efficient in terms of I/O than storing the thumbnails as individual image files in the filesystem.

The thumbnails themselves are stored in JPEG format, packed into 32 MB container files named `thumb-<n>`, with n starting from 0 and incrementing as needed.  The size of the JPEG images is determined by the user's configuration choice.  The choice of 32 MB is arbitrary, but it combines good I/O efficiency (many thumbnails per file and the ability to stream thumbnails efficiently) with backup efficiency (not modifying very large files constantly).  There is no header, delimiter, or descriptor for the thumbnails in the container files; they require the index described below to be of use.

Additionally, an index file named `thumbnailindex` contains an index allowing KPhotoAlbum to quickly locate the thumbnail for any given file.  The thumbnailindex file is stored in binary form as implemented by QDataStream, as depicted below.  The thumbnailindex cannot be regenerated from the thumbnail containers.

```
thumbnailindex
|
+-Header
| +-File version (int, currently 4)
| +-Current file (file index of last written thumbnail) being written to (int)
| +-Current offset into current file, in bytes (int)
| +-Total number of thumbnails indexed (int)
|
+-images
| +-image
|   +-relative pathname (QString)
|   +-file index (int)
|   +-file offset (int)
|   +-thumbnail size (int)
```

index.xml
---------

Below is a visualization of the DOM-Tree of the index.xml file. Attributes are
within parenthesis, comments in square brackets.

### Version 3 ###
Used in KPA v4.4 (and in KPA v4.5, if positionable tags are not used).

```
KPhotoAlbum
| (version=3,compressed=1)
|
+-Categories
| +-Category (name,icon,show,viewtype,thumbnailsize)
|   +-value (value, id)
|
+-images
| +-image
|   (file, label, startDate, endDate, angle, md5sum, width, height)
|   (desctiption, stackId, stackOrder, rating, videoLength) [optional]
|   (#Categories.Category.name#=#Categories.Category.value.id#) [optional]
|
+-blocklist
| +-block (file)
|
+-member-groups
  +-member (category,group-name,members)
```

```
KPhotoAlbum
| (version=3,compressed=0)
|
+-Categories
| +-Category (name,icon,show,viewtype,thumbnailsize)
|   +-value (value, id)
|
+-images
| +-image
|   (file, label, startDate, endDate, angle, md5sum, width, height)
|   (desctiption, stackId, stackOrder, rating, videoLength) [optional]
|   +-options
|     +-option(name=#Categories.Category.name#)
|       +-value(value=#Categories.Category.value.value#)
|
+-blocklist
| +-block (file)
|
+-member-groups
  +-member (category,group-name,member)
```


### Version 4 ###
Used in KPA v4.5.

```
KPhotoAlbum
| (version=4,compressed=1)
|
+-Categories
| +-Category (name,icon,show,viewtype,thumbnailsize,positionable)
|   +-value (value, id)
|
+-images
| +-image
|   (file, label, startDate, endDate, angle, md5sum, width, height)
|   (description, stackId, stackOrder, rating, videoLength) [optional]
|   (#Categories.Category.name#=#Categories.Category.value.id#) [optional]
|   +-options
|     +-option(name=#Categories.Category.name#)
|       +-value(value=#Categories.Category.value.value#, area="x y w h")
|
+-blocklist
| +-block (file)
|
+-member-groups
  +-member (category,group-name,members)
```

```
KPhotoAlbum
| (version=4,compressed=0)
|
+-Categories
| +-Category (name,icon,show,viewtype,thumbnailsize,positionable)
|   +-value (value, id)
|
+-images
| +-image
|   (file, label, startDate, endDate, angle, md5sum, width, height)
|   (description, stackId, stackOrder, rating, videoLength) [optional]
|   +-options
|     +-option(name=#Categories.Category.name#)
|       +-value(value=#Categories.Category.value.value#, area="x y w h")
|
+-blocklist
| +-block (file)
|
+-member-groups
  +-member (category,group-name,member)
```

#### Differences to version 3 ####
 * Tags can be positionable, i.e. the ```images.image.options.option.value```
   elements may have an additional attribute ```area```.
 * In the compressed format, ```images.image``` tags may have sub-elements ```options.option.value```.
   This format is used only for category values when an area attribute is present.


### Version 5 ###
Not used in an official release.


```
KPhotoAlbum
| (version=5,compressed=1)
|
+-Categories
| +-Category (name,icon,show,viewtype,thumbnailsize,positionable)
|   +-value
|     (value, id)
|     (birthDate) [optional]
|
+-images
| +-image
|   (file, label, startDate, endDate, angle, md5sum, width, height)
|   (description, stackId, stackOrder, rating, videoLength) [optional]
|   (#Categories.Category.name#=#Categories.Category.value.id#) [optional]
|   +-options
|     +-option(name=#Categories.Category.name#)
|       +-value(value=#Categories.Category.value.value#, area="x y w h")
|
+-blocklist
| +-block (file)
|
+-member-groups
  +-member (category,group-name,members)
```

```
KPhotoAlbum
| (version=5,compressed=0)
|
+-Categories
| +-Category (name,icon,show,viewtype,thumbnailsize,positionable)
|   +-value
|     (value, id)
|     (birthDate) [optional]
|
+-images
| +-image
|   (file, label, startDate, endDate, angle, md5sum, width, height)
|   (description, stackId, stackOrder, rating, videoLength) [optional]
|   +-options
|     +-option(name=#Categories.Category.name#)
|       +-value(value=#Categories.Category.value.value#, area="x y w h")
|
+-blocklist
| +-block (file)
|
+-member-groups
  +-member (category,group-name,member)
```

#### Differences to version 4 ####
 * ```Categories.Category.value``` has an optional attribute ```birthDate```


### Version 6 ###
Used in KPA v4.6.

Same structure as version 5.

#### Differences to version 5 ####
 * The legacy categories Keywords, Persons and Locations are not handled special any more.
   Upon upgrade from an older version, "Persons" is renamed to "People", and "Locations"
   is renamed to "Places".
 * Older versions of KPhotoAlbum stored the standard categories (People, Places, Events, Folder,
   Tokens, Media Type and Keywords; those have a translation) as their respective localized
   versions. This lead to several problems if a non-English locale was used and has been fixed in
   v4.6. Along with this update, it was also necessary to move all thumbnails in the CategoryImages
   directory refering to the old names and fix the respective category names in kphotoalbumrc.
 * The GPS related image tags (gpsAlt, gpsLat, gpsLon and gpsPrec) have been removed and are now
   superseded by storing GPS data in the EXIF database.


### Version 7 ###
Used in KPA v4.7

```
KPhotoAlbum
| (version=7, compressed=1)
|
+-Categories
| +-Category
|   (name, icon, show, viewtype, thumbnailsize, positionable)
|   (meta) [optional]
|   +-value
|     (value, id)
|     (birthDate) [optional]
|
+-images
| +-image
|   (file, label, startDate, endDate, angle, md5sum, width, height)
|   (description, stackId, stackOrder, rating, videoLength) [optional]
|   (#Categories.Category.name#=#Categories.Category.value.id#) [optional]
|   +-options
|     +-option(name=#Categories.Category.name#)
|       +-value(value=#Categories.Category.value.value#, area="x y w h")
|
+-blocklist
| +-block (file)
|
+-member-groups
  +-member (category,group-name,members)
```

```
KPhotoAlbum
| (version=7, compressed=0)
|
+-Categories
| +-Category
|   (name, icon, show, viewtype, thumbnailsize, positionable)
|   (meta) [optional]
|   +-value
|     (value, id)
|     (birthDate) [optional]
|
+-images
| +-image
|   (file, label, startDate, endDate, angle, md5sum, width, height)
|   (description, stackId, stackOrder, rating, videoLength) [optional]
|   +-options
|     +-option(name=#Categories.Category.name#)
|       +-value(value=#Categories.Category.value.value#, area="x y w h")
|
+-blocklist
| +-block (file)
|
+-member-groups
  +-member (category,group-name,member)
```

#### Differences to version 6 ####
The concept of translatable "standard" categories led to a lot of problems when users started KPA
with different locales. Some of them simply can't be solved, so we decided to remove translatable
category names. Now, each category is stored with it's literal name.
Added an additional optional "meta" attribute to the Category-tag, so that the "Tokens" category (a
"special" category like "Folder", but stored in the database and thus causing the same translation
problems like the old "standard" categories) can be marked as such and does not need to have a fixed
name anymore.


### Version 8 ###
Used in KPA v5.4

```
KPhotoAlbum
| (version=8, compressed=1)
|
+-Categories
| +-Category
|   (name, icon, show, viewtype, thumbnailsize, positionable)
|   (meta) [optional]
|   +-value
|     (value, id)
|     (birthDate) [optional]
|
+-images
| +-image
|   (file, startDate, md5sum, width, height)
|   (angle, description, endDate, label, rating, stackId, stackOrder, videoLength) [optional]
|   (#Categories.Category.name#=#Categories.Category.value.id#) [optional]
|   +-options
|     +-option(name=#Categories.Category.name#)
|       +-value(value=#Categories.Category.value.value#, area="x y w h")
|
+-blocklist
| +-block (file)
|
+-member-groups
  +-member (category,group-name,members)
```

```
KPhotoAlbum
| (version=8, compressed=0)
|
+-Categories
| +-Category
|   (name, icon, show, viewtype, thumbnailsize, positionable)
|   (meta) [optional]
|   +-value
|     (value, id)
|     (birthDate) [optional]
|
+-images
| +-image
|   (file, startDate, md5sum, width, height)
|   (angle, description, endDate, label, rating, stackId, stackOrder, videoLength) [optional]
|   +-options
|     +-option(name=#Categories.Category.name#)
|       +-value(value=#Categories.Category.value.value#, area="x y w h")
|
+-blocklist
| +-block (file)
|
+-member-groups
  +-member (category,group-name,member)
```

#### Differences to version 7 ####

 * ```images.image.angle``` is only saved when it differs from the default angle (0)
 * ```images.image.endDate``` is only saved when it differs from the start date
 * ```images.image.label``` is only saved when it differs from the default label


### Attribute values explained ###


 * blocklist
    - block
       + ```file```
         Relative filename to ignore.
 * Categories
    - Category
       + ```icon```<br/>
         XDG desktop icon name
       + ```name```<br/>
         Category name
       + ```show```<br/>
         ```0|1``` - hide or show category in the viewer.
       + ```thumbnailsize```<br/>
         Category-thumbnail size in pixel.
       + ```viewtype```<br/>
         Appearance of list views in the browser.
         ```TreeView=0, ThumbedTreeView=1, IconView=2, ThumbedIconView=3```
       + ```positionable``` (since version=4 / KPA v4.5)<br/>
         ```0|1``` - indicates whether this category can contain areas (positioned tags) or not.
       + ```meta``` (since version=7 / KPA v5.7)<br/>
         Meta information that holds an unique id for special categories (so that they can be tracked when they are renamed for localization).
       + value
          * ```id```<br/>
            Numerical tag id, unique within each Category.
          * ```value```<br/>
            Tag name.
          * ```birthDate``` (since version=5 / KPA v4.6)<br/>
            Birthdate (```yyyy-mm-dd```) of a person (but allowed on all categories).
            Is used to display the age of a person on an image.
 * images
    - image
      + ```angle```<br/>
        Image rotation in degrees; between 0 and 359.
      + ```description```<br/>
        Description field; Text.
      + ```endDate```<br/>
        End date of the image (see fuzzy dates) (```yyyy-mm-dd[Thh:mm:ss]```, second optional part starts with uppercase 'T')
      + ```file```<br/>
        Relative path to the image file.
      + ```gpsAlt``` (since KPA 3.1, deprecated in version=6 / KPA v4.6)<br/>
        GPS altitude data, double.
      + ```gpsLat``` (since KPA 3.1, deprecated in version=6 / KPA v4.6)<br/>
        GPS latitude data, double.
      + ```gpsLon``` (since KPA 3.1, deprecated in version=6 / KPA v4.6)<br/>
        GPS longitude data, double.
      + ```gpsPrec``` (since KPA 3.1, deprecated in version=6 / KPA v4.6)<br/>
        GPS precision data, integer (-1 for "no precision data").
      + ```heigth```<br/>
        Image height in pixel.
      + ```label```<br/>
        Textual label assigned to the image
      + ```md5sum```<br/>
        MD5 sum of the image file.
      + options
        * option
          - ```name```<br/>
            Category name; matches one of ```Categories.Category.name```
          - value
            + ```value```<br/>
              Tag name; matches one of ```Categories.Category.value.value```
            + ```area``` (since version=4 / KPA 4.5)<br/>
              Positional information for the tag.
              X,Y (upper left corner), width, height; all values in pixel.
      + ```rating``` (since KPA 3.1)<br/>
        Integer rating ("stars"), between 0 and 10.
      + ```stackId``` (since KPA 3.1)<br/>
        Numerical stack ID; images with the same stackId are displayed as an image stack.  Stack ID starts with 1.
      + ```stackOrder``` (since KPA 3.1)<br/>
        Image position within a stack; only valid when stackId is set.<br/>
        Unique within the same stack.  Stack order starts with 1.
      + ```startDate```<br/>
        Start date of the image (see fuzzy dates) (```yyyy-mm-dd[Thh:mm:ss]```, second optional part starts with uppercase 'T')
      + ```videoLength```<br/>
        Length of the video in seconds; -1 if length is not known.
        Only applicable to video files.
      + ```width```<br/>
        Image width in pixel.
 * member-groups
    - member
      + ```category```<br/>
        Category name; matches one of ```Categories.Category.name```
      + ```group-name```<br/>
        Name of the group, may equal a Tag name and is usually displayed like a Tag name.
      + ```member``` (uncompressed format)<br/>
        A single tag name.
      + ```members``` (compressed format)<br/>
        Numerical tag ids, separated by comma.
