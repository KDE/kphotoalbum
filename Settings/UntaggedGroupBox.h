// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UNTAGGEDGROUPBOX_H
#define UNTAGGEDGROUPBOX_H

#include <QGroupBox>

class QComboBox;
class QCheckBox;

namespace Settings
{
class SettingsData;

class UntaggedGroupBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit UntaggedGroupBox(QWidget *parent = 0);
    void loadSettings(Settings::SettingsData *opt);
    void saveSettings(Settings::SettingsData *opt);
    void categoryAdded(const QString &categoryName);
    void categoryDeleted(const QString &categoryName);
    void categoryRenamed(const QString &oldCategoryName, const QString &newCategoryName);

private Q_SLOTS:
    void populateCategoryComboBox();
    void populateTagsCombo();

private:
    QComboBox *m_category;
    QComboBox *m_tag;
    QCheckBox *m_showUntaggedImagesTag;
};

}

#endif /* UNTAGGEDGROUPBOX_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
