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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef EXIFTREEVIEW_H
#define EXIFTREEVIEW_H

#include <q3listview.h>
#include "Utilities/Set.h"

namespace Exif {
using Utilities::StringSet;

class TreeView : public Q3ListView {
    Q_OBJECT

public:
    TreeView( const QString& title, QWidget* parent, const char* name = 0 );
    StringSet selected();
    void setSelectedExif( const StringSet& selected );
    void reload();

protected slots:
    void toggleChildren( Q3ListViewItem* );
};

}



#endif /* EXIFTREEVIEW_H */

