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

#ifndef VIEWER_H
#define VIEWER_H

#include "imageinfo.h"
#include "options.h"
#include <kaction.h>
#include "infobox.h"
#include <qdialog.h>
#include <qimage.h>
class ImageInfo;
class QLabel;
class QPopupMenu;
class QAction;
class DisplayArea;

class Viewer :public QDialog
{
    Q_OBJECT
public:
    Viewer( QWidget* parent, const char* name = 0 );
    ~Viewer();
    static Viewer* latest();
    void load( const ImageInfoList& list, int index = 0 );
    void infoBoxMove();

protected:
    virtual void contextMenuEvent ( QContextMenuEvent * e );
    virtual void closeEvent( QCloseEvent* e );
    virtual void resizeEvent( QResizeEvent* );
    void moveInfoBox();


    void load();
    void updateInfoBox();
    void setupContextMenu();
    virtual void close();
    ImageInfo* currentInfo();

protected slots:
    void showNext();
    void showPrev();
    void showFirst();
    void showLast();
    void zoomIn();
    void zoomOut();
    void rotate90();
    void rotate180();
    void rotate270();
    void toggleShowInfoBox( bool );
    void toggleShowDescription( bool );
    void toggleShowDate( bool );
    void save();
    void startDraw();
    void stopDraw();
    void toggleShowOption( const QString& optionGroup, bool b ) ;

private:
    static Viewer* _latest;

    QAction* _firstAction;
    QAction* _lastAction;
    QAction* _nextAction;
    QAction* _prevAction;

    DisplayArea* _label;
    ImageInfoList _list;
    int _current;
    QRect _textRect;
    QPopupMenu* _popup;
    int _width, _height;
    QPixmap _pixmap;

#if 0
    KToolBar* _toolbar;
    KToggleAction* _select;
    KToggleAction* _line;
    KToggleAction* _rect;
    KToggleAction* _circle;
    KAction* _delete;
#endif

    InfoBox* _infoBox;
    QImage _currentImage;
};

#endif /* VIEWER_H */

