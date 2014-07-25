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

#ifndef COPYPOPUP_H
#define COPYPOPUP_H
#include <QMenu>
#include "DB/ImageInfo.h"
#include "DB/FileNameList.h"

namespace DB
{
    class ImageInfo;
}

namespace MainWindow
{

class CopyPopup : public QMenu {
    Q_OBJECT

public:
    explicit CopyPopup(
        QWidget *parent = 0,
        DB::ImageInfoPtr current = DB::ImageInfoPtr(),
        DB::FileNameList imageList = DB::FileNameList()
    );
    ~CopyPopup();

private slots:
    void slotCopy(QAction *action);

private:
    DB::FileNameList m_list;
    DB::ImageInfoPtr m_currentInfo;
};

}

#endif /* COPYPOPUP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
