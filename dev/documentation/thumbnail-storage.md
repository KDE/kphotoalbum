# Thumbnail storage file formats

## Version 4

Since KPhotoAlbum 4.3.

### thumbnailindex

The index file contains some header information as described below, followed by the actual cache entries.
String data is UTF-8 encoded, data is stored as big-endian.

````
using FileIndex = int32;
using ByteOffset = int32;
using ByteSize = int32;

struct CacheLine {
  int32 string_length;
  char filename_relative[string_length];
  FileIndex file_index;                  /* Index for thumbnail file. Name: "thumb-%d" */
  ByteOffset offset_bytes;               /* Offset in thumbnail file. */
  ByteSize size_bytes;                   /* Thumbnail data size in bytes. */
}

struct Cache {
  int32 FILE_VERSION;                    /* assert(FILE_VERSION == 4); */
  FileIndex CURRENT_FILE;                /* Highest index for thumbnail file. */
  ByteOffset CURRENT_OFFSET;             /* Offset for the next thumbnail in the thumbnail file. */
  ByteSize CACHE_SIZE;                   /* Number of files/thumbnails in the cache. */
  CacheLine CACHE_ENTRIES[CACHE_SIZE];
}
````

### Thumbnail files

Raw data files containing thumbnail data in JPEG format.
File size is limited by KPhotoAlbum to at most 32MB.

The files are names "thumb-%d" where "%d" corresponds to the thumbnail index used in the `thumbnailindex` file.


## Version 5

Since KPhotoAlbum 4.7.
The thumbnail dimensions are now stored in the `thumbnailindex` to prevent inconsistencies.

## thumbnailindex

````
using FileIndex = int32;
using ByteOffset = int32;
using ByteSize = int32;

struct CacheLine {
  int32 string_length;
  char filename_relative[string_length];
  FileIndex file_index;                  /* Index for thumbnail file. Name: "thumb-%d" */
  ByteOffset offset_bytes;               /* Offset in thumbnail file. */
  ByteSize size_bytes;                   /* Thumbnail data size in bytes. */
}

struct Cache {
  int32 FILE_VERSION;                    /* assert(FILE_VERSION == 5); */
  int32 THUMBNAIL_DIMENSIONS;            /* Size of the longer edge of a thumbnail in pixels. */
  FileIndex CURRENT_FILE;                /* Highest index for thumbnail file. */
  ByteOffset CURRENT_OFFSET;             /* Offset for the next thumbnail in the thumbnail file. */
  ByteSize CACHE_SIZE;                   /* Number of files/thumbnails in the cache. */
  CacheLine CACHE_ENTRIES[CACHE_SIZE];
}
````

## Thumbnail files

Unchanged to previous version.
