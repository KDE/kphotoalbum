/* Copyright (C) 2003-2014 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "CategoryPage.h"

// Qt includes
#include <QSpinBox>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QDebug>

// KDE includes
#include <KMessageBox>
#include <KComboBox>
#include <KLocale>
#include <KIconDialog>

// Local includes
#include "DB/ImageDB.h"
#include "DB/CategoryCollection.h"
#include "UntaggedGroupBox.h"
#include "SettingsDialog.h"
#include "CategoryItem.h"

Settings::CategoryPage::CategoryPage(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* categoryLayout = new QHBoxLayout;
    mainLayout->addLayout(categoryLayout);

    // Category list

    QVBoxLayout* categorySideLayout = new QVBoxLayout;
    categoryLayout->addLayout(categorySideLayout);

    m_categoriesListWidget = new QListWidget;

    connect(m_categoriesListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(editCategory(QListWidgetItem*)));
    connect(m_categoriesListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(categoryNameChanged(QListWidgetItem*)));

    // This is needed to fix some odd behavior if the "New" button is double clicked
    connect(m_categoriesListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(categoryDoubleClicked(QListWidgetItem*)));
    connect(m_categoriesListWidget->itemDelegate(), SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
            this, SLOT(listWidgetEditEnd(QWidget*,QAbstractItemDelegate::EndEditHint)));

    categorySideLayout->addWidget(m_categoriesListWidget);

    // New, Delete, and buttons

    QHBoxLayout* newDeleteRenameLayout = new QHBoxLayout;
    categorySideLayout->addLayout(newDeleteRenameLayout);

    QPushButton* newCategoryButton = new QPushButton(i18n("New"));
    connect(newCategoryButton, SIGNAL(clicked()), this, SLOT(newCategory()));
    newDeleteRenameLayout->addWidget(newCategoryButton);

    m_delItem = new QPushButton(i18n("Delete"));
    connect(m_delItem, SIGNAL(clicked()), this, SLOT(deleteCurrentCategory()));
    newDeleteRenameLayout->addWidget(m_delItem);

    m_renameItem = new QPushButton(i18n("Rename"));
    connect(m_renameItem, SIGNAL(clicked()), this, SLOT(renameCurrentCategory()));
    newDeleteRenameLayout->addWidget(m_renameItem);

    // Category settings

    QVBoxLayout* rightSideLayout = new QVBoxLayout;
    categoryLayout->addLayout(rightSideLayout);

    // Header
    m_categoryLabel = new QLabel;
    m_categoryLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    rightSideLayout->addWidget(m_categoryLabel);

    // Pending rename label
    m_renameLabel = new QLabel;
    m_renameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    rightSideLayout->addWidget(m_renameLabel);
    connect( parent, SIGNAL(cancelClicked()), m_renameLabel, SLOT(clear()));

    // Some space looks better here :-)
    QLabel* spacer = new QLabel;
    rightSideLayout->addWidget(spacer);

    // Here we start with the actual settings

    QGridLayout* settingsLayout = new QGridLayout;
    rightSideLayout->addLayout(settingsLayout);

    int row = 0;

    // Positionable
    m_positionableLabel = new QLabel(i18n("Positionable tags:"));
    settingsLayout->addWidget(m_positionableLabel, row, 0);
    m_positionable = new QCheckBox(i18n("Tags in this category can be associated with an area of the image"));
    settingsLayout->addWidget(m_positionable, row, 1);
    connect(m_positionable, SIGNAL(clicked(bool)), this, SLOT(positionableChanged(bool)));
    row++;

    // Icon
    m_iconLabel = new QLabel(i18n("Icon:"));
    settingsLayout->addWidget(m_iconLabel, row, 0);
    m_icon = new KIconButton;
    settingsLayout->addWidget(m_icon, row, 1);
    m_icon->setIconSize(32);
    m_icon->setIcon(QString::fromUtf8("personsIcon"));
    connect(m_icon, SIGNAL(iconChanged(QString)), this, SLOT(iconChanged(QString)));
    row++;

    // Thumbnail size
    m_thumbnailSizeInCategoryLabel = new QLabel(i18n("Thumbnail Size:"));
    settingsLayout->addWidget(m_thumbnailSizeInCategoryLabel, row, 0);
    m_thumbnailSizeInCategory = new QSpinBox;
    m_thumbnailSizeInCategory->setRange(32, 512);
    m_thumbnailSizeInCategory->setSingleStep(32);
    settingsLayout->addWidget(m_thumbnailSizeInCategory, row, 1);
    connect(m_thumbnailSizeInCategory, SIGNAL(valueChanged(int)),
            this, SLOT(thumbnailSizeChanged(int)));
    row++;

    // Preferred View
    m_preferredViewLabel = new QLabel(i18n("Preferred view:"));
    settingsLayout->addWidget(m_preferredViewLabel, row, 0);
    m_preferredView = new KComboBox;
    settingsLayout->addWidget(m_preferredView, row, 1);
    m_preferredView->addItems(QStringList()
                              << i18n("List View")
                              << i18n("List View with Custom Thumbnails")
                              << i18n("Icon View")
                              << i18n("Icon View with Custom Thumbnails"));
    connect(m_preferredView, SIGNAL(activated(int)), this, SLOT(preferredViewChanged(int)));

    rightSideLayout->addStretch();

    resetInterface();

    // Untagged images
    m_untaggedBox = new UntaggedGroupBox;
    mainLayout->addWidget(m_untaggedBox);

    m_currentCategory = 0;

    // This is needed to fix some odd behavior if the "New" button is double clicked
    m_editorOpen = false;
}

void Settings::CategoryPage::resetInterface()
{
    enableDisable(false);
    m_categoriesListWidget->setItemSelected(m_categoriesListWidget->currentItem(), false);
    m_categoryLabel->setText(i18n("<i>Choose a category to edit it</i>"));
    m_renameLabel->hide();
}

void Settings::CategoryPage::editCategory(QListWidgetItem* i)
{
    if (i == 0) {
        return;
    }

    m_categoryNameBeforeEdit = i->text();

    Settings::CategoryItem* item = static_cast<Settings::CategoryItem*>(i);
    m_currentCategory = item;
    m_categoryLabel->setText(QString::fromUtf8("%1 <b>%2</b>").arg(i18n("Settings for category")).arg(m_currentCategory->text()));

    if (m_currentCategory->text() != m_categoryNameBeforeEdit) {
        m_renameLabel->setText(i18n("<i>Pending change: rename to \"%1\"</i>").arg(m_categoryNameBeforeEdit));
        m_renameLabel->show();
    } else {
        m_renameLabel->clear();
        m_renameLabel->hide();
    }

    m_positionable->setChecked(item->positionable());
    m_icon->setIcon(item->icon());
    m_thumbnailSizeInCategory->setValue(item->thumbnailSize());
    m_preferredView->setCurrentIndex(static_cast<int>(item->viewType()));
    enableDisable(true);
}

void Settings::CategoryPage::categoryNameChanged(QListWidgetItem* item)
{
    QString newCategoryName = item->text().simplified();
    m_categoriesListWidget->blockSignals(true);
    item->setText(QString());
    m_categoriesListWidget->blockSignals(false);

    // Now let's check if the new name is valid :-)

    // If it's empty, we're done here. The new name can't be empty.
    if (newCategoryName.isEmpty()) {
        resetCategory(item);
        return;
    }

    // We don't want to have special category names.
    if (newCategoryName == QString::fromUtf8("Folder") || newCategoryName == i18n("Folder")
        || newCategoryName == QString::fromUtf8("Tokens") || newCategoryName == i18n("Tokens")
        || newCategoryName == QString::fromUtf8("Media Type") || newCategoryName == i18n("Media Type")
        || newCategoryName == QString::fromUtf8("Keywords") || newCategoryName == i18n("Keywords")) {

        resetCategory(item);
        KMessageBox::sorry(this,
                           i18n("<p>Can't change the name of category \"%1\" to \"%2\":</p>"
                                "<p>\"%2\" is a special category name which is reserved and can't "
                                "be used for a normal category.</p>",
                                m_currentCategory->text(), newCategoryName),
                           i18n("Invalid category name"));
        return;
    }

    // Let's see if we already have a category with this name.
    if (m_categoriesListWidget->findItems(newCategoryName, Qt::MatchExactly).size() > 0) {
        resetCategory(item);
        KMessageBox::sorry(this,
                           i18n("<p>Can't change the name of category \"%1\" to \"%2\":</p>"
                                "<p>A category with this name already exists.</p>",
                                m_currentCategory->text(), newCategoryName),
                           i18n("Invalid category name"));
        return;
    }

    // Let's see if we are about to rename a localized category to it's C locale version
    if (DB::Category::localizedCategoriesToC().contains(m_currentCategory->text())) {
        if (newCategoryName == DB::Category::localizedCategoriesToC()[m_currentCategory->text()]) {
            resetCategory(item);
            KMessageBox::sorry(this,
                               i18n("<p>Can't change the name of category \"%1\" to \"%2\":</p>"
                                    "<p>\"%2\" is a standard category which comes with a localized "
                                    "name. The localized name for \"%2\" is \"%1\", so this "
                                    "category already has this name.</p>",
                                    m_currentCategory->text(), newCategoryName),
                               i18n("Invalid category name"));
            return;
        }
    }

    // Check if the entered name has a C locale version
    if (DB::Category::standardCategories().contains(newCategoryName)) {

        // Let's see if we rename the category to the C locale version of another existing one
        if (m_categoriesListWidget->findItems(DB::Category::standardCategories()[newCategoryName], Qt::MatchExactly).size() > 0) {
            resetCategory(item);
            KMessageBox::sorry(this,
                               i18n("<p>Can't change the name of category \"%1\" to \"%2\":</p>"
                                    "<p>\"%2\" is a standard category which comes with a localized "
                                    "name. The localized name for \"%2\" is \"%3\" and this "
                                    "category already exists.</p>",
                                    m_currentCategory->text(), newCategoryName, DB::Category::standardCategories()[newCategoryName]),
                               i18n("Invalid category name"));
            return;
        }

        if (newCategoryName != DB::Category::standardCategories()[newCategoryName])
        {
            // The C locale name can be used, but we set the localized version.
            KMessageBox::information(this,
                    i18n("<p>\"%1\" is a standard category which comes with a "
                        "localized name. The localized name for \"%1\" is \"%2\", "
                        "this name will be used instead.</p>",
                        newCategoryName, DB::Category::standardCategories()[newCategoryName]),
                    i18n("Localized category name entered"));
            newCategoryName = DB::Category::standardCategories()[newCategoryName];
        }
    }

    // Let's see if we have any pending name changes that would cause collisions.
    for (int i = 0; i < m_categoriesListWidget->count(); i++) {
        Settings::CategoryItem* cat = static_cast<Settings::CategoryItem*>(m_categoriesListWidget->item(i));
        if (cat == m_currentCategory) {
            continue;
        }

        if (newCategoryName == cat->text()
            || DB::Category::unLocalizedCategoryName(newCategoryName) == cat->text()) {

            resetCategory(item);
            KMessageBox::sorry(this,
                               i18n("<p>Can't change the name of category \"%1\" to \"%2\":</p>"
                                    "<p>There's a pending rename action on the category \"%2\". "
                                    "Please save this change first.</p>",
                                    m_currentCategory->text(), newCategoryName),
                               i18n("Unsaved pending renaming action"));
            return;
        }
    }

    m_categoriesListWidget->blockSignals(true);
    item->setText(newCategoryName);
    m_categoriesListWidget->blockSignals(false);

    emit currentCategoryNameChanged(m_currentCategory->text(), newCategoryName);
    m_currentCategory->setLabel(newCategoryName);
    editCategory(m_currentCategory);
}

void Settings::CategoryPage::resetCategory(QListWidgetItem* item)
{
    m_categoriesListWidget->blockSignals(true);
    item->setText(m_categoryNameBeforeEdit);
    m_categoriesListWidget->blockSignals(false);
}

void Settings::CategoryPage::positionableChanged(bool positionable)
{
    if (! m_currentCategory) {
        return;
    }

    if (! positionable) {
        int answer = KMessageBox::questionYesNo(this,
                                                i18n("<p>Do you really want to make \"%1\" "
                                                     "non-positionable?</p>"
                                                     "<p>All areas linked against this category "
                                                     "will be deleted!</p>",
                                                     m_currentCategory->text()));
        if (answer == KMessageBox::No) {
            m_positionable->setCheckState(Qt::Checked);
            return;
        }
    }

    m_currentCategory->setPositionable(positionable);
#ifdef HAVE_KFACE
    m_unMarkedAsPositionable.append(m_currentCategory);
#endif
}

void Settings::CategoryPage::iconChanged(const QString& icon)
{
    if(m_currentCategory) {
        m_currentCategory->setIcon(icon);
    }
}

void Settings::CategoryPage::thumbnailSizeChanged(int size)
{
    if (m_currentCategory) {
        m_currentCategory->setThumbnailSize(size);
    }
}

void Settings::CategoryPage::preferredViewChanged(int i)
{
    if (m_currentCategory) {
        m_currentCategory->setViewType(static_cast<DB::Category::ViewType>(i));
    }
}

void Settings::CategoryPage::newCategory()
{
    // This is needed to fix some odd behavior if the "New" button is double clicked
    if (m_editorOpen) {
        return;
    } else {
        m_editorOpen = true;
    }

    // Here starts the real function

    QString newCategory = i18n("New category");
    QString checkedCategory = newCategory;
    int i = 1;
    while (m_categoriesListWidget->findItems(checkedCategory, Qt::MatchExactly).size() > 0) {
        i++;
        checkedCategory = QString::fromUtf8("%1 %2").arg(newCategory).arg(i);
    }

    m_categoriesListWidget->blockSignals(true);
    m_currentCategory = new Settings::CategoryItem(QString(),
                                                   checkedCategory,
                                                   QString(),
                                                   DB::Category::TreeView,
                                                   64,
                                                   m_categoriesListWidget);
    emit currentCategoryNameChanged(QString(), checkedCategory);
    m_currentCategory->setLabel(checkedCategory);
    m_categoriesListWidget->blockSignals(false);

    m_positionable->setChecked(false);
    m_icon->setIcon(QIcon());
    m_thumbnailSizeInCategory->setValue(64);
    enableDisable(true);

    m_currentCategory->setSelected(true);
    editCategory(m_currentCategory);
    m_categoriesListWidget->editItem(m_currentCategory);
}

void Settings::CategoryPage::deleteCurrentCategory()
{
    int answer = KMessageBox::questionYesNo(this,
                                            i18n("<p>Really delete category \"%1\"?</p>",
                                            m_currentCategory->text()));
    if (answer == KMessageBox::No) {
        return;
    }

    m_deletedCategories.append(m_currentCategory);
    m_categoriesListWidget->takeItem(m_categoriesListWidget->row(m_currentCategory));
    m_currentCategory = 0;
    m_positionable->setChecked(false);
    m_icon->setIcon(QIcon());
    m_thumbnailSizeInCategory->setValue(64);
    enableDisable(false);
    m_categoryLabel->setText(i18n("<i>choose a category to edit it</i>"));
}

void Settings::CategoryPage::renameCurrentCategory()
{
    // This is needed to fix some odd behavior if the "New" button is double clicked
    m_editorOpen = true;

    m_categoriesListWidget->editItem(m_currentCategory);
}

void Settings::CategoryPage::enableDisable(bool b)
{
    m_delItem->setEnabled(b);
    m_renameItem->setEnabled(b);
    m_positionableLabel->setEnabled(b);
    m_positionable->setEnabled(b);
    m_icon->setEnabled(b);
    m_iconLabel->setEnabled(b);
    m_thumbnailSizeInCategoryLabel->setEnabled(b);
    m_thumbnailSizeInCategory->setEnabled(b);
    m_preferredViewLabel->setEnabled(b);
    m_preferredView->setEnabled(b);
}

void Settings::CategoryPage::saveSettings(Settings::SettingsData* opt, DB::MemberMap* memberMap)
{
#ifdef HAVE_KFACE
    m_recognizer = FaceManagement::Recognizer::instance();
#endif

    // Delete items
    for (QList<CategoryItem*>::Iterator it = m_deletedCategories.begin(); it != m_deletedCategories.end(); ++it) {
#ifdef HAVE_KFACE
        m_recognizer->deleteCategory(nonLocalizedCategoryName((*it)->text()));
#endif
        (*it)->removeFromDatabase();
    }

#ifdef HAVE_KFACE
    m_deletedCategories = QList<CategoryItem*>();

    // Categories un-marked as positionable
    for (QList<CategoryItem*>::Iterator it = m_unMarkedAsPositionable.begin();
         it != m_unMarkedAsPositionable.end(); ++it) {
        // For the recognition database, this is the same as if the category had been deleted
        m_recognizer->deleteCategory(nonLocalizedCategoryName((*it)->text()));
    }
    m_unMarkedAsPositionable = QList<CategoryItem*>();
#endif

    // Created or Modified items
    for (int i = 0; i < m_categoriesListWidget->count(); ++i) {
        CategoryItem* item = static_cast<CategoryItem*>(m_categoriesListWidget->item(i));
        item->submit(memberMap);
    }

    m_untaggedBox->saveSettings(opt);
}

void Settings::CategoryPage::loadSettings(Settings::SettingsData* opt)
{
    m_categoriesListWidget->blockSignals(true);
    m_categoriesListWidget->clear();

    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for (QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it) {
        if (! (*it)->isSpecialCategory()) {
#ifdef HAVE_KFACE
            Settings::CategoryItem *item = new CategoryItem((*it)->name(),
                                                            (*it)->text(),
                                                            (*it)->iconName(),
                                                            (*it)->viewType(),
                                                            (*it)->thumbnailSize(),
                                                            m_categoriesListWidget,
                                                            (*it)->positionable());
            if ((*it)->positionable()) {
                connect(item, SIGNAL(newCategoryNameSaved(QString,QString)),
                        this, SLOT(renameRecognitionCategory(QString,QString)));
            }
#else
            new CategoryItem((*it)->name(),
                             (*it)->text(),
                             (*it)->iconName(),
                             (*it)->viewType(),
                             (*it)->thumbnailSize(),
                             m_categoriesListWidget,
                             (*it)->positionable());
#endif
        }
    }

    m_categoriesListWidget->blockSignals(false);

    m_untaggedBox->loadSettings(opt);
}

#ifdef HAVE_KFACE
void Settings::CategoryPage::renameRecognitionCategory(QString oldName, QString newName)
{
    m_recognizer->updateCategoryName(oldName, nonLocalizedCategoryName(newName));
}
#endif

QString Settings::CategoryPage::nonLocalizedCategoryName(QString category)
{
    QString nonLocalizedCategoryName = category;
    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for (QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it) {
        if (nonLocalizedCategoryName == (*it)->text()) {
            nonLocalizedCategoryName = (*it)->name();
            break;
        }
    }
    return nonLocalizedCategoryName;
}

void Settings::CategoryPage::categoryDoubleClicked(QListWidgetItem*)
{
    // This is needed to fix some odd behavior if the "New" button is double clicked
    m_editorOpen = true;
}

void Settings::CategoryPage::listWidgetEditEnd(QWidget*, QAbstractItemDelegate::EndEditHint)
{
    // This is needed to fix some odd behavior if the "New" button is double clicked
    m_editorOpen = false;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
