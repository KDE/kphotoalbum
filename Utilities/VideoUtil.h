/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIDEO_UTIL_H
#define VIDEO_UTIL_H
#include <kpabase/FileName.h>

#include <QSet>
#include <QString>

namespace Utilities
{
const QSet<QString> &supportedVideoExtensions();
bool isVideo(const DB::FileName &fileName);
}

#endif /* VIDEO_UTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
