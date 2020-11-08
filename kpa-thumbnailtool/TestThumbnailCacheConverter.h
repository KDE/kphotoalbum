/* SPDX-FileCopyrightText: 2020 The KPhotoAlbum development team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KPA_THUMBNAILTOOL_TESTTHUMBNAILCACHECONVERTER_H
#define KPA_THUMBNAILTOOL_TESTTHUMBNAILCACHECONVERTER_H

#include <QByteArray>
#include <QtTest/QtTest>

namespace KPATest
{
class TestThumbnailCacheConverter : public QObject
{
    Q_OBJECT
private slots:
    void convertV5toV4();
};
}

#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
