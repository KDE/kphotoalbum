// SPDX-FileCopyrightText: 2007-2010 Tuomas Suutari <thsuut@utu.fi>
// SPDX-FileCopyrightText: 2018 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2014-2020 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "MD5.h"

#include <kpabase/FileName.h>

#include <QCryptographicHash>
#include <QFile>
#include <QHash>
#include <QIODevice>
#include <QMutex>

static QMutex s_MD5CacheMutex;
static QHash<DB::FileName, DB::MD5> s_MD5Cache;

DB::MD5::MD5()
    : m_isNull(true)
    , m_v0(0)
    , m_v1(0)
{
}

DB::MD5::MD5(const QString &md5str)
    : m_isNull(md5str.isEmpty())
    , m_v0(md5str.left(16).toULongLong(nullptr, 16))
    , m_v1(md5str.mid(16, 16).toULongLong(nullptr, 16))
{
}

bool DB::MD5::isNull() const
{
    return m_isNull;
}

DB::MD5 &DB::MD5::operator=(const QString &md5str)
{
    if (md5str.isEmpty()) {
        m_isNull = true;
    } else {
        m_isNull = false;
        m_v0 = md5str.left(16).toULongLong(nullptr, 16);
        m_v1 = md5str.mid(16, 16).toULongLong(nullptr, 16);
    }
    return *this;
}

QString DB::MD5::toHexString() const
{
    QString res;
    static QChar ZERO(QChar::fromLatin1('0'));
    if (!isNull()) {
        res += QString::number(m_v0, 16).rightJustified(16, ZERO);
        res += QString::number(m_v1, 16).rightJustified(16, ZERO);
    }
    return res;
}

bool DB::MD5::operator==(const DB::MD5 &other) const
{
    if (isNull() || other.isNull())
        return isNull() == other.isNull();

    return (m_v0 == other.m_v0 && m_v1 == other.m_v1);
}

bool DB::MD5::operator!=(const DB::MD5 &other) const
{
    return !(*this == other);
}

bool DB::MD5::operator<(const DB::MD5 &other) const
{
    if (isNull() || other.isNull())
        return isNull() && !other.isNull();

    return (m_v0 < other.m_v0 || (m_v0 == other.m_v0 && (m_v1 < other.m_v1)));
}

void DB::MD5::resetMD5Cache()
{
    QMutexLocker locker(&s_MD5CacheMutex);
    s_MD5Cache.clear();
}

namespace
{
// Determined experimentally to yield best results (on Seagate 2TB 2.5" disk,
// 5400 RPM).  Performance is very similar at 524288.  Above that, performance
// was significantly worse.  Below that, performance also deteriorated.
// This assumes use of one image scout thread (see DB/ImageScout.cpp).  Without
// a scout thread, performance was about 10-15% worse.
constexpr int MD5_BUFFER_SIZE = 262144;
}

DB::MD5 DB::MD5Sum(const DB::FileName &fileName)
{
    QMutexLocker locker(&s_MD5CacheMutex);
    if (s_MD5Cache.contains(fileName)) {
        return s_MD5Cache[fileName];
    }
    locker.unlock();
    // It's possible that the checksum will be added between now
    // and when we add it, but as long as the file contents don't change
    // during that interval, the checksums will match.
    // Holding the lock while the checksum is being computed will
    // defeat the whole purpose.
    DB::MD5 checksum;
    QFile file(fileName.absolute());
    if (file.open(QIODevice::ReadOnly)) {
        QCryptographicHash md5calculator(QCryptographicHash::Md5);
        while (!file.atEnd()) {
            QByteArray md5Buffer(file.read(MD5_BUFFER_SIZE));
            md5calculator.addData(md5Buffer);
        }
        file.close();
        checksum = DB::MD5(QString::fromLatin1(md5calculator.result().toHex()));
    }
    locker.relock();
    s_MD5Cache[fileName] = checksum;
    return checksum;
}

void DB::PreloadMD5Sum(const DB::FileName &fileName)
{
    (void)MD5Sum(fileName);
}
