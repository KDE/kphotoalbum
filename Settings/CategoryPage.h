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
#ifndef CATEGORYPAGE_H
#define CATEGORYPAGE_H
#include "SettingsData.h"
#include <QLabel>
#include <QWidget>
class QListWidget;
class QListWidgetItem;
class KPushButton;
class QComboBox;
class QSpinBox;
class KIconButton;
class QLineEdit;

namespace DB { class MemberMap; }

namespace Settings
{
class CategoryItem;
class SettingsDialog;
class UntaggedGroupBox;
class SettingsData;

class CategoryPage :public QWidget
{
    Q_OBJECT
public:
    CategoryPage( QWidget* parent );
    void enableDisable( bool );
    void saveSettings( Settings::SettingsData* opt, DB::MemberMap* memberMap );
    void loadSettings( Settings::SettingsData* opt );

signals:
    void currentCategoryNameChanged( const QString& oldName, const QString& newName );

private slots:
    void edit( QListWidgetItem* );
    void slotLabelChanged( const QString& );
    void slotIconChanged( const QString& );
    void thumbnailSizeChanged( int );
    void slotPreferredViewChanged( int );
    void slotNewItem();
    void slotDeleteCurrent();

private:
    QListWidget* _categories;
    QLabel* _labelLabel;
    QLineEdit* _text;
    QLabel* _iconLabel;
    KIconButton* _icon;
    QLabel* _thumbnailSizeInCategoryLabel;
    QSpinBox* _thumbnailSizeInCategory;
    QLabel* _preferredViewLabel;
    QComboBox* _preferredView;
    KPushButton* _delItem;
    Settings::CategoryItem* _current;
    QList<CategoryItem*> _deleted;
    UntaggedGroupBox* _untaggedBox;
};

}

#endif /* CATEGORYPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
