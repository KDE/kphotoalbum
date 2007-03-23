/*
  Copyright (C) 2005-2006 Jesper K. Pedersen <blackie@kde.org>
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

#ifndef SQLTAGCATEGORY_H
#define SQLTAGCATEGORY_H

#include "SQLCategory.h"
#include "Connection.h"
#include "QueryHelper.h"

namespace SQLDB {
    class SQLTagCategory: public SQLCategory
    {
        Q_OBJECT

    public:
        virtual QString name() const;

        virtual QString iconName() const;
        virtual void setIconName(const QString& name);

        virtual ViewType viewType() const;
        virtual void setViewType(ViewType type);

        virtual bool doShow() const;
        virtual void setDoShow(bool b);

        virtual QStringList items() const;
        virtual void setItems(const QStringList& items);
        virtual void addOrReorderItems(const QStringList& items);
        virtual void addItem(const QString& item);
        virtual void removeItem(const QString& item);
        virtual void renameItem(const QString& oldValue,
                                const QString& newValue);

        virtual int thumbnailSize() const;
        virtual void setThumbnailSize(int size);

        virtual QMap<QString, uint> classify(const DB::ImageSearchInfo& scope,
                                             DB::MediaType typemask) const;

    protected:
        friend class SQLCategoryCollection;
        SQLTagCategory(QueryHelper* queryHelper, int categoryId);
        QueryHelper* _qh;
        int _categoryId;

    private:
        mutable QString _name;
    };
}

#endif /* SQLTAGCATEGORY_H */
