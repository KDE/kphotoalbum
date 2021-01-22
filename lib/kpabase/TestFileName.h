// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#ifndef KPATEST_FILENAME_H
#define KPATEST_FILENAME_H

#include <QtTest/QTest>

namespace KPATest
{
class TestFileName : public QObject
{
    Q_OBJECT
private slots:
    /**
     * @brief Check behaviour with uninitialized SettingsData.
     * Merely creating a null FileName should not trigger an assertion.
     */
    void uninitialized();
    void absolute();
    //void initTestCase();
    //void cleanupTestCase();
};
}

#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
