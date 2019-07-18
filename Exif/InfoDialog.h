/* Copyright (C) 2003-2016 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>

#include "DB/FileName.h"
#include "ImageManager/ImageClientInterface.h"

class QComboBox;
class QLineEdit;
class QLabel;
class QKeyEvent;
class QResizeEvent;

namespace DB
{

class Id;

}

namespace Exif
{

class Grid;

class InfoDialog : public QDialog, public ImageManager::ImageClientInterface
{
    Q_OBJECT

public:
    InfoDialog(const DB::FileName &fileName, QWidget *parent);
    void setImage(const DB::FileName &fileName);

    QSize sizeHint() const override;
    void enterEvent(QEvent *) override;

    // ImageManager::ImageClient interface.
    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;

private:
    QLineEdit *m_searchBox;
    QLabel *m_pix;
    QComboBox *m_iptcCharset;
    Grid *m_grid;
    QLabel *m_fileNameLabel;
};
}

#endif /* INFODIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
