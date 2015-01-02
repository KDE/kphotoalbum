/*
  Copyright (C) 2007-2010 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#ifndef DB_MD5_H
#define DB_MD5_H

#include <qglobal.h>

namespace DB
{
    class MD5
    {
    public:
        MD5():
            m_isNull(true),
            m_v0(0),
            m_v1(0),
            m_v2(0),
            m_v3(0)
        {
        }

        explicit MD5(const QString& md5str):
            m_isNull(md5str.isEmpty()),
            m_v0(md5str.mid(0, 8).toULong(0, 16)),
            m_v1(md5str.mid(8, 8).toULong(0, 16)),
            m_v2(md5str.mid(16, 8).toULong(0, 16)),
            m_v3(md5str.mid(24, 8).toULong(0, 16))
        {
        }

        bool isNull() const
        {
            return m_isNull;
        }

        MD5& operator=(const QString& md5str)
        {
            if (md5str.isEmpty()) {
                m_isNull = true;
            }
            else {
                m_isNull = false;
                m_v0 = md5str.mid(0, 8).toULong(0, 16);
                m_v1 = md5str.mid(8, 8).toULong(0, 16);
                m_v2 = md5str.mid(16, 8).toULong(0, 16);
                m_v3 = md5str.mid(24, 8).toULong(0, 16);
            }
            return *this;
        }

        /** Get hex string representation of this.
         * If this->isNull(), returns null string.
         */
        QString toHexString() const
        {
            QString res;
            if (!isNull()) {
                res += QString::number(m_v0, 16).rightJustified(8, QChar::fromLatin1('0'));
                res += QString::number(m_v1, 16).rightJustified(8, QChar::fromLatin1('0'));
                res += QString::number(m_v2, 16).rightJustified(8, QChar::fromLatin1('0'));
                res += QString::number(m_v3, 16).rightJustified(8, QChar::fromLatin1('0'));
            }
            return res;
        }

        bool operator==(const MD5& other) const
        {
            if (isNull() || other.isNull())
                return isNull() == other.isNull();

            return (m_v0 == other.m_v0 &&
                    m_v1 == other.m_v1 &&
                    m_v2 == other.m_v2 &&
                    m_v3 == other.m_v3);
        }

        bool operator!=(const MD5& other) const
        {
            return !(*this == other);
        }

        bool operator<(const MD5& other) const
        {
            if (isNull() || other.isNull())
                return isNull() && !other.isNull();

            return (m_v0 < other.m_v0 ||
                    (m_v0 == other.m_v0 &&
                     (m_v1 < other.m_v1 ||
                      (m_v1 == other.m_v1 &&
                       (m_v2 < other.m_v2 ||
                        (m_v2 == other.m_v2 &&
                         m_v3 < other.m_v3))))));
        }

    private:
        bool m_isNull;
        ulong m_v0;
        ulong m_v1;
        ulong m_v2;
        ulong m_v3;
    };
}

#endif /* DB_MD5_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
