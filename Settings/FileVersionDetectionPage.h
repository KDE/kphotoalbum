// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

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

private Q_SLOTS:
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
