/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
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
