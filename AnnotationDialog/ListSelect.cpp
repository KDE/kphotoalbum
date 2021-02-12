// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ListSelect.h"

#include "CompletableLineEdit.h"
#include "Dialog.h"
#include "ListViewItemHider.h"
#include "ShowSelectionOnlyManager.h"

#include <CategoryListView/CheckDropItem.h>
#include <CategoryListView/DragableTreeWidget.h>
#include <DB/CategoryItem.h>
#include <DB/ImageDB.h>
#include <DB/MemberMap.h>
#include <kpabase/StringSet.h>

#include <KIO/CopyJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <QButtonGroup>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QMenu>
#include <QMouseEvent>
#include <QRadioButton>
#include <QToolButton>
#include <QWidgetAction>

using namespace AnnotationDialog;
using CategoryListView::CheckDropItem;

namespace
{
inline QPixmap smallIcon(const QString &iconName)
{
    return QIcon::fromTheme(iconName).pixmap(KIconLoader::StdSizes::SizeSmall);
}
}

AnnotationDialog::ListSelect::ListSelect(const DB::CategoryPtr &category, QWidget *parent)
    : QWidget(parent)
    , m_category(category)
    , m_baseTitle()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_lineEdit = new CompletableLineEdit(this);
    m_lineEdit->setProperty("FocusCandidate", true);
    m_lineEdit->setProperty("WantsFocus", true);
    m_lineEdit->setPlaceholderText(i18nc("@label:textbox", "Enter a tag..."));
    layout->addWidget(m_lineEdit);

    m_treeWidget = new CategoryListView::DragableTreeWidget(m_category, this);
    m_treeWidget->setHeaderLabel(QString::fromLatin1("items"));
    m_treeWidget->header()->hide();
    connect(m_treeWidget, &CategoryListView::DragableTreeWidget::itemClicked, this, &ListSelect::itemSelected);
    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_treeWidget, &CategoryListView::DragableTreeWidget::customContextMenuRequested, this, &ListSelect::showContextMenu);
    connect(m_treeWidget, &CategoryListView::DragableTreeWidget::itemsChanged, this, &ListSelect::rePopulate);
    connect(m_treeWidget, &CategoryListView::DragableTreeWidget::itemClicked, this, &ListSelect::updateSelectionCount);

    layout->addWidget(m_treeWidget);

    // Merge CheckBox
    QHBoxLayout *lay2 = new QHBoxLayout;
    layout->addLayout(lay2);

    m_roIndicator = new QLabel;
    m_roIndicator->setPixmap(smallIcon(QString::fromLatin1("emblem-readonly")));
    lay2->addWidget(m_roIndicator);
    m_selectableIndicator = new QLabel;
    m_selectableIndicator->setPixmap(smallIcon(QString::fromLatin1("emblem-checked")));
    lay2->addWidget(m_selectableIndicator);

    m_or = new QRadioButton(i18n("or"), this);
    m_and = new QRadioButton(i18n("and"), this);
    lay2->addWidget(m_or);
    lay2->addWidget(m_and);
    lay2->addStretch(1);

    // Sorting tool button
    QButtonGroup *grp = new QButtonGroup(this);
    grp->setExclusive(true);

    m_alphaTreeSort = new QToolButton;
    m_alphaTreeSort->setIcon(smallIcon(QString::fromLatin1("view-list-tree")));
    m_alphaTreeSort->setCheckable(true);
    m_alphaTreeSort->setToolTip(i18n("Sort Alphabetically (Tree)"));
    grp->addButton(m_alphaTreeSort);

    m_alphaFlatSort = new QToolButton;
    m_alphaFlatSort->setIcon(smallIcon(QString::fromLatin1("draw-text")));
    m_alphaFlatSort->setCheckable(true);
    m_alphaFlatSort->setToolTip(i18n("Sort Alphabetically (Flat)"));
    grp->addButton(m_alphaFlatSort);

    m_dateSort = new QToolButton;
    m_dateSort->setIcon(smallIcon(QString::fromLatin1("x-office-calendar")));
    m_dateSort->setCheckable(true);
    m_dateSort->setToolTip(i18n("Sort by date"));
    grp->addButton(m_dateSort);

    m_showSelectedOnly = new QToolButton;
    m_showSelectedOnly->setIcon(smallIcon(QString::fromLatin1("view-filter")));
    m_showSelectedOnly->setCheckable(true);
    m_showSelectedOnly->setToolTip(i18n("Show only selected Ctrl+S"));
    m_showSelectedOnly->setChecked(ShowSelectionOnlyManager::instance().selectionIsLimited());

    m_alphaTreeSort->setChecked(Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaTree);
    m_alphaFlatSort->setChecked(Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaFlat);
    m_dateSort->setChecked(Settings::SettingsData::instance()->viewSortType() == Settings::SortLastUse);
    connect(m_dateSort, &QToolButton::clicked, this, &ListSelect::slotSortDate);
    connect(m_alphaTreeSort, &QToolButton::clicked, this, &ListSelect::slotSortAlphaTree);
    connect(m_alphaFlatSort, &QToolButton::clicked, this, &ListSelect::slotSortAlphaFlat);
    connect(m_showSelectedOnly, &QToolButton::clicked, &ShowSelectionOnlyManager::instance(), &ShowSelectionOnlyManager::toggle);

    lay2->addWidget(m_alphaTreeSort);
    lay2->addWidget(m_alphaFlatSort);
    lay2->addWidget(m_dateSort);
    lay2->addWidget(m_showSelectedOnly);

    connectLineEdit(m_lineEdit);

    populate();

    connect(Settings::SettingsData::instance(), &Settings::SettingsData::viewSortTypeChanged,
            this, &ListSelect::setViewSortType);
    connect(Settings::SettingsData::instance(), &Settings::SettingsData::matchTypeChanged,
            this, &ListSelect::updateListview);

    connect(&ShowSelectionOnlyManager::instance(), &ShowSelectionOnlyManager::limitToSelected, this, &ListSelect::limitToSelection);
    connect(&ShowSelectionOnlyManager::instance(), &ShowSelectionOnlyManager::broaden, this, &ListSelect::showAllChildren);

    if (category->isSpecialCategory()) {
        if (category->type() == DB::Category::TokensCategory)
            setEditMode(ListSelectEditMode::Selectable);
        else
            setEditMode(ListSelectEditMode::ReadOnly);
    }
    updateLineEditMode();
}

