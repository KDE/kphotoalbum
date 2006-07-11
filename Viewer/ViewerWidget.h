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

#ifndef VIEWER_H
#define VIEWER_H

#include "Settings/SettingsData.h"
#include <kxmlguiclient.h>
#include <kaction.h>
#include <qdialog.h>
#include <qimage.h>
#include "DB/ImageInfoList.h"

class QWidgetStack;
class QLabel;
class QPopupMenu;
class KAction;
class CategoryImageConfig;

namespace DB { class ImageInfo; }
namespace MainWindow { class ExternalPopup; }

namespace Viewer
{
class VideoDisplay;
class ImageDisplay;
class Display;
class SpeedDisplay;
class InfoBox;

class ViewerWidget :public QWidget
{
    Q_OBJECT
public:
    ViewerWidget( const char* name = 0 );
    ~ViewerWidget();
    static ViewerWidget* latest();
    void load( const QStringList& list, int index = 0 );
    void infoBoxMove();
    bool showingFullScreen() const;
    void setShowFullScreen( bool on );
    void show( bool slideShow );
    KActionCollection* actions();

public slots:
    virtual bool close(bool alsoDelete );

protected:
    virtual void contextMenuEvent ( QContextMenuEvent * e );
    virtual void resizeEvent( QResizeEvent* );
    virtual void keyPressEvent( QKeyEvent* );

    void moveInfoBox();
    void createToolBar();
    void setAsWallpaper(int mode);

    void load();
    void updateInfoBox();
    void setupContextMenu();
    DB::ImageInfoPtr currentInfo();
    friend class InfoBox;

signals:
    void dirty();

protected slots:
    void showNext();
    void showPrev();
    void showFirst();
    void showLast();
    void rotate90();
    void rotate180();
    void rotate270();
    void toggleShowInfoBox( bool );
    void toggleShowDescription( bool );
    void toggleShowDate( bool );
    void toggleShowTime( bool );
    void toggleShowEXIF( bool );
    void save();
    void startDraw();
    void stopDraw();
    void toggleShowOption( const QString& category, bool b ) ;
    void toggleFullScreen();
    void slotStartStopSlideShow();
    void slotSlideShowNext();
    void slotSlideShowFaster();
    void slotSlideShowSlower();
    void editImage();
    void makeCategoryImage();
    void updateCategoryConfig();
    void slotSetWallpaperC();
    void slotSetWallpaperT();
    void slotSetWallpaperCT();
    void slotSetWallpaperCM();
    void slotSetWallpaperTM();
    void slotSetWallpaperS();
    void slotSetWallpaperCAF();
    void populateExternalPopup();
    void videoStopped();


private:
    static ViewerWidget* _latest;

    KAction* _firstAction;
    KAction* _lastAction;
    KAction* _nextAction;
    KAction* _prevAction;
    KAction* _startStopSlideShow;
    KAction* _slideShowRunFaster;
    KAction* _slideShowRunSlower;

    QWidgetStack* _stack;
    Display* _display;
    ImageDisplay* _imageDisplay;
    VideoDisplay* _videoDisplay;

    QStringList _list;
    int _current;
    QRect _textRect;
    QPopupMenu* _popup;
    MainWindow::ExternalPopup* _externalPopup;
    int _width, _height;
    QPixmap _pixmap;

    KToolBar* _toolbar;
    KToggleAction* _select;
    KToggleAction* _line;
    KToggleAction* _rect;
    KToggleAction* _circle;
    KAction* _delete;

    InfoBox* _infoBox;
    QImage _currentImage;

    bool _showingFullScreen;

    int _slideShowPause;
    SpeedDisplay* _speedDisplay;
    KActionCollection* _actions;
    bool _sized;
    bool _forward;
    QTimer* _slideShowTimer;
    bool _isRunningSlideShow;
};

}

#endif /* VIEWER_H */

