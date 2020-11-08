/* SPDX-FileCopyrightText: 2003-2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGS_CATEGORYITEM_H
#define SETTINGS_CATEGORYITEM_H

// Qt includes
#include <QListWidgetItem>

// Local includes
#include <DB/Category.h>

namespace DB
{

// Local classes
class MemberMap;

}

namespace Settings
{

class CategoryItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    CategoryItem(
        const QString &category,
        const QString &icon,
        DB::Category::ViewType type,
        int thumbnailSize,
        QListWidget *parent,
        bool positionable = false);
    void setLabel(const QString &label);
    void setPositionable(bool positionable);
    void submit(DB::MemberMap *memberMap);
    void removeFromDatabase();
    bool positionable() const;
    int thumbnailSize() const;
    DB::Category::ViewType viewType() const;
    void setThumbnailSize(int size);
    void setViewType(DB::Category::ViewType type);
    QString icon() const;
    void setIcon(const QString &icon);
    QString originalName() const;
    void markAsNewCategory();

protected:
    void renameCategory(DB::MemberMap *memberMap);

private: // Variables
    QString m_categoryOrig;
    QString m_iconOrig;
    bool m_positionable;
    bool m_positionableOrig;
    QString m_category;
    QString m_text;
    QString m_icon;
    DB::Category::ViewType m_type;
    DB::Category::ViewType m_typeOrig;
    int m_thumbnailSize;
    int m_thumbnailSizeOrig;
};

}

#endif // SETTINGS_CATEGORYITEM_H

// vi:expandtab:tabstop=4 shiftwidth=4:
