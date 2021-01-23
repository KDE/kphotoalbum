// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "TestFileNameUtil.h"

#include "FileName.h"
#include "FileNameUtil.h"

#include <kpabase/SettingsData.h>

#include <QRegularExpression>
#include <QUrl>

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

void KPATest::TestFileNameUtil::fileNameFromUserData()
{
    using Utilities::fileNameFromUserData;
    const auto emptyString = QString();
    const auto emptyUrl = QUrl::fromLocalFile(emptyString).toString();
    const auto absoluteOutsideFilePath = QStringLiteral("/external/path/to/image.jpg");
    const auto absoluteOutsideUrl = QUrl::fromLocalFile(absoluteOutsideFilePath).toString();
    const auto emptyNameWarning = "Relative or absolute filename cannot be empty!";
    QTest::ignoreMessage(QtWarningMsg, emptyNameWarning);
    QCOMPARE(fileNameFromUserData(emptyString), DB::FileName());
    QTest::ignoreMessage(QtWarningMsg, emptyNameWarning);
    QCOMPARE(fileNameFromUserData(emptyUrl), DB::FileName());
    const QRegularExpression imageRootWarning { QStringLiteral("Absolute filename is outside of image root:") };
    QTest::ignoreMessage(QtWarningMsg, imageRootWarning);
    QCOMPARE(fileNameFromUserData(absoluteOutsideFilePath), DB::FileName());
    QTest::ignoreMessage(QtWarningMsg, imageRootWarning);
    QCOMPARE(fileNameFromUserData(absoluteOutsideUrl), DB::FileName());

    const auto relativeFilePath1 = QStringLiteral("image.jpg");
    const auto absoluteFilePath1 = tmpDir.filePath(relativeFilePath1);
    const auto absoluteUrl1 = QUrl::fromLocalFile(absoluteFilePath1).toString();
    const auto relativeUrl1 = QUrl::fromLocalFile(relativeFilePath1).toString();
    const auto fileName1 = DB::FileName::fromRelativePath(relativeFilePath1);
    QVERIFY(!fileName1.isNull());
    QCOMPARE(fileName1.absolute(), absoluteFilePath1);
    QCOMPARE(fileName1.relative(), relativeFilePath1);
    QCOMPARE(fileNameFromUserData(relativeFilePath1), fileName1);
    QCOMPARE(fileNameFromUserData(absoluteFilePath1), fileName1);
    // current implementation does not deal with relative urls
    //QCOMPARE(fileNameFromUserData(relativeUrl1), fileName1);
    QCOMPARE(fileNameFromUserData(absoluteUrl1), fileName1);

    const auto relativeFilePath2 = QStringLiteral("subdir/image.jpg");
    const auto absoluteFilePath2 = tmpDir.filePath(relativeFilePath2);
    const auto absoluteUrl2 = QUrl::fromLocalFile(absoluteFilePath2).toString();
    const auto relativeUrl2 = QUrl::fromLocalFile(relativeFilePath2).toString();
    const auto fileName2 = DB::FileName::fromRelativePath(relativeFilePath2);
    QVERIFY(!fileName2.isNull());
    QCOMPARE(fileName2.absolute(), absoluteFilePath2);
    QCOMPARE(fileName2.relative(), relativeFilePath2);
    QCOMPARE(fileNameFromUserData(relativeFilePath2), fileName2);
    QCOMPARE(fileNameFromUserData(absoluteFilePath2), fileName2);
    // current implementation does not deal with relative urls
    // QCOMPARE(fileNameFromUserData(relativeUrl2), fileName2);
    QCOMPARE(fileNameFromUserData(absoluteUrl2), fileName2);
}

//void KPATest::TestFileNameUtil::cleanupTestCase()
//{
//}

QTEST_MAIN(KPATest::TestFileNameUtil)

// vi:expandtab:tabstop=4 shiftwidth=4:
