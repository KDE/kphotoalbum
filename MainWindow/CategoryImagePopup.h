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

#ifndef CATEGORYIMAGEPOPUP_H
#define CATEGORYIMAGEPOPUP_H
#include <DB/ImageDB.h>

#include <QImage>
#include <QMenu>

namespace MainWindow
{

class CategoryImagePopup : public QMenu
{
    Q_OBJECT

public:
    explicit CategoryImagePopup(QWidget *parent);
    void populate(const QImage &image, const DB::FileName &imageName);

protected slots:
    void slotExecuteService(QAction *);
    void makeCategoryImage();

private:
    QImage m_image;
    DB::ImageInfoPtr m_imageInfo;
};
}

#endif /* CATEGORYIMAGEPOPUP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
