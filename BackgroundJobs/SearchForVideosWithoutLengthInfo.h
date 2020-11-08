/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEARCHFORVIDEOSWITHOUTLENGTHINFO_H
#define SEARCHFORVIDEOSWITHOUTLENGTHINFO_H

#include <BackgroundTaskManager/JobInterface.h>

namespace BackgroundJobs
{

/**
  \brief Search for videos for which the database still has no length information
  \see \ref videothumbnails
*/
class SearchForVideosWithoutLengthInfo : public BackgroundTaskManager::JobInterface
{
public:
    SearchForVideosWithoutLengthInfo();
    void execute() override;
    QString title() const override;
    QString details() const override;
};

}

#endif // SEARCHFORVIDEOSWITHOUTLENGTHINFO_H
// vi:expandtab:tabstop=4 shiftwidth=4:
