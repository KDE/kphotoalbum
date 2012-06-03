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
#include "ImagePreviewWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <klocale.h>
#include <QWidget>
#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "MainWindow/DeleteDialog.h"
using namespace AnnotationDialog;

ImagePreviewWidget::ImagePreviewWidget() : QWidget()
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    _preview = new ImagePreview( this );
    layout->addWidget( _preview, 1 );

    QHBoxLayout* hlay = new QHBoxLayout;
    layout->addLayout( hlay );
    hlay->addStretch(1);

    _prevBut = new KPushButton( this );
    _prevBut->setIcon( KIcon( QString::fromLatin1( "arrow-left" ) ) );
    _prevBut->setFixedWidth( 40 );
    hlay->addWidget( _prevBut );
    _prevBut->setToolTip( i18n("Annotate previous image") );

    _nextBut = new KPushButton( this );
    _nextBut->setIcon( KIcon( QString::fromLatin1( "arrow-right" ) ) );
    _nextBut->setFixedWidth( 40 );
    hlay->addWidget( _nextBut );
    _nextBut->setToolTip( i18n("Annotate next image") );

    hlay->addStretch(1);

    _rotateLeft = new KPushButton( this );
    hlay->addWidget( _rotateLeft );
    _rotateLeft->setIcon( KIcon( QString::fromLatin1( "object-rotate-left" ) ) );
    _rotateLeft->setFixedWidth( 40 );
    _rotateLeft->setToolTip( i18n("Rotate counterclockwise") );

    _rotateRight = new KPushButton( this );
    hlay->addWidget( _rotateRight );
    _rotateRight->setIcon( KIcon( QString::fromLatin1( "object-rotate-right" ) ) );
    _rotateRight->setFixedWidth( 40 );
    _rotateRight->setToolTip( i18n("Rotate clockwise") );

    _copyPreviousBut = new KPushButton( this );
    hlay->addWidget( _copyPreviousBut );
    _copyPreviousBut->setIcon( KIcon( QString::fromLatin1( "go-bottom" ) ) );
    _copyPreviousBut->setFixedWidth( 40 );
    _copyPreviousBut->setToolTip( i18n("Copy tags from previously tagged image") );

    hlay->addStretch( 1 );
    _delBut = new KPushButton( this );
    _delBut->setIcon( KIcon( QString::fromLatin1( "edit-delete" ) ) );
    hlay->addWidget( _delBut );
    _delBut->setToolTip( i18n("Delete image") );
    _delBut->setAutoDefault( false );

    hlay->addStretch(1);

    connect( _copyPreviousBut, SIGNAL( clicked() ), this, SLOT( slotCopyPrevious() ) );
    connect( _delBut, SIGNAL( clicked() ), this, SLOT( slotDeleteImage() ) );
    connect( _nextBut, SIGNAL( clicked() ), this, SLOT( slotNext() ) );
    connect( _prevBut, SIGNAL( clicked() ), this, SLOT( slotPrev() ) );
    connect( _rotateLeft, SIGNAL( clicked() ), this, SLOT( rotateLeft() ) );
    connect( _rotateRight, SIGNAL( clicked() ), this, SLOT( rotateRight() ) );

    _current = -1;
}
int ImagePreviewWidget::angle() const { return _preview->angle(); }

void ImagePreviewWidget::anticipate(DB::ImageInfo &info1) { _preview->anticipate( info1 ); }

void ImagePreviewWidget::configure( QList<DB::ImageInfo>* imageList, bool singleEdit )
{
  _imageList = imageList;
  _current = 0;
  setImage(_imageList->at( _current ));
  _singleEdit = singleEdit;

  _delBut->setEnabled( _singleEdit );
  _copyPreviousBut->setEnabled( _singleEdit );
  _rotateLeft->setEnabled( _singleEdit );
  _rotateRight->setEnabled( _singleEdit );
}

void ImagePreviewWidget::slotPrev()
{
    if ( ( _current <= 0 ) )
        return;

    _current--;
    if ( _current != 0 )
        _preview->anticipate( (*_imageList)[ _current-1 ] );
    setImage( _imageList->at( _current ) );

    emit indexChanged( _current );

}

void ImagePreviewWidget::slotNext()
{
    if ( (_current == -1) || ( _current == (int)_imageList->count()-1 ) )
        return;

    _current++;

    if ( _current != (int)_imageList->count()-1 )
        _preview->anticipate( (*_imageList)[ _current+1 ]);
    setImage( _imageList->at( _current ) );

    emit indexChanged( _current );

}

void ImagePreviewWidget::slotCopyPrevious()
{
    emit copyPrevClicked();
}

void ImagePreviewWidget::rotateLeft()
{
    rotate(-90);
}

void ImagePreviewWidget::rotateRight()
{
    rotate(90);
}

void ImagePreviewWidget::rotate( int angle )
{
    if( ! _singleEdit ) return;

    _preview->rotate( angle );

    emit imageRotated( angle );
}

void ImagePreviewWidget::slotDeleteImage()
{
  if( ! _singleEdit ) return;

    MainWindow::DeleteDialog dialog( this );
    DB::ImageInfo info = _imageList->at( _current );

    const DB::FileNameList deleteList = DB::FileNameList() << info.fileName();

    int ret = dialog.exec( deleteList );
    if ( ret == QDialog::Rejected ) //Delete Dialog rejected, do nothing
	  return;

  emit imageDeleted( _imageList->at( _current ) );

  if( ! _nextBut->isEnabled() ) //No next image exists, select previous
      _current--;

  if( _imageList->count() == 0 ) return; //No images left

  setImage(_imageList->at( _current ) );

}
void ImagePreviewWidget::setImage( const DB::ImageInfo& info )
{
    _nextBut->setEnabled( _current != (int) _imageList->count()-1 );
    _prevBut->setEnabled( _current != 0 );
    _copyPreviousBut->setEnabled( _current != 0 && _singleEdit);

    _preview->setImage( info );

    emit imageChanged( info );
}

void ImagePreviewWidget::setImage( const int index )
{
    _current = index;

    setImage( _imageList->at( _current ) );
}


void ImagePreviewWidget::setImage( const QString& fileName )
{
    _preview->setImage( fileName );
    _current = -1;

    _nextBut->setEnabled( false );
    _prevBut->setEnabled( false );
    _rotateLeft->setEnabled( false );
    _rotateRight->setEnabled( false );
    _delBut->setEnabled( false );
    _copyPreviousBut->setEnabled( false );

}
