// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LISTSELECT_H
#define LISTSELECT_H

#include <DB/CategoryPtr.h>
#include <kpabase/SettingsData.h>
#include <kpabase/enums.h>

#include <QLabel>
#include <QList>

class QTreeWidgetItem;
class QToolButton;
class QEvent;
class QRadioButton;
class QLabel;

namespace DB
{
class CategoryItem;
class ImageInfo;
}
namespace CategoryListView
{
class DragableTreeWidget;
}
namespace CategoryListView
{
class CheckDropItem;
}

namespace AnnotationDialog
{

using Utilities::StringSet;

class CompletableLineEdit;

class ListSelect : public QWidget
{
    Q_OBJECT

public:
    ListSelect(const DB::CategoryPtr &category, QWidget *parent);
    QString category() const;
    QString text() const;
    void setText(const QString &);
    void setSelection(const StringSet &on, const StringSet &partiallyOn = StringSet());
    StringSet itemsOn() const;
    StringSet itemsOff() const;
    StringSet itemsUnchanged() const;

    bool isAND() const;
    /**
     * @brief setMode for a search dialog or for annotating images individually or by batch.
     * @param mode
     */
    void setMode(UsageMode mode);

    void populate();

    void showOnlyItemsMatching(const QString &text);
    QWidget *lineEdit() const;
    void setPositionable(bool positionableState);
    bool positionable() const;
    bool tagIsChecked(QString tag) const;

    void connectLineEdit(CompletableLineEdit *le);

    void deselectTag(QString tag);

public Q_SLOTS:
    void slotReturn();
    void slotExternalReturn(const QString &text);
    void slotSortDate();
    void slotSortAlphaTree();
    void slotSortAlphaFlat();
    void toggleSortType();
    void updateListview();
    void rePopulate();
    void ensureTagIsSelected(QString category, QString tag);

Q_SIGNALS:
    /**
     * This signal is emitted whenever a positionable tag is (de)selected.
     */
    void positionableTagSelected(const QString category, const QString tag);
    void positionableTagDeselected(const QString category, const QString tag);
    void positionableTagRenamed(const QString category, const QString oldTag, const QString newTag);

protected Q_SLOTS:
    void itemSelected(QTreeWidgetItem *);
    void showContextMenu(const QPoint &);
    void setViewSortType(Settings::ViewSortType);
    void limitToSelection();
    void showAllChildren();
    void updateSelectionCount();

protected:
    void addItems(DB::CategoryItem *item, QTreeWidgetItem *parent);
    void populateAlphaTree();
    void populateAlphaFlat();
    void populateMRU();
    void configureItem(CategoryListView::CheckDropItem *item);
    /**
     * @brief computedEditMode computes the edit mode taking into account the current UsageMode.
     * @return \c Selectable if mode is \c SearchMode, editMode otherwise.
     */
    ListSelectEditMode computedEditMode() const;
    StringSet itemsOfState(Qt::CheckState state) const;
    void checkItem(const QString itemText, bool);
    void ensureAllInstancesAreStateChanged(QTreeWidgetItem *item);
    /**
     * @brief setEditMode to control how the ListSelect handles input.
     * For normal categories, \c ListSelectEditMode::Editable is recommended.
     * Special categories should either be \c ReadOnly (folder, media type) or
     * \c Selectable (tokens category).
     * @param mode
     */
    void setEditMode(ListSelectEditMode mode);

private: // Functions
    bool searchForUntaggedImagesTagNeeded();
    void hideUntaggedImagesTag();
    QTreeWidgetItem *getUntaggedImagesTag();
    /**
     * @brief updateLineEditMode sets the mode for the CompleteableLineEdit according to mode and editMode.
     */
    void updateLineEditMode();

private: // Variables
    DB::CategoryPtr m_category;
    CompletableLineEdit *m_lineEdit;
    CategoryListView::DragableTreeWidget *m_treeWidget;
    QRadioButton *m_or;
    QRadioButton *m_and;
    UsageMode m_mode;
    ListSelectEditMode m_editMode = ListSelectEditMode::Editable;
    QLabel *m_roIndicator; ///< indicates computedEditMode() ReadOnly
    QLabel *m_selectableIndicator; ///< indicates computedEditMode() Selectable
    QToolButton *m_alphaTreeSort;
    QToolButton *m_alphaFlatSort;
    QToolButton *m_dateSort;
    QToolButton *m_showSelectedOnly;
    QString m_baseTitle;
    bool m_positionable;
};
}

#endif /* LISTSELECT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
