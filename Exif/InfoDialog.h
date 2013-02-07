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
#ifndef EXIFDIALOG_H
#define EXIFDIALOG_H
#include <KDialog>
#include "Utilities/Set.h"
#include "ImageManager/ImageClientInterface.h"
#include <DB/FileName.h>
class KComboBox;
class QLabel;
class QKeyEvent;
class QResizeEvent;

namespace DB { class Id; }

namespace Exif
{
class Grid;

class InfoDialog : public KDialog, public ImageManager::ImageClientInterface {
    Q_OBJECT

public:
    InfoDialog( const DB::FileName& fileName, QWidget* parent );
    void setImage( const DB::FileName& fileName );

    OVERRIDE QSize sizeHint() const;
    OVERRIDE void enterEvent( QEvent* );

    // ImageManager::ImageClient interface.
    OVERRIDE void pixmapLoaded( const DB::FileName& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, const bool loadedOK);

protected slots:
    void updateSearchString( const QString& );

private:
    QLabel* m_searchLabel;
    QLabel* m_pix;
    KComboBox* m_iptcCharset;
    Grid* m_grid;
    QLabel* m_fileNameLabel;
};

}

#endif /* EXIFDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
