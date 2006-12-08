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
#ifndef CATEGORY_H
#define CATEGORY_H

#include <qstring.h>
#include <qpixmap.h>
#include <qobject.h>
#include <ksharedptr.h>
#include <Utilities/Set.h>

namespace DB
{
class CategoryItem;


/**
   This class stores information about categories (People/Places/Keywords)
*/
class Category :public QObject, public KShared
{
    Q_OBJECT

public:
    enum ViewType { ListView, ThumbedListView, IconView, ThumbedIconView };

    virtual QString name() const = 0;
    virtual void setName( const QString& name ) = 0;

    virtual QString text() const;
    static QMap<QString,QString> standardCategories();

    virtual QString iconName() const = 0;
    virtual void setIconName( const QString& name ) = 0;
    virtual QPixmap icon( int size = 22 ) const;

    virtual void setViewType( ViewType type ) = 0;
    virtual ViewType viewType() const = 0;

    virtual void setThumbnailSize( int ) = 0;
    virtual int thumbnailSize() const = 0;

    virtual void setDoShow( bool b ) = 0;
    virtual bool doShow() const = 0;

    virtual void setSpecialCategory( bool b ) = 0;
    virtual bool isSpecialCategory() const = 0;

    virtual void addOrReorderItems( const QStringList& items ) = 0;
    virtual void setItems( const QStringList& items ) = 0;
    virtual void removeItem( const QString& item ) = 0;
    virtual void renameItem( const QString& oldValue, const QString& newValue ) = 0;
    virtual void addItem( const QString& item ) = 0;
    virtual QStringList items() const = 0;
    virtual QStringList itemsInclCategories() const;
    KSharedPtr<CategoryItem> itemsCategories() const;

signals:
    void changed();
    void itemRenamed( const QString& oldName, const QString& newName );
    void itemRemoved( const QString& name );
};

typedef KSharedPtr<Category> CategoryPtr;

}

#endif /* CATEGORY_H */

