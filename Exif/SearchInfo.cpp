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

#include "SearchInfo.h"
#include <KLocalizedString>

#include "Exif/Database.h"
#include <DB/FileName.h>

/**
 * \class Exif::SearchInfo
 * This class represents a search for Exif information. It is similar in functionality for category searches which is in the
 * class \ref DB::ImageSearchInfo.
 *
 * The search is build, from \ref Exif::SearchDialog, using the functions addRangeKey(), addSearchKey(), and addCamara().
 * The search is stored in an instance of \ref DB::ImageSearchInfo, and may later be executed using search().
 * Once a search has been executed, the application may ask if a given image is in the search result using matches()
 */
void Exif::SearchInfo::addSearchKey(const QString &key, const IntList &values)
{
    m_intKeys.append(qMakePair(key, values));
}

QStringList Exif::SearchInfo::buildIntKeyQuery() const
{
    QStringList andArgs;
    for (IntKeyList::ConstIterator intIt = m_intKeys.begin(); intIt != m_intKeys.end(); ++intIt) {
        QStringList orArgs;
        QString key = (*intIt).first;
        IntList values = (*intIt).second;

        Q_FOREACH (int value, values) {
            orArgs << QString::fromLatin1("(%1 == %2)").arg(key).arg(value);
        }
        if (orArgs.count() != 0)
            andArgs << QString::fromLatin1("(%1)").arg(orArgs.join(QString::fromLatin1(" or ")));
    }

    return andArgs;
}

void Exif::SearchInfo::addRangeKey(const Range &range)
{
    m_rangeKeys.append(range);
}

Exif::SearchInfo::Range::Range(const QString &key)
    : isLowerMin(false)
    , isLowerMax(false)
    , isUpperMin(false)
    , isUpperMax(false)
    , key(key)
{
}

QString Exif::SearchInfo::buildQuery() const
{
    QStringList subQueries;
    subQueries += buildIntKeyQuery();
    subQueries += buildRangeQuery();
    QString cameraQuery = buildCameraSearchQuery();
    if (!cameraQuery.isEmpty())
        subQueries.append(cameraQuery);
    QString lensQuery = buildLensSearchQuery();
    if (!lensQuery.isEmpty())
        subQueries.append(lensQuery);

    if (subQueries.empty())
        return QString();
    else
        return QString::fromLatin1("SELECT filename from exif WHERE %1")
            .arg(subQueries.join(QString::fromLatin1(" and ")));
}

QStringList Exif::SearchInfo::buildRangeQuery() const
{
    QStringList result;
    for (QList<Range>::ConstIterator it = m_rangeKeys.begin(); it != m_rangeKeys.end(); ++it) {
        QString str = sqlForOneRangeItem(*it);
        if (!str.isEmpty())
            result.append(str);
    }
    return result;
}

QString Exif::SearchInfo::sqlForOneRangeItem(const Range &range) const
{
    // Notice I multiplied factors on each value to ensure that we do not fail due to rounding errors for say 1/3

    if (range.isLowerMin) {
        //  Min to Min  means < x
        if (range.isUpperMin)
            return QString::fromLatin1("%1 < %2 and %3 > 0").arg(range.key).arg(range.min * 1.01).arg(range.key);

        //  Min to Max means all images
        if (range.isUpperMax)
            return QString();

        //  Min to y   means <= y
        return QString::fromLatin1("%1 <= %2 and %3 > 0").arg(range.key).arg(range.max * 1.01).arg(range.key);
    }

    //  MAX to MAX   means >= y
    if (range.isLowerMax)
        return QString::fromLatin1("%1 > %2").arg(range.key).arg(range.max * 0.99);

    //  x to Max   means >= x
    if (range.isUpperMax)
        return QString::fromLatin1("%1 >= %2").arg(range.key).arg(range.min * 0.99);

    //  x to y     means >=x and <=y
    return QString::fromLatin1("(%1 <= %2 and %3 <= %4)")
        .arg(range.min * 0.99)
        .arg(range.key)
        .arg(range.key)
        .arg(range.max * 1.01);
}

void Exif::SearchInfo::search() const
{
    QString queryStr = buildQuery();
    m_emptyQuery = queryStr.isEmpty();

    // ensure to do SQL queries as little as possible.
    static QString lastQuery;
    if (queryStr == lastQuery)
        return;
    lastQuery = queryStr;

    m_matches.clear();
    if (m_emptyQuery)
        return;
    m_matches = Exif::Database::instance()->filesMatchingQuery(queryStr);
}

bool Exif::SearchInfo::matches(const DB::FileName &fileName) const
{
    if (m_emptyQuery)
        return true;

    return m_matches.contains(fileName);
}

bool Exif::SearchInfo::isNull() const
{
    return buildQuery().isEmpty();
}

void Exif::SearchInfo::addCamera(const CameraList &list)
{
    m_cameras = list;
}

void Exif::SearchInfo::addLens(const LensList &list)
{
    m_lenses = list;
}

QString Exif::SearchInfo::buildCameraSearchQuery() const
{
    QStringList subResults;
    for (CameraList::ConstIterator cameraIt = m_cameras.begin(); cameraIt != m_cameras.end(); ++cameraIt) {
        subResults.append(QString::fromUtf8("(Exif_Image_Make='%1' and Exif_Image_Model='%2')")
                              .arg((*cameraIt).first)
                              .arg((*cameraIt).second));
    }
    if (subResults.count() != 0)
        return QString::fromUtf8("(%1)").arg(subResults.join(QString::fromLatin1(" or ")));
    else
        return QString();
}

QString Exif::SearchInfo::buildLensSearchQuery() const
{
    QStringList subResults;
    for (LensList::ConstIterator lensIt = m_lenses.begin(); lensIt != m_lenses.end(); ++lensIt) {
        if (*lensIt == i18nc("As in No persons, no locations etc.", "None"))
            // compare to null (=entry from old db schema) and empty string (=entry w/o exif lens info)
            subResults.append(QString::fromUtf8("(nullif(Exif_Photo_LensModel,'') is null)"));
        else
            subResults.append(QString::fromUtf8("(Exif_Photo_LensModel='%1')")
                                  .arg(*lensIt));
    }
    if (subResults.count() != 0)
        return QString::fromUtf8("(%1)").arg(subResults.join(QString::fromLatin1(" or ")));
    else
        return QString();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
