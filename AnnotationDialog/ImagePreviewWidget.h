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

#ifndef IMAGEPREVIEWWIDGET_H
#define IMAGEPREVIEWWIDGET_H
#include <QWidget>
#include <QList>
#include "DB/ImageInfo.h"
#include "ImagePreview.h"
#include <KPushButton>
#include "config-kpa-kface.h"

class QCheckBox;

namespace AnnotationDialog
{

class ImagePreviewWidget :public QWidget{
    Q_OBJECT
public:
    ImagePreviewWidget();
    void rotate(int angle) ;
    void setImage( const DB::ImageInfo& info );
    void setImage( const QString& fileName );
    void setImage( const int index );
    void configure( QList<DB::ImageInfo>* imageList, bool singleEdit );
    int angle() const;
    void anticipate(DB::ImageInfo &info1);
    const QString& lastImage();
    ImagePreview *preview() const;
    bool showAreas() const;
    void canCreateAreas(bool state);
    void setFacedetectButEnabled(bool state);
#ifdef HAVE_KFACE
    bool automatedTraining();
#endif
    void setSearchMode(bool state);

public slots:
    void slotNext();
    void slotPrev();
    void slotCopyPrevious();
    void slotDeleteImage();
    void rotateLeft();
    void rotateRight();
    void slotShowAreas(bool show);

signals:
    void imageDeleted( const DB::ImageInfo& deletedImage );
    void imageRotated( int angle);
    void imageChanged( const DB::ImageInfo& newImage );
    void indexChanged( int newIndex );
    void copyPrevClicked();
    void areaVisibilityChanged(bool visible);

private:
    /**
     * Update labels and tooltip texts when canCreateAreas() changes.
     */
    void updateTexts();
    ImagePreview* m_preview;
    KPushButton* m_prevBut;
    KPushButton* m_nextBut;
    KPushButton* m_rotateLeft;
    KPushButton* m_rotateRight;
    KPushButton* m_delBut;
    KPushButton* m_copyPreviousBut;
    KPushButton *m_facedetectBut;
    KPushButton *m_toggleAreasBut;
    QList<DB::ImageInfo>* m_imageList;
    int m_current;
    bool m_singleEdit;
#ifdef HAVE_KFACE
    QCheckBox *m_autoTrainDatabase;
#endif
    QWidget* m_controlWidget;
};

}

#endif /* IMAGEPREVIEWWIDGET_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
