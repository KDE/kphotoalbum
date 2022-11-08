// SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef XMLCATEGORY_H
#define XMLCATEGORY_H
#include <DB/Category.h>

#include <QMap>
#include <qstringlist.h>

namespace XMLDB
{
class XMLCategory : public DB::Category
{
    Q_OBJECT

public:
    XMLCategory(const QString &name, const QString &icon, ViewType type, int thumbnailSize, bool show, bool positionable = false);

    QString name() const override;
    void setName(const QString &name) override;

    void setPositionable(bool) override;
    bool positionable() const override;

    QString iconName() const override;
    void setIconName(const QString &name) override;

    void setViewType(ViewType type) override;
    ViewType viewType() const override;

    void setThumbnailSize(int) override;
    int thumbnailSize() const override;

    void setDoShow(bool b) override;
    bool doShow() const override;

    void setType(DB::Category::CategoryType t) override;
    CategoryType type() const override;
    bool isSpecialCategory() const override;

    void addOrReorderItems(const QStringList &items) override;
    void setItems(const QStringList &items) override;
    void removeItem(const QString &item) override;
    void renameItem(const QString &oldValue, const QString &newValue) override;
    void addItem(const QString &item) override;
    QStringList items() const override;
    DB::TagInfo *itemForName(const QString &tag) override;
    int idForName(const QString &name) const;
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
    void setBirthDate(const QString &item, const QDate &birthDate) override;
    QDate birthDate(const QString &item) const override;

private:
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
};
}

#endif /* XMLCATEGORY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
