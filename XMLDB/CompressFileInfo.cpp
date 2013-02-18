#include "CompressFileInfo.h"

static bool _useCompress;

void setUseCompressedFileFormat(bool b){
    _useCompress = b;
}

bool useCompressedFileFormat()
{
    return _useCompress;
}
