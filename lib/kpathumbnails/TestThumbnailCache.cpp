// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "TestThumbnailCache.h"

#include "ThumbnailCache.h"

#include <kpabase/SettingsData.h>
#include <kpabase/UIDelegate.h>

#include <QLoggingCategory>

namespace
{
constexpr auto v4IndexHexData {
    "00000004" // version
    "00000003" // current file
    "00033714" // offset in file
    "00000003" // number of thumbnails
    "0000001600730070006900660066005F0032002E006A0070006700000000000038A900001DFD" // "spiff_2.jpg"
    "0000001C006E00650077005F0077006100760065005F0032002E006A0070006700000000000000000000229D" // "new_wave_2.jpg"
    "000000160062006C00610063006B00690065002E006A00700067000000000000462C00001F0B" // "blackie.jpg"
};
constexpr auto v5IndexHexData {
    "00000005" // version
    "00000100" // v5: thumbnailsize
    "00000003" // current file
    "00033714" // offset in file
    "00000003" // number of thumbnails
    "0000001600730070006900660066005F0032002E006A0070006700000000000038A900001DFD" // "spiff_2.jpg"
    "0000001C006E00650077005F0077006100760065005F0032002E006A0070006700000000000000000000229D" // "new_wave_2.jpg"
    "000000160062006C00610063006B00690065002E006A00700067000000000000462C00001F0B" // "blackie.jpg"
};
}

void KPATest::TestThumbnailCache::initTestCase()
{
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
    QFile thumnailIndex { thumbnailDir.filePath(QStringLiteral("thumbnailindex")) };
    QVERIFY(thumnailIndex.open(QIODevice::WriteOnly));
    thumnailIndex.write(QByteArray::fromHex(v4IndexHexData));
    thumnailIndex.close();
    QCOMPARE(thumnailIndex.size(), 136);

    ImageManager::ThumbnailCache thumbnailCache { thumbnailDir.path() + QDir::separator() };
    QCOMPARE(thumbnailCache.size(), 3);
    QVERIFY(thumbnailCache.contains(DB::FileName::fromRelativePath(QStringLiteral("spiff_2.jpg"))));
    // remove an empty file list to force the dirty flag:
    thumbnailCache.removeThumbnails(DB::FileNameList());
    thumbnailCache.save();

    QVERIFY(thumnailIndex.open(QIODevice::ReadOnly));

    const QByteArray v5Index { thumnailIndex.readAll() };
    thumnailIndex.close();
    QCOMPARE(v5Index, QByteArray::fromHex(v5IndexHexData));
}

QTEST_MAIN(KPATest::TestThumbnailCache)

// vi:expandtab:tabstop=4 shiftwidth=4:
