/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "UntaggedGroupBox.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <kpabase/SettingsData.h>

#include <KLocalizedString>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

Settings::UntaggedGroupBox::UntaggedGroupBox(QWidget *parent)
    : QGroupBox(i18n("Untagged Images"), parent)
{
    setWhatsThis(i18n("If a tag is selected here, it will be added to new (untagged) images "
                      "automatically, so that they can be easily found. It will be removed as "
                      "soon as the image has been annotated."));

    QGridLayout *grid = new QGridLayout(this);
    int row = -1;

    QLabel *label = new QLabel(i18n("Category:"));
    grid->addWidget(label, ++row, 0);

    m_category = new QComboBox;
    grid->addWidget(m_category, row, 1);
    connect(m_category, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &UntaggedGroupBox::populateTagsCombo);

    label = new QLabel(i18n("Tag:"));
    grid->addWidget(label, ++row, 0);

    m_tag = new QComboBox;
    grid->addWidget(m_tag, row, 1);
    m_tag->setEditable(true);

    m_showUntaggedImagesTag = new QCheckBox(i18n("Show the untagged images tag as a normal tag"));
    grid->addWidget(m_showUntaggedImagesTag, ++row, 0, 1, 2);

    grid->setColumnStretch(1, 1);
}

void Settings::UntaggedGroupBox::populateCategoryComboBox()
{
    m_category->clear();
    m_category->addItem(i18n("None Selected"));
    const auto categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for (DB::CategoryPtr category : categories) {
        if (!category->isSpecialCategory()) {
            m_category->addItem(category->name(), category->name());
        }
    }
}

void Settings::UntaggedGroupBox::populateTagsCombo()
{
    m_tag->clear();
    const QString currentCategory = m_category->itemData(m_category->currentIndex()).value<QString>();

    if (currentCategory.isEmpty()) {
        m_tag->setEnabled(false);
    } else {
        m_tag->setEnabled(true);
        const QStringList items = DB::ImageDB::instance()->categoryCollection()->categoryForName(currentCategory)->items();
        m_tag->addItems(items);
    }
}

void Settings::UntaggedGroupBox::loadSettings(Settings::SettingsData *opt)
{
    populateCategoryComboBox();

    const QString category = opt->untaggedCategory();
    const QString tag = opt->untaggedTag();

    int categoryIndex = m_category->findData(category);
    if (categoryIndex == -1)
        categoryIndex = 0;

    m_category->setCurrentIndex(categoryIndex);
    populateTagsCombo();

    if (categoryIndex != 0) {
        int tagIndex = m_tag->findText(tag);
        if (tagIndex == -1) {
            m_tag->addItem(tag);
            tagIndex = m_tag->findText(tag);
            Q_ASSERT(tagIndex != -1);
        }
        m_tag->setCurrentIndex(tagIndex);
    }

    m_showUntaggedImagesTag->setChecked(opt->untaggedImagesTagVisible());
}

void Settings::UntaggedGroupBox::saveSettings(Settings::SettingsData *opt)
{
    const QString category = m_category->itemData(m_category->currentIndex()).value<QString>();
    QString untaggedTag = m_tag->currentText().simplified();

    if (!category.isEmpty()) {
        // Add a new tag if the entered one is not in the DB yet
        DB::CategoryPtr categoryPointer = DB::ImageDB::instance()->categoryCollection()->categoryForName(category);
        if (!categoryPointer->items().contains(untaggedTag)) {
            categoryPointer->addItem(untaggedTag);
            QMessageBox::information(this,
                                     i18n("New tag added"),
                                     i18n("<p>The new tag \"%1\" has been added to the category \"%2\" and will be used "
                                          "for untagged images now.</p>"
                                          "<p>Please save now, so that this tag will be stored in the database. "
                                          "Otherwise, it will be lost, and you will get an error about this tag being "
                                          "not present on the next start.</p>",
                                          untaggedTag, category));
        }

        opt->setUntaggedCategory(category);
        opt->setUntaggedTag(untaggedTag);
    } else {
        // If no untagged images tag is selected, remove the setting by using an empty string
        opt->setUntaggedCategory(QString());
        opt->setUntaggedTag(QString());
    }

    opt->setUntaggedImagesTagVisible(m_showUntaggedImagesTag->isChecked());
}

void Settings::UntaggedGroupBox::categoryAdded(const QString &categoryName)
{
    m_category->addItem(categoryName);
}

void Settings::UntaggedGroupBox::categoryDeleted(const QString &categoryName)
{
    if (categoryName == m_category->itemData(m_category->currentIndex()).value<QString>()) {
        m_category->setCurrentIndex(0);
    }

    m_category->removeItem(m_category->findText(categoryName));
}

void Settings::UntaggedGroupBox::categoryRenamed(const QString &oldCategoryName, const QString &newCategoryName)
{
    const int index = m_category->findText(oldCategoryName);
    m_category->setItemText(index, newCategoryName);
    m_category->setItemData(index, newCategoryName);
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_UntaggedGroupBox.cpp"
