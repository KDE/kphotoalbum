/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMAGESEARCHINFO_H
#define IMAGESEARCHINFO_H
#include "DB/ImageDate.h"
#include <qmap.h>
#include <qdict.h>
#include "DB/ImageInfoPtr.h"
#include "Exif/SearchInfo.h"
#include <config.h>

namespace DB
{


class OptionAndMatcher;
class OptionSimpleMatcher;
class ImageInfo;
class CategoryMatcher;


class ImageSearchInfo {
public:
    ImageSearchInfo();
    ~ImageSearchInfo();
    ImageSearchInfo( const ImageDate& date,
                     const QString& label, const QString& description );
    ImageSearchInfo( const ImageSearchInfo& other );

    ImageDate date() const;

    QString option( const QString& name ) const;
    void setOption( const QString& name, const QString& value );

    QString label() const;
    QString description() const;

    bool isNull() const;
    bool match( ImageInfoPtr ) const;
    QValueList< QValueList<OptionSimpleMatcher*> > query() const;

    void addAnd( const QString& category, const QString& value );
    QString toString() const;

    void saveLock() const;
    static ImageSearchInfo loadLock();

    void debug();
    void debugMatcher() const;
    QDict<void> findAlreadyMatched( const QString &group ) const;

#ifdef HASEXIV2
    void addExifSearchInfo( const Exif::SearchInfo info );
#endif

protected:
    void compile() const;
    void deleteMatchers() const;

    QValueList<OptionSimpleMatcher*> extractAndMatcher( CategoryMatcher* andMatcher ) const;
    QValueList< QValueList<OptionSimpleMatcher*> > convertMatcher( CategoryMatcher* ) const;

private:
    ImageDate _date;
    QMap<QString, QString> _options;
    QString _label;
    QString _description;
    bool _isNull;
    mutable bool _compiled;
    mutable QValueList<CategoryMatcher*> _optionMatchers;

#ifdef HASEXIV2
    Exif::SearchInfo _exifSearchInfo;
#endif
    // When adding new instance variable, please notice that this class as an explicit written copy constructor.
};

}


#endif /* IMAGESEARCHINFO_H */

