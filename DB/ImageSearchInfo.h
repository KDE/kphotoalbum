/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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

#ifndef IMAGESEARCHINFO_H
#define IMAGESEARCHINFO_H
#include "ImageDate.h"
#include "ImageInfoPtr.h"
#include <config-kpa-marble.h>

#include <Exif/SearchInfo.h>
#ifdef HAVE_MARBLE
#include <Map/GeoCoordinates.h>
#endif
#include <Utilities/StringSet.h>

#include <QList>
#include <QMap>
namespace DB
{

class SimpleCategoryMatcher;
class ImageInfo;
class CategoryMatcher;

class ImageSearchInfo
{
public:
    ImageSearchInfo();
    ImageSearchInfo(const ImageDate &date,
                    const QString &label, const QString &description);
    ImageSearchInfo(const ImageDate &date,
                    const QString &label, const QString &description,
                    const QString &fnPattern);

    ImageDate date() const;

    QString categoryMatchText(const QString &name) const;
    void setCategoryMatchText(const QString &name, const QString &value);
    void renameCategory(const QString &oldName, const QString &newName);

    QString label() const;
    QRegExp fnPattern() const;
    QString description() const;

    /**
     * @brief checkIfNull evaluates whether the filter is indeed empty and
     * sets isNull() to \c true if that is the case.
     * You only need to call this if you re-use an existing ImageSearchInfo
     * and set/reset search parameters.
     * @see ThumbnailView::toggleRatingFilter
     */
    void checkIfNull();
    bool isNull() const;
    bool match(ImageInfoPtr) const;
    QList<QList<SimpleCategoryMatcher *>> query() const;

    void addAnd(const QString &category, const QString &value);
    short rating() const;
    void setRating(short rating);
    /**
     * @brief toString generates a description of the ImageSearchInfo.
     * The idea is not to give a complete description, but rather something
     * useful for the breadcrumbs at the bottom of the main window.
     * @return a textual description of the ImageSearchInfo
     */
    QString toString() const;

    void setMegaPixel(short megapixel);
    void setMaxMegaPixel(short maxmegapixel);
    void setSearchRAW(bool m_searchRAW);
    void setSearchMode(int index);

    QVariantMap getLockData() const;
    static ImageSearchInfo loadLock(const QMap<QString, QVariant> &keyValuePairs);

    void debug();
    void debugMatcher() const;
    Utilities::StringSet findAlreadyMatched(const QString &group) const;

    void addExifSearchInfo(const Exif::SearchInfo info);

    // By default, an ImageSearchInfo is cacheable, but only one search
    // is cached per image.  For a search that's only going to be
    // performed once, don't try to cache the result.
    void setCacheable(bool cacheable);
    bool isCacheable() const;

#ifdef HAVE_MARBLE
    Map::GeoCoordinates::LatLonBox regionSelection() const;
    void setRegionSelection(const Map::GeoCoordinates::LatLonBox &actRegionSelection);
#endif

protected:
    void compile() const;

    QList<SimpleCategoryMatcher *> extractAndMatcher(CategoryMatcher *andMatcher) const;
    QList<QList<SimpleCategoryMatcher *>> convertMatcher(CategoryMatcher *) const;

private:
    /**
     * @brief The CompiledDataPrivate struct encapsulates the non-copyable data members of the ImageSearchInfo.
     * Its copy constructor and copy operator invalidate the object,
     * This allows the ImageSearchInfo to just use the default copy/move constructors/operators.
     */
    struct CompiledDataPrivate {
        CompiledDataPrivate() = default;
        CompiledDataPrivate(const CompiledDataPrivate &other);
        CompiledDataPrivate(CompiledDataPrivate &&other) = default;
        ~CompiledDataPrivate();
        CompiledDataPrivate &operator=(const CompiledDataPrivate &other);
        CompiledDataPrivate &operator=(CompiledDataPrivate &&other) = default;

        bool valid = false;
        QList<CategoryMatcher *> categoryMatchers;
    };
    ImageDate m_date;
    QMap<QString, QString> m_categoryMatchText;
    QString m_label;
    QString m_description;
    QRegExp m_fnPattern;
    short m_rating = -1;
    short m_megapixel = 0;
    short m_max_megapixel = 0;
    int m_ratingSearchMode = 0;
    bool m_searchRAW = false;
    bool m_isNull = true;
    bool m_isCacheable = true;
    mutable CompiledDataPrivate m_compiled;

    Exif::SearchInfo m_exifSearchInfo;

    int m_matchGeneration;
    bool doMatch(ImageInfoPtr) const;

#ifdef HAVE_MARBLE
    Map::GeoCoordinates::LatLonBox m_regionSelection;
#endif
    // When adding new instance variable, please notice that this class has an explicit written copy constructor.
};
}

#endif /* IMAGESEARCHINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
