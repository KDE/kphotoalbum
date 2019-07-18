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
#ifndef DATABASEBACKENDPAGE_H
#define DATABASEBACKENDPAGE_H
#include <QWidget>
class QCheckBox;
class QSpinBox;

namespace Settings
{
class SettingsData;

class DatabaseBackendPage : public QWidget
{
    Q_OBJECT
public:
    explicit DatabaseBackendPage(QWidget *parent);
    void loadSettings(Settings::SettingsData *);
    void saveSettings(Settings::SettingsData *);

private slots:
    void markDirty();

private:
    QSpinBox *m_autosave;
    QSpinBox *m_backupCount;
    QCheckBox *m_compressBackup;
    QCheckBox *m_compressedIndexXML;
};

}

#endif /* DATABASEBACKENDPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
