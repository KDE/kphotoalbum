/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
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

#ifndef ICONVIEWTOOLTIP_H
#define ICONVIEWTOOLTIP_H
#include <qiconview.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qdict.h>
#include "imageclient.h"
class ImageInfo;

class IconViewToolTip :public QLabel, public ImageClient {
    Q_OBJECT

public:
    IconViewToolTip( QIconView* view, const char* name = 0 );
    void showToolTips( bool force );
    virtual void setActive(bool);
    void clear();
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QImage& );

protected:
    virtual bool eventFilter( QObject*, QEvent* e );
    QIconViewItem* itemAtCursor();
    bool loadImage( const ImageInfo& info );
    void placeWindow();

private:
    QIconView* _view;
    QIconViewItem* _current;
    QString _currentFileName;
    QStringList _loadedImages;
    bool _widthInverse;
    bool _heightInverse;
};


#endif /* ICONVIEWTOOLTIP_H */

