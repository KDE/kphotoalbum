// SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ReReadDialog.h"

#include "Database.h"

#include <DB/ImageDB.h>
#include <kpabase/SettingsData.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

Exif::ReReadDialog::ReReadDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Read Exif Info from Files"));

    QWidget *top = new QWidget;
    QVBoxLayout *lay1 = new QVBoxLayout(top);
    setLayout(lay1);
    lay1->addWidget(top);

    m_exifDB = new QCheckBox(i18n("Update Exif search database"), top);
    lay1->addWidget(m_exifDB);
    if (!DB::ImageDB::instance()->exifDB()->isUsable()) {
        m_exifDB->hide();
    }

    m_date = new QCheckBox(i18n("Update image date"), top);
    lay1->addWidget(m_date);

    m_force_date = new QCheckBox(i18n("Use modification date if Exif not found"), top);
    lay1->addWidget(m_force_date);

    m_orientation = new QCheckBox(i18n("Update image orientation from Exif information"), top);
    lay1->addWidget(m_orientation);

    m_description = new QCheckBox(i18n("Update image description from Exif information"), top);
    lay1->addWidget(m_description);

    QGroupBox *box = new QGroupBox(i18n("Affected Files"));
    lay1->addWidget(box);

    QHBoxLayout *boxLayout = new QHBoxLayout(box);
    m_fileList = new QListWidget;
    m_fileList->setSelectionMode(QAbstractItemView::NoSelection);
    boxLayout->addWidget(m_fileList);

    connect(m_date, &QCheckBox::toggled, m_force_date, &QCheckBox::setEnabled);
    connect(m_date, &QCheckBox::toggled, this, &ReReadDialog::warnAboutDates);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ReReadDialog::readInfo);
    lay1->addWidget(buttonBox);
}

int Exif::ReReadDialog::exec(const DB::FileNameList &list)
{
    Settings::SettingsData *opt = Settings::SettingsData::instance();

    m_exifDB->setChecked(opt->updateExifData());
    m_date->setChecked(opt->updateImageDate());
    m_force_date->setChecked(opt->useModDateIfNoExif());
    m_force_date->setEnabled(opt->updateImageDate());
    m_orientation->setChecked(opt->updateOrientation());
    m_description->setChecked(opt->updateDescription());

    m_list = list;
    m_fileList->clear();
    m_fileList->addItems(list.toStringList(DB::RelativeToImageRoot));

    return QDialog::exec();
}

void Exif::ReReadDialog::readInfo()
{
    Settings::SettingsData *opt = Settings::SettingsData::instance();

    opt->setUpdateExifData(m_exifDB->isChecked());
    opt->setUpdateImageDate(m_date->isChecked());
    opt->setUseModDateIfNoExif(m_force_date->isChecked());
    opt->setUpdateOrientation(m_orientation->isChecked());
    opt->setUpdateDescription(m_description->isChecked());

    KSharedConfig::openConfig()->sync();

    DB::ExifMode mode = DB::EXIFMODE_FORCE;

    if (m_exifDB->isChecked())
        mode |= DB::EXIFMODE_DATABASE_UPDATE;

    if (m_date->isChecked())
        mode |= DB::EXIFMODE_DATE;
    if (m_force_date->isChecked())
        mode |= DB::EXIFMODE_USE_IMAGE_DATE_IF_INVALID_EXIF_DATE;
    if (m_orientation->isChecked())
        mode |= DB::EXIFMODE_ORIENTATION;
    if (m_description->isChecked())
        mode |= DB::EXIFMODE_DESCRIPTION;

    accept();
    DB::ImageDB::instance()->slotReread(m_list, mode);
}

void Exif::ReReadDialog::warnAboutDates(bool b)
{
    if (!b)
        return;

    int ret = KMessageBox::warningContinueCancel(this, i18n("<p>Be aware that setting the data from Exif may "
                                                            "<b>overwrite</b> data you have previously entered "
                                                            "manually using the image configuration dialog.</p>"),
                                                 i18n("Override image dates"));
    if (ret == KMessageBox::Cancel)
        m_date->setChecked(false);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
