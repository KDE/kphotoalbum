/* Copyright 2012  Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "VideoThumbnailsExtractor.h"
#include <Utilities/Process.h>
#include <QTextStream>
#include <QImage>
#include <QDir>
#include <MainWindow/FeatureDialog.h>
#include "VideoManager.h"

#define STR(x) QString::fromUtf8(x)

ImageManager::VideoThumbnailsExtractor::VideoThumbnailsExtractor( const DB::FileName& fileName, int videoLength )
    :m_fileName(fileName), m_length(videoLength)
{
    m_process = new Utilities::Process(this);
    m_process->setWorkingDirectory(QDir::tempPath());
    connect( m_process, SIGNAL(finished(int)), this, SLOT(frameFetched()));

    m_frameNumber = -1;
    requestNextFrame();
}

void ImageManager::VideoThumbnailsExtractor::requestNextFrame()
{
    m_frameNumber++;
    if ( m_frameNumber == 10 ) {
        emit completed();
        return;
    }

    const double offset = m_length * m_frameNumber / 10;
    QStringList arguments;
    arguments << STR("-nosound") << STR("-ss") << QString::number(offset,'g',2) << STR("-vf")
              << STR("screenshot") << STR("-frames") << STR("1") << STR("-vo") << STR("png:z=9") << m_fileName.absolute();

    m_process->start(MainWindow::FeatureDialog::mplayerBinary(), arguments);
}

void ImageManager::VideoThumbnailsExtractor::frameFetched()
{
    QImage image(QDir::tempPath() + STR("/00000001.png"));
    image.save(frameName(m_fileName.absolute(),m_frameNumber),"JPEG"); // ZZZ
    emit frameLoaded(m_frameNumber, image);
    requestNextFrame();
}

QString ImageManager::VideoThumbnailsExtractor::frameName(const QString &videoName, int frameNumber)
{
    return ImageManager::VideoManager::pathForRequest(videoName) + QLatin1String("-") + QString::number(frameNumber);
}

