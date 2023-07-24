// SPDX-FileCopyrightText: 2004-2010 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2006 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2015-2022 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CATEGORYCOLLECTION_H
#define CATEGORYCOLLECTION_H

#include "Category.h"
#include "CategoryPtr.h"
#include "GlobalCategorySortOrder.h"
#include <QList>
#include <memory>

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
    ~CategoryCollection();

    enum class IncludeSpecialCategories {
        Yes,
        No
    };
    CategoryPtr categoryForName(const QString &name) const;
    QStringList categoryNames(IncludeSpecialCategories include = IncludeSpecialCategories::Yes) const;
    QList<CategoryPtr> categories() const;
    CategoryPtr categoryForSpecial(const Category::CategoryType type) const;

    void addCategory(DB::CategoryPtr);
    void addCategory(const QString &text, const QString &icon, Category::ViewType type,
                     int thumbnailSize, bool show, bool positionable = false);
    void removeCategory(const QString &name);
    void rename(const QString &oldName, const QString &newName);
    GlobalCategorySortOrder *globalSortOrder();

    // FIXME(jzarl): this should be private and FileWriter should be a friend class
    void initIdMap();

Q_SIGNALS:
    void categoryCollectionChanged();
    void categoryRemoved(const QString &categoryName);
    void itemRenamed(DB::Category *category, const QString &oldName, const QString &newName);
    void itemRemoved(DB::Category *category, const QString &name);

protected Q_SLOTS:
    void slotItemRenamed(const QString &oldName, const QString &newName);
    void slotItemRemoved(const QString &item);

private:
    friend class ImageDB;
    CategoryCollection();

    QList<DB::CategoryPtr> m_categories;
    QMap<DB::Category::CategoryType, DB::CategoryPtr> m_specialCategories;
    std::unique_ptr<GlobalCategorySortOrder> m_globalSortOrder;
};

}

#endif /* CATEGORYCOLLECTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
