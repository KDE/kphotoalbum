// SPDX-FileCopyrightText: 2021 Tobias Leupold <tl@l3u.de>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "MetaDataDisplay.h"

#include <QDebug>

Exif::MetaDataDisplay::MetaDataDisplay(QWidget *parent) : QLabel(parent)
{
}

void Exif::MetaDataDisplay::setFileName(const QString &fileName)
{
    qDebug() << "fill me with info about" << fileName;
}
