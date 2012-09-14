/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#include "DB/ImageDate.h"
#include <qmap.h>
#include <q3dict.h>
#include <QList>
#include "DB/ImageInfoPtr.h"
#include "Exif/SearchInfo.h"
#include <config-kpa-exiv2.h>

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
    void setSearchRAW( bool _searchRAW );
    void setSearchMode( int index );

    void saveLock() const;
    static ImageSearchInfo loadLock();

    void debug();
    void debugMatcher() const;
    Q3Dict<void> findAlreadyMatched( const QString &group ) const;

#ifdef HAVE_EXIV2
    void addExifSearchInfo( const Exif::SearchInfo info );
#endif

protected:
    void compile() const;
    void deleteMatchers() const;

    QList<SimpleCategoryMatcher*> extractAndMatcher( CategoryMatcher* andMatcher ) const;
    QList<QList<SimpleCategoryMatcher*> > convertMatcher( CategoryMatcher* ) const;

private:
    ImageDate _date;
    QMap<QString, QString> _categoryMatchText;
    QString _label;
    QString _description;
    QRegExp _fnPattern;
    short _rating;
    short _megapixel;
    int ratingSearchMode;
    bool _searchRAW;
    bool _isNull;
    mutable bool _compiled;
    mutable QList<CategoryMatcher*> _categoryMatchers;

#ifdef HAVE_EXIV2
    Exif::SearchInfo _exifSearchInfo;
#endif
    // When adding new instance variable, please notice that this class as an explicit written copy constructor.
};

}


#endif /* IMAGESEARCHINFO_H */

