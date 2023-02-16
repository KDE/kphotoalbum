// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "CategoryPage.h"

// Qt includes
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

// KDE includes
#include <KIconButton>
#include <KLocalizedString>
#include <KMessageBox>
#include <kwidgetsaddons_version.h>

// Local includes
#include "CategoryItem.h"
#include "SettingsDialog.h"
#include "UntaggedGroupBox.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <DB/MemberMap.h>
#include <MainWindow/DirtyIndicator.h>
#include <MainWindow/Window.h>

Settings::CategoryPage::CategoryPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // The category settings

    QGroupBox *categoryGroupBox = new QGroupBox;
    mainLayout->addWidget(categoryGroupBox);
    categoryGroupBox->setTitle(i18n("Category Settings"));
    QHBoxLayout *categoryLayout = new QHBoxLayout(categoryGroupBox);

    // Category list

    QVBoxLayout *categorySideLayout = new QVBoxLayout;
    categoryLayout->addLayout(categorySideLayout);

    m_categoriesListWidget = new QListWidget;

    connect(m_categoriesListWidget, &QListWidget::itemClicked, this, &CategoryPage::editCategory);
    connect(m_categoriesListWidget, &QListWidget::itemSelectionChanged, this, &CategoryPage::editSelectedCategory);
    connect(m_categoriesListWidget, &QListWidget::itemChanged, this, &CategoryPage::categoryNameChanged);

    categorySideLayout->addWidget(m_categoriesListWidget);

    // New, Delete, and buttons

    QHBoxLayout *newDeleteRenameLayout = new QHBoxLayout;
    categorySideLayout->addLayout(newDeleteRenameLayout);

    m_newCategoryButton = new QPushButton(i18n("New"));
    connect(m_newCategoryButton, &QPushButton::clicked, this, &CategoryPage::newCategory);
    newDeleteRenameLayout->addWidget(m_newCategoryButton);

    m_delItem = new QPushButton(i18n("Delete"));
    connect(m_delItem, &QPushButton::clicked, this, &CategoryPage::deleteCurrentCategory);
    newDeleteRenameLayout->addWidget(m_delItem);

    m_renameItem = new QPushButton(i18n("Rename"));
    connect(m_renameItem, &QPushButton::clicked, this, &CategoryPage::renameCurrentCategory);
    newDeleteRenameLayout->addWidget(m_renameItem);

    // Category settings

    QVBoxLayout *rightSideLayout = new QVBoxLayout;
    categoryLayout->addLayout(rightSideLayout);

    // Header
    m_categoryLabel = new QLabel;
    m_categoryLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    rightSideLayout->addWidget(m_categoryLabel);

    // Pending rename label
    m_renameLabel = new QLabel;
    m_renameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    rightSideLayout->addWidget(m_renameLabel);
    QDialog *parentDialog = qobject_cast<QDialog *>(parent);
    connect(parentDialog, &QDialog::rejected, m_renameLabel, &QLabel::clear);

    // Some space looks better here :-)
    QLabel *spacer = new QLabel;
    rightSideLayout->addWidget(spacer);

    // Here we start with the actual settings

    QGridLayout *settingsLayout = new QGridLayout;
    rightSideLayout->addLayout(settingsLayout);

    int row = 0;

    // Positionable
    m_positionableLabel = new QLabel(i18n("Positionable tags:"));
    settingsLayout->addWidget(m_positionableLabel, row, 0);
    m_positionable = new QCheckBox(i18n("Tags in this category can be\n"
                                        "associated with areas within images"));
    settingsLayout->addWidget(m_positionable, row, 1);
    connect(m_positionable, &QCheckBox::clicked, this, &CategoryPage::positionableChanged);
    row++;

    // Icon
    m_iconLabel = new QLabel(i18n("Icon:"));
    settingsLayout->addWidget(m_iconLabel, row, 0);
    m_icon = new KIconButton;
    settingsLayout->addWidget(m_icon, row, 1);
    m_icon->setIconSize(32);
    m_icon->setIcon(QString::fromUtf8("personsIcon"));
    connect(m_icon, &KIconButton::iconChanged, this, &CategoryPage::iconChanged);
    row++;

    // Thumbnail size
    m_thumbnailSizeInCategoryLabel = new QLabel(i18n("Thumbnail size:"));
    settingsLayout->addWidget(m_thumbnailSizeInCategoryLabel, row, 0);
    m_thumbnailSizeInCategory = new QSpinBox;
    m_thumbnailSizeInCategory->setRange(32, 512);
    m_thumbnailSizeInCategory->setSingleStep(32);
    settingsLayout->addWidget(m_thumbnailSizeInCategory, row, 1);
    connect(m_thumbnailSizeInCategory, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CategoryPage::thumbnailSizeChanged);
    row++;

    // Preferred View
    m_preferredViewLabel = new QLabel(i18n("Preferred view:"));
    settingsLayout->addWidget(m_preferredViewLabel, row, 0);
    m_preferredView = new QComboBox;
    settingsLayout->addWidget(m_preferredView, row, 1);
    m_preferredView->addItems(QStringList()
                              << i18n("List View")
                              << i18n("List View with Custom Thumbnails")
                              << i18n("Icon View")
                              << i18n("Icon View with Custom Thumbnails"));
    connect(m_preferredView, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &CategoryPage::preferredViewChanged);

    rightSideLayout->addStretch();

    // Info about the database not being saved

    QHBoxLayout *dbNotSavedLayout = new QHBoxLayout;
    mainLayout->addLayout(dbNotSavedLayout);

    m_dbNotSavedLabel = new QLabel(i18n("<font color='red'>"
                                        "The database has unsaved changes. As long as those are "
                                        "not saved,<br/>the names of categories can't be changed "
                                        "and new ones can't be added."
                                        "</font>"));
    m_dbNotSavedLabel->setWordWrap(true);
    dbNotSavedLayout->addWidget(m_dbNotSavedLabel);

    m_saveDbNowButton = new QPushButton(i18n("Save the DB now"));
    m_saveDbNowButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    connect(m_saveDbNowButton, &QPushButton::clicked, this, &CategoryPage::saveDbNow);
    dbNotSavedLayout->addWidget(m_saveDbNowButton);

    resetInterface();

    // Untagged images
    m_untaggedBox = new UntaggedGroupBox;
    mainLayout->addWidget(m_untaggedBox);

    m_currentCategory = nullptr;

    m_categoryNamesChanged = false;
}

