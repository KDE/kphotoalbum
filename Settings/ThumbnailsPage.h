/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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
#ifndef THUMBNAILSPAGE_H
#define THUMBNAILSPAGE_H
#include <QWidget>

class KColorButton;
class QLabel;
class QCheckBox;
class KComboBox;
class QSpinBox;
class QDoubleSpinBox;
namespace Settings
{
class SettingsData;

class ThumbnailsPage : public QWidget
{
public:
    explicit ThumbnailsPage(QWidget *parent);
    void loadSettings(Settings::SettingsData *);
    void saveSettings(Settings::SettingsData *);

private:
    QSpinBox *m_previewSize;
    QSpinBox *m_thumbnailSize;
    KComboBox *m_thumbnailAspectRatio;
    QSpinBox *m_thumbnailSpace;
    QCheckBox *m_thumbnailDisplayGrid;
    QCheckBox *m_displayLabels;
    QCheckBox *m_displayCategories;
    QSpinBox *m_autoShowThumbnailView;
    QCheckBox *m_incrementalThumbnails;
};

}

#endif /* THUMBNAILSPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