void AnnotationDialog::ListSelect::slotReturn()
{
    if (computedEditMode() == ListSelectEditMode::Editable) {
        QString enteredText = m_lineEdit->text().trimmed();
        if (enteredText.isEmpty()) {
            return;
        }

        if (searchForUntaggedImagesTagNeeded()) {
            if (enteredText == Settings::SettingsData::instance()->untaggedTag()) {
                KMessageBox::information(
                    this,
                    i18n("The tag you entered is the tag that is set automatically for newly "
                         "found, untagged images (cf. <interface>Settings|Configure KPhotoAlbum..."
                         "|Categories|Untagged Images</interface>). It will not show up here as "
                         "long as it is selected for this purpose."));
                m_lineEdit->setText(QString());
                return;
            }
        }

        m_category->addItem(enteredText);
        rePopulate();

        QList<QTreeWidgetItem *> items = m_treeWidget->findItems(enteredText, Qt::MatchExactly | Qt::MatchRecursive, 0);
        if (!items.isEmpty()) {
            items.at(0)->setCheckState(0, Qt::Checked);
            if (m_positionable) {
                emit positionableTagSelected(m_category->name(), items.at(0)->text(0));
            }
        } else {
            Q_ASSERT(false);
        }

        m_lineEdit->clear();
    }
    updateSelectionCount();
}

void ListSelect::slotExternalReturn(const QString &text)
{
    m_lineEdit->setText(text);
    slotReturn();
}

QString AnnotationDialog::ListSelect::category() const
{
    return m_category->name();
}

void AnnotationDialog::ListSelect::setSelection(const StringSet &on, const StringSet &partiallyOn)
{
    for (QTreeWidgetItemIterator itemIt(m_treeWidget); *itemIt; ++itemIt) {
        if (partiallyOn.contains((*itemIt)->text(0)))
            (*itemIt)->setCheckState(0, Qt::PartiallyChecked);
        else
            (*itemIt)->setCheckState(0, on.contains((*itemIt)->text(0)) ? Qt::Checked : Qt::Unchecked);
    }

    m_lineEdit->clear();
    updateSelectionCount();
}

bool AnnotationDialog::ListSelect::isAND() const
{
    return m_and->isChecked();
}

void AnnotationDialog::ListSelect::setMode(UsageMode mode)
{
    m_mode = mode;
    updateLineEditMode();
    if (mode == SearchMode) {
        // "0" below is sorting key which ensures that None is always at top.
        CheckDropItem *item = new CheckDropItem(m_treeWidget, DB::ImageDB::NONE(), QString::fromLatin1("0"));
        configureItem(item);
        m_and->show();
        m_or->show();
        m_or->setChecked(true);
        m_showSelectedOnly->hide();
    } else {
        m_and->hide();
        m_or->hide();
        m_showSelectedOnly->show();
    }
    for (QTreeWidgetItemIterator itemIt(m_treeWidget); *itemIt; ++itemIt)
        configureItem(dynamic_cast<CategoryListView::CheckDropItem *>(*itemIt));

    // ensure that the selection count indicator matches the current mode:
    updateSelectionCount();
}

void ListSelect::setEditMode(ListSelectEditMode mode)
{
    m_editMode = mode;
    updateLineEditMode();
}

void AnnotationDialog::ListSelect::setViewSortType(Settings::ViewSortType tp)
{
    showAllChildren();

    // set sortType and redisplay with new sortType
    QString text = m_lineEdit->text();
    rePopulate();
    m_lineEdit->setText(text);
    setMode(m_mode); // generate the ***NONE*** entry if in search mode

    m_alphaTreeSort->setChecked(tp == Settings::SortAlphaTree);
    m_alphaFlatSort->setChecked(tp == Settings::SortAlphaFlat);
    m_dateSort->setChecked(tp == Settings::SortLastUse);
}

QString AnnotationDialog::ListSelect::text() const
{
    return m_lineEdit->text();
}

void AnnotationDialog::ListSelect::setText(const QString &text)
{
    m_lineEdit->setText(text);
    m_treeWidget->clearSelection();
}

void AnnotationDialog::ListSelect::itemSelected(QTreeWidgetItem *item)
{
    if (!item) {
        // click outside any item
        return;
    }

    if (m_mode == SearchMode) {
        QString txt = item->text(0);
        QString res;
        QRegExp regEnd(QString::fromLatin1("\\s*[&|!]\\s*$"));
        QRegExp regStart(QString::fromLatin1("^\\s*[&|!]\\s*"));

        if (item->checkState(0) == Qt::Checked) {
            int matchPos = m_lineEdit->text().indexOf(txt);
            if (matchPos != -1) {
                return;
            }

            int index = m_lineEdit->cursorPosition();
            QString start = m_lineEdit->text().left(index);
            QString end = m_lineEdit->text().mid(index);

            res = start;
            if (!start.isEmpty() && !start.contains(regEnd)) {
                res += isAND() ? QString::fromLatin1(" & ") : QString::fromLatin1(" | ");
            }
            res += txt;
            if (!end.isEmpty() && !end.contains(regStart)) {
                res += isAND() ? QString::fromLatin1(" & ") : QString::fromLatin1(" | ");
            }
            res += end;
        } else {
            int index = m_lineEdit->text().indexOf(txt);
            if (index == -1)
                return;

            QString start = m_lineEdit->text().left(index);
            QString end = m_lineEdit->text().mid(index + txt.length());
            if (start.contains(regEnd))
                start.replace(regEnd, QString::fromLatin1(""));
            else
                end.replace(regStart, QString::fromLatin1(""));

            res = start + end;
        }
        m_lineEdit->setText(res);
    }

    else {
        if (m_positionable) {
            if (item->checkState(0) == Qt::Checked) {
                emit positionableTagSelected(m_category->name(), item->text(0));
            } else {
                emit positionableTagDeselected(m_category->name(), item->text(0));
            }
        }

        m_lineEdit->clear();
        showAllChildren();
        ensureAllInstancesAreStateChanged(item);
    }
}

