/* SPDX-FileCopyrightText: 2013 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CompressFileInfo.h"

static bool _useCompress;

void setUseCompressedFileFormat(bool b)
{
    _useCompress = b;
}

bool useCompressedFileFormat()
{
    return _useCompress;
}
