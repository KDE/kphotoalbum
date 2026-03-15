// SPDX-FileCopyrightText: 2021-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#ifndef KPATEST_FILENAME_H
#define KPATEST_FILENAME_H

#include "UIDelegate.h"

#include <QTemporaryDir>
#include <QtTest/QTest>

namespace KPATest
{
class TestFileName : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    /**
     * @brief Check behaviour with uninitialized SettingsData.
     * Merely creating a null FileName should not trigger an assertion.
     */
    void uninitialized();
    void absolute();
    void relative();
    void operators();
    void initTestCase();

private:
    QTemporaryDir m_tmpDir;
    DB::DummyUIDelegate m_uiDelegate;
};
}

#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