void AnnotationDialog::ListSelect::showContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);

    QTreeWidgetItem *item = m_treeWidget->itemAt(pos);
    // click on any item
    QString title = i18n("No Item Selected");
    if (item)
        title = item->text(0);

    QLabel *label = new QLabel(i18n("<b>%1</b>", title), menu);
    label->setAlignment(Qt::AlignCenter);
    QWidgetAction *action = new QWidgetAction(menu);
    action->setDefaultWidget(label);
    menu->addAction(action);

    QAction *deleteAction = menu->addAction(smallIcon(QString::fromLatin1("edit-delete")), i18n("Delete"));
    QAction *renameAction = menu->addAction(i18n("Rename..."));

    QLabel *categoryTitle = new QLabel(i18n("<b>Tag Groups</b>"), menu);
    categoryTitle->setAlignment(Qt::AlignCenter);
    action = new QWidgetAction(menu);
    action->setDefaultWidget(categoryTitle);
    menu->addAction(action);

    // -------------------------------------------------- Add/Remove member group
    DB::MemberMap &memberMap = DB::ImageDB::instance()->memberMap();
    QMenu *members = new QMenu(i18n("Tag groups"));
    menu->addMenu(members);
    QAction *newCategoryAction = nullptr;
    if (item) {
        QStringList grps = memberMap.groups(m_category->name());

        for (QStringList::ConstIterator it = grps.constBegin(); it != grps.constEnd(); ++it) {
            if (!memberMap.canAddMemberToGroup(m_category->name(), *it, item->text(0)))
                continue;
            QAction *action = members->addAction(*it);
            action->setCheckable(true);
            action->setChecked((bool)memberMap.members(m_category->name(), *it, false).contains(item->text(0)));
            action->setData(*it);
        }

        if (!grps.isEmpty())
            members->addSeparator();
        newCategoryAction = members->addAction(i18n("Add this tag to a new tag group..."));
    }

    QAction *newSubcategoryAction = menu->addAction(i18n("Make this tag a tag group and add a tag..."));

    // -------------------------------------------------- Take item out of category
    QTreeWidgetItem *parent = item ? item->parent() : nullptr;
    QAction *takeAction = nullptr;
    if (parent)
        takeAction = menu->addAction(i18n("Remove from tag group %1", parent->text(0)));

    // -------------------------------------------------- sort
    QLabel *sortTitle = new QLabel(i18n("<b>Sorting</b>"));
    sortTitle->setAlignment(Qt::AlignCenter);
    action = new QWidgetAction(menu);
    action->setDefaultWidget(sortTitle);
    menu->addAction(action);

    QAction *usageAction = menu->addAction(i18n("Usage"));
    QAction *alphaFlatAction = menu->addAction(i18n("Alphabetical (Flat)"));
    QAction *alphaTreeAction = menu->addAction(i18n("Alphabetical (Tree)"));
    usageAction->setCheckable(true);
    usageAction->setChecked(Settings::SettingsData::instance()->viewSortType() == Settings::SortLastUse);
    alphaFlatAction->setCheckable(true);
    alphaFlatAction->setChecked(Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaFlat);
    alphaTreeAction->setCheckable(true);
    alphaTreeAction->setChecked(Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaTree);

    if (!item || computedEditMode() != Editable) {
        deleteAction->setEnabled(false);
        renameAction->setEnabled(false);
        members->setEnabled(false);
        newCategoryAction->setEnabled(false);
        newSubcategoryAction->setEnabled(false);
        if (takeAction)
            takeAction->setEnabled(false);
    }
    // -------------------------------------------------- exec
    QAction *which = menu->exec(m_treeWidget->mapToGlobal(pos));
    if (which == nullptr)
        return;
    else if (which == deleteAction) {
        Q_ASSERT(item);
        Q_ASSERT(computedEditMode() == Editable);
        int code = KMessageBox::warningContinueCancel(this, i18n("<p>Do you really want to delete \"%1\"?<br/>"
                                                                 "Deleting the item will remove any information "
                                                                 "about it from any image containing the item.</p>",
                                                                 title),
                                                      i18n("Really Delete %1?", item->text(0)),
                                                      KGuiItem(i18n("&Delete"), QString::fromLatin1("editdelete")));
        if (code == KMessageBox::Continue) {
            if (item->checkState(0) == Qt::Checked && m_positionable) {
                // An area could be linked against this. We can use positionableTagDeselected
                // here, as the procedure is the same as if the tag had been deselected.
                emit positionableTagDeselected(m_category->name(), item->text(0));
            }

            m_category->removeItem(item->text(0));
            rePopulate();
        }
    } else if (which == renameAction) {
        Q_ASSERT(item);
        Q_ASSERT(computedEditMode() == Editable);
        bool ok;
        QString newStr = QInputDialog::getText(this,
                                               i18n("Rename Item"), i18n("Enter new name:"),
                                               QLineEdit::Normal,
                                               item->text(0), &ok);

        if (ok && !newStr.isEmpty() && newStr != item->text(0)) {
            int code = KMessageBox::questionYesNo(this, i18n("<p>Do you really want to rename \"%1\" to \"%2\"?<br/>"
                                                             "Doing so will rename \"%3\" "
                                                             "on any image containing it.</p>",
                                                             item->text(0), newStr, item->text(0)),
                                                  i18n("Really Rename %1?", item->text(0)));
            if (code == KMessageBox::Yes) {
                QString oldStr = item->text(0);
                m_category->renameItem(oldStr, newStr);
                bool checked = item->checkState(0) == Qt::Checked;
                rePopulate();
                // rePopuldate doesn't ask the backend if the item should be checked, so we need to do that.
                checkItem(newStr, checked);

                // rename the category image too
                QString oldFile = m_category->fileForCategoryImage(category(), oldStr);
                QString newFile = m_category->fileForCategoryImage(category(), newStr);
                KIO::move(QUrl::fromLocalFile(oldFile), QUrl::fromLocalFile(newFile));

                if (m_positionable) {
                    // Also take care of areas that could be linked against this
                    emit positionableTagRenamed(m_category->name(), oldStr, newStr);
                }
            }
        }
    } else if (which == usageAction) {
        Settings::SettingsData::instance()->setViewSortType(Settings::SortLastUse);
    } else if (which == alphaTreeAction) {
        Settings::SettingsData::instance()->setViewSortType(Settings::SortAlphaTree);
    } else if (which == alphaFlatAction) {
        Settings::SettingsData::instance()->setViewSortType(Settings::SortAlphaFlat);
    } else if (which == newCategoryAction) {
        Q_ASSERT(item);
        Q_ASSERT(computedEditMode() == Editable);
        QString superCategory = QInputDialog::getText(this,
                                                      i18n("New tag group"),
                                                      i18n("Name for the new tag group the tag will be added to:"));
        if (superCategory.isEmpty())
            return;
        m_category->addItem(superCategory);
        memberMap.addGroup(m_category->name(), superCategory);
        memberMap.addMemberToGroup(m_category->name(), superCategory, item->text(0));
        //DB::ImageDB::instance()->setMemberMap( memberMap );
        rePopulate();
    } else if (which == newSubcategoryAction) {
        Q_ASSERT(item);
        Q_ASSERT(computedEditMode() == Editable);
        QString subCategory = QInputDialog::getText(this,
                                                    i18n("Add a tag"),
                                                    i18n("Name for the tag to be added to this tag group:"));
        if (subCategory.isEmpty())
            return;

        m_category->addItem(subCategory);
        memberMap.addGroup(m_category->name(), item->text(0));
        memberMap.addMemberToGroup(m_category->name(), item->text(0), subCategory);
        //DB::ImageDB::instance()->setMemberMap( memberMap );
        m_category->addItem(subCategory);

        rePopulate();
        checkItem(subCategory, true);
    } else if (which == takeAction) {
        Q_ASSERT(item);
        Q_ASSERT(computedEditMode() == Editable);
        memberMap.removeMemberFromGroup(m_category->name(), parent->text(0), item->text(0));
        rePopulate();
    } else {
        Q_ASSERT(item);
        Q_ASSERT(computedEditMode() == Editable);
        QString checkedItem = which->data().value<QString>();
        if (which->isChecked()) // choosing the item doesn't check it, so this is the value before.
            memberMap.addMemberToGroup(m_category->name(), checkedItem, item->text(0));
        else
            memberMap.removeMemberFromGroup(m_category->name(), checkedItem, item->text(0));
        rePopulate();
    }

    delete menu;
}

