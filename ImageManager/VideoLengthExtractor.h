// SPDX-FileCopyrightText: 2012-2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2025 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
  \brief Attempts to determine the length and creation date/time for a video file.
  \see \ref videothumbnails
*/
class VideoLengthExtractor : public QObject
{
    Q_OBJECT
public:
    explicit VideoLengthExtractor(QObject *parent = nullptr);
    void extract(const DB::FileName &fileName);

Q_SIGNALS:
    // These are emitted when valid metadata is extracted.
    void creationTimeFound(QDateTime dateTime);
    void lengthFound(int length);

    // These are emitted when valid metadata cannot be extracted.
    void unableToDetermineCreationTime();
    void unableToDetermineLength();

private Q_SLOTS:
    void processEnded();

private:
    Utilities::Process *m_process;
    DB::FileName m_fileName;
};

}

#endif // VIDEOLENGTHEXTRACTOR_H
// vi:expandtab:tabstop=4 shiftwidth=4:
