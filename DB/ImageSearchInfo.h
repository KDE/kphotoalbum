/* Copyright (C) 2003-2015 Jesper K. Pedersen <blackie@kde.org>

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
#include <config-kpa-kgeomap.h>

#include <QMap>
#include <QList>

#include <DB/ImageDate.h>
#include <DB/ImageInfoPtr.h>
#include <Exif/SearchInfo.h>
#ifdef HAVE_KGEOMAP
#include <KGeoMap/GeoCoordinates>
#endif
#include <Utilities/StringSet.h>
namespace DB
{


class SimpleCategoryMatcher;
class ImageInfo;
class CategoryMatcher;


class ImageSearchInfo {
public:
    ImageSearchInfo();
    ~ImageSearchInfo();
    ImageSearchInfo( const ImageDate& date,
                     const QString& label, const QString& description );
    ImageSearchInfo( const ImageDate& date,
                     const QString& label, const QString& description,
             const QString& fnPattern );
    ImageSearchInfo( const ImageSearchInfo& other );

    ImageDate date() const;

    QString categoryMatchText( const QString& name ) const;
    void setCategoryMatchText( const QString& name, const QString& value );
    void renameCategory( const QString& oldName, const QString& newName );

    QString label() const;
    QRegExp fnPattern() const;
    QString description() const;

    bool isNull() const;
    bool match( ImageInfoPtr ) const;
    QList<QList<SimpleCategoryMatcher*> > query() const;

    void addAnd( const QString& category, const QString& value );
    void setRating( short rating);
    QString toString() const;

    void setMegaPixel( short megapixel );
    void setMaxMegaPixel( short maxmegapixel );
    void setSearchRAW( bool m_searchRAW );
    void setSearchMode( int index );

    void saveLock() const;
    static ImageSearchInfo loadLock();

    void debug();
    void debugMatcher() const;
    Utilities::StringSet findAlreadyMatched( const QString &group ) const;

    void addExifSearchInfo( const Exif::SearchInfo info );

#ifdef HAVE_KGEOMAP
    KGeoMap::GeoCoordinates::Pair regionSelection() const;
    void setRegionSelection(const KGeoMap::GeoCoordinates::Pair& actRegionSelection);
#endif

protected:
    void compile() const;
    void deleteMatchers() const;

    QList<SimpleCategoryMatcher*> extractAndMatcher( CategoryMatcher* andMatcher ) const;
    QList<QList<SimpleCategoryMatcher*> > convertMatcher( CategoryMatcher* ) const;

private:
    ImageDate m_date;
    QMap<QString, QString> m_categoryMatchText;
    QString m_label;
    QString m_description;
    QRegExp m_fnPattern;
    short m_rating;
    short m_megapixel;
    short m_max_megapixel;
    int m_ratingSearchMode;
    bool m_searchRAW;
    bool m_isNull;
    mutable bool m_compiled;
    mutable QList<CategoryMatcher*> m_categoryMatchers;

    Exif::SearchInfo m_exifSearchInfo;

#ifdef HAVE_KGEOMAP
    KGeoMap::GeoCoordinates::Pair m_regionSelection;
    mutable bool m_usingRegionSelection = false;
    mutable float m_regionSelectionMinLat;
    mutable float m_regionSelectionMaxLat;
    mutable float m_regionSelectionMinLon;
    mutable float m_regionSelectionMaxLon;
#endif
    // When adding new instance variable, please notice that this class as an explicit written copy constructor.
};

}


#endif /* IMAGESEARCHINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