void Settings::CategoryPage::resetInterface()
{
    enableDisable(false);
    if (m_categoriesListWidget->currentItem())
        m_categoriesListWidget->currentItem()->setSelected(false);
    resetCategoryLabel();
    m_renameLabel->hide();
}

void Settings::CategoryPage::editSelectedCategory()
{
    editCategory(m_categoriesListWidget->currentItem());
}

void Settings::CategoryPage::editCategory(QListWidgetItem *i)
{
    if (i == nullptr) {
        return;
    }

    m_categoryNameBeforeEdit = i->text();

    Settings::CategoryItem *item = static_cast<Settings::CategoryItem *>(i);
    m_currentCategory = item;
    m_categoryLabel->setText(i18n("Settings for category <b>%1</b>", item->originalName()));

    if (m_currentCategory->originalName() != m_categoryNameBeforeEdit) {
        m_renameLabel->setText(i18n("<i>Pending change: rename to \"%1\"</i>", m_categoryNameBeforeEdit));
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

    if (item->originalName()
        == DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory)->name()) {

        m_delItem->setEnabled(false);
        m_positionableLabel->setEnabled(false);
        m_positionable->setEnabled(false);
        m_thumbnailSizeInCategoryLabel->setEnabled(false);
        m_thumbnailSizeInCategory->setEnabled(false);
        m_preferredViewLabel->setEnabled(false);
        m_preferredView->setEnabled(false);
    }
}