void AnnotationDialog::ListSelect::addItems(DB::CategoryItem *item, QTreeWidgetItem *parent)
{
    const bool isReadOnly = computedEditMode() == ListSelectEditMode::ReadOnly;
    for (QList<DB::CategoryItem *>::ConstIterator subcategoryIt = item->mp_subcategories.constBegin(); subcategoryIt != item->mp_subcategories.constEnd(); ++subcategoryIt) {
        CheckDropItem *newItem = nullptr;

        if (parent == nullptr)
            newItem = new CheckDropItem(m_treeWidget, (*subcategoryIt)->mp_name, QString());
        else
            newItem = new CheckDropItem(m_treeWidget, parent, (*subcategoryIt)->mp_name, QString());

        newItem->setExpanded(true);
        configureItem(newItem);
        if (isReadOnly) {
            newItem->setFlags(newItem->flags() ^ Qt::ItemIsUserCheckable);
        }

        addItems(*subcategoryIt, newItem);
    }
}

void AnnotationDialog::ListSelect::populate()
{
    m_treeWidget->clear();

    if (Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaTree)
        populateAlphaTree();
    else if (Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaFlat)
        populateAlphaFlat();
    else
        populateMRU();

    hideUntaggedImagesTag();
}

bool AnnotationDialog::ListSelect::searchForUntaggedImagesTagNeeded()
{
    if (!DB::ImageDB::instance()->untaggedCategoryFeatureConfigured()
        || Settings::SettingsData::instance()->untaggedImagesTagVisible()) {
        return false;
    }

    if (Settings::SettingsData::instance()->untaggedCategory() != category()) {
        return false;
    }

    return true;
}

