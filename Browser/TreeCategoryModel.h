/* SPDX-FileCopyrightText: 2003-2015 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TREECATEGORYMODEL_H
#define TREECATEGORYMODEL_H

// Local includes
#include "AbstractCategoryModel.h"

#include <DB/MemberMap.h>

// Qt classes
class QMimeData;

namespace DB
{

class CategoryItem;

}

namespace Browser
{

/**
 * \brief A QAbstractItemModel subclass that represent the items of a given category as a tree
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * This class implements the QAbstractItemModel interface, which is
 * actually what most of the methods is about. The constructor queries
 * the category information from the back end, and builds an internal
 * data structure representing the tree. It does build its own data structure for two reasons:
 * \li The \ref DB::CategoryItem's do not have an easy way to go from child
 * to parent, something that was needed by the \ref parent method. It was
 * considered too risky to add that to the \ref DB::CategoryItem
 * data structure at the time this was implemented.
 * \li By building its own data structure it can ensure that the data is
 * not changing behind the scene, something that might have happened if
 * this class was constructed, categories was added or removed, and the
 * class was asked information abouts its data.
 *
 * The drag and drop support is in some ways similar to the CategoryListView classes.
 * Any bugs there probably apply here as well and vice versa.
 */

class TreeCategoryModel : public AbstractCategoryModel
{
    Q_OBJECT

public:
    TreeCategoryModel(const DB::CategoryPtr &category, const DB::ImageSearchInfo &info);
    ~TreeCategoryModel() override;

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    QString indexToName(const QModelIndex &) const override;

    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;

    struct tagData {
        QString tagName;
        QString tagGroup;
    };

signals:
    void dataChanged();

private: // Functions
    struct Data;
    bool createData(DB::CategoryItem *parentCategoryItem, Data *parent);
    Data *indexToData(const QModelIndex &index) const;
    TreeCategoryModel::tagData getDroppedTagData(QByteArray &encodedData);

private: // Variables
    Data *m_data;
    DB::MemberMap m_memberMap;
};

}

#endif // TREECATEGORYMODEL_H

// vi:expandtab:tabstop=4 shiftwidth=4:
