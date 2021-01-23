// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
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
private slots:
    void initTestCase();
    //void cleanupTestCase();

    void stripEndingForwardSlash();
    void folderName();

private:
    QTemporaryDir tmpDir;
    DB::DummyUIDelegate uiDelegate;
};
}

#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
