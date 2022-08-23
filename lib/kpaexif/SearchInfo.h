// SPDX-FileCopyrightText: 2005-2007, 2010, 2012 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2013 Dominik Broj <broj.dominik@gmail.com>
// SPDX-FileCopyrightText: 2013, 2015, 2019-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIFSEARCHINFO_H
#define EXIFSEARCHINFO_H

#include "Database.h"

#include <kpabase/FileName.h>

#include <QList>
#include <QPair>
#include <QStringList>

namespace Exif
{

class SearchInfo
{
public:
    /**
     * @brief SearchInfo creates an invalid SearchInfo.
     * Methods such as addSearchKey() or others can be called on an invalid SearchInfo,
     * but the searchInfo will not become valid by this and isNull() will always return \c true.
     */
    SearchInfo();
    SearchInfo(const Database *db);

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
        bool isLowerMin = false;
        bool isLowerMax = false;
        bool isUpperMin = false;
        bool isUpperMax = false;
        double min = 0;
        double max = 0;
        QString key;
    };

    void addSearchKey(const QString &key, const IntList &values);
    void addRangeKey(const Range &range);
    void addCamera(const CameraList &list);
    void addLens(const LensList &list);

    void search() const;
    bool matches(const DB::FileName &fileName) const;

    /**
     * @brief isNull
     * @return \c false if the SearchInfo is bound to an Exif::Database object, \c true otherwise.
     */
    bool isNull() const;
    /**
     * @brief isEmpty
     * @return \c true if the SearchInfo is null or if the query is empty, \c false otherwise.
     */
    bool isEmpty() const;

protected:
    QString buildQuery() const;
    QStringList buildIntKeyQuery() const;
    QStringList buildRangeQuery() const;
    QString buildCameraSearchQuery() const;
    QString buildLensSearchQuery() const;
    QString sqlForOneRangeItem(const Range &) const;

private:
    const Database *m_exifDB;
    typedef QList<QPair<QString, IntList>> IntKeyList;
    IntKeyList m_intKeys;
    QList<Range> m_rangeKeys;
    CameraList m_cameras;
    LensList m_lenses;
    mutable DB::FileNameSet m_matches;
    mutable bool m_emptyQuery = false;
};

}

#endif /* EXIFSEARCHINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
