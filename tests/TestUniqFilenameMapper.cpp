// SPDX-FileCopyrightText: 2021-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "TestUniqFilenameMapper.h"

#include <KLocalizedString>

#include <kpabase/SettingsData.h>
#include "UniqFilenameMapper.h"

namespace
{
constexpr auto msgPreconditionFailed = "Precondition for test failed - please fix unit test!";
}

void KPATest::TestUniqFilenameMapper::initTestCase()
{
    KLocalizedString::setApplicationDomain("kphotoalbum");
    QVERIFY2(m_tmpDir.isValid(), msgPreconditionFailed);
    Settings::SettingsData::setup(m_tmpDir.path(), m_uiDelegate);
}

static QString relativePath(const char* directory, const char *pathname)
{
    if (directory) {
        return QString::fromLatin1("%1/%2").arg(directory).arg(pathname);
    }

    return QString::fromLatin1(pathname);
}

static DB::FileName dbRelativePath(const char* directory, const char *pathname)
{
    return DB::FileName::fromRelativePath(relativePath(directory, pathname));
}

static void testData(const char* directory = nullptr)
{
    QTest::addColumn<DB::FileName>("original");
    QTest::addColumn<QString>("mapped");

    QTest::addRow("unique cd1/def.jpg") << dbRelativePath(directory, "cd1/def.jpg") << relativePath(directory, "def.jpg");
    QTest::addRow("unique cd1/abc/file.jpg") << dbRelativePath(directory, "cd1/abc/file.jpg") << relativePath(directory, "file.jpg");
    QTest::addRow("collision cd3/file.jpg") << dbRelativePath(directory, "cd3/file.jpg") << relativePath(directory, "file-1.jpg");
    QTest::addRow("duplicate cd1/abc/file.jpg") << dbRelativePath(directory, "cd1/abc/file.jpg") << relativePath(directory, "file.jpg");
}

void KPATest::TestUniqFilenameMapper::testWithNoDirectory_data()
{
    testData();
}

void KPATest::TestUniqFilenameMapper::testWithNoDirectory()
{
    QFETCH(DB::FileName, original);
    QFETCH(QString, mapped);

    // Static because this method is called once per row in the test data.
    static Utilities::UniqFilenameMapper ufm;
    QCOMPARE(ufm.uniqNameFor(original), mapped);
}

void KPATest::TestUniqFilenameMapper::testWithDirectory_data()
{
    testData("Pictures");
}

void KPATest::TestUniqFilenameMapper::testWithDirectory()
{
    QFETCH(DB::FileName, original);
    QFETCH(QString, mapped);

    // Static because this method is called once per row in the test data.
    static Utilities::UniqFilenameMapper ufm(QString::fromLatin1("Pictures"));
    QCOMPARE(ufm.uniqNameFor(original), mapped);
}

void KPATest::TestUniqFilenameMapper::testReset_data()
{
    testData();
}

void KPATest::TestUniqFilenameMapper::testReset()
{
    QFETCH(DB::FileName, original);
    QFETCH(QString, mapped);

    // Static because this method is called once per row in the test data.
    static Utilities::UniqFilenameMapper ufm;

    // The test data contains a collision which should not be mapped after a
    // reset.
    ufm.reset();

    // Comparing to the original filename, not the mapped filename in the test
    // data.
    QCOMPARE(ufm.uniqNameFor(original), QFileInfo(original.relative()).fileName());
}

QTEST_MAIN(KPATest::TestUniqFilenameMapper)

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_TestUniqFilenameMapper.cpp"
