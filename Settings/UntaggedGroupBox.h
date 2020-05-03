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
    void categoryDeleted(QString categoryName);
    void categoryRenamed(QString oldCategoryName, QString newCategoryName);

private slots:
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
