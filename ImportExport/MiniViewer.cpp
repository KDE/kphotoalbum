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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "MiniViewer.h"
#include <qpushbutton.h>
#include <klocale.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qimage.h>
#include "DB/ImageInfo.h"
#include <qwmatrix.h>

using namespace ImportExport;

MiniViewer* MiniViewer::_instance = 0;

void MiniViewer::show( QImage img, DB::ImageInfoPtr info )
{
    if ( !_instance )
        _instance = new MiniViewer();

    if ( info->angle() != 0 ) {
        QWMatrix matrix;
        matrix.rotate( info->angle() );
        img = img.xForm( matrix );
    }
    if ( img.width() > 800 || img.height() > 600 )
        img = img.scale( 800, 600, QImage::ScaleMin );

    _instance->_pixmap->setPixmap( img );
    _instance->QDialog::show();
    _instance->raise();
}

void MiniViewer::closeEvent( QCloseEvent* )
{
    slotClose();
}

void MiniViewer::slotClose()
{
    _instance = 0;
    deleteLater();
}

MiniViewer::MiniViewer()
{
    QVBoxLayout* vlay = new QVBoxLayout( this, 6 );
    _pixmap = new QLabel( this );
    vlay->addWidget( _pixmap );
    QHBoxLayout* hlay = new QHBoxLayout( vlay );
    hlay->addStretch(1);
    QPushButton* but = new QPushButton( i18n("Close"), this );
    connect( but, SIGNAL( clicked() ), this, SLOT( slotClose() ) );
    hlay->addWidget( but );
}

#include "MiniViewer.moc"
