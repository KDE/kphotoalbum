// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "CategoryImageConfig.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <DB/MemberMap.h>
#include <Utilities/FileUtil.h>
#include <kpabase/SettingsData.h>

#include <KComboBox>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

using Utilities::StringSet;

Viewer::CategoryImageConfig *Viewer::CategoryImageConfig::s_instance = nullptr;

Viewer::CategoryImageConfig::CategoryImageConfig()
    : m_image(QImage())
{
    setWindowTitle(i18nc("@title:window", "Configure Category Image"));
    QWidget *top = new QWidget;

    QVBoxLayout *lay1 = new QVBoxLayout(top);
    setLayout(lay1);
    QGridLayout *lay2 = new QGridLayout;
    lay1->addLayout(lay2);

    // Group
    QLabel *label = new QLabel(i18nc("@label:listbox As in 'select the tag category'", "Category:"), top);
    lay2->addWidget(label, 0, 0);
    m_group = new KComboBox(top);
    lay2->addWidget(m_group, 0, 1);
    connect(m_group, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &CategoryImageConfig::groupChanged);

    // Member
    label = new QLabel(i18nc("@label:listbox As in 'select a tag'", "Tag:"), top);
    lay2->addWidget(label, 1, 0);
    m_member = new KComboBox(top);
    lay2->addWidget(m_member, 1, 1);
    connect(m_member, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &CategoryImageConfig::memberChanged);

    // Current Value
    QGridLayout *lay3 = new QGridLayout;
    lay1->addLayout(lay3);
    label = new QLabel(i18nc("@label The current category image", "Current image:"), top);
    lay3->addWidget(label, 0, 0);

    m_current = new QLabel(top);
    m_current->setFixedSize(128, 128);
    lay3->addWidget(m_current, 0, 1);

    // New Value
    m_imageLabel = new QLabel(i18nc("@label Preview of the new category imape", "New image:"), top);
    lay3->addWidget(m_imageLabel, 1, 0);

    m_imageLabel = new QLabel(top);
    m_imageLabel->setFixedSize(128, 128);
    lay3->addWidget(m_imageLabel, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QPushButton *user1Button = new QPushButton;
    user1Button->setText(i18nc("@action:button As in 'Set the category image'", "Set"));
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    connect(user1Button, &QPushButton::clicked, this, &CategoryImageConfig::slotSet);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CategoryImageConfig::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CategoryImageConfig::reject);
    lay1->addWidget(buttonBox);
}

void Viewer::CategoryImageConfig::groupChanged()
{
    QString categoryName = currentGroup();
    if (categoryName.isNull())
        return;

    QString currentText = m_member->currentText();
    m_member->clear();
    StringSet directMembers = m_info->itemsOfCategory(categoryName);

    StringSet set = directMembers;
    QMap<QString, StringSet> map = DB::ImageDB::instance()->memberMap().inverseMap(categoryName);
    for (auto directMembersIt = directMembers.begin(); directMembersIt != directMembers.end(); ++directMembersIt) {
        set += map[*directMembersIt];
    }

    QStringList list(set.begin(), set.end());

    list.sort();
    m_member->addItems(list);
    int index = list.indexOf(currentText);
    if (index != -1)
        m_member->setCurrentIndex(index);

    memberChanged();
}

void Viewer::CategoryImageConfig::memberChanged()
{
    QString categoryName = currentGroup();
    if (categoryName.isNull())
        return;
    QPixmap pix = DB::ImageDB::instance()->categoryCollection()->categoryForName(categoryName)->categoryImage(categoryName, m_member->currentText(), 128, 128);
    m_current->setPixmap(pix);
}

void Viewer::CategoryImageConfig::slotSet()
{
    QString categoryName = currentGroup();
    if (categoryName.isNull())
        return;
    DB::ImageDB::instance()->categoryCollection()->categoryForName(categoryName)->setCategoryImage(categoryName, m_member->currentText(), m_image);
    memberChanged();
}

QString Viewer::CategoryImageConfig::currentGroup()
{
    int index = m_group->currentIndex();
    if (index == -1)
        return QString();
    return m_categoryNames[index];
}

void Viewer::CategoryImageConfig::setCurrentImage(const QImage &image, const DB::ImageInfoPtr &info)
{
    m_image = image;
    m_imageLabel->setPixmap(QPixmap::fromImage(image));
    m_info = info;
    groupChanged();
}

Viewer::CategoryImageConfig *Viewer::CategoryImageConfig::instance()
{
    if (!s_instance)
        s_instance = new CategoryImageConfig();
    return s_instance;
}

void Viewer::CategoryImageConfig::show()
{
    QString currentCategory = m_group->currentText();
    m_group->clear();
    m_categoryNames.clear();
    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    int index = 0;
    int currentIndex = -1;
    for (QList<DB::CategoryPtr>::ConstIterator categoryIt = categories.constBegin(); categoryIt != categories.constEnd(); ++categoryIt) {
        if (!(*categoryIt)->isSpecialCategory()) {
            m_group->addItem((*categoryIt)->name());
            m_categoryNames.push_back((*categoryIt)->name());
            if ((*categoryIt)->name() == currentCategory)
                currentIndex = index;
            ++index;
        }
    }

    if (currentIndex != -1)
        m_group->setCurrentIndex(currentIndex);
    groupChanged();

    QDialog::show();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_CategoryImageConfig.cpp"
