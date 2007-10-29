/*
  Copyright (C) 2007 Tuomas Suutari <thsuut@utu.fi>

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
            _isNull(true),
            _v0(0),
            _v1(0),
            _v2(0),
            _v3(0)
        {
        }

        explicit MD5(const QString& md5str):
            _isNull(md5str.isEmpty()),
            _v0(md5str.mid(0, 8).toULong(0, 16)),
            _v1(md5str.mid(8, 8).toULong(0, 16)),
            _v2(md5str.mid(16, 8).toULong(0, 16)),
            _v3(md5str.mid(24, 8).toULong(0, 16))
        {
        }

        bool isNull() const
        {
            return _isNull;
        }

        MD5& operator=(const QString& md5str)
        {
            if (md5str.isEmpty()) {
                _isNull = true;
            }
            else {
                _isNull = false;
                _v0 = md5str.mid(0, 8).toULong(0, 16);
                _v1 = md5str.mid(8, 8).toULong(0, 16);
                _v2 = md5str.mid(16, 8).toULong(0, 16);
                _v3 = md5str.mid(24, 8).toULong(0, 16);
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
                res += QString::number(_v0, 16).rightJustify(8, '0');
                res += QString::number(_v1, 16).rightJustify(8, '0');
                res += QString::number(_v2, 16).rightJustify(8, '0');
                res += QString::number(_v3, 16).rightJustify(8, '0');
            }
            return res;
        }

        bool operator==(const MD5& other) const
        {
            if (isNull() || other.isNull())
                return isNull() == other.isNull();

            return (_v0 == other._v0 &&
                    _v1 == other._v1 &&
                    _v2 == other._v2 &&
                    _v3 == other._v3);
        }

        bool operator!=(const MD5& other) const
        {
            return !(*this == other);
        }

        bool operator<(const MD5& other) const
        {
            if (isNull() || other.isNull())
                return isNull() && !other.isNull();

            return (_v0 < other._v0 ||
                    (_v0 == other._v0 &&
                     (_v1 < other._v1 ||
                      (_v1 == other._v1 &&
                       (_v2 < other._v2 ||
                        (_v2 == other._v2 &&
                         _v3 < other._v3))))));
        }

    private:
        bool _isNull;
        ulong _v0;
        ulong _v1;
        ulong _v2;
        ulong _v3;
    };
}

#endif /* DB_MD5_H */
