// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "TestFileNameUtil.h"

#include "FileNameUtil.h"

#include <kpabase/SettingsData.h>

namespace
{
constexpr auto msgPreconditionFailed = "Precondition for test failed - please fix unit test!";
}

void KPATest::TestFileNameUtil::initTestCase()
{
    QVERIFY2(tmpDir.isValid(), msgPreconditionFailed);
    Settings::SettingsData::setup(tmpDir.path(), uiDelegate);
    // qSetGlobalQHashSeed(0);
}

void KPATest::TestFileNameUtil::stripEndingForwardSlash()
{
    const auto emptyString = QString();
    const auto absolutePath = QStringLiteral("/path/to/imageroot");
    const auto slash = QStringLiteral("/");

    using Utilities::stripEndingForwardSlash;
    QCOMPARE(stripEndingForwardSlash(emptyString), emptyString);
    QCOMPARE(stripEndingForwardSlash(absolutePath), absolutePath);
    QCOMPARE(stripEndingForwardSlash(absolutePath + slash), absolutePath);
    // currently not implemented that way:
    // QCOMPARE(stripEndingForwardSlash(absolutePath + slash + slash), absolutePath);
}

void KPATest::TestFileNameUtil::folderName()
{
    const auto emptyString = QString();
    const auto absolutePath = QStringLiteral("/path/to/imageroot");
    const auto relativePath = QStringLiteral("dirname/otherdir");
    const auto slash = QStringLiteral("/");
    const auto filename = QStringLiteral("filename.jpg");
    using Utilities::relativeFolderName;

    QCOMPARE(relativeFolderName(emptyString), emptyString);
    QCOMPARE(relativeFolderName(absolutePath), QStringLiteral("/path/to"));
    QCOMPARE(relativeFolderName(absolutePath + slash), absolutePath);
    QCOMPARE(relativeFolderName(absolutePath + slash + filename), absolutePath);
    // currently not implemented that way:
    //QCOMPARE(relativeFolderName(absolutePath + slash + slash + filename), absolutePath);
    QCOMPARE(relativeFolderName(relativePath + slash), relativePath);
    QCOMPARE(relativeFolderName(relativePath + slash + filename), relativePath);
}

//void KPATest::TestFileNameUtil::cleanupTestCase()
//{
//}

QTEST_MAIN(KPATest::TestFileNameUtil)

// vi:expandtab:tabstop=4 shiftwidth=4:
