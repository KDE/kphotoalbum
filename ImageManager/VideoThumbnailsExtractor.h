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

#ifndef VIDEOTHUMBNAILSEXTRACTOR_H
#define VIDEOTHUMBNAILSEXTRACTOR_H

#include <QObject>
#include <DB/FileName.h>
class QProcess;
class QImage;

namespace Utilities { class Process; }

namespace ImageManager
{

class VideoThumbnailsExtractor :public QObject
{
Q_OBJECT

public:
    VideoThumbnailsExtractor( const DB::FileName& fileName, int videoLength );
    static QString frameName(const DB::FileName& videoName, int frameNumber );

private slots:
    void frameFetched();

signals:
    void frameLoaded(int index, const QImage& image );
    void completed();

private:
    void requestNextFrame();

    Utilities::Process* m_process;
    DB::FileName m_fileName;
    double m_length;
    int m_frameNumber;
};

}

#endif // VIDEOTHUMBNAILSEXTRACTOR_H
