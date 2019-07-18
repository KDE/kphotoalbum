/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef EXIFPAGE_H
#define EXIFPAGE_H
#include <QComboBox>
#include <QWidget>

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
    Exif::TreeView *m_exifForViewer;
    Exif::TreeView *m_exifForDialog;
    QComboBox *m_iptcCharset;
};

}

#endif /* EXIFPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
