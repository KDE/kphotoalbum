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
#ifndef XMLCATEGORY_H
#define XMLCATEGORY_H
#include "DB/Category.h"
#include <qstringlist.h>
#include <qmap.h>

namespace XMLDB {
    class XMLCategory :public DB::Category
    {
        Q_OBJECT

    public:
        XMLCategory( const QString& name, const QString& icon, ViewType type, int thumbnailSize, bool show );

        virtual QString name() const;
        virtual void setName( const QString& name );

        virtual QString iconName() const;
        virtual void setIconName( const QString& name );

        virtual void setViewType( ViewType type );
        virtual ViewType viewType() const;

        virtual void setThumbnailSize( int );
        virtual int thumbnailSize() const;

        virtual void setDoShow( bool b );
        virtual bool doShow() const;

        virtual void setSpecialCategory( bool b );
        virtual bool isSpecialCategory() const;

        virtual void addOrReorderItems( const QStringList& items );
        virtual void setItems( const QStringList& items );
        virtual void removeItem( const QString& item );
        virtual void renameItem( const QString& oldValue, const QString& newValue );
        virtual void addItem( const QString& item );
        virtual QStringList items() const;
        int idForName( const QString& name ) const;
        void initIdMap();
        void setIdMapping( const QString& name, int id );
        QString nameForId( int id ) const;

        bool shouldSave();
        void setShouldSave( bool b);

    private:
        QString _name;
        QString _icon;
        bool _show;
        ViewType _type;
        int _thumbnailSize;

        bool _isSpecial;
        QStringList _items;
        QMap<QString,int> _idMap;
        QMap<int,QString> _nameMap;

        bool _shouldSave;
    };
}

#endif /* XMLCATEGORY_H */

