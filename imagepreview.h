/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H
#include <qlabel.h>
#include "imageinfo.h"
#include "imageclient.h"
#include <qimage.h>

class ImagePreview :public QLabel {
    Q_OBJECT
public:
    ImagePreview( QWidget* parent, const char* name = 0);
    virtual QSize sizeHint() const;
    void rotate(int angle);
    void setImage( const QImage& img, int angle );
    int angle() const;

protected:
    virtual void resizeEvent( QResizeEvent* );
    void reload();

private:
    QImage _img;
    int _angle;
};


#endif /* IMAGEPREVIEW_H */

