// SPDX-FileCopyrightText: 2004-2014 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2006 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2014-2022 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CATEGORY_H
#define CATEGORY_H

#include "ImageDate.h"

#include <KIconLoader>
#include <QDate>
#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QString>

class QImage;
class QPixmap;
namespace DB
{
class CategoryItem;
class TagInfo;

struct CountWithRange {
    uint count = 0;
    ImageDate range {};
    void add(const ImageDate &date)
    {
        count++;
        range.extendTo(date);
    }
};

/**
   This class stores information about categories (People/Places/Events)
*/
class Category : public QObject, public QSharedData
{
    Q_OBJECT

public:
    enum ViewType { TreeView,
                    ThumbedTreeView,
                    IconView,
                    ThumbedIconView };
    enum CategoryType { PlainCategory,
                        FolderCategory,
                        MediaTypeCategory,
                        TokensCategory };

    Category(const QString &name, const QString &icon, ViewType type, int thumbnailSize, bool show, bool positionable = false);

    QString name() const;
    void setName(const QString &name);

    bool positionable() const;
    void setPositionable(bool positionable);

    QString iconName() const;
    void setIconName(const QString &name);
    QPixmap icon(int size = 22, KIconLoader::States state = KIconLoader::DefaultState) const;

    ViewType viewType() const;
    void setViewType(ViewType type);

    int thumbnailSize() const;
    void setThumbnailSize(int size);

    bool doShow() const;
    void setDoShow(bool b);

    CategoryType type() const;
    void setType(CategoryType t);

    bool isSpecialCategory() const;

    QStringList items() const;
    QStringList itemsInclCategories() const;
    QExplicitlySharedDataPointer<CategoryItem> itemsCategories() const;
    void addOrReorderItems(const QStringList &items);
    void setItems(const QStringList &items);
    void removeItem(const QString &item);
    void renameItem(const QString &oldValue, const QString &newValue);
    void addItem(const QString &item);
    /**
     * @brief itemForName returns a TagInfo for a given tag.
     * In contrast to the usual tag representation as a QString, a TagInfo maintains a connection to the Category and is notified of tag renaming and deletion.
     *
     * @param item the name of the tag
     * @return a TagInfo object for the given tag name, or a \c nullptr if the tag does not exist.
     */
    DB::TagInfo *itemForName(const QString &tag);

    QPixmap categoryImage(const QString &category, const QString &, int width, int height) const;
    void setCategoryImage(const QString &category, const QString &, const QImage &image);
    QString fileForCategoryImage(const QString &category, QString member) const;

    QDate birthDate(const QString &item) const;
    void setBirthDate(const QString &item, const QDate &birthDate);

    int idForName(const QString &name) const;
    /**
     * @brief initIdMap is needed when writing categories into the XML file.
     * FIXME: make private and add CategoryCollection as friend class
     */
    void initIdMap();
    /**
     * @brief setIdMapping sets the mapping from id to name and vice versa.
     * The id must be a positive value.
     * @param name
     * @param id > 0
     */
    void setIdMapping(const QString &name, int id);
    /**
     * @brief addZeroMapping allows adding of category names with id 0.
     * This id is not allowed normally, but can happen in corrupted index.xml files.
     * @param name
     */
    void addZeroMapping(const QString &name);
    QString nameForId(int id) const;
    /**
     * @brief namesForIdZero returns all names for id 0.
     * Obviously, this is not how ids usually work.
     * The only time when this makes sense is when reading a damaged index.xml file that is to be repaired.
     * After loading the database is complete, the mapping between id and name must always 1:1!
     * @return
     */
    QStringList namesForIdZero() const;
    /**
     * @brief clearNullIds clears the IdMapping for tags with id=0.
     * Category names with id 0 can only happen when loading a corrupted database file.
     */
    void clearNullIds();

    bool shouldSave();
    void setShouldSave(bool b);

private:
    QString defaultIconName() const;

    QString m_name;
    QString m_icon;
    bool m_show;
    ViewType m_type;
    int m_thumbnailSize;
    bool m_positionable;

    CategoryType m_categoryType;
    QStringList m_items;
    QMap<QString, int> m_idMap;
    QMap<int, QString> m_nameMap;
    QMap<QString, QDate> m_birthDates;
    QStringList m_namesWithIdZero;

    bool m_shouldSave;

Q_SIGNALS:
    void changed();
    void itemRenamed(const QString &oldName, const QString &newName);
    void itemRemoved(const QString &name);
};

}

#endif /* CATEGORY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
