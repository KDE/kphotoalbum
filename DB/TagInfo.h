// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
//  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef DB_TAGINFO_H
#define DB_TAGINFO_H

#include "Category.h"
#include "CategoryPtr.h"

#include <QObject>

namespace DB
{

/**
 * @brief The TagInfo class is a convenience class to bundle category and tag into a single object.
 * Instead of a pair of QStrings, this interface keeps a connection to the category object to update the tag name if a tag is renamed.
 */
class TagInfo : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief TagInfo
     * @param category a valid category
     * @param tag a valid tag within the given category
     */
    TagInfo(const CategoryPtr &category, const QString &tag, QObject *parent = nullptr);

    /**
     * @brief category
     * @return A pointer to the DB::Category object for the tag.
     */
    CategoryPtr category() const;
    /**
     * @brief categoryName is a pure convenience function.
     * @return The category name.
     */
    QString categoryName() const;
    /**
     * @brief tagName returns the name of the tag.
     * If you store the TagInfo object for a longer time, the tag name may change to a different value (if a tag is renamed) and even be empty (if a tag is removed).
     * Make sure that you check for an empty tag name if you don't use the TagInfo immediately.
     * @return The tag string.
     */
    QString tagName() const;

    bool isValid() const;

private:
    CategoryPtr m_category;
    QString m_tag;

private Q_SLOTS:
    void updateTagName(const QString &oldName, const QString &newName);
    void removeTagName(const QString &name);
};

} // namespace DB

#endif // DB_TAGINFO_H
