/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef XMLCATEGORYCOLLECTION_H
#define XMLCATEGORYCOLLECTION_H

#include <DB/CategoryCollection.h>

#include <QList>
#include <QMap>

namespace XMLDB
{

class XMLCategoryCollection : public DB::CategoryCollection
{
    Q_OBJECT

public:
    DB::CategoryPtr categoryForName(const QString &name) const override;
    void addCategory(DB::CategoryPtr);
    QStringList categoryNames(IncludeSpecialCategories include = IncludeSpecialCategories::Yes) const override;
    void removeCategory(const QString &name) override;
    void rename(const QString &oldName, const QString &newName) override;
    QList<DB::CategoryPtr> categories() const override;
    void addCategory(const QString &text, const QString &icon, DB::Category::ViewType type,
                     int thumbnailSize, bool show, bool positionable = false) override;
    DB::CategoryPtr categoryForSpecial(const DB::Category::CategoryType type) const override;

    void initIdMap();

private:
    QList<DB::CategoryPtr> m_categories;
    QMap<DB::Category::CategoryType, DB::CategoryPtr> m_specialCategories;
};
}

#endif /* XMLCATEGORYCOLLECTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
