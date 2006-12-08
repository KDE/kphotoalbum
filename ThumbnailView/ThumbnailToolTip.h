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

#ifndef THUMBNAILTOOLTIP_H
#define THUMBNAILTOOLTIP_H
#include <qtimer.h>
#include <qlabel.h>
#include "ImageManager/ImageClient.h"
namespace DB { class ImageInfo; }

namespace ThumbnailView
{
class ThumbnailWidget;

class ThumbnailToolTip :public QLabel, public ImageManager::ImageClient {
    Q_OBJECT

public:
    ThumbnailToolTip( ThumbnailWidget* view, const char* name = 0 );
    void showToolTips( bool force );
    virtual void setActive(bool);
    void clear();
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, bool loadedOK );

protected:
    virtual bool eventFilter( QObject*, QEvent* e );
    bool loadImage( const QString& fileName );
    void placeWindow();

private:
    ThumbnailWidget* _view;
    QString _currentFileName;
    QStringList _loadedImages;
    bool _widthInverse;
    bool _heightInverse;
};

}


#endif /* THUMBNAILTOOLTIP_H */

