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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMAGECONFIG_H
#define IMAGECONFIG_H
#include "ListSelect.h"
#include "DB/ImageSearchInfo.h"
#include <qdialog.h>
#include <QResizeEvent>
#include <QEvent>
#include <QMoveEvent>
#include <Q3ValueList>
#include <QCloseEvent>
#include <Q3PtrList>
#include "DB/ImageInfoList.h"
#include "DB/Category.h"
#include "enums.h"
#include <KActionCollection>
#include "config-kpa-nepomuk.h"


class KTextEdit;
class DockWidget;
class QDockWidget;
class QTimeEdit;
class QMainWindow;
class QSplitter;
class QPushButton;
class KLineEdit;
class KPushButton;

#ifdef HAVE_NEPOMUK
class KRatingWidget;
#endif

namespace Viewer
{
    class ViewerWidget;
}

namespace DB
{
    class ImageInfo;
}


namespace AnnotationDialog
{
class ImagePreview;
class KDateEdit;
class ShortCutManager;

class Dialog :public QDialog {
    Q_OBJECT
public:
    Dialog( QWidget* parent );
    int configure( DB::ImageInfoList list,  bool oneAtATime );
    DB::ImageSearchInfo search( DB::ImageSearchInfo* search = 0 );
    bool thumbnailShouldReload() const;
    bool thumbnailTextShouldReload() const;
    KActionCollection* actions();

protected slots:
    void slotRevert();
    void slotPrev();
    void slotNext();
    void slotOK();
    void slotClear();
    void viewerDestroyed();
    void slotOptions();
    void slotSaveWindowSetup();
    void slotDeleteOption( DB::Category*, const QString& );
    void slotRenameOption( DB::Category* , const QString& , const QString&  );
    virtual void reject();
    void rotateLeft();
    void rotateRight();
    void rotate( int angle );
    void slotAddTimeInfo();
    void slotDeleteImage();
    void slotResetLayout();
    void slotStartDateChanged( const DB::ImageDate& );
    void slotCopyPrevious();
    void slotRatingChanged( unsigned int );

protected:
    QDockWidget* createDock( const QString& title, const QString& name, Qt::DockWidgetArea location, QWidget* widget );
    QWidget* createDateWidget(ShortCutManager& shortCutManager);
    QWidget* createPreviewWidget();
    ListSelect* createListSel( const DB::CategoryPtr& category );

    void load();
    void writeToInfo();
    void setup();
    void loadInfo( const DB::ImageSearchInfo& );
    int exec();
    virtual void closeEvent( QCloseEvent* );
    void showTornOfWindows();
    void hideTornOfWindows();
    bool hasChanges();
    void showHelpDialog( UsageMode );
    virtual void resizeEvent( QResizeEvent* );
    virtual void moveEvent ( QMoveEvent * );
    void setupFocus();
    void closeDialog();
    void loadWindowLayout();
    void setupActions();
    void setUpCategoryListBoxForMultiImageSelection( ListSelect*, const DB::ImageInfoList& images );
    QPair<StringSet,StringSet> selectionForMultiSelect( ListSelect*, const DB::ImageInfoList& images );

private:
    DB::ImageInfoList _origList;
    Q3ValueList<DB::ImageInfo> _editList;
    int _current;
    UsageMode _setup;
    Q3PtrList< ListSelect > _optionList;
    DB::ImageSearchInfo _oldSearch;
    QSplitter* _splitter;
    Viewer::ViewerWidget* _viewer;
    int _accept;
    QList<QDockWidget*> _dockWidgets;
    bool _thumbnailShouldReload;
    bool _thumbnailTextShouldReload;

    // Widgets
    QMainWindow* _dockWindow;
    KLineEdit* _imageLabel;
    KDateEdit* _startDate;
    KDateEdit* _endDate;

    ImagePreview* _preview;
    QPushButton* _revertBut;
    KPushButton* _okBut;
    QPushButton* _prevBut;
    QPushButton* _nextBut;
    QPushButton* _rotateLeft;
    QPushButton* _rotateRight;
    QPushButton* _delBut;
    QPushButton* _copyPreviousBut;
    KTextEdit* _description;
    QTimeEdit* _time;
    QPushButton* _addTime;
#ifdef HAVE_NEPOMUK
    KRatingWidget* _rating;
#endif
    bool _ratingChanged;

    KActionCollection* _actions;

    /** Clean state of the dock window.
     *
     * Used in slotResetLayout().
     */
    QByteArray _dockWindowCleanState;
};

}

#endif /* IMAGECONFIG_H */

