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

#ifndef VIEWER_H
#define VIEWER_H

#include <QStackedWidget>
#include <qimage.h>
#include <QPixmap>
#include <QMap>
#include "DB/ImageInfoPtr.h"
#include "DB/IdList.h"
#include <config-kpa-exiv2.h>
#include <QPointer>

class QKeyEvent;
class QResizeEvent;
class QWheelEvent;
class QContextMenuEvent;
class QStackedWidget;
class KActionCollection;
class QMenu;
class KAction;

namespace DB { class ImageInfo; class Id; }
namespace MainWindow { class ExternalPopup; class CategoryImagePopup; }
namespace Exif { class InfoDialog; }
namespace Viewer
{
class VideoDisplay;
class ImageDisplay;
class TextDisplay;
class Display;
class SpeedDisplay;
class InfoBox;

class ViewerWidget :public QStackedWidget
{
    Q_OBJECT
public:
    enum UsageType { InlineViewer, ViewerWindow };

    ViewerWidget( UsageType type = ViewerWindow,
                  QMap<Qt::Key, QPair<QString,QString> > *macroStore = 0);
    ~ViewerWidget();
    static ViewerWidget* latest();
    void load( const QStringList& list, int index = 0 );
    void infoBoxMove();
    bool showingFullScreen() const;
    void setShowFullScreen( bool on );
    void show( bool slideShow );
    KActionCollection* actions();

public slots:
    virtual bool close(bool alsoDelete = false );
    void updateInfoBox();
    void test();
    void moveInfoBox( int );
    void stopPlayback();

signals:
    void soughtTo( const DB::Id& id );

protected:
    OVERRIDE void contextMenuEvent ( QContextMenuEvent * e );
    OVERRIDE void resizeEvent( QResizeEvent* );
    OVERRIDE void keyPressEvent( QKeyEvent* );
    OVERRIDE void wheelEvent( QWheelEvent* event );

    void moveInfoBox();
    void setAsWallpaper(int mode);

    void load();
    void setupContextMenu();
    void createShowContextMenu();
    void createWallPaperMenu();
    void createInvokeExternalMenu();
    void createRotateMenu();
    void createSkipMenu();
    void createZoomMenu();
    void createSlideShowMenu();
    void createVideoMenu();
    void createCategoryImageMenu();
    void createFilterMenu();
    void changeSlideShowInterval( int delta );
    void createVideoViewer();
    void inhibitScreenSaver( bool inhibit );
    DB::ImageInfoPtr currentInfo() const;
    friend class InfoBox;

private:
    void showNextN(int);
    void showPrevN(int);
    int  find_tag_in_list(const QStringList &list, QString &namefound);
    void invalidateThumbnail() const;
    enum RemoveAction { RemoveImageFromDatabase, OnlyRemoveFromViewer };
    void removeOrDeleteCurrent( RemoveAction );


protected slots:
    void showNext();
    void showNext10();
    void showNext100();
    void showNext1000();
    void showPrev();
    void showPrev10();
    void showPrev100();
    void showPrev1000();
    void showFirst();
    void showLast();
    void deleteCurrent();
    void removeCurrent();
    void rotate90();
    void rotate180();
    void rotate270();
    void toggleFullScreen();
    void slotStartStopSlideShow();
    void slotSlideShowNext();
    void slotSlideShowNextFromTimer();
    void slotSlideShowFaster();
    void slotSlideShowSlower();
    void editImage();
    void filterNone();
    void filterSelected();
    void filterBW();
    void filterContrastStretch();
    void filterHistogramEqualization();
    void filterMono();
    void slotSetStackHead();
    void updateCategoryConfig();
    void slotSetWallpaperC();
    void slotSetWallpaperT();
    void slotSetWallpaperCT();
    void slotSetWallpaperCM();
    void slotSetWallpaperTM();
    void slotSetWallpaperS();
    void slotSetWallpaperCAF();
    void populateExternalPopup();
    void populateCategoryImagePopup();
    void videoStopped();
    void showExifViewer();
    void zoomIn();
    void zoomOut();
    void zoomFull();
    void zoomPixelForPixel();

    /** Set the current window title (filename) and add the given detail */
    void setCaptionWithDetail( const QString& detail );

private:
    static ViewerWidget* _latest;

    QList<KAction*> _forwardActions;
    QList<KAction*> _backwardActions;

    KAction* _startStopSlideShow;
    KAction* _slideShowRunFaster;
    KAction* _slideShowRunSlower;
    KAction* _setStackHead;
    KAction* _filterNone;
    KAction* _filterSelected;
    KAction* _filterBW;
    KAction* _filterContrastStretch;
    KAction* _filterHistogramEqualization;
    KAction* _filterMono;

    Display* _display;
    ImageDisplay* _imageDisplay;
    VideoDisplay* _videoDisplay;
    TextDisplay* _textDisplay;

    int m_screenSaverCookie;
    QStringList _list;
    DB::IdList _removed;
    int _current;
    QRect _textRect;
    QMenu* _popup;
    QMenu* _rotateMenu;
    QMenu* _wallpaperMenu;
    QMenu* _filterMenu;
    MainWindow::ExternalPopup* _externalPopup;
    MainWindow::CategoryImagePopup* _categoryImagePopup;
    int _width, _height;
    QPixmap _pixmap;

    KAction* _delete;
#ifdef HAVE_EXIV2
    KAction* _showExifViewer;
    QPointer<Exif::InfoDialog> _exifViewer;
#endif

    InfoBox* _infoBox;
    QImage _currentImage;

    bool _showingFullScreen;

    int _slideShowPause;
    SpeedDisplay* _speedDisplay;
    KActionCollection* _actions;
    bool _forward;
    QTimer* _slideShowTimer;
    bool _isRunningSlideShow;

    QList<QAction*> _videoActions;
    KAction* _stop;
    KAction* _playPause;
    bool _videoPlayerStoppedManually;
    UsageType _type;

    enum InputMode { InACategory, AlwaysStartWithCategory };

    InputMode _currentInputMode;
    QString _currentInput;
    QString _currentCategory;
    QString _currentInputList;

    QString _lastFound;
    QString _lastCategory;
    QMap<Qt::Key, QPair<QString,QString> >* _inputMacros;
    QMap<Qt::Key, QPair<QString,QString> >* _myInputMacros;
};

}

#endif /* VIEWER_H */

