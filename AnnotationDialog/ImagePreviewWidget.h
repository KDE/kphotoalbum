/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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

public slots:
    void slotNext();
    void slotPrev();
    void slotCopyPrevious();
    void slotDeleteImage();
    void rotateLeft();
    void rotateRight();
    
signals:
    void imageDeleted( const DB::ImageInfo& deletedImage );
    void imageRotated( int angle);
    void imageChanged( const DB::ImageInfo& newImage );
    void indexChanged( int newIndex );
    void copyPrevClicked();

private:
    ImagePreview* _preview;
    KPushButton* _prevBut;
    KPushButton* _nextBut;
    KPushButton* _rotateLeft;
    KPushButton* _rotateRight;
    KPushButton* _delBut;
    KPushButton* _copyPreviousBut;
    QList<DB::ImageInfo>* _imageList;
    int _current;
    bool _singleEdit;
};

}

#endif /* IMAGEPREVIEWWIDGET_H */

