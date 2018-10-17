/*
  Copyright (C) 2018 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
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
#include <QString>

namespace DB
{
class FileName;

class MD5
{
public:
    MD5();

    explicit MD5(const QString& md5str);

    bool isNull() const;

    MD5& operator=(const QString& md5str);

    /** Get hex string representation of this.
     * If this->isNull(), returns null string.
     */
    QString toHexString() const;

    bool operator==(const MD5 &other) const;

    bool operator!=(const MD5& other) const;

    bool operator<(const MD5& other) const;

    inline uint hash() const { return (uint) m_v0 ^ m_v1; }

private:
    bool m_isNull;
    qulonglong m_v0;
    qulonglong m_v1;
};

inline uint qHash(const MD5 &key)
{
    return key.hash();
}

DB::MD5 MD5Sum( const DB::FileName& fileName );

}

#endif /* DB_MD5_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
