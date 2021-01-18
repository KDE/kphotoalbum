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

    QCOMPARE(thumbnailCache.size(), 3);
    QVERIFY(thumbnailCache.contains(DB::FileName::fromRelativePath(QStringLiteral("spiff_2.jpg"))));
    // remove an empty file list to force the dirty flag:
    thumbnailCache.removeThumbnails(DB::FileNameList());
    thumbnailCache.save();

    QCOMPARE(cacheSavedSpy.count(), 1);

    QVERIFY(thumbnailIndex.open(QIODevice::ReadOnly));

    const QByteArray v5Index { thumbnailIndex.readAll() };
    thumbnailIndex.close();
    QCOMPARE(v5Index, QByteArray::fromHex(v5IndexHexData));
}

QTEST_MAIN(KPATest::TestThumbnailCache)

// vi:expandtab:tabstop=4 shiftwidth=4:
