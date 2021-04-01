// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ExifPage.h"

#include <Exif/TreeView.h>
#include <kpabase/SettingsData.h>
#include <kpaexif/Info.h>

#include <KComboBox>
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextCodec>
#include <QVBoxLayout>

Settings::ExifPage::ExifPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vlay = new QVBoxLayout(this);
    QHBoxLayout *hlay1 = new QHBoxLayout();
    QHBoxLayout *hlay2 = new QHBoxLayout();
    vlay->addLayout(hlay1);
    vlay->addLayout(hlay2);

    m_exifForViewer = new Exif::TreeView(i18n("Exif/IPTC info to show in the viewer"), this);
    hlay1->addWidget(m_exifForViewer);

    m_exifForDialog = new Exif::TreeView(i18n("Exif/IPTC info to show in the Exif dialog"), this);
    hlay1->addWidget(m_exifForDialog);

    QLabel *iptcCharsetLabel = new QLabel(i18n("Character set for image metadata:"), this);
    m_iptcCharset = new KComboBox(this);
    QStringList charsets;
    QList<QByteArray> charsetsBA = QTextCodec::availableCodecs();
    for (QList<QByteArray>::const_iterator it = charsetsBA.constBegin(); it != charsetsBA.constEnd(); ++it)
        charsets << QString::fromLatin1(*it);
    m_iptcCharset->insertItems(m_iptcCharset->count(), charsets);

    hlay2->addStretch(1);
    hlay2->addWidget(iptcCharsetLabel);
    hlay2->addWidget(m_iptcCharset);
}

void Settings::ExifPage::saveSettings(Settings::SettingsData *opt)
{
    opt->setExifForViewer(m_exifForViewer->selected());
    opt->setExifForDialog(m_exifForDialog->selected());
    opt->setIptcCharset(m_iptcCharset->currentText());
}

void Settings::ExifPage::loadSettings(Settings::SettingsData *opt)
{
    m_exifForViewer->reload();
    m_exifForDialog->reload();
    m_exifForViewer->setSelectedExif(Settings::SettingsData::instance()->exifForViewer());
    m_exifForDialog->setSelectedExif(Settings::SettingsData::instance()->exifForDialog());
    m_iptcCharset->setCurrentIndex(qMax(0, QTextCodec::availableCodecs().indexOf(opt->iptcCharset().toLatin1())));
}
// vi:expandtab:tabstop=4 shiftwidth=4:
