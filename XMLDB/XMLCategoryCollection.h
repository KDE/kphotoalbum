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
#ifndef XMLCATEGORYCOLLECTION_H
#define XMLCATEGORYCOLLECTION_H

#include "DB/CategoryCollection.h"

namespace XMLDB {

    class XMLCategoryCollection :public DB::CategoryCollection
    {
        Q_OBJECT

    public:
        virtual DB::CategoryPtr categoryForName( const QString& name ) const;
        void addCategory( DB::Category* );
        virtual QStringList categoryNames() const;
        virtual void removeCategory( const QString& name );
        virtual void rename( const QString& oldName, const QString& newName );
        virtual QValueList<DB::CategoryPtr> categories() const;
        virtual void addCategory( const QString& text, const QString& icon, DB::Category::ViewType type,
                                  int thumbnailSize, bool show );

        void initIdMap();

    private:
        QValueList<DB::CategoryPtr> _categories;
    };
}

#endif /* XMLCATEGORYCOLLECTION_H */

