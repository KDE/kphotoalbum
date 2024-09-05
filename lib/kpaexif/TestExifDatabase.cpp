// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "TestExifDatabase.h"

#include "Database.h"

#include <kpabase/SettingsData.h>
#include <kpabase/UIDelegate.h>

#include <QLoggingCategory>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QHashSeed>

namespace
{
constexpr auto msgPreconditionFailed = "Precondition for test failed - please fix unit test!";
}

void KPATest::TestExifDatabase::initTestCase()
{
    // ThumbnailCache uses QHash, which is randomized by default
    QHashSeed::setDeterministicGlobalSeed();

    QVERIFY2(Exif::Database::isAvailable(), msgPreconditionFailed);
}

void KPATest::TestExifDatabase::trivialTests()
{
    QTemporaryDir tmpDir;
    QVERIFY2(tmpDir.isValid(), msgPreconditionFailed);

    DB::DummyUIDelegate uiDelegate;
    Settings::SettingsData::setup(tmpDir.path());
    Settings::SettingsData::instance()->setUiDelegate(&uiDelegate);

    // create a new, empty db:
    Exif::Database db(tmpDir.filePath(QStringLiteral("exif-db.sqlite")), uiDelegate);

    // this should be usable
    QVERIFY(db.isUsable());
    // its version should not surprise us
    QCOMPARE(db.DBFileVersion(), Exif::Database::DBVersion());
    QCOMPARE(db.DBFileVersionGuaranteed(), Exif::Database::DBVersion());

    QCOMPARE(db.size(), 0);
    QVERIFY(db.cameras().isEmpty());
    QVERIFY(db.lenses().isEmpty());

    DB::DummyProgressIndicator progress;
    db.recreate(DB::FileNameList(), progress);

    QVERIFY(db.isUsable());
    QCOMPARE(db.size(), 0);
    QCOMPARE(progress.value(), 0);
    QCOMPARE(progress.minimum(), 0);
    QCOMPARE(progress.maximum(), 0);
}

QTEST_MAIN(KPATest::TestExifDatabase)

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_TestExifDatabase.cpp"
