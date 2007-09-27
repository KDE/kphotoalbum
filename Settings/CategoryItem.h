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
#ifndef SETTINGS_CATEGORYITEM_H
#define SETTINGS_CATEGORYITEM_H

#include <qlistbox.h>
#include <DB/Category.h>
namespace DB { class MemberMap; }

namespace Settings
{
class CategoryItem :public QObject, public QListBoxText
{
    Q_OBJECT
public:
    CategoryItem( const QString& category, const QString& text, const QString& icon,
                  DB::Category::ViewType type, int thumbnailSize, QListBox* parent );
    void setLabel( const QString& label );
    void submit( DB::MemberMap* memberMap );
    void removeFromDatabase();

    QString text() const;
    QString icon() const;
    int thumbnailSize() const;
    DB::Category::ViewType viewType() const;

    void setIcon( const QString& icon );
    void setThumbnailSize( int size );
    void setViewType( DB::Category::ViewType type );

signals:
    void categoryRenamed( const QString& oldName, const QString& newName );
    void categoryAdded( const QString& newName );
    void categoryRemoved( const QString& newName );

protected:
    void renameCategory(DB::MemberMap* memberMap);

private:
    QString _categoryOrig, _textOrig, _iconOrig;
    QString _category, _text, _icon;
    DB::Category::ViewType _type, _typeOrig;
    int _thumbnailSize, _thumbnailSizeOrig;
};

}


#endif /* SETTINGS_CATEGORYITEM_H */

