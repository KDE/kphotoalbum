/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEXTUTIL_H
#define TEXTUTIL_H

// Local includes
#include "DB/CategoryPtr.h"
#include "DB/ImageInfoPtr.h"

// Qt includes
#include <QDate>
#include <QMap>
#include <QPair>
#include <QString>

namespace Utilities
{
QString createInfoText(DB::ImageInfoPtr info, QMap<int, QPair<QString, QString>> *);
QString formatAge(DB::CategoryPtr category, const QString &item, DB::ImageInfoPtr info);
QString timeAgo(const DB::ImageInfoPtr info);
QString timeAgo(const QDate &date);
}

#endif /* TEXTUTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
