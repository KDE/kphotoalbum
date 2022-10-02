// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

// Qt includes
#include <QMimeData>

// Local includes
#include "TreeCategoryModel.h"

#include <DB/Category.h>
#include <DB/CategoryItem.h>
#include <DB/ImageDB.h>
#include <MainWindow/DirtyIndicator.h>

struct Browser::TreeCategoryModel::Data {
    Data(const QString &name)
        : name(name)
        , parent(nullptr)
    {
    }

    ~Data()
    {
        qDeleteAll(children);
    }

    void addChild(Data *child)
    {
        child->parent = this;
        children.append(child);
    }

    QString name;
    QList<Data *> children;
    Data *parent;
};

Browser::TreeCategoryModel::TreeCategoryModel(const DB::CategoryPtr &category,
                                              const DB::ImageSearchInfo &info)
    : AbstractCategoryModel(category, info)
{
    m_data = new Data(QString());
    createData(m_category->itemsCategories().data(), 0);

    if (hasNoneEntry()) {
        Data *data = new Data(DB::ImageDB::NONE());
        data->parent = m_data;
        m_data->children.prepend(data);
    }

    m_memberMap = DB::ImageDB::instance()->memberMap();
}

int Browser::TreeCategoryModel::rowCount(const QModelIndex &index) const
{
    return indexToData(index)->children.count();
}

int Browser::TreeCategoryModel::columnCount(const QModelIndex &) const
{
    return 5;
}

QModelIndex Browser::TreeCategoryModel::index(int row, int column, const QModelIndex &parent) const
{
    const Data *data = indexToData(parent);
    QList<Data *> children = data->children;
    int size = children.count();
    if (row >= size || row < 0 || column >= columnCount(parent) || column < 0) {
        // Invalid index
        return QModelIndex();
    } else {
        return createIndex(row, column, children[row]);
    }
}

QModelIndex Browser::TreeCategoryModel::parent(const QModelIndex &index) const
{
    Data *me = indexToData(index);
    if (me == m_data) {
        return QModelIndex();
    }

    Data *parent = me->parent;
    if (parent == m_data) {
        return QModelIndex();
    }

    Data *grandParent = parent->parent;

    return createIndex(grandParent->children.indexOf(parent), 0, parent);
}

Browser::TreeCategoryModel::~TreeCategoryModel()
{
    delete m_data;
}

bool Browser::TreeCategoryModel::createData(DB::CategoryItem *parentCategoryItem, Data *parent)
{
    const QString name = parentCategoryItem->mp_name;
    const int imageCount = m_images.contains(name) ? m_images[name].count : 0;
    const int videoCount = m_videos.contains(name) ? m_videos[name].count : 0;

    Data *myData = new Data(name);
    bool anyItems = imageCount != 0 || videoCount != 0;

    for (QList<DB::CategoryItem *>::ConstIterator subCategoryIt = parentCategoryItem->mp_subcategories.constBegin();
         subCategoryIt != parentCategoryItem->mp_subcategories.constEnd(); ++subCategoryIt) {
        anyItems = createData(*subCategoryIt, myData) || anyItems;
    }

    if (parent) {
        if (anyItems) {
            parent->addChild(myData);
        } else {
            delete myData;
        }
    } else {
        m_data = myData;
    }

    return anyItems;
}

Browser::TreeCategoryModel::Data *Browser::TreeCategoryModel::indexToData(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_data;
    } else {
        return static_cast<Browser::TreeCategoryModel::Data *>(index.internalPointer());
    }
}

QString Browser::TreeCategoryModel::indexToName(const QModelIndex &index) const
{
    const Browser::TreeCategoryModel::Data *data = indexToData(index);
    return data->name;
}

Qt::DropActions Browser::TreeCategoryModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags Browser::TreeCategoryModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (m_category->isSpecialCategory() || indexToName(index) == QString::fromUtf8("**NONE**")) {
        return defaultFlags;
    }

    if (indexToData(index)->parent != nullptr) {
        return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    } else if (index.column() == -1) {
        return defaultFlags | Qt::ItemIsDropEnabled;
    } else {
        return defaultFlags | Qt::ItemIsDragEnabled;
    }
}

QStringList Browser::TreeCategoryModel::mimeTypes() const
{
    return QStringList() << QString::fromUtf8("x-kphotoalbum/x-browser-tag-drag");
}

QMimeData *Browser::TreeCategoryModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    // only use the first index, even if more than one are selected:
    stream << indexToName(indexes[0]);
    if (!indexes[0].parent().isValid()) {
        stream << QString();
    } else {
        stream << indexToName(indexes[0].parent());
    }

    mimeData->setData(QString::fromUtf8("x-kphotoalbum/x-browser-tag-drag"), encodedData);
    return mimeData;
}

Browser::TreeCategoryModel::tagData Browser::TreeCategoryModel::getDroppedTagData(QByteArray &encodedData)
{
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    Browser::TreeCategoryModel::tagData droppedTagData;
    stream >> droppedTagData.tagName;
    stream >> droppedTagData.tagGroup;
    return droppedTagData;
}

bool Browser::TreeCategoryModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                              int, int, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction) {
        return true;
    }

    const QString thisCategory = indexToName(parent);
    QByteArray encodedData = data->data(QString::fromUtf8("x-kphotoalbum/x-browser-tag-drag"));
    Browser::TreeCategoryModel::tagData droppedTagData = getDroppedTagData(encodedData);

    // the difference between a CopyAction and a MoveAction is that with the MoveAction,
    // we have to remove the tag from its current group first
    if (action == Qt::MoveAction) {
        // Remove the tag from its group and remove the group if it's empty now
        m_memberMap.removeMemberFromGroup(m_category->name(), droppedTagData.tagGroup, droppedTagData.tagName);
        if (m_memberMap.members(m_category->name(), droppedTagData.tagGroup, true) == QStringList()) {
            m_memberMap.deleteGroup(m_category->name(), droppedTagData.tagGroup);
        }
    }
    if (parent.isValid()) {
        // Check if the tag is dropped onto a copy of itself
        const DB::CategoryItemPtr categoryInfo = m_category->itemsCategories();
        if (thisCategory == droppedTagData.tagName
            || categoryInfo->isDescendentOf(thisCategory, droppedTagData.tagName)) {
            return true;
        }

        // Add the tag to a group, create it if we don't have it yet
        if (!m_memberMap.groups(m_category->name()).contains(thisCategory)) {
            m_memberMap.addGroup(m_category->name(), thisCategory);
            DB::ImageDB::instance()->memberMap() = m_memberMap;
        }
        m_memberMap.addMemberToGroup(m_category->name(), thisCategory, droppedTagData.tagName);
    }

    DB::ImageDB::instance()->memberMap() = m_memberMap;
    MainWindow::DirtyIndicator::markDirty();
    Q_EMIT dataChanged();

    return true;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_TreeCategoryModel.cpp"
