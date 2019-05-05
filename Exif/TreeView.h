/* Copyright (C) 2003-2019 Jesper K. Pedersen <blackie@kde.org>

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

#include "Utilities/StringSet.h"
#include <QTreeWidget>

namespace Exif {
using Utilities::StringSet;

class TreeView : public QTreeWidget {
    Q_OBJECT

public:
    TreeView( const QString& title, QWidget* parent );
    StringSet selected();
    void setSelectedExif( const StringSet& selected );
    void reload();

protected slots:
    void toggleChildren( QTreeWidgetItem* );
};

}



#endif /* EXIFTREEVIEW_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
