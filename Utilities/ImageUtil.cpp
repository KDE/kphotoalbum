// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImageUtil.h"

#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>

#include <QDir>
#include <QFileInfo>

namespace
{
constexpr QFileDevice::Permissions FILE_PERMISSIONS { QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther };
}

QImage Utilities::scaleImage(const QImage &image, const QSize &size, Qt::AspectRatioMode mode)
{
    return image.scaled(size, mode, Settings::SettingsData::instance()->smoothScale() ? Qt::SmoothTransformation : Qt::FastTransformation);
}

void Utilities::saveImage(const DB::FileName &fileName, const QImage &image, const char *format)
{
    const QFileInfo info(fileName.absolute());
    QDir().mkpath(info.path());
    QFile imageFile { fileName.absolute() };
    if (!imageFile.open(QIODevice::ReadWrite)) {
        qCWarning(UtilitiesLog) << "Could not open file for writing:" << imageFile.fileName();
        return;
    }
    if (!imageFile.setPermissions(FILE_PERMISSIONS)) {
        qCInfo(UtilitiesLog) << "Could not set permissions on file:" << imageFile.fileName();
        // don't abort if permissions could not be set
    }
    const bool ok = image.save(&imageFile, format);
    if (!ok) {
        qCWarning(UtilitiesLog) << "Could not save image:" << fileName.absolute();
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:
