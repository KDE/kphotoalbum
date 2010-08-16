#ifndef CACHEFILEINFO_H
#define CACHEFILEINFO_H

struct CacheFileInfo
{
    CacheFileInfo() {}
    CacheFileInfo( int fileIndex, int offset, int width, int height )
        : fileIndex( fileIndex ), offset( offset ), width( width ), height( height ) {}

    int fileIndex;
    int offset;
    int width;
    int height;
};

#endif /* CACHEFILEINFO_H */

