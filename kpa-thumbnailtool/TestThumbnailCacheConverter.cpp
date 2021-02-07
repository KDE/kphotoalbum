// SPDX-FileCopyrightText: 2020 The KPhotoAlbum development team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "TestThumbnailCacheConverter.h"

#include "ThumbnailCacheConverter.h"

#include <QBuffer>
#include <QTextStream>

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
    "000001c4" // v5: thumbnailsize
    "00000003" // current file
    "00033714" // offset in file
    "00000003" // number of thumbnails
    "0000001600730070006900660066005F0032002E006A0070006700000000000038A900001DFD" // "spiff_2.jpg"
    "0000001C006E00650077005F0077006100760065005F0032002E006A0070006700000000000000000000229D" // "new_wave_2.jpg"
    "000000160062006C00610063006B00690065002E006A00700067000000000000462C00001F0B" // "blackie.jpg"
};
}

void KPATest::TestThumbnailCacheConverter::convertV5toV4()
{
    QByteArray v5Index { QByteArray::fromHex(v5IndexHexData) };
    QBuffer v5input { &v5Index };
    v5input.open(QIODevice::ReadOnly);

    QByteArray v4Index {};
    QBuffer v4output { &v4Index };
    v4output.open(QIODevice::WriteOnly);

    QString errorOutput;
    QTextStream err { &errorOutput };

    KPAThumbnailTool::convertV5ToV4Cache(v5input, v4output, err);
    const QByteArray expectedV4Index { QByteArray::fromHex(v4IndexHexData) };
    QCOMPARE(v4Index, expectedV4Index);
    QVERIFY2(errorOutput.isEmpty(), "convertV5ToV4Cache() reported error!");
}

QTEST_MAIN(KPATest::TestThumbnailCacheConverter)

// vi:expandtab:tabstop=4 shiftwidth=4:
