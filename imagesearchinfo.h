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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef IMAGESEARCHINFO_H
#define IMAGESEARCHINFO_H
#include "imagesearchinfo.h"
#include "imagedate.h"
#include <qdom.h>
#include <qmap.h>
class ImageInfo;
class OptionMatcher;

class ImageSearchInfo {
public:
    ImageSearchInfo();
    ~ImageSearchInfo();
    ImageSearchInfo( const ImageDate& startDate, const ImageDate& endDate,
                     const QString& label, const QString& description );
    ImageSearchInfo( const ImageSearchInfo& other );

    void setStartDate( const ImageDate& );
    void setEndDate( const ImageDate& );
    ImageDate startDate() const;
    ImageDate endDate() const;

    QString option( const QString& name ) const;
    void setOption( const QString& name, const QString& value );

    QString label() const;
    QString description() const;

    bool isNull();
    bool match( ImageInfo* ) const;

    void addAnd( const QString& group, const QString& value );
    QString toString() const;

    QDomElement toXML( QDomDocument );
    void load( QDomElement );

    void debug();

protected:
    void compile() const;

private:
    ImageDate _startDate;
    ImageDate _endDate;
    QMap<QString, QString> _options;
    QString _label;
    QString _description;
    bool _isNull;
    mutable bool _compiled;
    mutable OptionMatcher* _optionMatcher;
};


#endif /* IMAGESEARCHINFO_H */