void AnnotationDialog::ListSelect::hideUntaggedImagesTag()
{
    if (!searchForUntaggedImagesTagNeeded()) {
        return;
    }

    QTreeWidgetItem *untaggedImagesTag = getUntaggedImagesTag();
    if (untaggedImagesTag) {
        untaggedImagesTag->setHidden(true);
    }
}

void AnnotationDialog::ListSelect::slotSortDate()
{
    Settings::SettingsData::instance()->setViewSortType(Settings::SortLastUse);
}

void AnnotationDialog::ListSelect::slotSortAlphaTree()
{
    Settings::SettingsData::instance()->setViewSortType(Settings::SortAlphaTree);
}

void AnnotationDialog::ListSelect::slotSortAlphaFlat()
{
    Settings::SettingsData::instance()->setViewSortType(Settings::SortAlphaFlat);
}

void AnnotationDialog::ListSelect::rePopulate()
{
    const StringSet on = itemsOn();
    const StringSet noChange = itemsUnchanged();
    populate();
    setSelection(on, noChange);

    if (ShowSelectionOnlyManager::instance().selectionIsLimited())
        limitToSelection();
}

void AnnotationDialog::ListSelect::showOnlyItemsMatching(const QString &text)
{
    ListViewTextMatchHider dummy(text, Settings::SettingsData::instance()->matchType(), m_treeWidget);
    ShowSelectionOnlyManager::instance().unlimitFromSelection();
}

void AnnotationDialog::ListSelect::populateAlphaTree()
{
    DB::CategoryItemPtr item = m_category->itemsCategories();

    m_treeWidget->setRootIsDecorated(true);
    addItems(item.data(), 0);
    m_treeWidget->sortByColumn(0, Qt::AscendingOrder);
    m_treeWidget->setSortingEnabled(true);
}

void AnnotationDialog::ListSelect::populateAlphaFlat()
{
    QStringList items = m_category->itemsInclCategories();
    items.sort();

    m_treeWidget->setRootIsDecorated(false);
    for (QStringList::ConstIterator itemIt = items.constBegin(); itemIt != items.constEnd(); ++itemIt) {
        CheckDropItem *item = new CheckDropItem(m_treeWidget, *itemIt, *itemIt);
        configureItem(item);
    }
    m_treeWidget->sortByColumn(1, Qt::AscendingOrder);
    m_treeWidget->setSortingEnabled(true);
}

void AnnotationDialog::ListSelect::populateMRU()
{
    QStringList items = m_category->itemsInclCategories();

    m_treeWidget->setRootIsDecorated(false);
    int index = 100000; // This counter will be converted to a string, and compared, and we don't want "1111" to be less than "2"
    for (QStringList::ConstIterator itemIt = items.constBegin(); itemIt != items.constEnd(); ++itemIt) {
        ++index;
        CheckDropItem *item = new CheckDropItem(m_treeWidget, *itemIt, QString::number(index));
        configureItem(item);
    }
    m_treeWidget->sortByColumn(1, Qt::AscendingOrder);
    m_treeWidget->setSortingEnabled(true);
}

