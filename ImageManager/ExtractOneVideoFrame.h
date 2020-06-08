/* Copyright 2012-2020 The KPhotoAlbum Development Team

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

#ifndef IMAGEMANAGER_EXTRACTONEVIDEOFRAME_H
#define IMAGEMANAGER_EXTRACTONEVIDEOFRAME_H

#include <DB/FileName.h>

#include <QObject>
#include <QProcess>

class QImage;
class QTemporaryDir;

namespace Utilities
{
class Process;
}

namespace ImageManager
{

/**
  \brief Extract a thumbnail given a filename and offset.
  \see \ref videothumbnails
*/
class ExtractOneVideoFrame : public QObject
{
    Q_OBJECT
public:
    static void extract(const DB::FileName &filename, double offset, QObject *receiver, const char *slot);

private slots:
    /**
     * @brief processFinished call the appropriate handler function based on exit status.
     * @param exitCode
     * @param status
     */
    void processFinished(int exitCode, QProcess::ExitStatus status);

signals:
    void result(const QImage &);

private:
    void frameFetched();
    void handleError(QProcess::ProcessError);
    ExtractOneVideoFrame(const DB::FileName &filename, double offset, QObject *receiver, const char *slot);
    void markShortVideo(const DB::FileName &fileName);

    QTemporaryDir *m_workingDirectory;
    Utilities::Process *m_process;
    DB::FileName m_fileName;
    static QString s_tokenForShortVideos;
};

} // namespace ImageManager

#endif // IMAGEMANAGER_EXTRACTONEVIDEOFRAME_H
// vi:expandtab:tabstop=4 shiftwidth=4:
