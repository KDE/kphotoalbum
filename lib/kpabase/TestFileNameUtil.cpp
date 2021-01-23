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
    const auto absolutePathWithSlash = QStringLiteral("/path/to/imageroot/");
    //const auto absolutePathWithSlashes = QStringLiteral("/path/to/imageroot//");

    using Utilities::stripEndingForwardSlash;
    QCOMPARE(stripEndingForwardSlash(emptyString), emptyString);
    QCOMPARE(stripEndingForwardSlash(absolutePath), absolutePath);
    QCOMPARE(stripEndingForwardSlash(absolutePathWithSlash), absolutePath);
    // QCOMPARE(stripEndingForwardSlash(absolutePathWithSlashes), absolutePath);
}

void KPATest::TestFileNameUtil::folderName()
{
}

//void KPATest::TestFileNameUtil::cleanupTestCase()
//{
//}

QTEST_MAIN(KPATest::TestFileNameUtil)

// vi:expandtab:tabstop=4 shiftwidth=4:
