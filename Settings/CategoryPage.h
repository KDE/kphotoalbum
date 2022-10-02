// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CATEGORYPAGE_H
#define CATEGORYPAGE_H

#include <kpabase/SettingsData.h>

#include <QAbstractItemDelegate>
#include <QLabel>
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QGroupBox;
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

Q_SIGNALS:
    void categoryChangesPending();

protected Q_SLOTS:
    friend class SettingsDialog;
    void resetCategoryLabel();

private Q_SLOTS:
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
