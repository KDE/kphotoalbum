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
#include <QWidget>
#include <QDebug>
#include <QApplication>
#include <QCheckBox>
#include <QPushButton>

#include <KLocalizedString>

#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <MainWindow/DeleteDialog.h>

using namespace AnnotationDialog;

ImagePreviewWidget::ImagePreviewWidget() : QWidget()
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    m_preview = new ImagePreview( this );
    layout->addWidget( m_preview, 1 );
    connect( this, SIGNAL(areaVisibilityChanged(bool)), m_preview, SLOT(setAreaCreationEnabled(bool)) );

    m_controlWidget = new QWidget;
    layout->addWidget(m_controlWidget);
    QVBoxLayout* controlLayout = new QVBoxLayout(m_controlWidget);
    QHBoxLayout* controlButtonsLayout = new QHBoxLayout;
    controlLayout->addLayout(controlButtonsLayout);
    controlButtonsLayout->addStretch(1);

    m_prevBut = new QPushButton( this );
    m_prevBut->setIcon( QIcon::fromTheme( QString::fromLatin1( "arrow-left" ) ) );
    m_prevBut->setFixedWidth( 40 );
    controlButtonsLayout->addWidget( m_prevBut );
    m_prevBut->setToolTip( i18n("Annotate previous image") );

    m_nextBut = new QPushButton( this );
    m_nextBut->setIcon( QIcon::fromTheme( QString::fromLatin1( "arrow-right" ) ) );
    m_nextBut->setFixedWidth( 40 );
    controlButtonsLayout->addWidget( m_nextBut );
    m_nextBut->setToolTip( i18n("Annotate next image") );

    controlButtonsLayout->addStretch(1);

    m_rotateLeft = new QPushButton( this );
    controlButtonsLayout->addWidget( m_rotateLeft );
    m_rotateLeft->setIcon( QIcon::fromTheme( QString::fromLatin1( "object-rotate-left" ) ) );
    m_rotateLeft->setFixedWidth( 40 );
    m_rotateLeft->setToolTip( i18n("Rotate counterclockwise") );

    m_rotateRight = new QPushButton( this );
    controlButtonsLayout->addWidget( m_rotateRight );
    m_rotateRight->setIcon( QIcon::fromTheme( QString::fromLatin1( "object-rotate-right" ) ) );
    m_rotateRight->setFixedWidth( 40 );
    m_rotateRight->setToolTip( i18n("Rotate clockwise") );

    m_copyPreviousBut = new QPushButton( this );
    controlButtonsLayout->addWidget( m_copyPreviousBut );
    m_copyPreviousBut->setIcon( QIcon::fromTheme( QString::fromLatin1( "go-bottom" ) ) );
    m_copyPreviousBut->setFixedWidth( 40 );
    m_copyPreviousBut->setToolTip( i18n("Copy tags from previously tagged image") );
    m_copyPreviousBut->setWhatsThis( i18nc( "@info:whatsthis", "<para>Set the same tags on this image than on the previous one. The image date, label, rating, and description are left unchanged.</para>" ));

    m_toggleAreasBut = new QPushButton(this);
    controlButtonsLayout->addWidget(m_toggleAreasBut);
    m_toggleAreasBut->setIcon(QIcon::fromTheme(QString::fromLatin1("document-preview")));
    m_toggleAreasBut->setFixedWidth(40);
    m_toggleAreasBut->setCheckable(true);
    m_toggleAreasBut->setChecked(true);
    // tooltip text is set in updateTexts()

#ifdef HAVE_KFACE
    m_facedetectBut = new QPushButton(this);
    controlButtonsLayout->addWidget(m_facedetectBut);
    m_facedetectBut->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-find-user")));
    m_facedetectBut->setFixedWidth(40);
    m_facedetectBut->setCheckable(true);
    m_facedetectBut->setChecked(false);
    // tooltip text is set in updateTexts()
#endif

    controlButtonsLayout->addStretch(1);
    m_delBut = new QPushButton( this );
    m_delBut->setIcon( QIcon::fromTheme( QString::fromLatin1( "edit-delete" ) ) );
    controlButtonsLayout->addWidget( m_delBut );
    m_delBut->setToolTip( i18n("Delete image") );
    m_delBut->setAutoDefault( false );

    controlButtonsLayout->addStretch(1);

    connect( m_copyPreviousBut, SIGNAL(clicked()), this, SLOT(slotCopyPrevious()) );
    connect( m_delBut, SIGNAL(clicked()), this, SLOT(slotDeleteImage()) );
    connect( m_nextBut, SIGNAL(clicked()), this, SLOT(slotNext()) );
    connect( m_prevBut, SIGNAL(clicked()), this, SLOT(slotPrev()) );
    connect( m_rotateLeft, SIGNAL(clicked()), this, SLOT(rotateLeft()) );
    connect( m_rotateRight, SIGNAL(clicked()), this, SLOT(rotateRight()) );
    connect( m_toggleAreasBut, SIGNAL(clicked(bool)), this, SLOT(slotShowAreas(bool)) );
#ifdef HAVE_KFACE
    connect(m_facedetectBut, SIGNAL(clicked()), m_preview, SLOT(detectFaces()));

    m_autoTrainDatabase = new QCheckBox(i18n("Train face recognition database automatically"), this);
    // whatsThis text is set in updateTexts()
    m_autoTrainDatabase->setChecked(Qt::Checked);
    controlLayout->addWidget(m_autoTrainDatabase, 0, Qt::AlignCenter);
#endif

    m_current = -1;
    updateTexts();
}

#ifdef HAVE_KFACE
bool ImagePreviewWidget::automatedTraining()
{
    return m_autoTrainDatabase->isChecked();
}
#endif

int ImagePreviewWidget::angle() const
{
    return m_preview->angle();
}

void ImagePreviewWidget::anticipate(DB::ImageInfo &info1) { m_preview->anticipate( info1 ); }

void ImagePreviewWidget::configure( QList<DB::ImageInfo>* imageList, bool singleEdit )
{
  m_imageList = imageList;
  m_current = 0;
  setImage(m_imageList->at( m_current ));
  m_singleEdit = singleEdit;

  m_delBut->setEnabled( m_singleEdit );
  m_copyPreviousBut->setEnabled( m_singleEdit );
  m_rotateLeft->setEnabled( m_singleEdit );
  m_rotateRight->setEnabled( m_singleEdit );
#ifdef HAVE_KFACE
  m_autoTrainDatabase->setEnabled(m_singleEdit);
#endif
}

void ImagePreviewWidget::slotPrev()
{
    if ( ( m_current <= 0 ) )
        return;

    m_current--;
    if ( m_current != 0 )
        m_preview->anticipate( (*m_imageList)[ m_current-1 ] );
    setImage( m_imageList->at( m_current ) );

    emit indexChanged( m_current );

}

void ImagePreviewWidget::slotNext()
{
    if ( (m_current == -1) || ( m_current == (int)m_imageList->count()-1 ) )
        return;

    m_current++;

    if ( m_current != (int)m_imageList->count()-1 )
        m_preview->anticipate( (*m_imageList)[ m_current+1 ]);
    setImage( m_imageList->at( m_current ) );

    emit indexChanged( m_current );

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
    if( ! m_singleEdit ) return;

    m_preview->rotate( angle );

    emit imageRotated( angle );
}

void ImagePreviewWidget::slotDeleteImage()
{
  if( ! m_singleEdit ) return;

    MainWindow::DeleteDialog dialog( this );
    DB::ImageInfo info = m_imageList->at( m_current );

    const DB::FileNameList deleteList = DB::FileNameList() << info.fileName();

    int ret = dialog.exec( deleteList );
    if ( ret == QDialog::Rejected ) //Delete Dialog rejected, do nothing
      return;

  emit imageDeleted( m_imageList->at( m_current ) );

  if( ! m_nextBut->isEnabled() ) //No next image exists, select previous
      m_current--;

  if( m_imageList->count() == 0 ) return; //No images left

  setImage(m_imageList->at( m_current ) );

}
void ImagePreviewWidget::setImage( const DB::ImageInfo& info )
{
    m_nextBut->setEnabled( m_current != (int) m_imageList->count()-1 );
    m_prevBut->setEnabled( m_current != 0 );
    m_copyPreviousBut->setEnabled( m_current != 0 && m_singleEdit);

    m_preview->setImage( info );

    emit imageChanged( info );
}

void ImagePreviewWidget::setImage( const int index )
{
    m_current = index;

    setImage( m_imageList->at( m_current ) );
}


void ImagePreviewWidget::setImage( const QString& fileName )
{
    m_preview->setImage( fileName );
    m_current = -1;

    m_nextBut->setEnabled( false );
    m_prevBut->setEnabled( false );
    m_rotateLeft->setEnabled( false );
    m_rotateRight->setEnabled( false );
    m_delBut->setEnabled( false );
    m_copyPreviousBut->setEnabled( false );

}

ImagePreview *ImagePreviewWidget::preview() const
{
    return m_preview;
}

void ImagePreviewWidget::slotShowAreas(bool show)
{
    // slot can be triggered by something else than the button:
    m_toggleAreasBut->setChecked(show);

    emit areaVisibilityChanged(show);
}

bool ImagePreviewWidget::showAreas() const
{
    return m_toggleAreasBut->isChecked();
}

void ImagePreviewWidget::canCreateAreas(bool state)
{
    m_toggleAreasBut->setChecked(state);
    m_toggleAreasBut->setEnabled(state);
#ifdef HAVE_KFACE
    m_facedetectBut->setEnabled(state);
    m_autoTrainDatabase->setEnabled(state);
#endif
    m_preview->setAreaCreationEnabled(state);
    updateTexts();
}

void ImagePreviewWidget::updateTexts()
{
    if (m_toggleAreasBut->isEnabled())
    {
        // positionable tags enabled
        m_toggleAreasBut->setToolTip(i18nc("@info:tooltip", "Hide or show areas on the image"));
#ifdef HAVE_KFACE
        m_facedetectBut->setToolTip(i18nc("@info:tooltip", "Search for faces on the current image"));
        m_autoTrainDatabase->setWhatsThis(i18nc("@info:whatsthis",
            "If a tag for an area found by the face detector is set manually, the face recognition "
            "database will be trained automatically with that tag."
        ));
#endif
    } else {
        if (m_singleEdit) {
            // positionable tags disabled
            m_toggleAreasBut->setToolTip(i18nc("@info:tooltip",
                "If you enable <emphasis>positionable tags</emphasis> for at least one category in "
                "<interface>Settings|Configure KPhotoAlbum...|Categories</interface>, you can "
                "associate specific image areas with tags."
            ));
        } else {
            m_toggleAreasBut->setToolTip(i18nc("@info:tooltip",
                "Areas on an image can only be shown in single-image annotation mode."
            ));
        }
#ifdef HAVE_KFACE
        QString faceDetectionPlaceholderText;
        if (m_singleEdit) {
            faceDetectionPlaceholderText = i18nc("@info",
                "To use face detection, enable <emphasis>positionable tags</emphasis> for at least "
                "one category in <interface>Settings|Configure KPhotoAlbum..."
                "|Categories</interface>."
            );
        } else {
            faceDetectionPlaceholderText = i18nc("@info",
                "Face detection is only available in single-image annotation mode."
            );
        }
        m_facedetectBut->setToolTip( faceDetectionPlaceholderText );
        m_autoTrainDatabase->setWhatsThis( faceDetectionPlaceholderText );
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

    m_facedetectBut->setChecked(! state);
    m_facedetectBut->setEnabled(state);

    // Better disable the whole widget so that the user can't
    // change or delete the image during face detection.
    this->setEnabled(state);
}

void ImagePreviewWidget::setSearchMode(bool state)
{
    m_controlWidget->setVisible(! state);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
