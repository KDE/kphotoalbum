/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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
#ifndef FILEVERSIONDETECTIONPAGE_H
#define FILEVERSIONDETECTIONPAGE_H
#include <QWidget>

class KComboBox;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QCheckBox;
namespace Settings
{
class SettingsData;

class FileVersionDetectionPage : public QWidget
{
public:
    explicit FileVersionDetectionPage(QWidget *parent);
    ~FileVersionDetectionPage() override;
    void loadSettings(Settings::SettingsData *);
    void saveSettings(Settings::SettingsData *);

private slots:
    /**
     * @brief Enable/disable UI elements for manual optimization settings based on
     * the active m_loadOptimizationPreset combobox value.
     */
    void slotUpdateOptimizationUI();

private:
    QCheckBox *m_searchForImagesOnStart;
    QCheckBox *m_ignoreFileExtension;
    QCheckBox *m_skipSymlinks;
    QCheckBox *m_skipRawIfOtherMatches;
    QLineEdit *m_excludeDirectories; // Directories to exclude
    QCheckBox *m_detectModifiedFiles;
    QLineEdit *m_modifiedFileComponent;
    QLineEdit *m_originalFileComponent;
    QCheckBox *m_moveOriginalContents;
    QCheckBox *m_autoStackNewFiles;
    QLineEdit *m_copyFileComponent;
    QLineEdit *m_copyFileReplacementComponent;
    KComboBox *m_loadOptimizationPreset;
    QCheckBox *m_overlapLoadMD5;
    QSpinBox *m_preloadThreadCount;
    QSpinBox *m_thumbnailPreloadThreadCount;
    QSpinBox *m_thumbnailBuilderThreadCount;
};

}

#endif /* FILEVERSIONDETECTIONPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
