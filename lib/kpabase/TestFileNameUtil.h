// SPDX-FileCopyrightText: 2021-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#ifndef KPATEST_FILENAMEUTIL_H
#define KPATEST_FILENAMEUTIL_H

#include "UIDelegate.h"

#include <QTemporaryDir>
#include <QtTest/QTest>

namespace KPATest
{
class TestFileNameUtil : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    //void cleanupTestCase();

    void stripEndingForwardSlash();
    void folderName();
    void fileNameFromUserData();

private:
    QTemporaryDir m_tmpDir;
    DB::DummyUIDelegate m_uiDelegate;
};
}

#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
