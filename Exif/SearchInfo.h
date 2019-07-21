/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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
#ifndef EXIFSEARCHINFO_H
#define EXIFSEARCHINFO_H

#include "Database.h"

#include <DB/FileName.h>

#include <QList>
#include <QPair>
#include <QStringList>

namespace Exif
{

class SearchInfo
{
public:
    typedef Database::CameraList CameraList;
    typedef Database::Camera Camera;
    typedef Database::LensList LensList;
    typedef Database::Lens Lens;
    typedef QList<int> IntList;

    class Range
    {
    public:
        Range() {}
        explicit Range(const QString &key);
        bool isLowerMin, isLowerMax, isUpperMin, isUpperMax;
        double min, max;
        QString key;
    };

    void addSearchKey(const QString &key, const IntList &values);
    void addRangeKey(const Range &range);
    void addCamera(const CameraList &list);
    void addLens(const LensList &list);

    void search() const;
    bool matches(const DB::FileName &fileName) const;

    bool isNull() const;

protected:
    QString buildQuery() const;
    QStringList buildIntKeyQuery() const;
    QStringList buildRangeQuery() const;
    QString buildCameraSearchQuery() const;
    QString buildLensSearchQuery() const;
    QString sqlForOneRangeItem(const Range &) const;

private:
    typedef QList<QPair<QString, IntList>> IntKeyList;
    IntKeyList m_intKeys;
    QList<Range> m_rangeKeys;
    CameraList m_cameras;
    LensList m_lenses;
    mutable DB::FileNameSet m_matches;
    mutable bool m_emptyQuery;
};

}

#endif /* EXIFSEARCHINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