void AnnotationDialog::ListSelect::toggleSortType()
{
    Settings::SettingsData *data = Settings::SettingsData::instance();
    if (data->viewSortType() == Settings::SortLastUse)
        data->setViewSortType(Settings::SortAlphaTree);
    else if (data->viewSortType() == Settings::SortAlphaTree)
        data->setViewSortType(Settings::SortAlphaFlat);
    else
        data->setViewSortType(Settings::SortLastUse);
}

void AnnotationDialog::ListSelect::updateListview()
{
    // update item list (e.g. when MatchType changes):
    showOnlyItemsMatching(text());
}

void AnnotationDialog::ListSelect::limitToSelection()
{
    if (computedEditMode() != Editable)
        return;

    m_showSelectedOnly->setChecked(true);
    ListViewCheckedHider dummy(m_treeWidget);

    hideUntaggedImagesTag();
}

void AnnotationDialog::ListSelect::showAllChildren()
{
    m_showSelectedOnly->setChecked(false);
    showOnlyItemsMatching(QString());
    hideUntaggedImagesTag();
}

QTreeWidgetItem *AnnotationDialog::ListSelect::getUntaggedImagesTag()
{
    QList<QTreeWidgetItem *> matchingTags = m_treeWidget->findItems(
        Settings::SettingsData::instance()->untaggedTag(),
        Qt::MatchExactly | Qt::MatchRecursive, 0);

    // Be sure not to crash here in case the config points to a non-existent tag
    if (matchingTags.at(0) == nullptr) {
        return 0;
    } else {
        return matchingTags.at(0);
    }
}

void ListSelect::updateLineEditMode()
{
    if (m_editMode == Selectable)
        m_lineEdit->setMode(SearchMode);
    else
        m_lineEdit->setMode(m_mode);

    const bool isReadOnly = computedEditMode() == ListSelectEditMode::ReadOnly;
    m_roIndicator->setVisible(isReadOnly);
    // deactivate read-only fields when editing:
    m_lineEdit->setEnabled((m_mode == UsageMode::SearchMode) || !isReadOnly);
    const bool isSelectable = computedEditMode() == ListSelectEditMode::Selectable;
    m_selectableIndicator->setVisible(isSelectable);
}

void AnnotationDialog::ListSelect::updateSelectionCount()
{
    if (m_baseTitle.isEmpty() /* --> first time */
        || !parentWidget()->windowTitle().startsWith(m_baseTitle) /* --> title has changed */) {

        // save the original parentWidget title
        m_baseTitle = parentWidget()->windowTitle();
    }

    int itemsOnCount = itemsOn().size();
    // Don't count the untagged images tag:
    if (searchForUntaggedImagesTagNeeded()) {
        QTreeWidgetItem *untaggedImagesTag = getUntaggedImagesTag();
        if (untaggedImagesTag) {
            if (untaggedImagesTag->checkState(0) != Qt::Unchecked) {
                itemsOnCount--;
            }
        }
    }

    switch (m_mode) {
    case InputMultiImageConfigMode:
        if (itemsUnchanged().size() > 0) {
            // if min != max
            // tri-state selection -> show min-max (selected items vs. partially selected items):
            parentWidget()->setWindowTitle(i18nc(
                "Category name, then min-max of selected tags across several images. E.g. 'People (1-2)'",
                "%1 (%2-%3)",
                m_baseTitle,
                itemsOnCount,
                itemsOnCount + itemsUnchanged().size()));
            break;
        } // else fall through and only show one number:
        Q_FALLTHROUGH();
    case InputSingleImageConfigMode:
        if (itemsOnCount > 0) {
            // if any tags have been selected
            // "normal" on/off states -> show selected items
            parentWidget()->setWindowTitle(
                i18nc("Category name, then number of selected tags. E.g. 'People (1)'",
                      "%1 (%2)",
                      m_baseTitle, itemsOnCount));
            break;
        } // else fall through and only show category
        Q_FALLTHROUGH();
    case SearchMode:
        // no indicator while searching
        parentWidget()->setWindowTitle(m_baseTitle);
        break;
    }
}

void AnnotationDialog::ListSelect::configureItem(CategoryListView::CheckDropItem *item)
{
    bool isDNDAllowed = Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaTree;
    item->setDNDEnabled(isDNDAllowed && !m_category->isSpecialCategory());
}

