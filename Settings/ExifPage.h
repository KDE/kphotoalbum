// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIFPAGE_H
#define EXIFPAGE_H

#include <QComboBox>
#include <QWidget>
#include <QStringList>

namespace Exif
{
class TreeView;
}

namespace Settings
{
class SettingsData;

class ExifPage : public QWidget
{
public:
    explicit ExifPage(QWidget *parent);
    void loadSettings(Settings::SettingsData *);
    void saveSettings(Settings::SettingsData *);

private:
    const QStringList m_availableCodecs;
    Exif::TreeView *m_exifForViewer;
    Exif::TreeView *m_exifForDialog;
    QComboBox *m_iptcCharset;
};

}

#endif /* EXIFPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
