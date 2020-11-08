/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FASTJPEG_H
#define FASTJPEG_H
#include <QImage>
#include <QString>

namespace DB
{
class FileName;
}

namespace Utilities
{
bool loadJPEG(QImage *img, const DB::FileName &imageFile, QSize *fullSize, int dim = -1);
bool loadJPEG(QImage *img, const DB::FileName &imageFile, QSize *fullSize, int dim = -1, char *membuf = NULL, size_t membufSize = 0);
bool loadJPEG(QImage *img, const QByteArray &data, QSize *fullSize, int dim = -1);
bool isJPEG(const DB::FileName &fileName);
}

#endif /* FASTJPEG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
