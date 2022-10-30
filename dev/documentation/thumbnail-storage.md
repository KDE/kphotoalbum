<!--
SPDX-FileCopyrightText: 2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>

SPDX-License-Identifier: CC-BY-SA-4.0
-->

# Thumbnail storage file formats

Thumbnails are stored in packed form in `.thumbnails` in the image root folder.  Thumbnails can be recreated from the image files.  The packed format is much more efficient in terms of I/O than storing the thumbnails as individual image files in the filesystem.

The thumbnails themselves are stored in JPEG format, packed into 32 MB container files named `thumb-<n>`, with n starting from 0 and incrementing as needed.  The size of the JPEG images is determined by the user's configuration choice.  The choice of 32 MB is arbitrary, but it combines good I/O efficiency (many thumbnails per file and the ability to stream thumbnails efficiently) with backup efficiency (not modifying very large files constantly).  There is no header, delimiter, or descriptor for the thumbnails in the container files; they require the thumbnailindex described below to be of use.

Additionally, an index file named `thumbnailindex` contains an index allowing KPhotoAlbum to quickly locate the thumbnail for any given file.  The thumbnailindex file is stored in binary form as implemented by QDataStream, as depicted below.  The thumbnailindex cannot be regenerated from the thumbnail containers.

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