ListSelectEditMode ListSelect::computedEditMode() const
{
    if (m_mode == SearchMode)
        return ListSelectEditMode::Selectable;
    return m_editMode;
}

StringSet AnnotationDialog::ListSelect::itemsOn() const
{
    return itemsOfState(Qt::Checked);
}

StringSet AnnotationDialog::ListSelect::itemsOff() const
{
    return itemsOfState(Qt::Unchecked);
}

StringSet AnnotationDialog::ListSelect::itemsOfState(Qt::CheckState state) const
{
    StringSet res;
    for (QTreeWidgetItemIterator itemIt(m_treeWidget); *itemIt; ++itemIt) {
        if ((*itemIt)->checkState(0) == state)
            res.insert((*itemIt)->text(0));
    }
    return res;
}

StringSet AnnotationDialog::ListSelect::itemsUnchanged() const
{
    return itemsOfState(Qt::PartiallyChecked);
}

void AnnotationDialog::ListSelect::checkItem(const QString itemText, bool b)
{
    QList<QTreeWidgetItem *> items = m_treeWidget->findItems(itemText, Qt::MatchExactly | Qt::MatchRecursive);
    if (!items.isEmpty())
        items.at(0)->setCheckState(0, b ? Qt::Checked : Qt::Unchecked);
    else
        Q_ASSERT(false);
}

/**
 * An item may be member of a number of categories. Mike may be a member of coworkers and friends.
 * Selecting the item in one subcategory, should select him in all.
 */
void AnnotationDialog::ListSelect::ensureAllInstancesAreStateChanged(QTreeWidgetItem *item)
{
    const bool on = item->checkState(0) == Qt::Checked;
    for (QTreeWidgetItemIterator itemIt(m_treeWidget); *itemIt; ++itemIt) {
        if ((*itemIt) != item && (*itemIt)->text(0) == item->text(0))
            (*itemIt)->setCheckState(0, on ? Qt::Checked : Qt::Unchecked);
    }
}

QWidget *AnnotationDialog::ListSelect::lineEdit() const
{
    return m_lineEdit;
}

void AnnotationDialog::ListSelect::setPositionable(bool positionableState)
{
    m_positionable = positionableState;
}

bool AnnotationDialog::ListSelect::positionable() const
{
    return m_positionable;
}

bool AnnotationDialog::ListSelect::tagIsChecked(QString tag) const
{
    QList<QTreeWidgetItem *> matchingTags = m_treeWidget->findItems(tag, Qt::MatchExactly | Qt::MatchRecursive, 0);

    if (matchingTags.isEmpty()) {
        return false;
    }

    return (bool)matchingTags.first()->checkState(0);
}

/**
 * @brief ListSelect::connectLineEdit associates a CompletableLineEdit with this ListSelect
 * This method also allows to connect an external CompletableLineEdit to work with this ListSelect.
 * @param le
 */
void ListSelect::connectLineEdit(CompletableLineEdit *le)
{
    le->setObjectName(m_category->name());
    le->setListView(m_treeWidget);
    connect(le, &KLineEdit::returnPressed, this, &ListSelect::slotExternalReturn);
}

void AnnotationDialog::ListSelect::ensureTagIsSelected(QString category, QString tag)
{
    if (category != m_lineEdit->objectName()) {
        // The selected tag's category does not belong to this ListSelect
        return;
    }

    // Be sure that tag is actually checked
    QList<QTreeWidgetItem *> matchingTags = m_treeWidget->findItems(tag, Qt::MatchExactly | Qt::MatchRecursive, 0);

    // If we have the requested category, but not this tag, add it.
    // This should only happen if the recognition database is copied from another database
    // or has been changed outside of KPA. But this _can_ happen and simply adding a
    // missing tag does not hurt ;-)
    if (matchingTags.isEmpty()) {
        m_category->addItem(tag);
        rePopulate();
        // Now, we find it
        matchingTags = m_treeWidget->findItems(tag, Qt::MatchExactly | Qt::MatchRecursive, 0);
    }

    matchingTags.first()->setCheckState(0, Qt::Checked);
}

void AnnotationDialog::ListSelect::deselectTag(QString tag)
{
    QList<QTreeWidgetItem *> matchingTags = m_treeWidget->findItems(tag, Qt::MatchExactly | Qt::MatchRecursive, 0);
    matchingTags.first()->setCheckState(0, Qt::Unchecked);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
