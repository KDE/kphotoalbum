// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "TestThumbnailCache.h"

#include "ThumbnailCache.h"

#include <kpabase/SettingsData.h>
#include <kpabase/UIDelegate.h>

#include <QLoggingCategory>
#include <QSignalSpy>

namespace
{
constexpr auto v4IndexHexData {
    "00000004" // version
    "00000003" // current file
    "00033714" // offset in file
    "00000003" // number of thumbnails
    "000000160062006C00610063006B00690065002E006A00700067000000000000462C00001F0B" // "blackie.jpg"
    "0000001600730070006900660066005F0032002E006A0070006700000000000038A900001DFD" // "spiff_2.jpg"
    "0000001C006E00650077005F0077006100760065005F0032002E006A0070006700000000000000000000229D" // "new_wave_2.jpg"
};
constexpr auto v5IndexHexData {
    "00000005" // version
    "00000100" // v5: thumbnailsize
    "00000003" // current file
    "00033714" // offset in file
    "00000003" // number of thumbnails
    "000000160062006C00610063006B00690065002E006A00700067000000000000462C00001F0B" // "blackie.jpg"
    "0000001600730070006900660066005F0032002E006A0070006700000000000038A900001DFD" // "spiff_2.jpg"
    "0000001C006E00650077005F0077006100760065005F0032002E006A0070006700000000000000000000229D" // "new_wave_2.jpg"
};
}

void KPATest::TestThumbnailCache::initTestCase()
{
    // ThumbnailCache uses QHash, which is randomized by default
    qSetGlobalQHashSeed(0);
}

void KPATest::TestThumbnailCache::loadV4ThumbnailIndex()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    tmpDir.setAutoRemove(false);

    DB::DummyUIDelegate uiDelegate;
    Settings::SettingsData::setup(tmpDir.path(), uiDelegate);

    const QDir thumbnailDir { tmpDir.filePath(ImageManager::defaultThumbnailDirectory()) };
    QDir().mkdir(thumbnailDir.path());
    QFile thumbnailIndex { thumbnailDir.filePath(QStringLiteral("thumbnailindex")) };
    QVERIFY(thumbnailIndex.open(QIODevice::WriteOnly));
    thumbnailIndex.write(QByteArray::fromHex(v4IndexHexData));
    thumbnailIndex.close();
    QCOMPARE(thumbnailIndex.size(), 136);

    ImageManager::ThumbnailCache thumbnailCache { thumbnailDir.path() };

    QSignalSpy cacheSavedSpy { &thumbnailCache, &ImageManager::ThumbnailCache::saveComplete };
    QVERIFY(cacheSavedSpy.isValid());

    // change this when the version changes:
    QCOMPARE(thumbnailCache.preferredFileVersion(), 5);
    // verify input as defined in v4IndexHexData:
    QCOMPARE(thumbnailCache.actualFileVersion(), 4);
    QCOMPARE(thumbnailCache.size(), 3);
    QVERIFY(thumbnailCache.contains(DB::FileName::fromRelativePath(QStringLiteral("blackie.jpg"))));
    QVERIFY(thumbnailCache.contains(DB::FileName::fromRelativePath(QStringLiteral("spiff_2.jpg"))));
    QVERIFY(thumbnailCache.contains(DB::FileName::fromRelativePath(QStringLiteral("new_wave_2.jpg"))));
    QVERIFY(!thumbnailCache.contains(DB::FileName::fromRelativePath(QString())));

    // remove an empty file list to force the dirty flag:
    thumbnailCache.removeThumbnails(DB::FileNameList());
    // actually, removeThumbnails causes a save, but in case we change we want to call save explicitly:
    thumbnailCache.save();
    // save is actually called more than once, but should only be executed once:
    QCOMPARE(cacheSavedSpy.count(), 1);

    // after saving, the actual file version should have been updated
    QCOMPARE(thumbnailCache.actualFileVersion(), thumbnailCache.preferredFileVersion());
    QCOMPARE(thumbnailCache.thumbnailSize(), Settings::SettingsData::instance()->thumbnailSize());

    // verify data on disk:
    QVERIFY(thumbnailIndex.open(QIODevice::ReadOnly));
    const QByteArray v5Index { thumbnailIndex.readAll() };
    thumbnailIndex.close();
    QCOMPARE(v5Index, QByteArray::fromHex(v5IndexHexData));

    // we only have the index data - trying a lookup won't work, but shouldn't crash or something
    QVERIFY(thumbnailCache.lookup(DB::FileName::fromRelativePath(QStringLiteral("blackie.jpg"))).isNull());
}

QTEST_MAIN(KPATest::TestThumbnailCache)

// vi:expandtab:tabstop=4 shiftwidth=4:
