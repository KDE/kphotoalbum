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

#include "Detector.h"

#include "config-kpa-kface.h"
#ifdef HAVE_KFACE
#include "Settings/SettingsData.h"
#include <libkface/facedetector.h>
#endif

using namespace KFaceIface;

FaceManagement::Detector *FaceManagement::Detector::m_instance = nullptr;

FaceManagement::Detector * FaceManagement::Detector::instance()
{
    if (! m_instance) {
        m_instance = new FaceManagement::Detector();
    }

    m_instance->updateSettings();

    return m_instance;
}

FaceManagement::Detector::Detector()
{
#ifdef HAVE_KFACE
    m_faceDetector = new FaceDetector();
    m_settingsData = Settings::SettingsData::instance();
    updateSettings();
#endif
}

FaceManagement::Detector::~Detector()
{
#ifdef HAVE_KFACE
    delete m_faceDetector;
#endif
}

void FaceManagement::Detector::updateSettings()
{
#ifdef HAVE_KFACE
    m_params[QString::fromLatin1("accuracy")] = m_settingsData->faceDetectionAccuracy();
    m_params[QString::fromLatin1("specificity")] = float(m_settingsData->faceDetectionSensitivity()) / 100;
    m_faceDetector->setParameters(m_params);
#endif
}

QList<QRect> FaceManagement::Detector::detectFaces(QImage &image)
{
#ifdef HAVE_KFACE
    QSize imageSize = image.size();
    QList<QRectF> faces = m_faceDetector->detectFaces(image);
    QList<QRect> mappedFaces;

    for (int i = 0; i < faces.size(); ++i) {
        mappedFaces << FaceDetector::toAbsoluteRect(faces.at(i), imageSize);
    }

    return mappedFaces;
#else
    Q_UNUSED(image);
    return QList<QRect>();
#endif
}

// vi:expandtab:tabstop=4 shiftwidth=4:
