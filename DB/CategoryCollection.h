// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CATEGORYCOLLECTION_H
#define CATEGORYCOLLECTION_H

#include "Category.h"
#include "CategoryPtr.h"

#include <QList>

namespace DB
{

/**
   \class CategoryCollection
   This class is the collection of categories. It is the basic anchor point to categories.
*/

class CategoryCollection : public QObject
{
    Q_OBJECT

public:
    enum class IncludeSpecialCategories {
        Yes,
        No
    };
    virtual CategoryPtr categoryForName(const QString &name) const = 0;
    virtual QStringList categoryNames(IncludeSpecialCategories include = IncludeSpecialCategories::Yes) const = 0;
    virtual void removeCategory(const QString &name) = 0;
    virtual void rename(const QString &oldName, const QString &newName) = 0;
    virtual QList<CategoryPtr> categories() const = 0;
    virtual void addCategory(const QString &text, const QString &icon, Category::ViewType type,
                             int thumbnailSize, bool show, bool positionable = false)
        = 0;
    virtual CategoryPtr categoryForSpecial(const Category::CategoryType type) const = 0;

Q_SIGNALS:
    void categoryCollectionChanged();
    void categoryRemoved(const QString &categoryName);
    void itemRenamed(DB::Category *category, const QString &oldName, const QString &newName);
    void itemRemoved(DB::Category *category, const QString &name);

protected Q_SLOTS:
    void slotItemRenamed(const QString &oldName, const QString &newName);
    void slotItemRemoved(const QString &item);
};

}

#endif /* CATEGORYCOLLECTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
