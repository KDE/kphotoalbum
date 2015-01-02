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

#ifndef DETECTOR_H
#define DETECTOR_H

// Qt includes
#include <QObject>
#include <QList>
#include <QRect>
#include <QVariant>

// Qt classes
class QImage;

namespace Settings
{

// Local classes
class SettingsData;

}

namespace KFaceIface
{

// KFaceIface classes
class FaceDetector;

}

namespace FaceManagement
{

class Detector : public QObject
{
    Q_OBJECT

public:
    static FaceManagement::Detector* instance();
    Detector();
    ~Detector();
    QList<QRect> detectFaces(QImage& image);

private: // Variables
    static FaceManagement::Detector *s_instance;
    Settings::SettingsData *m_settingsData;
    QVariantMap m_params;
    KFaceIface::FaceDetector *m_faceDetector;

private: // Functions
    void updateSettings();
};

}

#endif // DETECTOR_H

// vi:expandtab:tabstop=4 shiftwidth=4:
