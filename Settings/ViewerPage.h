/* Copyright (C) 2003-2019 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef VIEWERPAGE_H
#define VIEWERPAGE_H
#include <QWidget>

class KComboBox;
class QSpinBox;
class QComboBox;
class QCheckBox;
namespace Settings
{
class SettingsData;
class ViewerSizeConfig;

class ViewerPage : public QWidget
{
public:
    explicit ViewerPage(QWidget *parent);
    void loadSettings(Settings::SettingsData *);
    void saveSettings(Settings::SettingsData *);

private:
    Settings::ViewerSizeConfig *m_slideShowSetup;
    Settings::ViewerSizeConfig *m_viewImageSetup;
    QComboBox *m_smoothScale;
    QSpinBox *m_slideShowInterval;
    QSpinBox *m_cacheSize;
    KComboBox *m_viewerStandardSize;
    QCheckBox *m_useExternalViewer;
};

}

#endif /* VIEWERPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
