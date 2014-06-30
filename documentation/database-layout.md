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


index.xml
---------

Below is a visualization of the DOM-Tree of the index.xml file. Attributes are
within parenthesis, comments in square brackets.

### Version 3 ###

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
|   (file, label, description, startDate, endDate, angle, md5sum, width, height)
|   (stackId, stackOrder, rating) [optional]
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
```


### Version 4 ###


```
KPhotoAlbum
| (version=4,compressed=1)
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
| +-Category (name,icon,show,viewtype,thumbnailsize)
|   +-value (value, id)
|
+-images
| +-image
|   (file, label, description, startDate, endDate, angle, md5sum, width, height)
|   (stackId, stackOrder, rating) [optional]
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


### Attribute values explained ###


 * blocklist
    - block
       + ```file```
         Relative filename to ignore.
 * Categories
    - Category
       + ```icon```
         XDG desktop icon name
       + ```name```
         Category name
       + ```show```
         ```0|1``` - hide or show category in the viewer.
       + ```thumbnailsize```
         Category-thumbnail size in pixel.
       + ```viewtype```
         Appearance of list views in the browser.
         ```TreeView=0, ThumbedTreeView=1, IconView=2, ThumbedIconView=3```
       + value
          * ```id```
            Numerical tag id, unique within each Category.
          * ```value```
            Tag name.
 * images
    - image
      + ```angle```
        Image rotation in degrees; between 0 and 359.
      + ```description```
        Description field; Text.
      + ```endDate```
        End date of the image (see fuzzy dates) (```yyyy-mm-dd[Thh:mm:ss]```, second optional part starts with uppercase 'T')
      + ```file```
        Relative path to the image file.
      + ```gpsAlt``` (since KPA 3.1)
        GPS altitude data, double.
      + ```gpsLat``` (since KPA 3.1)
        GPS latitude data, double.
      + ```gpsLon``` (since KPA 3.1)
        GPS longitude data, double.
      + ```gpsPrec``` (since KPA 3.1)
        GPS precision data, integer (-1 for "no precision data").
      + ```heigth```
        Image height in pixel.
      + ```label```
        Textual label assigned to the image
      + ```md5sum```
        MD5 sum of the image file.
      + options
        * option
          - ```name```
            Category name; matches one of ```Categories.Category.name```
          - value
            + ```value```
              Tag name; matches one of ```Categories.Category.value.value```
            + ```area``` (since version=4 / KPA 4.5)
              Positional information for the tag.
              X,Y (upper left corner), width, height; all values in pixel.
      + ```rating``` (since KPA 3.1)
        Integer rating ("stars"), between 0 and 10.
      + ```stackId``` (since KPA 3.1)
        Numerical stack ID; images with the same stackId are displayed as an image stack.
      + ```stackOrder``` (since KPA 3.1)
        Image position within a stack; only valid when stackId is set.
        Unique within the same stack.
      + ```startDate```
        Start date of the image (see fuzzy dates) (```yyyy-mm-dd[Thh:mm:ss]```, second optional part starts with uppercase 'T')
      + ```width```
        Image width in pixel.
 * member-groups
    - member
      + ```category```
        Category name; matches one of ```Categories.Category.name```
      + ```group-name```
        Name of the group, may equal a Tag name and is usually displayed like a Tag name.
      + ```member``` (uncompressed format)
        A single tag name.
      + ```members``` (compressed format)
        Numerical tag ids, separated by comma.
