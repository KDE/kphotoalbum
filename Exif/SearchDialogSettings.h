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
#ifndef SEARCHDIALOGSETTINGS_H
#define SEARCHDIALOGSETTINGS_H
#include <qcheckbox.h>

namespace Exif{

template <class T>
class Setting
{
public:
    Setting() {}
    Setting( QCheckBox* cb, T value ) : cb( cb ), value( value ) {}
    QCheckBox* cb;
    T value;
};

template <class T>
class Settings :public QValueList< Setting<T> >
{
public:
    QValueList<T> selected()
    {
        QValueList<T> result;
        for( typename QValueList< Setting<T> >::Iterator it = this->begin(); it != this->end(); ++it ) {
            if ( (*it).cb->isChecked() )
                result.append( (*it).value );
        }
        return result;
    }
};

}


#endif /* SEARCHDIALOGSETTINGS_H */

