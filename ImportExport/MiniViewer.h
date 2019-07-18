/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef MINIVIEWER_H
#define MINIVIEWER_H

#include "DB/ImageInfoPtr.h"
#include <qdialog.h>
#include <qimage.h>

class QCloseEvent;
class QLabel;

namespace DB
{
class ImageInfo;
}

namespace ImportExport
{

class MiniViewer : public QDialog
{
    Q_OBJECT

public:
    static void show(QImage img, DB::ImageInfoPtr info, QWidget *parent = nullptr);
    void closeEvent(QCloseEvent *event) override;

protected slots:
    void slotClose();

private:
    explicit MiniViewer(QWidget *parent = nullptr);
    static MiniViewer *s_instance;
    QLabel *m_pixmap;
};

}

#endif /* MINIVIEWER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
