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

#ifndef CATEGORYPAGE_H
#define CATEGORYPAGE_H

// Qt includes
#include <QAbstractItemDelegate>
#include <QLabel>
#include <QWidget>

// Local includes
#include "SettingsData.h"

// Qt classes
class QListWidget;
class QListWidgetItem;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QGroupBox;

// KDE classes
class KIconButton;

namespace DB
{

// Local classes
class MemberMap;

}

namespace Settings
{

// Local classes
class CategoryItem;
class SettingsDialog;
class UntaggedGroupBox;
class SettingsData;

class CategoryPage : public QWidget
{
    Q_OBJECT

public:
    explicit CategoryPage(QWidget *parent);
    void enableDisable(bool);
    void saveSettings(Settings::SettingsData *opt, DB::MemberMap *memberMap);
    void loadSettings(Settings::SettingsData *opt);
    void resetInterface();
    void resetCategoryNamesChanged();

signals:
    void categoryChangesPending();

protected slots:
    friend class SettingsDialog;
    void resetCategoryLabel();

private slots:
    void editSelectedCategory();
    void editCategory(QListWidgetItem *);
    void positionableChanged(bool);
    void iconChanged(const QString &icon);
    void thumbnailSizeChanged(int);
    void preferredViewChanged(int);
    void newCategory();
    void deleteCurrentCategory();
    void renameCurrentCategory();
    void categoryNameChanged(QListWidgetItem *item);
    void saveDbNow();

private: // Functions
    void resetCategory(QListWidgetItem *item);

private: // Variables
    QListWidget *m_categoriesListWidget;
    QLabel *m_categoryLabel;
    QLabel *m_renameLabel;
    QLabel *m_positionableLabel;
    QCheckBox *m_positionable;
    QLabel *m_iconLabel;
    KIconButton *m_icon;
    QLabel *m_thumbnailSizeInCategoryLabel;
    QSpinBox *m_thumbnailSizeInCategory;
    QLabel *m_preferredViewLabel;
    QComboBox *m_preferredView;
    QPushButton *m_delItem;
    QPushButton *m_renameItem;
    Settings::CategoryItem *m_currentCategory;
    QList<CategoryItem *> m_deletedCategories;
    UntaggedGroupBox *m_untaggedBox;
    QString m_categoryNameBeforeEdit;
    QLabel *m_dbNotSavedLabel;
    QPushButton *m_saveDbNowButton;
    bool m_categoryNamesChanged;
    QPushButton *m_newCategoryButton;
};

}

#endif // CATEGORYPAGE_H

// vi:expandtab:tabstop=4 shiftwidth=4:
