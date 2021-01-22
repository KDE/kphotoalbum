// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "TestFileName.h"

#include "FileName.h"

#include <kpabase/SettingsData.h>
#include <kpabase/UIDelegate.h>

#include <QRegularExpression>

namespace
{
constexpr auto msgPreconditionFailed = "Precondition for test failed - please fix unit test!";
}

//void KPATest::FileName::initTestCase()
//{
//    qSetGlobalQHashSeed(0);
//}

//void KPATest::FileName::cleanupTestCase()
//{
//}

void KPATest::TestFileName::uninitialized()
{
    const DB::FileName nullFN;
    QVERIFY(nullFN.isNull());
}

void KPATest::TestFileName::absolute()
{
    QTemporaryDir tmpDir;
    QVERIFY2(tmpDir.isValid(), msgPreconditionFailed);
    tmpDir.setAutoRemove(false);
    DB::DummyUIDelegate uiDelegate;
    Settings::SettingsData::setup(tmpDir.path(), uiDelegate);

    using DB::FileName;

    // empty filename
    const QRegularExpression imageRootWarning { QStringLiteral("Absolute filename is outside of image root:") };
    QTest::ignoreMessage(QtWarningMsg, imageRootWarning);
    const auto emptyFN = FileName::fromAbsolutePath({});
    QVERIFY(emptyFN.isNull());

    // incorrect root
    const auto outsidePath = QStringLiteral("/notarealdirectory/test.jpg");
    QVERIFY2(!outsidePath.startsWith(tmpDir.path()), msgPreconditionFailed);
    QTest::ignoreMessage(QtWarningMsg, imageRootWarning);
    const auto outsideFN = FileName::fromAbsolutePath(outsidePath);
    QVERIFY(outsideFN.isNull());

    // relative filename
    QDir cwd;
    QTest::ignoreMessage(QtWarningMsg, imageRootWarning);
    const auto relativeFN = FileName::fromAbsolutePath(cwd.relativeFilePath(tmpDir.path() + QStringLiteral("/test.jpg")));
    QVERIFY(relativeFN.isNull());

    // correct filename
    QDir imageRoot { tmpDir.path() };
    const auto absoluteFilePath = imageRoot.absoluteFilePath(QStringLiteral("atest.jpg"));
    const auto correctFN = FileName::fromAbsolutePath(absoluteFilePath);
    QVERIFY(!correctFN.isNull());
    QCOMPARE(correctFN.absolute(), absoluteFilePath);
    QCOMPARE(correctFN.relative(), imageRoot.relativeFilePath(absoluteFilePath));
    QVERIFY(!correctFN.exists());
    QCOMPARE(correctFN.operator QUrl().toLocalFile(), absoluteFilePath);

    const auto sameFN = FileName::fromAbsolutePath(absoluteFilePath);
    QVERIFY(sameFN == correctFN);
    QVERIFY(sameFN != emptyFN);

    const auto relativeCorrectFN = FileName::fromRelativePath(imageRoot.relativeFilePath(absoluteFilePath));
    QVERIFY(!relativeCorrectFN.isNull());
    QCOMPARE(relativeCorrectFN.absolute(), correctFN.absolute());
    QCOMPARE(relativeCorrectFN.relative(), correctFN.relative());
    QVERIFY(relativeCorrectFN == correctFN);

    // existing correct filename
    QFile existingFile { imageRoot.absoluteFilePath(QStringLiteral("existing.jpg")) };
    QVERIFY2(existingFile.open(QIODevice::WriteOnly), msgPreconditionFailed);
    existingFile.close();
    const auto existingFN = FileName::fromAbsolutePath(existingFile.fileName());
    QVERIFY(!existingFN.isNull());
    QCOMPARE(existingFN.absolute(), existingFile.fileName());
    QCOMPARE(existingFN.relative(), imageRoot.relativeFilePath(existingFile.fileName()));
    QVERIFY(existingFN.exists());
    QVERIFY(correctFN != existingFN);

    // alphabetical order
    QVERIFY(correctFN < existingFN);
}

QTEST_MAIN(KPATest::TestFileName)

// vi:expandtab:tabstop=4 shiftwidth=4:
