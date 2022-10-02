// SPDX-FileCopyrightText: 2012-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef IMAGEMANAGER_EXTRACTONEVIDEOFRAME_H
#define IMAGEMANAGER_EXTRACTONEVIDEOFRAME_H

#include <kpabase/FileName.h>

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

private Q_SLOTS:
    /**
     * @brief processFinished call the appropriate handler function based on exit status.
     * @param exitCode
     * @param status
     */
    void processFinished(int exitCode, QProcess::ExitStatus status);

Q_SIGNALS:
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
