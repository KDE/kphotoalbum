/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MAINWINDOW_UPDATEVIDEOTHUMBNAIL_H
#define MAINWINDOW_UPDATEVIDEOTHUMBNAIL_H

#include <DB/FileNameList.h>

namespace MainWindow
{

namespace UpdateVideoThumbnail
{
    void useNext(const DB::FileNameList &);
    void usePrevious(const DB::FileNameList &);
}

} // namespace MainWindow

#endif // MAINWINDOW_UPDATEVIDEOTHUMBNAIL_H
// vi:expandtab:tabstop=4 shiftwidth=4:
