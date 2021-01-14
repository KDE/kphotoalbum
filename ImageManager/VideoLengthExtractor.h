/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef VIDEOLENGTHEXTRACTOR_H
#define VIDEOLENGTHEXTRACTOR_H

#include <kpabase/FileName.h>

#include <QObject>

namespace Utilities
{
class Process;
}

namespace ImageManager
{

/**
  \brief \todo
  \see \ref videothumbnails
*/
class VideoLengthExtractor : public QObject
{
    Q_OBJECT
public:
    explicit VideoLengthExtractor(QObject *parent = nullptr);
    void extract(const DB::FileName &fileName);

signals:
    void lengthFound(int length);
    void unableToDetermineLength();

private slots:
    void processEnded();

private:
    Utilities::Process *m_process;
    DB::FileName m_fileName;
};

}

#endif // VIDEOLENGTHEXTRACTOR_H
// vi:expandtab:tabstop=4 shiftwidth=4:
