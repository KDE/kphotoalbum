/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef CATEGORYIMAGECONFIG_H
#define CATEGORYIMAGECONFIG_H

#include <kdialogbase.h>
#include <qimage.h>
#include "imageinfoptr.h"
class ImageInfo;
class QComboBox;
class QLabel;

class CategoryImageConfig :public KDialogBase {
    Q_OBJECT

public:
    static CategoryImageConfig* instance();
    void setCurrentImage( const QImage& image, const ImageInfoPtr& info );
    void show();

protected slots:
    void groupChanged();
    void memberChanged();
    void slotSet();

protected:
    QString currentGroup();

private:
    static CategoryImageConfig* _instance;
    CategoryImageConfig();
    QComboBox* _group;
    QComboBox* _member;
    QLabel* _current;
    QImage _image;
    QLabel* _imageLabel;
    ImageInfoPtr _info;
};


#endif /* CATEGORYIMAGECONFIG_H */

