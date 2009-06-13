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

#ifndef SQLFOLDERCATEGORY_H
#define SQLFOLDERCATEGORY_H

#include "SQLSpecialCategory.h"
#include "QueryHelper.h"

namespace SQLDB {
    class SQLFolderCategory: public SQLSpecialCategory
    {
        Q_OBJECT

    public:
        virtual QString name() const { return QString::fromLatin1("Folder"); }

        virtual QString iconName() const
        { return QString::fromLatin1("folder"); }
        virtual void setIconName(const QString&) {}

        virtual ViewType viewType() const { return TreeView; }
        virtual void setViewType(ViewType) {}

        virtual bool doShow() const { return false; }
        virtual void setDoShow(bool) {}

        virtual int thumbnailSize() const { return 32; }
        virtual void setThumbnailSize(int) {}

        virtual QStringList items() const;
        virtual void setItems(const QStringList& items);
        virtual void addOrReorderItems(const QStringList& items);
        virtual void addItem(const QString& item);
        virtual void removeItem(const QString& item);
        virtual void renameItem(const QString& oldValue, const QString& newValue);
        virtual QMap<QString, uint> classify(const DB::ImageSearchInfo& scope,
                                             DB::MediaType typemask) const;

    protected:
        friend class SQLCategoryCollection;
        SQLFolderCategory(QueryHelper* queryHelper);
        QueryHelper* _qh;
    };
}

#endif /* SQLFOLDERCATEGORY_H */
