// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#ifndef KPATHUMBNAILS_TEST_EXIFDATABASE_H
#define KPATHUMBNAILS_TEST_EXIFDATABASE_H

#include <QtTest/QTest>

namespace KPATest
{
class TestExifDatabase : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void trivialTests();
};
}

#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
