// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImageUtil.h"

#include <kpabase/Logging.h>
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
    if (!ok) {
        qCWarning(UtilitiesLog) << "Could not save image:" << fileName.absolute();
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:
