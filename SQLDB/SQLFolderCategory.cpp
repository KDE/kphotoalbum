/*
  Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#include "SQLFolderCategory.h"

using namespace SQLDB;

SQLFolderCategory::SQLFolderCategory():
    _name("Folder"),
    _iconName("folder"),
    _isSpecial(true)
{
    readItems();
}

void SQLFolderCategory::readItems()
{
    // TODO: this
    _items = QStringList() << "Folder1" << "Folder2";
}

QString SQLFolderCategory::name() const
{
    return _name;
}

void SQLFolderCategory::setName(const QString& name)
{
    _name = name;
}

QString SQLFolderCategory::iconName() const
{
    return _iconName;
}

void SQLFolderCategory::setIconName(const QString& name)
{
    _iconName = name;
}

SQLFolderCategory::ViewSize SQLFolderCategory::viewSize() const
{
    return _viewSize;
}

void SQLFolderCategory::setViewSize(ViewSize size)
{
    _viewSize = size;
}

SQLFolderCategory::ViewType SQLFolderCategory::viewType() const
{
    return _viewType;
}

void SQLFolderCategory::setViewType(ViewType type)
{
    _viewType = type;
}

bool SQLFolderCategory::doShow() const
{
    return _doShow;
}

void SQLFolderCategory::setDoShow(bool b)
{
    _doShow = b;
}

bool SQLFolderCategory::isSpecialCategory() const
{
    return _isSpecial;
}

void SQLFolderCategory::setSpecialCategory(bool b)
{
    _isSpecial = b;
}

QStringList SQLFolderCategory::items() const
{
    return _items;
}

void SQLFolderCategory::setItems(const QStringList& items)
{
    Q_UNUSED(items);
    readItems();
    //_items = items;
}

void SQLFolderCategory::addItem(const QString& item)
{
    Q_UNUSED(item);
    readItems();
    //if (_items.contains(item))
    //    _items.remove(item);
    //_items.prepend(item);
}

void SQLFolderCategory::removeItem(const QString& item)
{
    Q_UNUSED(item);
    readItems();
    //_items.remove(item);
}

void SQLFolderCategory::renameItem(const QString& oldValue, const QString& newValue)
{
    Q_UNUSED(oldValue);
    Q_UNUSED(newValue);
    readItems();
    //_items.remove(oldValue);
    //addItem(newValue);
}