void Settings::CategoryPage::categoryNameChanged(QListWidgetItem *item)
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
    // We do have to search both for the localized version and the C locale version, because a user
    // could start KPA e. g. with a German locale and create a "Folder" category (which would not
    // be caught by i18n("Folder")), and then start KPA with the C locale, which would produce a
    // doubled "Folder" category.
    if (newCategoryName == i18n("Folder")
        || newCategoryName == QString::fromUtf8("Folder")
        || newCategoryName == i18n("Media Type")
        || newCategoryName == QString::fromUtf8("Media Type")) {

        resetCategory(item);
        KMessageBox::error(this,
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
        KMessageBox::error(this,
                           i18n("<p>Can't change the name of category \"%1\" to \"%2\":</p>"
                                "<p>A category with this name already exists.</p>",
                                m_currentCategory->text(), newCategoryName),
                           i18n("Invalid category name"));
        return;
    }

    // Let's see if we have any pending name changes that would cause collisions.
    for (int i = 0; i < m_categoriesListWidget->count(); i++) {
        Settings::CategoryItem *cat = static_cast<Settings::CategoryItem *>(m_categoriesListWidget->item(i));
        if (cat == m_currentCategory) {
            continue;
        }

        if (newCategoryName == cat->originalName()) {
            resetCategory(item);
            KMessageBox::error(this,
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

    Q_EMIT categoryChangesPending();
    m_untaggedBox->categoryRenamed(m_categoryNameBeforeEdit, newCategoryName);
    m_currentCategory->setLabel(newCategoryName);
    editCategory(m_currentCategory);

    m_categoryNamesChanged = true;
}

void Settings::CategoryPage::resetCategory(QListWidgetItem *item)
{
    m_categoriesListWidget->blockSignals(true);
    item->setText(m_categoryNameBeforeEdit);
    m_categoriesListWidget->blockSignals(false);
}

void Settings::CategoryPage::positionableChanged(bool positionable)
{
    if (!m_currentCategory) {
        return;
    }

    if (!positionable) {
        const QString question = i18n("<p>Do you really want to make \"%1\" "
                                      "non-positionable?</p>"
                                      "<p>All areas linked against this category "
                                      "will be discarded!</p>",
                                      m_currentCategory->text());

#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
        const auto answer = KMessageBox::questionTwoActions(this,
                                                            question,
                                                            i18nc("@title", "Confirm Action"),
                                                            KStandardGuiItem::discard(),
                                                            KStandardGuiItem::cancel());
        if (answer == KMessageBox::ButtonCode::SecondaryAction) {
            m_positionable->setCheckState(Qt::Checked);
            return;
        }
#else
        int answer = KMessageBox::questionYesNo(this, question);
        if (answer == KMessageBox::No) {
            m_positionable->setCheckState(Qt::Checked);
            return;
        }
#endif
    }

    m_currentCategory->setPositionable(positionable);
}

void Settings::CategoryPage::iconChanged(const QString &icon)
{
    if (m_currentCategory) {
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
    // Here starts the real function

    QString newCategory = i18n("New category");
    QString checkedCategory = newCategory;
    int i = 1;
    while (m_categoriesListWidget->findItems(checkedCategory, Qt::MatchExactly).size() > 0) {
        i++;
        checkedCategory = QString::fromUtf8("%1 %2").arg(newCategory).arg(i);
    }

    m_categoriesListWidget->blockSignals(true);
    m_currentCategory = new Settings::CategoryItem(checkedCategory,
                                                   QString(),
                                                   DB::Category::TreeView,
                                                   64,
                                                   m_categoriesListWidget);
    m_currentCategory->markAsNewCategory();
    Q_EMIT categoryChangesPending();
    m_currentCategory->setLabel(checkedCategory);
    m_currentCategory->setSelected(true);
    m_categoriesListWidget->blockSignals(false);

    m_positionable->setChecked(false);
    m_icon->setIcon(QIcon());
    m_thumbnailSizeInCategory->setValue(64);
    enableDisable(true);

    editCategory(m_currentCategory);
    m_categoriesListWidget->editItem(m_currentCategory);
    m_untaggedBox->categoryAdded(newCategory);
}

void Settings::CategoryPage::deleteCurrentCategory()
{
    const QString question = i18n("<p>Really delete category \"%1\"?</p>",
                                  m_currentCategory->text());
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
    const auto answer = KMessageBox::questionTwoActions(this,
                                                        question,
                                                        i18n("Confirm Action"),
                                                        KStandardGuiItem::del(),
                                                        KStandardGuiItem::cancel());
    if (answer == KMessageBox::ButtonCode::SecondaryAction) {
        return;
    }
#else
    const auto answer = KMessageBox::questionYesNo(this, question);
    if (answer == KMessageBox::No) {
        return;
    }
#endif

    m_untaggedBox->categoryDeleted(m_currentCategory->text());
    m_deletedCategories.append(m_currentCategory);
    m_categoriesListWidget->takeItem(m_categoriesListWidget->row(m_currentCategory));
    m_currentCategory = nullptr;
    m_positionable->setChecked(false);
    m_icon->setIcon(QIcon());
    m_thumbnailSizeInCategory->setValue(64);
    enableDisable(false);
    resetCategoryLabel();

    editCategory(m_categoriesListWidget->currentItem());
    Q_EMIT categoryChangesPending();
}

void Settings::CategoryPage::renameCurrentCategory()
{
    m_categoriesListWidget->editItem(m_currentCategory);
}

void Settings::CategoryPage::enableDisable(bool b)
{
    m_delItem->setEnabled(b);
    m_positionableLabel->setEnabled(b);
    m_positionable->setEnabled(b);
    m_icon->setEnabled(b);
    m_iconLabel->setEnabled(b);
    m_thumbnailSizeInCategoryLabel->setEnabled(b);
    m_thumbnailSizeInCategory->setEnabled(b);
    m_preferredViewLabel->setEnabled(b);
    m_preferredView->setEnabled(b);

    m_categoriesListWidget->blockSignals(true);

    if (MainWindow::Window::theMainWindow()->dbIsDirty()) {
        m_dbNotSavedLabel->show();
        m_saveDbNowButton->show();
        m_renameItem->setEnabled(false);
        m_newCategoryButton->setEnabled(false);

        for (int i = 0; i < m_categoriesListWidget->count(); i++) {
            QListWidgetItem *currentItem = m_categoriesListWidget->item(i);
            currentItem->setFlags(currentItem->flags() & ~Qt::ItemIsEditable);
        }
    } else {
        m_dbNotSavedLabel->hide();
        m_saveDbNowButton->hide();
        m_renameItem->setEnabled(b);
        m_newCategoryButton->setEnabled(true);

        for (int i = 0; i < m_categoriesListWidget->count(); i++) {
            QListWidgetItem *currentItem = m_categoriesListWidget->item(i);
            currentItem->setFlags(currentItem->flags() | Qt::ItemIsEditable);
        }
    }

    m_categoriesListWidget->blockSignals(false);
}

void Settings::CategoryPage::saveSettings(Settings::SettingsData *opt, DB::MemberMap *memberMap)
{
    // Delete items
    for (CategoryItem *item : qAsConst(m_deletedCategories)) {
        item->removeFromDatabase();
    }

    // Created or Modified items
    for (int i = 0; i < m_categoriesListWidget->count(); ++i) {
        CategoryItem *item = static_cast<CategoryItem *>(m_categoriesListWidget->item(i));
        item->submit(memberMap);
    }

    // FIXME(jzarl): wtf? we need to fix this atrocity...
    DB::ImageDB::instance()->memberMap() = *memberMap;
    m_untaggedBox->saveSettings(opt);

    if (m_categoryNamesChanged) {
        // Probably, one or more category names have been edited. Save the database so that
        // all thumbnails are referenced with the correct name.
        MainWindow::Window::theMainWindow()->slotSave();
        m_categoryNamesChanged = false;
    }
}

void Settings::CategoryPage::loadSettings(Settings::SettingsData *opt)
{
    m_categoriesListWidget->blockSignals(true);
    m_categoriesListWidget->clear();

    const QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for (const DB::CategoryPtr &category : categories) {
        if (category->type() == DB::Category::PlainCategory
            || category->type() == DB::Category::TokensCategory) {
            Settings::CategoryItem *item = new CategoryItem(category->name(),
                                                            category->iconName(),
                                                            category->viewType(),
                                                            category->thumbnailSize(),
                                                            m_categoriesListWidget,
                                                            category->positionable());
            Q_UNUSED(item)
        }
    }

    m_categoriesListWidget->blockSignals(false);

    m_untaggedBox->loadSettings(opt);
}

void Settings::CategoryPage::resetCategoryLabel()
{
    m_categoryLabel->setText(i18n("<i>Choose a category to edit it</i>"));
}

void Settings::CategoryPage::saveDbNow()
{
    MainWindow::Window::theMainWindow()->slotSave();
    resetInterface();
    enableDisable(false);
}

void Settings::CategoryPage::resetCategoryNamesChanged()
{
    m_categoryNamesChanged = false;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_CategoryPage.cpp"
