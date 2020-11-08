/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEARCHFORVIDEOSWITHOUTVIDEOTHUMBNAILSJOB_H
#define SEARCHFORVIDEOSWITHOUTVIDEOTHUMBNAILSJOB_H

#include <BackgroundTaskManager/JobInterface.h>

namespace BackgroundJobs
{

/**
  \brief Search for videos without a thumbnail needed for cycling thumbnails.
  \see \ref videothumbnails
*/
class SearchForVideosWithoutVideoThumbnailsJob : public BackgroundTaskManager::JobInterface
{
    Q_OBJECT

public:
    SearchForVideosWithoutVideoThumbnailsJob();
    void execute() override;
    QString title() const override;
    QString details() const override;
};

}

#endif // SEARCHFORVIDEOSWITHOUTVIDEOTHUMBNAILSJOB_H
// vi:expandtab:tabstop=4 shiftwidth=4:
