/* Copyright (C) 2018 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
   Copyright (C) 2007-2010 Tuomas Suutari <thsuut@utu.fi>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "MD5.h"

DB::MD5::MD5():
    m_isNull(true),
    m_v0(0),
    m_v1(0)
{
}

DB::MD5::MD5(const QString &md5str):
    m_isNull(md5str.isEmpty()),
    m_v0(md5str.left(16).toULongLong(0, 16)),
    m_v1(md5str.mid(16, 16).toULongLong(0, 16))
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
    }
    else {
        m_isNull = false;
        m_v0 = md5str.left(16).toULongLong(0, 16);
        m_v1 = md5str.mid(16, 16).toULongLong(0, 16);
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

    return (m_v0 == other.m_v0 &&
            m_v1 == other.m_v1);
}

bool DB::MD5::operator!=(const DB::MD5 &other) const
{
    return !(*this == other);
}

bool DB::MD5::operator<(const DB::MD5 &other) const
{
    if (isNull() || other.isNull())
        return isNull() && !other.isNull();

    return (m_v0 < other.m_v0 ||
            (m_v0 == other.m_v0 &&
             (m_v1 < other.m_v1)));
}
