/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef IMAGESEARCHINFO_H
#define IMAGESEARCHINFO_H
#include "imagesearchinfo.h"
#include "imagedate.h"
#include "imageinfo.h"

class ImageSearchInfo {
public:
    ImageSearchInfo();
    ImageSearchInfo( const ImageDate& startDate, const ImageDate& endDate,
                     const QString& label, const QString& description );

    void setStartDate( const ImageDate& );
    void setEndDate( const ImageDate& );
    ImageDate startDate() const;
    ImageDate endDate() const;

    QString option( const QString& name ) const;
    void setOption( const QString& name, const QString& value );

    QString label() const;
    QString description() const;

    bool isNull();
    bool match( ImageInfo* );

    void addAnd( const QString& group, const QString& value );
    QString toString() const;

protected:
    bool stringMatch( const QString& key, ImageInfo* info );

private:
    ImageDate _startDate;
    ImageDate _endDate;
    QMap<QString, QString> _options;
    QString _label;
    QString _description;
    bool _isNull;
};


#endif /* IMAGESEARCHINFO_H */

