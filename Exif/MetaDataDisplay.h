// SPDX-FileCopyrightText: 2021 Tobias Leupold <tl@l3u.de>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef METADATADISPLAY_H
#define METADATADISPLAY_H

#include <QLabel>

namespace Exif
{

class MetaDataDisplay : public QLabel
{
    Q_OBJECT

public:
    explicit MetaDataDisplay(QWidget *parent = nullptr);
    void setFileName(const QString &fileName);

};

}

#endif // METADATADISPLAY_H
