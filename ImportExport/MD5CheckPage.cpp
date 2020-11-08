/* SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "MD5CheckPage.h"

#include <DB/ImageDB.h>
#include <DB/MD5Map.h>

#include <KLocalizedString>
#include <QButtonGroup>
#include <QFrame>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

ImportExport::ClashInfo::ClashInfo(const QStringList &categories)
    : label(false)
    , description(false)
    , orientation(false)
    , date(false)
{
    for (const QString &category : categories)
        this->categories[category] = false;
}

bool ImportExport::MD5CheckPage::pageNeeded(const ImportSettings &settings)
{
    if (countOfMD5Matches(settings) != 0 && clashes(settings).anyClashes())
        return true;

    return false;
}

ImportExport::MD5CheckPage::MD5CheckPage(const ImportSettings &settings)
{
    QVBoxLayout *vlay = new QVBoxLayout(this);

    const QString txt = i18np("One image from the import file, has the same MD5 sum as an image in the Database, how should that be resolved?",
                              "%1 images from the import file, have the same MD5 sum as images in the Database, how should that be resolved?",
                              countOfMD5Matches(settings));
    QLabel *label = new QLabel(txt);
    label->setWordWrap(true);

    vlay->addWidget(label);

    QGridLayout *grid = new QGridLayout;
    grid->setHorizontalSpacing(0);
    vlay->addLayout(grid);

    int row = -1;

    // Titles
    label = new QLabel(i18n("Use data from\nImport File"));
    grid->addWidget(label, ++row, 1);

    label = new QLabel(i18n("Use data from\nDatabase"));
    grid->addWidget(label, row, 2);

    label = new QLabel(i18n("Merge data"));
    grid->addWidget(label, row, 3);

    ClashInfo clashes = this->clashes(settings);
    createRow(grid, row, QString::fromLatin1("*Label*"), i18n("Label"), clashes.label, false);
    createRow(grid, row, QString::fromLatin1("*Description*"), i18n("Description"), clashes.description, true);
    createRow(grid, row, QString::fromLatin1("*Orientation*"), i18n("Orientation"), clashes.orientation, false);
    createRow(grid, row, QString::fromLatin1("*Date*"), i18n("Date and Time"), clashes.date, false);
    for (QMap<QString, bool>::const_iterator it = clashes.categories.constBegin(); it != clashes.categories.constEnd(); ++it) {
        createRow(grid, row, it.key(), it.key(), *it, true);
    }

    vlay->addStretch(1);
}

/**
 * Return the number of images in the import set which has the same MD5 sum as those from the DB.
 */
int ImportExport::MD5CheckPage::countOfMD5Matches(const ImportSettings &settings)
{
    int count = 0;
    DB::ImageInfoList list = settings.selectedImages();
    for (DB::ImageInfoPtr info : list) {
        if (DB::ImageDB::instance()->md5Map()->contains(info->MD5Sum()))
            ++count;
    }
    return count;
}

ImportExport::ClashInfo ImportExport::MD5CheckPage::clashes(const ImportSettings &settings)
{
    QStringList myCategories;
    const auto categoryMatchSettings = settings.categoryMatchSetting();
    for (const CategoryMatchSetting &matcher : categoryMatchSettings) {
        myCategories.append(matcher.DBCategoryName());
    }

    ClashInfo res(myCategories);
    const DB::ImageInfoList list = settings.selectedImages();
    for (const DB::ImageInfoPtr &info : list) {
        if (!DB::ImageDB::instance()->md5Map()->contains(info->MD5Sum()))
            continue;

        const DB::FileName name = DB::ImageDB::instance()->md5Map()->lookup(info->MD5Sum());
        DB::ImageInfoPtr other = DB::ImageDB::instance()->info(name);
        if (info->label() != other->label())
            res.label = true;
        if (info->description() != other->description())
            res.description = true;

        if (info->angle() != other->angle())
            res.orientation = true;

        if (info->date() != other->date())
            res.date = true;

        const auto categoryMatchSettings = settings.categoryMatchSetting();
        for (const CategoryMatchSetting &matcher : categoryMatchSettings) {
            const QString XMLFileCategory = matcher.XMLCategoryName();
            const QString DBCategory = matcher.DBCategoryName();
            if (mapCategoriesToDB(matcher, info->itemsOfCategory(XMLFileCategory)) != other->itemsOfCategory(DBCategory))
                res.categories[DBCategory] = true;
        }
    }
    return res;
}

bool ImportExport::ClashInfo::anyClashes()
{
    if (label || description || orientation || date)
        return true;

    for (QMap<QString, bool>::ConstIterator categoryIt = categories.constBegin(); categoryIt != categories.constEnd(); ++categoryIt) {
        if (categoryIt.value())
            return true;
    }

    return false;
}

void ImportExport::MD5CheckPage::createRow(QGridLayout *layout, int &row, const QString &name, const QString &title, bool anyClashes, bool allowMerge)
{
    if (row % 3 == 0) {
        QFrame *line = new QFrame;
        line->setFrameShape(QFrame::HLine);
        layout->addWidget(line, ++row, 0, 1, 4);
    }

    QLabel *label = new QLabel(title);
    label->setEnabled(anyClashes);
    layout->addWidget(label, ++row, 0);

    QButtonGroup *group = new QButtonGroup(this);
    m_groups[name] = group;

    for (int i = 1; i < 4; ++i) {
        if (i == 3 && !allowMerge)
            continue;

        QRadioButton *rb = new QRadioButton;
        layout->addWidget(rb, row, i);
        group->addButton(rb, i);
        rb->setEnabled(anyClashes);
        if (i == 1)
            rb->setChecked(true);
    }
}

Utilities::StringSet ImportExport::MD5CheckPage::mapCategoriesToDB(const CategoryMatchSetting &matcher, const Utilities::StringSet &items)
{
    Utilities::StringSet res;

    for (const QString &item : items) {
        if (matcher.XMLtoDB().contains(item))
            res.insert(matcher.XMLtoDB()[item]);
    }
    return res;
}

QMap<QString, ImportExport::ImportSettings::ImportAction> ImportExport::MD5CheckPage::settings()
{
    QMap<QString, ImportSettings::ImportAction> res;
    for (QMap<QString, QButtonGroup *>::iterator it = m_groups.begin(); it != m_groups.end(); ++it) {
        res.insert(it.key(), static_cast<ImportSettings::ImportAction>(it.value()->checkedId()));
    }
    return res;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
