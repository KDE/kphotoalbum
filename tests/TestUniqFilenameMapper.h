// SPDX-FileCopyrightText: 2021-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#ifndef KPATEST_UNIQ_FILENAME_MAPPER_H
#define KPATEST_UNIQ_FILENAME_MAPPER_H

#include "UIDelegate.h"

#include <QTemporaryDir>
#include <QtTest/QTest>

namespace KPATest
{
class TestUniqFilenameMapper : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testWithNoDirectory();
    void testWithNoDirectory_data();
    void testWithDirectory();
    void testWithDirectory_data();
    void testReset();
    void testReset_data();

private:
    QTemporaryDir m_tmpDir;
    DB::DummyUIDelegate m_uiDelegate;
};
}

#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
