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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

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
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, bool loadedOK );

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

