// SPDX-FileCopyrightText: 2003-2024 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DESCRIPTIONUTIL_H
#define DESCRIPTIONUTIL_H

// Local includes
#include "DB/ImageInfoPtr.h"

// Qt includes
#include <QMap>
#include <QString>

namespace Utilities
{
QString createInfoText(DB::ImageInfoPtr info, QMap<int, QPair<QString, QString>> *);
}

#endif // DESCRIPTIONUTIL_H

// vi:expandtab:tabstop=4 shiftwidth=4:
