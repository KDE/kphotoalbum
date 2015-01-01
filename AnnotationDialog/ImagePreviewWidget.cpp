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
#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "MainWindow/DeleteDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <klocale.h>
#include <QWidget>
#include <QDebug>
#include <QApplication>
#include <QCheckBox>

using namespace AnnotationDialog;

ImagePreviewWidget::ImagePreviewWidget() : QWidget()
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    _preview = new ImagePreview( this );
    layout->addWidget( _preview, 1 );
    connect( this, SIGNAL(areaVisibilityChanged(bool)), _preview, SLOT(setAreaCreationEnabled(bool)) );

    _controlWidget = new QWidget;
    layout->addWidget(_controlWidget);
    QVBoxLayout* controlLayout = new QVBoxLayout(_controlWidget);
    QHBoxLayout* controlButtonsLayout = new QHBoxLayout;
    controlLayout->addLayout(controlButtonsLayout);
    controlButtonsLayout->addStretch(1);

    _prevBut = new KPushButton( this );
    _prevBut->setIcon( KIcon( QString::fromLatin1( "arrow-left" ) ) );
    _prevBut->setFixedWidth( 40 );
    controlButtonsLayout->addWidget( _prevBut );
    _prevBut->setToolTip( i18n("Annotate previous image") );

    _nextBut = new KPushButton( this );
    _nextBut->setIcon( KIcon( QString::fromLatin1( "arrow-right" ) ) );
    _nextBut->setFixedWidth( 40 );
    controlButtonsLayout->addWidget( _nextBut );
    _nextBut->setToolTip( i18n("Annotate next image") );

    controlButtonsLayout->addStretch(1);

    _rotateLeft = new KPushButton( this );
    controlButtonsLayout->addWidget( _rotateLeft );
    _rotateLeft->setIcon( KIcon( QString::fromLatin1( "object-rotate-left" ) ) );
    _rotateLeft->setFixedWidth( 40 );
    _rotateLeft->setToolTip( i18n("Rotate counterclockwise") );

    _rotateRight = new KPushButton( this );
    controlButtonsLayout->addWidget( _rotateRight );
    _rotateRight->setIcon( KIcon( QString::fromLatin1( "object-rotate-right" ) ) );
    _rotateRight->setFixedWidth( 40 );
    _rotateRight->setToolTip( i18n("Rotate clockwise") );

    _copyPreviousBut = new KPushButton( this );
    controlButtonsLayout->addWidget( _copyPreviousBut );
    _copyPreviousBut->setIcon( KIcon( QString::fromLatin1( "go-bottom" ) ) );
    _copyPreviousBut->setFixedWidth( 40 );
    _copyPreviousBut->setToolTip( i18n("Copy tags from previously tagged image") );

    _toggleAreasBut = new KPushButton(this);
    controlButtonsLayout->addWidget(_toggleAreasBut);
    _toggleAreasBut->setIcon(KIcon(QString::fromLatin1("document-preview")));
    _toggleAreasBut->setFixedWidth(40);
    _toggleAreasBut->setCheckable(true);
    _toggleAreasBut->setChecked(true);
    // tooltip text is set in updateTexts()

#ifdef HAVE_KFACE
    _facedetectBut = new KPushButton(this);
    controlButtonsLayout->addWidget(_facedetectBut);
    _facedetectBut->setIcon(KIcon(QString::fromLatin1("edit-find-user")));
    _facedetectBut->setFixedWidth(40);
    _facedetectBut->setCheckable(true);
    _facedetectBut->setChecked(false);
    // tooltip text is set in updateTexts()
#endif

    controlButtonsLayout->addStretch(1);
    _delBut = new KPushButton( this );
    _delBut->setIcon( KIcon( QString::fromLatin1( "edit-delete" ) ) );
    controlButtonsLayout->addWidget( _delBut );
    _delBut->setToolTip( i18n("Delete image") );
    _delBut->setAutoDefault( false );

    controlButtonsLayout->addStretch(1);

    connect( _copyPreviousBut, SIGNAL(clicked()), this, SLOT(slotCopyPrevious()) );
    connect( _delBut, SIGNAL(clicked()), this, SLOT(slotDeleteImage()) );
    connect( _nextBut, SIGNAL(clicked()), this, SLOT(slotNext()) );
    connect( _prevBut, SIGNAL(clicked()), this, SLOT(slotPrev()) );
    connect( _rotateLeft, SIGNAL(clicked()), this, SLOT(rotateLeft()) );
    connect( _rotateRight, SIGNAL(clicked()), this, SLOT(rotateRight()) );
    connect( _toggleAreasBut, SIGNAL(clicked(bool)), this, SLOT(slotShowAreas(bool)) );
#ifdef HAVE_KFACE
    connect(_facedetectBut, SIGNAL(clicked()), _preview, SLOT(detectFaces()));

    _autoTrainDatabase = new QCheckBox(i18n("Train face recognition database automatically"), this);
    // whatsThis text is set in updateTexts()
    _autoTrainDatabase->setChecked(Qt::Checked);
    controlLayout->addWidget(_autoTrainDatabase, 0, Qt::AlignCenter);
#endif

    _current = -1;
    updateTexts();
}

#ifdef HAVE_KFACE
bool ImagePreviewWidget::automatedTraining()
{
    return _autoTrainDatabase->isChecked();
}
#endif

int ImagePreviewWidget::angle() const
{
    return _preview->angle();
}

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
#ifdef HAVE_KFACE
  _autoTrainDatabase->setEnabled(_singleEdit);
#endif
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

ImagePreview *ImagePreviewWidget::preview() const
{
    return _preview;
}

void ImagePreviewWidget::slotShowAreas(bool show)
{
    // slot can be triggered by something else than the button:
    _toggleAreasBut->setChecked(show);

    emit areaVisibilityChanged(show);
}

bool ImagePreviewWidget::showAreas() const
{
    return _toggleAreasBut->isChecked();
}

void ImagePreviewWidget::canCreateAreas(bool state)
{
    _toggleAreasBut->setChecked(state);
    _toggleAreasBut->setEnabled(state);
#ifdef HAVE_KFACE
    _facedetectBut->setEnabled(state);
    _autoTrainDatabase->setEnabled(state);
#endif
    _preview->setAreaCreationEnabled(state);
    updateTexts();
}

void ImagePreviewWidget::updateTexts()
{
    if (_toggleAreasBut->isEnabled())
    {
        // positionable tags enabled
        _toggleAreasBut->setToolTip(i18nc("@info:tooltip", "Hide or show areas on the image"));
#ifdef HAVE_KFACE
        _facedetectBut->setToolTip(i18nc("@info:tooltip", "Search for faces on the current image"));
        _autoTrainDatabase->setWhatsThis(i18nc("@info:whatsthis",
            "If a tag for an area found by the face detector is set manually, the face recognition "
            "database will be trained automatically with that tag."
        ));
#endif
    } else {
        // positionable tags disabled
        _toggleAreasBut->setToolTip(i18nc("@info:tooltip",
                    "If you enable <emphasis>positionable tags</emphasis> under <interface>Settings|Configure KPhotoAlbum...|Categories</interface>, "
                    "you can associate specific image areas with tags."
                    ));
#ifdef HAVE_KFACE
        QString faceDetectionPlaceholderText { i18nc("@info",
                "To use face detection, enable <emphasis>positionable tags</emphasis> "
                "under <interface>Settings|Configure KPhotoAlbum...|Categories</interface>.") };
        _facedetectBut->setToolTip( faceDetectionPlaceholderText );
        _autoTrainDatabase->setWhatsThis( faceDetectionPlaceholderText );
#endif
    }
}

void ImagePreviewWidget::setFacedetectButEnabled(bool state)
{
    if (state == false) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    } else {
        QApplication::restoreOverrideCursor();
    }

    _facedetectBut->setChecked(! state);
    _facedetectBut->setEnabled(state);

    // Better disable the whole widget so that the user can't
    // change or delete the image during face detection.
    this->setEnabled(state);
}

void ImagePreviewWidget::setSearchMode(bool state)
{
    _controlWidget->setVisible(! state);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
