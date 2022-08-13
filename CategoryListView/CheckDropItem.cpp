/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes

#include "CheckDropItem.h"
#include "DragItemInfo.h"
#include "DragableTreeWidget.h"

#include "DB/Category.h"
#include "DB/CategoryItem.h"
#include "DB/ImageDB.h"
#include "DB/MemberMap.h"

// KDE includes
#include <KLocalizedString>
#include <KMessageBox>

// Qt includes
#include <QDropEvent>

CategoryListView::CheckDropItem::CheckDropItem(DragableTreeWidget *parent, const QString &column1,
                                               const QString &column2)
    : QTreeWidgetItem(parent)
    , m_listView(parent)
{
    setCheckState(0, Qt::Unchecked);
    setText(0, column1);
    setText(1, column2);
}

CategoryListView::CheckDropItem::CheckDropItem(DragableTreeWidget *listView, QTreeWidgetItem *parent, const QString &column1,
                                               const QString &column2)
    : QTreeWidgetItem(parent)
    , m_listView(listView)
{
    setCheckState(0, Qt::Unchecked);
    setText(0, column1);
    setText(1, column2);
}

CategoryListView::DragItemInfoSet CategoryListView::CheckDropItem::extractData(const QMimeData *data) const
{
    DragItemInfoSet items;
    QByteArray array = data->data(QString::fromUtf8("x-kphotoalbum/x-categorydrag"));
    QDataStream stream(array);
    stream >> items;

    return items;
}

bool CategoryListView::CheckDropItem::dataDropped(const QMimeData *data)
{
    DragItemInfoSet items = extractData(data);
    const QString newParent = text(0);
    if (!verifyDropWasIntended(newParent, items))
        return false;

    DB::MemberMap &memberMap = DB::ImageDB::instance()->memberMap();
    memberMap.addGroup(m_listView->category()->name(), newParent);

    for (DragItemInfoSet::const_iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt) {
        const QString oldParent = (*itemIt).parent();
        const QString child = (*itemIt).child();

        memberMap.addMemberToGroup(m_listView->category()->name(), newParent, child);
        memberMap.removeMemberFromGroup(m_listView->category()->name(), oldParent, child);
    }

    // DB::ImageDB::instance()->setMemberMap( memberMap );

    m_listView->emitItemsChanged();
    return true;
}

bool CategoryListView::CheckDropItem::isSelfDrop(const QMimeData *data) const
{
    const QString thisCategory = text(0);
    const DragItemInfoSet children = extractData(data);
    const DB::CategoryItemPtr categoryInfo = m_listView->category()->itemsCategories();

    for (DragItemInfoSet::const_iterator childIt = children.begin(); childIt != children.end(); ++childIt) {
        if (thisCategory == (*childIt).child() || categoryInfo->isDescendentOf(thisCategory, (*childIt).child()))
            return true;
    }
    return false;
}

void CategoryListView::CheckDropItem::setTristate(bool b)
{
    if (b)
        setFlags(flags() | Qt::ItemIsAutoTristate);
    else
        setFlags(flags() & ~Qt::ItemIsAutoTristate);
}

bool CategoryListView::CheckDropItem::verifyDropWasIntended(const QString &parent, const DragItemInfoSet &items)
{
    QStringList children;
    for (DragItemInfoSet::const_iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt) {
        children.append((*itemIt).child());
    }

    QString allChildren;
    if (children.size() == 1) {
        allChildren = children[0];
    } else if (children.size() == 2) {
        allChildren = i18n("\"%1\" and \"%2\"", children[0], children[1]);
    } else {
        for (int i = 0; i < children.size() - 1; i++) {
            if (i == 0) {
                allChildren += i18n("\"%1\"", children[i]);
            } else {
                allChildren += i18n(", \"%1\"", children[i]);
            }
        }
        allChildren += i18n(" and \"%1\"", children[children.size() - 1]);
    }

    const QString msg = i18np(
        "<p>"
        "You have just dragged an item onto another. This will make the target item a tag group "
        "and define the dragged item as a member of this group. "
        "Tag groups may be used to denote facts such as 'Las Vegas is in the USA'. In that example "
        "you would drag Las Vegas onto USA. "
        "When you have set up tag groups, you can, for instance, see all images from the USA by "
        "simply selecting that item in the Browser."
        "</p>"
        "<p>"
        "Was it really your intention to make \"%3\" a tag group and add \"%2\" as a member?"
        "</p>",

        "<p>"
        "You have just dragged some items onto one other item. This will make the target item a "
        "tag group and define the dragged items as members of this group. "
        "Tag groups may be used to denote facts such as 'Las Vegas and New York are in the USA'. "
        "In that example you would drag Las Vegas and New York onto USA. "
        "When you have set up tag groups, you can, for instance, see all images from the USA by "
        "simply selecting that item in the Browser."
        "</p>"
        "<p>"
        "Was it really your intention to make \"%3\" a tag group and add %2 as members?"
        "</p>",

        children.size(), allChildren, parent);

    const int answer = KMessageBox::warningContinueCancel(nullptr, msg, i18n("Move Items"), KStandardGuiItem::cont(),
                                                          KStandardGuiItem::cancel(),
                                                          QString::fromLatin1("DoYouReallyWantToMessWithMemberGroups"));
    return answer == KMessageBox::Continue;
}

void CategoryListView::CheckDropItem::setDNDEnabled(const bool b)
{
    if (b)
        setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | flags());
    else
        setFlags(flags() & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
}
// vi:expandtab:tabstop=4 shiftwidth=4:
