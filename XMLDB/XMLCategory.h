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
#ifndef XMLCATEGORY_H
#define XMLCATEGORY_H
#include "DB/Category.h"
#include <qstringlist.h>
#include <QMap>

namespace XMLDB {
    class XMLCategory :public DB::Category
    {
        Q_OBJECT

    public:
        XMLCategory( const QString& name, const QString& icon, ViewType type, int thumbnailSize, bool show, bool positionable=false );

        QString name() const override;
        void setName( const QString& name ) override;

        void setPositionable( bool ) override;
        bool positionable() const override;

        QString iconName() const override;
        void setIconName( const QString& name ) override;

        void setViewType( ViewType type ) override;
        ViewType viewType() const override;

        void setThumbnailSize( int ) override;
        int thumbnailSize() const override;

        void setDoShow( bool b ) override;
        bool doShow() const override;

        void setType( DB::Category::CategoryType t ) override;
        CategoryType type() const override;
        bool isSpecialCategory() const override;

        void addOrReorderItems( const QStringList& items ) override;
        void setItems( const QStringList& items ) override;
        void removeItem( const QString& item ) override;
        void renameItem( const QString& oldValue, const QString& newValue ) override;
        void addItem( const QString& item ) override;
        QStringList items() const override;
        int idForName( const QString& name ) const;
        void initIdMap();
        void setIdMapping( const QString& name, int id );
        QString nameForId( int id ) const;

        bool shouldSave();
        void setShouldSave( bool b);
        void setBirthDate(const QString& item, const QDate& birthDate) override;
        QDate birthDate(const QString& item) const override;

    private:
        QString m_name;
        QString m_icon;
        bool m_show;
        ViewType m_type;
        int m_thumbnailSize;
        bool m_positionable;

        CategoryType m_categoryType;
        QStringList m_items;
        QMap<QString,int> m_idMap;
        QMap<int,QString> m_nameMap;
        QMap<QString,QDate> m_birthDates;

        bool m_shouldSave;
    };
}

#endif /* XMLCATEGORY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
