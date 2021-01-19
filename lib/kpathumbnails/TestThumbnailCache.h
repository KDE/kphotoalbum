// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#ifndef KPATHUMBNAILS_TEST_THUMBNAILCACHE_H
#define KPATHUMBNAILS_TEST_THUMBNAILCACHE_H

#include <QtTest/QTest>

namespace KPATest
{
class TestThumbnailCache : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    /**
     * @brief Set up v4 index file, check index file after conversion.
     */
    void loadV4ThumbnailIndex();
    void insertRemove();
};
}

#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
