/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef RANGEWIDGET_H
#define RANGEWIDGET_H

#include <qobject.h>
#include "Exif/SearchInfo.h"
class QGrid;
class QComboBox;

namespace Exif{

class RangeWidget :public QObject{
    Q_OBJECT

public:
    class Value
    {
    public:
        Value() {}
        Value( double value, const QString& text ) :value( value ), text( text ) {}
        double value;
        QString text;
    };

    typedef QValueList<Value> ValueList ;

    RangeWidget( const QString& text, const QString& searchTag, const ValueList& list, QGrid* parent);
    Exif::SearchInfo::Range range() const;

protected slots:
    void slotUpdateTo( int index );

protected:
    QString tagToLabel( const QString& tag );

private:
    QString _searchTag;
    QComboBox* _from;
    QComboBox* _to;
    ValueList _list;
};

}

#endif /* RANGEWIDGET_H */

