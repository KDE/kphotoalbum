/* Copyright (C) 2014 Tobias Leupold <tobias.leupold@web.de>

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

#ifndef FACEMANAGEMENTPAGE_H
#define FACEMANAGEMENTPAGE_H

// Qt includes
#include <QWidget>

// Local includes
#include <FaceManagement/Recognizer.h>

// Qt classes
class QPushButton;
class QSlider;
class QTreeWidget;

// KDE classes
class KPageWidgetItem;

namespace KFaceIface {

// KFaceIface classes
class Identity;

};

namespace Settings
{

// Local classes
class SettingsData;

class FaceManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit FaceManagementPage(QWidget* parent);
    ~FaceManagementPage();
    void clearDatabaseEntries();
    void loadSettings(Settings::SettingsData*);
    void saveSettings(Settings::SettingsData*);

public slots:
    void slotPageChange(KPageWidgetItem* page);

private slots:
    void slotEraseDatabase();
    void slotDeleteSelected();
    void checkSelection();

private: // Functions
    void loadDatabase();
    void setSelection(bool state);
    void deleteIdentities(QList<KFaceIface::Identity> identitiesToDelete);

private: // Variables
    QSlider* m_speedSlider;
    QSlider* m_sensitivitySlider;
    QTreeWidget* m_databaseEntries;
    QPushButton* m_deleteSelectedButton;
    FaceManagement::Recognizer* m_recognizer;
};

}

#endif // FACEMANAGEMENTPAGE_H

// vi:expandtab:tabstop=4 shiftwidth=4:
