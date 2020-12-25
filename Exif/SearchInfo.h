/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
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
        Range() { }
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
