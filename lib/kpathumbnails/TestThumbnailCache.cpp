// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "TestThumbnailCache.h"

#include "ThumbnailCache.h"

#include <kpabase/SettingsData.h>
#include <kpabase/UIDelegate.h>

#include <QLoggingCategory>
#include <QRegularExpression>
#include <QSignalSpy>

namespace
{
constexpr auto msgPreconditionFailed = "Precondition for test failed - please fix unit test!";
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
    QVERIFY2(tmpDir.isValid(), msgPreconditionFailed);
    // tmpDir.setAutoRemove(false);

    DB::DummyUIDelegate uiDelegate;
    Settings::SettingsData::setup(tmpDir.path(), uiDelegate);

    const QDir thumbnailDir { tmpDir.filePath(ImageManager::defaultThumbnailDirectory()) };
    QDir().mkdir(thumbnailDir.path());
    QFile thumbnailIndex { thumbnailDir.filePath(QStringLiteral("thumbnailindex")) };
    QVERIFY2(thumbnailIndex.open(QIODevice::WriteOnly), msgPreconditionFailed);
    thumbnailIndex.write(QByteArray::fromHex(v4IndexHexData));
    thumbnailIndex.close();
    QCOMPARE(thumbnailIndex.size(), 136);

    ImageManager::ThumbnailCache thumbnailCache { thumbnailDir.path() };

    QSignalSpy cacheSavedSpy { &thumbnailCache, &ImageManager::ThumbnailCache::saveComplete };
    QVERIFY2(cacheSavedSpy.isValid(), msgPreconditionFailed);

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
    QVERIFY2(thumbnailIndex.open(QIODevice::ReadOnly), msgPreconditionFailed);
    const QByteArray v5Index { thumbnailIndex.readAll() };
    thumbnailIndex.close();
    QCOMPARE(v5Index, QByteArray::fromHex(v5IndexHexData));

    // we only have the index data - trying a lookup won't work, but shouldn't crash or something
    QTest::ignoreMessage(QtWarningMsg, "Failed to open thumbnail file");
    QTest::ignoreMessage(QtWarningMsg, "Failed to map thumbnail file");
    QTest::ignoreMessage(QtWarningMsg, "Failed to map thumbnail file");
    QVERIFY(thumbnailCache.lookup(DB::FileName::fromRelativePath(QStringLiteral("blackie.jpg"))).isNull());

    QSignalSpy flushedSpy { &thumbnailCache, &ImageManager::ThumbnailCache::cacheFlushed };
    QVERIFY2(flushedSpy.isValid(), msgPreconditionFailed);
    thumbnailCache.flush();
    QCOMPARE(flushedSpy.count(), 1);
    QCOMPARE(thumbnailCache.size(), 0);
}

void KPATest::TestThumbnailCache::insertRemove()
{
    QTemporaryDir tmpDir;
    QVERIFY2(tmpDir.isValid(), msgPreconditionFailed);
    // tmpDir.setAutoRemove(false);

    DB::DummyUIDelegate uiDelegate;
    Settings::SettingsData::setup(tmpDir.path(), uiDelegate);

    const QDir thumbnailDir { tmpDir.filePath(ImageManager::defaultThumbnailDirectory()) };
    QDir().mkdir(thumbnailDir.path());

    const QRegularExpression thumbnailIndexNotFoundRegex { QStringLiteral("Thumbnail index file \"%1\" not found!")
                                                               .arg(thumbnailDir.filePath(QStringLiteral("thumbnailindex"))) };
    QTest::ignoreMessage(QtWarningMsg, thumbnailIndexNotFoundRegex);
    ImageManager::ThumbnailCache thumbnailCache { thumbnailDir.path() };

    QSignalSpy cacheSavedSpy { &thumbnailCache, &ImageManager::ThumbnailCache::saveComplete };
    QVERIFY2(cacheSavedSpy.isValid(), msgPreconditionFailed);

    thumbnailCache.save();
    QCOMPARE(thumbnailCache.size(), 0);
    // nothing stored yet - no need to save:
    QCOMPARE(cacheSavedSpy.count(), 0);
    QCOMPARE(thumbnailCache.actualFileVersion(), -1);

    // insert some images
    // the image needs to be valid
    QImage nullImage {};
    const auto nullImageFileName = DB::FileName::fromRelativePath(QStringLiteral("nullImage.jpg"));
    QTest::ignoreMessage(QtWarningMsg, "Thumbnail for file \"nullImage.jpg\" is invalid!");
    thumbnailCache.insert(nullImageFileName, nullImage);
    QCOMPARE(thumbnailCache.size(), 0);
    QVERIFY(!thumbnailCache.contains(nullImageFileName));

    const int thumbnailSize = thumbnailCache.thumbnailSize();
    QImage someImage { thumbnailSize + 1, thumbnailSize + 1, QImage::Format_RGB32 };
    someImage.fill(Qt::red);
    QVERIFY(!someImage.isNull());
    const auto someImageFileName = DB::FileName::fromRelativePath(QStringLiteral("someImage.jpg"));
    thumbnailCache.insert(someImageFileName, someImage);
    QCOMPARE(thumbnailCache.size(), 1);
    QVERIFY(thumbnailCache.contains(someImageFileName));

    QImage otherImage { thumbnailSize, thumbnailSize, QImage::Format_RGB32 };
    otherImage.fill(Qt::green);
    QVERIFY(!otherImage.isNull());
    const auto otherImageFileName = DB::FileName::fromRelativePath(QStringLiteral("otherImage.jpg"));
    thumbnailCache.insert(otherImageFileName, otherImage);
    QCOMPARE(thumbnailCache.size(), 2);
    QVERIFY(thumbnailCache.contains(otherImageFileName));

    // TODO(jzarl) inserted images should be the same as the ones we look up

    // this should do nothing:
    thumbnailCache.removeThumbnails(DB::FileNameList());
    QCOMPARE(thumbnailCache.size(), 2);
    QCOMPARE(thumbnailCache.actualFileVersion(), thumbnailCache.preferredFileVersion());

    thumbnailCache.save();
    // the someImage has an incorrect size:
    QCOMPARE(thumbnailCache.findIncorrectlySizedThumbnails().size(), 1);

    // removals:
    QVERIFY(thumbnailCache.contains(someImageFileName));
    QVERIFY(thumbnailCache.contains(otherImageFileName));
    thumbnailCache.removeThumbnail(someImageFileName);
    QCOMPARE(thumbnailCache.size(), 1);
    QVERIFY(!thumbnailCache.contains(someImageFileName));
    QVERIFY(thumbnailCache.contains(otherImageFileName));
    thumbnailCache.save();
    QVERIFY(thumbnailCache.findIncorrectlySizedThumbnails().isEmpty());

    thumbnailCache.removeThumbnails(DB::FileNameList({ otherImageFileName }));
    QCOMPARE(thumbnailCache.size(), 0);

    // insert again and invalidate by changing thumbnail size:
    thumbnailCache.insert(someImageFileName, someImage);
    thumbnailCache.insert(otherImageFileName, otherImage);
    QCOMPARE(thumbnailCache.size(), 2);
    QSignalSpy invalidatedSpy { &thumbnailCache, &ImageManager::ThumbnailCache::cacheInvalidated };
    QVERIFY(invalidatedSpy.isValid());
    thumbnailCache.setThumbnailSize(thumbnailSize + 1);
    QCOMPARE(invalidatedSpy.count(), 1);
    QCOMPARE(thumbnailCache.size(), 0);
}

QTEST_MAIN(KPATest::TestThumbnailCache)

// vi:expandtab:tabstop=4 shiftwidth=4:
