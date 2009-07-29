/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include <QMenu>
#include <QImage>
#include "DB/ImageDB.h"


namespace MainWindow
{


class CategoryImagePopup :public QMenu {
    Q_OBJECT

public:
    CategoryImagePopup( QWidget* parent );
    void populate( const QImage& image, const QString& imageName );

protected slots:
    void slotExecuteService( QAction* );
    void makeCategoryImage();

private:
    QImage _image;
    DB::ImageInfoPtr _imageInfo;
};

}

#endif /* CATEGORYIMAGEPOPUP_H */

