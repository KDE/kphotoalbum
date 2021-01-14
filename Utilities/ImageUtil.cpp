/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImageUtil.h"

#include <kpabase/SettingsData.h>

#include <QDir>
#include <QFileInfo>

QImage Utilities::scaleImage(const QImage &image, const QSize &size, Qt::AspectRatioMode mode)
{
    return image.scaled(size, mode, Settings::SettingsData::instance()->smoothScale() ? Qt::SmoothTransformation : Qt::FastTransformation);
}

void Utilities::saveImage(const DB::FileName &fileName, const QImage &image, const char *format)
{
    const QFileInfo info(fileName.absolute());
    QDir().mkpath(info.path());
    const bool ok = image.save(fileName.absolute(), format);
    Q_ASSERT(ok);
    Q_UNUSED(ok);
}
// vi:expandtab:tabstop=4 shiftwidth=4:
