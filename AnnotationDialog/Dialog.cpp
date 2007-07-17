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

#include "Dialog.h"
#include "ListSelect.h"
#include <qpushbutton.h>
#include <qlabel.h>
#include <Q3HBoxLayout>
#include <Q3ValueList>
#include <Q3CString>
#include <QPixmap>
#include <QResizeEvent>
#include <QEvent>
#include <Q3VBoxLayout>
#include <QMoveEvent>
#include <QCloseEvent>
#include "Settings/SettingsData.h"
#include "ImagePreview.h"
#include "Viewer/ViewerWidget.h"
#include <q3accel.h>
#include <kstandarddirs.h>
#include "Editor.h"
#include <klocale.h>
#include <qlayout.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <q3popupmenu.h>
#include <qpoint.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <qdialog.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kiconloader.h>
#include "Utilities/ShowBusyCursor.h"
#include "KDateEdit.h"
#include "MainWindow/DeleteDialog.h"
#include <kguiitem.h>
#include <qobject.h>
#include "DB/CategoryCollection.h"
#include "DB/ImageInfo.h"
#include <kconfig.h>
#include "Utilities/Util.h"
#include "DB/ImageDB.h"
#include <qfile.h>
#include <qfileinfo.h>
#include "ShowSelectionOnlyManager.h"
#include "enums.h"
#include "MainWindow/DirtyIndicator.h"
#include <qtooltip.h>
#include <kdebug.h>
#include <QMainWindow>
#include <QDockWidget>
#include <QTimeEdit>
#include <QDir>

AnnotationDialog::Dialog::Dialog( QWidget* parent )
    : QDialog( parent ), _viewer(0)
{
    Utilities::ShowBusyCursor dummy;
    QVBoxLayout* layout = new QVBoxLayout( this );
    _dockWindow = new QMainWindow( this );
    _dockWindow->setWindowFlags(Qt::Widget);

    _dockWindow->setDockNestingEnabled( true );

    layout->addWidget( _dockWindow );

    // -------------------------------------------------- Label and Date
    // If I make the dateDock a child of 'this', then things seems to break.
    // The datedock isn't shown at all
    QDockWidget* dateDock = new QDockWidget( i18n("Label and Dates"), _dockWindow );
    dateDock->setObjectName( "Label and Dates" );
    dateDock->setAllowedAreas( Qt::AllDockWidgetAreas );
    dateDock->setWidget( createDateWidget() );
    _dockWindow->addDockWidget( Qt::TopDockWidgetArea, dateDock );



    // -------------------------------------------------- Image preview
    QDockWidget* previewDock = new QDockWidget( i18n("Image Preview"), _dockWindow );
    previewDock->setObjectName( "Image Preview" );
    previewDock->setAllowedAreas( Qt::AllDockWidgetAreas );
    previewDock->setWidget( createPreviewWidget() );
    _dockWindow->addDockWidget( Qt::TopDockWidgetArea, previewDock );




    // -------------------------------------------------- The editor
    QDockWidget* descriptionDock = new QDockWidget( i18n("Description"), _dockWindow );
    descriptionDock->setObjectName( "description" );
    descriptionDock->setAllowedAreas( Qt::AllDockWidgetAreas );
    _dockWindow->addDockWidget( Qt::LeftDockWidgetArea, descriptionDock );

    _description = new Editor( descriptionDock, "_description" );
    descriptionDock->setWidget( _description );

    // -------------------------------------------------- Categrories
    Q3ValueList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( Q3ValueList<DB::CategoryPtr>::ConstIterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
        if ( (*categoryIt)->isSpecialCategory() )
            continue;
        QDockWidget* dock = new QDockWidget( (*categoryIt)->text(), _dockWindow );
        dock->setAllowedAreas( Qt::AllDockWidgetAreas );
        dock->setObjectName( (*categoryIt)->name() );
        dock->setWidget( createListSel( *categoryIt ) );
        _dockWindow->addDockWidget( Qt::BottomDockWidgetArea, dock );
    }

    // -------------------------------------------------- The buttons.
    QHBoxLayout* lay1 = new QHBoxLayout;
    layout->addLayout( lay1 );

    _revertBut = new QPushButton( i18n("Revert This Item"), this );
    lay1->addWidget( _revertBut );

    QPushButton* clearBut = new KPushButton( KGuiItem(i18n("Clear Form"),QApplication::isRightToLeft()
                                             ? QString::fromLatin1("clear_left")
                                             : QString::fromLatin1("locationbar_erase")), this );
    lay1->addWidget( clearBut );

    QPushButton* optionsBut = new QPushButton( i18n("Options" ), this );
    lay1->addWidget( optionsBut );

    lay1->addStretch(1);

    _okBut = new KPushButton( KStandardGuiItem::ok(), this );
    lay1->addWidget( _okBut );

    QPushButton* cancelBut = new KPushButton( KStandardGuiItem::cancel(), this );
    lay1->addWidget( cancelBut );

    connect( _revertBut, SIGNAL( clicked() ), this, SLOT( slotRevert() ) );
    connect( _okBut, SIGNAL( clicked() ), this, SLOT( slotOK() ) );
    connect( cancelBut, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( clearBut, SIGNAL( clicked() ), this, SLOT(slotClear() ) );
    connect( optionsBut, SIGNAL( clicked() ), this, SLOT( slotOptions() ) );

    // Disable so no button accept return (which would break with the line edits)
    _revertBut->setAutoDefault( false );
    _okBut->setAutoDefault( false );
    _delBut->setAutoDefault( false );
    cancelBut->setAutoDefault( false );
    clearBut->setAutoDefault( false );
    optionsBut->setAutoDefault( false );

    _optionList.setAutoDelete( true );
    loadWindowLayout();

    // If I don't explicit show _dockWindow here, then no windows will show up.
    _dockWindow->show();

    setGeometry( Settings::SettingsData::instance()->windowGeometry( Settings::ConfigWindow ) );

    setupActions();
}

QWidget* AnnotationDialog::Dialog::createDateWidget()
{
    QWidget* top = new QWidget;
    QVBoxLayout* lay2 = new QVBoxLayout( top );

    // Image Label
    QHBoxLayout* lay3 = new QHBoxLayout;
    lay2->addLayout( lay3 );

    QLabel* label = new QLabel( i18n("Label: " ) );
    lay3->addWidget( label );
    _imageLabel = new KLineEdit( top );
    label->setBuddy( _imageLabel );
    lay3->addWidget( _imageLabel );


    // Date
    QHBoxLayout* lay4 = new QHBoxLayout;
    lay2->addLayout( lay4 );

    label = new QLabel( i18n("Date: ") );
    lay4->addWidget( label );

    _startDate = new ::AnnotationDialog::KDateEdit( true, top, "date config" );
    lay4->addWidget( _startDate, 1 );
    connect( _startDate, SIGNAL( dateChanged( const DB::ImageDate& ) ), this, SLOT( slotStartDateChanged( const DB::ImageDate& ) ) );
    label->setBuddy( _startDate );

    label = new QLabel( QString::fromLatin1( "-" ), top );
    lay4->addWidget( label );

    _endDate = new ::AnnotationDialog::KDateEdit( false, top, "date config" );
    lay4->addWidget( _endDate, 1 );

    // Time
    QHBoxLayout* lay7 = new QHBoxLayout;
    lay2->addLayout( lay7 );

    label = new QLabel( i18n("Time: ") );
    lay7->addWidget( label );

    _time= new QTimeEdit(top);
    lay7->addWidget( _time );
    lay7->addStretch(1);
    _time->hide();

    _addTime= new QPushButton(i18n("Add Time Info..."),top);
    lay7->addWidget( _addTime );
    lay7->addStretch(1);
    _addTime->hide();
    connect(_addTime,SIGNAL(clicked()), this, SLOT(slotAddTimeInfo()));

    lay2->addStretch(1);

    return top;
}


QWidget* AnnotationDialog::Dialog::createPreviewWidget()
{
    QWidget* top = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout( top );

    _preview = new ImagePreview( top );
    layout->addWidget( _preview, 1 );

    QHBoxLayout* hlay = new QHBoxLayout;
    layout->addLayout( hlay );
    hlay->addStretch(1);

    _prevBut = new QPushButton( top );
    _prevBut->setIconSet( KIconLoader::global()->loadIconSet( QString::fromLatin1( "1leftarrow" ), K3Icon::Desktop, 22 ) );
    _prevBut->setFixedWidth( 40 );
    hlay->addWidget( _prevBut );
    _prevBut->setToolTip( i18n("Annotate previous image") );

    _nextBut = new QPushButton( top );
    _nextBut->setIconSet( KIconLoader::global()->loadIconSet( QString::fromLatin1( "1rightarrow" ), K3Icon::Desktop, 22 ) );
    _nextBut->setFixedWidth( 40 );
    hlay->addWidget( _nextBut );
    _nextBut->setToolTip( i18n("Annotate next image") );

    hlay->addStretch(1);

    _rotateLeft = new QPushButton( top );
    hlay->addWidget( _rotateLeft );
    _rotateLeft->setIconSet( KIconLoader::global()->loadIconSet( QString::fromLatin1( "rotate_ccw" ), K3Icon::Desktop, 22 ) );
    _rotateLeft->setFixedWidth( 40 );
    _rotateLeft->setToolTip( i18n("Rotate contra-clockwise (to the left)") );

    _rotateRight = new QPushButton( top );
    hlay->addWidget( _rotateRight );
    _rotateRight->setIconSet( KIconLoader::global()->loadIconSet( QString::fromLatin1( "rotate_cw" ), K3Icon::Desktop, 22 ) );
    _rotateRight->setFixedWidth( 40 );
    _rotateRight->setToolTip( i18n("Rotate clockwise (to the right)") );

    _copyPreviousBut = new QPushButton( top );
    hlay->addWidget( _copyPreviousBut );
    _copyPreviousBut->setIconSet( KIconLoader::global()->loadIconSet( QString::fromLatin1( "legalmoves" ), K3Icon::Desktop, 22 ) );
    _copyPreviousBut->setFixedWidth( 40 );
    connect( _copyPreviousBut, SIGNAL( clicked() ), this, SLOT( slotCopyPrevious() ) );
    _copyPreviousBut->setToolTip( i18n("Copy tags from previously tagged image") );

    hlay->addStretch( 1 );
    _delBut = new QPushButton( top );
    _delBut->setPixmap( KIconLoader::global()->loadIcon( QString::fromLatin1( "editdelete" ), K3Icon::Desktop, 22 ) );
    hlay->addWidget( _delBut );
    connect( _delBut, SIGNAL( clicked() ), this, SLOT( slotDeleteImage() ) );
    _delBut->setToolTip( i18n("Delete image") );

    hlay->addStretch(1);

    return top;
}


void AnnotationDialog::Dialog::slotRevert()
{
    if ( _setup == InputSingleImageConfigMode )
        load();
}

void AnnotationDialog::Dialog::slotPrev()
{
    if ( _setup != InputSingleImageConfigMode )
        return;

    writeToInfo();
    if ( _current == 0 )
        return;

    _current--;
    if ( _setup == InputSingleImageConfigMode && _current != 0 )
        _preview->anticipate(_editList[ _current-1 ]);
    load();
}

void AnnotationDialog::Dialog::slotNext()
{
    if ( _setup != InputSingleImageConfigMode )
        return;

    if ( _current != -1 ) {
        writeToInfo();
    }
    if ( _current == (int)_origList.count()-1 )
        return;

    _current++;
    if ( _setup == InputSingleImageConfigMode && _current != (int)_origList.count()-1 )
        _preview->anticipate(_editList[ _current+1 ]);
    load();
}

void AnnotationDialog::Dialog::slotOK()
{
// I need to check for the changes first, as the case for _setup == InputSingleImageConfigMode, saves to the _origList,
    // and we can thus not check for changes anymore
    const bool anyChanges = hasChanges();

    if ( _setup == InputSingleImageConfigMode )  {
        writeToInfo();
        for ( uint i = 0; i < _editList.count(); ++i )  {
            *(_origList[i]) = _editList[i];
        }
    }
    else if ( _setup == InputMultiImageConfigMode ) {
        for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
            (*it)->slotReturn();
        }

        for( DB::ImageInfoListConstIterator it = _origList.constBegin(); it != _origList.constEnd(); ++it ) {
            DB::ImageInfoPtr info = *it;
            info->delaySavingChanges(true);
            info->rotate( _preview->angle() );
            if ( !_startDate->date().isNull() )
                info->setDate( DB::ImageDate( _startDate->date(), _endDate->date(), _time->time() ) );

            for( Q3PtrListIterator<ListSelect> listSelectIt( _optionList ); *listSelectIt; ++listSelectIt ) {
                info->addCategoryInfo( (*listSelectIt)->category(), (*listSelectIt)->itemsOn() );
                info->removeCategoryInfo( (*listSelectIt)->category(), (*listSelectIt)->itemsOff() );
            }

            if ( !_imageLabel->text().isEmpty() ) {
                info->setLabel( _imageLabel->text() );
            }

            if ( !_description->text().isEmpty() ) {
                info->setDescription( _description->text() );
            }

            info->delaySavingChanges(false);
        }
    }
    _accept = QDialog::Accepted;

    // I shouldn't emit changed before I've actually commited the changes, otherwise the listeners will act on the old data.
    if ( anyChanges ) {
        MainWindow::DirtyIndicator::markDirty();
        _thumbnailTextShouldReload = true;
    }

    QDialog::accept();
}

/*
 * Copy tags (only tags/categories, not description/label/...) from previous image to the currently showed one
 */
void AnnotationDialog::Dialog::slotCopyPrevious()
{
    if ( _setup != InputSingleImageConfigMode )
        return;
    if ( _current < 1 )
        return;

    // FIXME: it would be better to compute the "previous image" in a better way, but let's stick with this for now...
    DB::ImageInfo& old_info = _editList[ _current - 1 ];

    for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setSelection( old_info.itemsOfCategory( (*it)->category() ) );
    }
}

void AnnotationDialog::Dialog::load()
{
    DB::ImageInfo& info = _editList[ _current ];
    _startDate->setDate( info.date().start().date() );

    if( info.date().hasValidTime() ) {
        _time->show();
        _addTime->hide();
        _time->setTime( info.date().start().time());
    }
    else {
        _time->hide();
        _addTime->show();
    }

    if ( info.date().start().date() == info.date().end().date() )
        _endDate->setDate( QDate() );
    else
        _endDate->setDate( info.date().end().date() );

    _imageLabel->setText( info.label() );
    _description->setText( info.description() );

    for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setSelection( info.itemsOfCategory( (*it)->category() ) );
    }

    _nextBut->setEnabled( _current != (int)_origList.count()-1 );
    _prevBut->setEnabled( _current != 0 );
    _copyPreviousBut->setEnabled( _current != 0 );

    _preview->setImage( info );

    if ( _viewer )
        _viewer->load( Utilities::infoListToStringList(_origList), _current );

    if ( _setup == InputSingleImageConfigMode )
        setWindowTitle( i18n("KPhotoAlbum Annotations (%1/%2)", _current+1, _origList.count() ) );
}

void AnnotationDialog::Dialog::writeToInfo()
{
    for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->slotReturn();
    }

    DB::ImageInfo& info = _editList[ _current ];
    if ( !_time->isShown() ) {
        if ( _endDate->date().isValid() )
            info.setDate( DB::ImageDate( QDateTime( _startDate->date(), QTime(0,0,0) ),
                                     QDateTime( _endDate->date(), QTime( 23,59,59) ) ) );
        else
            info.setDate( DB::ImageDate( QDateTime( _startDate->date(), QTime(0,0,0) ),
                                     QDateTime( _startDate->date(), QTime( 23,59,59) ) ) );
    }
    else
        info.setDate( DB::ImageDate( _startDate->date(), _endDate->date(), _time->time() ) );


    info.setLabel( _imageLabel->text() );
    info.setDescription( _description->text() );
    for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        info.setCategoryInfo( (*it)->category(), (*it)->itemsOn() );
    }
}


int AnnotationDialog::Dialog::configure( DB::ImageInfoList list, bool oneAtATime )
{
    if ( oneAtATime )
        _setup = InputSingleImageConfigMode;
    else
        _setup = InputMultiImageConfigMode;

    _origList = list;
    _editList.clear();

    for( DB::ImageInfoListConstIterator it = list.constBegin(); it != list.constEnd(); ++it ) {
        _editList.append( *(*it) );
    }

    setup();

    if ( oneAtATime )  {
        _current = -1;
        slotNext();
    }
    else {
        _startDate->setDate( QDate() );
        _endDate->setDate( QDate() );
        _time->hide();
        _addTime->show();


        for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it )
            setUpCategoryListBoxForMultiImageSelection( *it, list );

        _imageLabel->setText( QString::fromLatin1("") );
        _description->setText( QString::fromLatin1("") );

        _prevBut->setEnabled( false );
        _nextBut->setEnabled( false );
        _copyPreviousBut->setEnabled( false );
    }

    _thumbnailShouldReload = false;
    _thumbnailTextShouldReload = false;

    showHelpDialog( oneAtATime ? InputSingleImageConfigMode : InputMultiImageConfigMode );
    return exec();
}

DB::ImageSearchInfo AnnotationDialog::Dialog::search( DB::ImageSearchInfo* search  )
{
    _setup = SearchMode;
    if ( search )
        _oldSearch = *search;

    setup();
    showHelpDialog( SearchMode );
    int ok = exec();
    if ( ok == QDialog::Accepted )  {
        _oldSearch = DB::ImageSearchInfo( DB::ImageDate( QDateTime(_startDate->date()), QDateTime(_endDate->date()) ),
                                      _imageLabel->text(), _description->text() );

        for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
            _oldSearch.setOption( (*it)->category(), (*it)->text() );
        }

        return _oldSearch;
    }
    else
        return DB::ImageSearchInfo();
}

void AnnotationDialog::Dialog::setup()
{
// Repopulate the listboxes in case data has changed
    // An group might for example have been renamed.
    for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->populate();
    }

    if ( _setup == SearchMode )  {
        _okBut->setGuiItem( KGuiItem(i18n("&Search"), QString::fromLatin1("find")) );
        _revertBut->hide();
        setCaption( i18n("Search") );
        loadInfo( _oldSearch );
        _preview->setImage( KStandardDirs::locate("data", QString::fromLatin1("kphotoalbum/pics/search.jpg") ) );
        _nextBut->setEnabled( false );
        _prevBut->setEnabled( false );
        _rotateLeft->setEnabled( false );
        _rotateRight->setEnabled( false );
    }
    else {
        _okBut->setGuiItem( KStandardGuiItem::ok() );
        _revertBut->setEnabled( _setup == InputSingleImageConfigMode );
        _revertBut->show();
        setWindowTitle( i18n("Annotations") );
        if ( _setup == InputMultiImageConfigMode ) {
            _preview->setImage( KStandardDirs::locate("data", QString::fromLatin1("kphotoalbum/pics/multiconfig.jpg") ) );
        }
        _rotateLeft->setEnabled( true );
        _rotateRight->setEnabled( true );
    }

    _delBut->setEnabled( _setup == InputSingleImageConfigMode );
    _copyPreviousBut->setEnabled( _setup == InputSingleImageConfigMode );

    for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it )
        (*it)->setMode( _setup );
}


void AnnotationDialog::Dialog::slotClear()
{
    loadInfo( DB::ImageSearchInfo() );
}

void AnnotationDialog::Dialog::loadInfo( const DB::ImageSearchInfo& info )
{
    _startDate->setDate( info.date().start().date() );
    _endDate->setDate( info.date().end().date() );

    for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setText( info.option( (*it)->category() ) );
    }

    _imageLabel->setText( info.label() );
    _description->setText( info.description() );
}

void AnnotationDialog::Dialog::viewerDestroyed()
{
    _viewer = 0;
}

void AnnotationDialog::Dialog::slotOptions()
{
    Q3PopupMenu menu( this, "context popup menu");
    menu.insertItem( i18n("Save Current Window Setup"), 1 );
    menu.insertItem( i18n( "Reset layout" ), 2 );
    int res = menu.exec( QCursor::pos() );
    if ( res == 1 )
        slotSaveWindowSetup();
    else if ( res == 2 )
        slotResetLayout();
}

/**
 * What I was trying (I guess) was to make the dialog a modal dialog, but I
 * couldn't do that (I guess) because the dialog has tear off windows,
 * which then would be inaccessable.
 *
 * Thefore I instead install an event filter that blocks events to the rest
 * of the world.
 * (Written years after this code, darn I would have loved if I had written
 * a comment here. tsk tsk)
 */
int AnnotationDialog::Dialog::exec()
{
    setupFocus();
    return QDialog::exec();

#ifdef TEMPORARILY_REMOVED
    show();
    showTornOfWindows();
    qApp->installEventFilter( this );
    qApp->eventLoop()->enterLoop();

    // Executed when the window is gone.
    qApp->removeEventFilter( this );
    hide();
    hideTornOfWindows();
    return _accept;
#endif
}

void AnnotationDialog::Dialog::slotSaveWindowSetup()
{
    const QByteArray data = _dockWindow->saveState();

    QFile file( QString::fromLatin1( "%1/layout.dat" ).arg( Settings::SettingsData::instance()->imageDirectory() ) );
    file.open( QIODevice::WriteOnly );
    file.write( data );
    file.close();
}

void AnnotationDialog::Dialog::closeEvent( QCloseEvent* e )
{
    e->ignore();
    reject();
}

void AnnotationDialog::Dialog::hideTornOfWindows()
{
#ifdef TEMPORARILY_REMOVED
    _tornOfWindows.clear();
    for( Q3ValueList<KDockWidget*>::Iterator it = _dockWidgets.begin(); it != _dockWidgets.end(); ++it ) {
        if ( (*it)->isTopLevel() && (*it)->isShown() ) {
            (*it)->hide();
            _tornOfWindows.append( *it );
        }
    }
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

void AnnotationDialog::Dialog::showTornOfWindows()
{
#ifdef TEMPORARILY_REMOVED
    for( Q3ValueList<KDockWidget*>::Iterator it = _tornOfWindows.begin(); it != _tornOfWindows.end(); ++it ) {
        (*it)->show();
    }
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

/**
 * See comment above for exec()
 */
bool AnnotationDialog::Dialog::eventFilter( QObject* watched, QEvent* event )
{
    if ( !watched->isWidgetType() )
        return false;

    QWidget* w = static_cast<QWidget*>( watched );

    if ( event->type() != QEvent::MouseButtonPress &&
         event->type() != QEvent::MouseButtonRelease &&
         event->type() != QEvent::MouseButtonDblClick &&
         event->type() != QEvent::MouseMove &&
         event->type() != QEvent::KeyPress &&
         event->type() != QEvent::KeyRelease &&
         event->type() != QEvent::ContextMenu )
        return false;

    // Initially I used an allow list, but combo boxes pop up menu's did for example not work then.
    if ( w->topLevelWidget()->className() == Q3CString( "MainWindow" ) || w->topLevelWidget()->className() == Q3CString( "Viewer" )) {
        if ( isMinimized() )
            showNormal();
        raise();
        return true;
    }

    return false;
}


QWidget* AnnotationDialog::Dialog::createListSel( const DB::CategoryPtr& category )
{
    ListSelect* sel = new ListSelect( category, _dockWindow );
    _optionList.append( sel );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL( itemRemoved( DB::Category*, const QString& ) ),
             this, SLOT( slotDeleteOption( DB::Category*, const QString& ) ) );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL( itemRenamed( DB::Category* , const QString& , const QString&  ) ),
             this, SLOT( slotRenameOption( DB::Category* , const QString& , const QString&  ) ) );

    return sel;
}

void AnnotationDialog::Dialog::slotDeleteOption( DB::Category* category, const QString& value )
{
    for( Q3ValueListIterator<DB::ImageInfo> it = _editList.begin(); it != _editList.end(); ++it ) {
        (*it).removeCategoryInfo( category->name(), value );
    }
}

void AnnotationDialog::Dialog::slotRenameOption( DB::Category* category, const QString& oldValue, const QString& newValue )
{
    for( Q3ValueListIterator<DB::ImageInfo> it = _editList.begin(); it != _editList.end(); ++it ) {
        (*it).renameItem( category->name(), oldValue, newValue );
    }
}

void AnnotationDialog::Dialog::reject()
{
    if ( hasChanges() ) {
        int code =  KMessageBox::questionYesNo( this, i18n("<p>Some changes are made to annotations. Do you really want to cancel all recent changes for each affected file?</p>") );
        if ( code == KMessageBox::No )
            return;
    }
    closeDialog();
}

void AnnotationDialog::Dialog::closeDialog()
{
    _accept = QDialog::Rejected;
    QDialog::reject();
}

bool AnnotationDialog::Dialog::hasChanges()
{
    bool changed = false;
    if ( _setup == InputSingleImageConfigMode )  {
        // PENDING(blackie) how about description and label?
        writeToInfo();
        for ( uint i = 0; i < _editList.count(); ++i )  {
            changed |= (*(_origList[i]) != _editList[i]);
        }
    }

    else if ( _setup == InputMultiImageConfigMode ) {
        changed |= ( !_startDate->date().isNull() );
        changed |= ( !_endDate->date().isNull() );

        for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
            QPair<StringSet, StringSet> origSelection = selectionForMultiSelect( *it, _origList );
            changed |= origSelection.first != (*it)->itemsOn();
            changed |= origSelection.second != (*it)->itemsUnchanged();
        }

        changed |= ( !_imageLabel->text().isEmpty() );
        changed |= ( !_description->text().isEmpty() );
    }
    return changed;
}

void AnnotationDialog::Dialog::rotateLeft()
{
    rotate(-90);
}

void AnnotationDialog::Dialog::rotateRight()
{
    rotate(90);
}

void AnnotationDialog::Dialog::rotate( int angle )
{
    _thumbnailShouldReload = true;
    if ( _setup == InputMultiImageConfigMode ) {
        // In slotOK the preview will be queried for its angle.
    }
    else {
        DB::ImageInfo& info = _editList[ _current ];
        info.rotate(angle);
    }
    _preview->rotate( angle );
}

bool AnnotationDialog::Dialog::thumbnailShouldReload() const
{
    return _thumbnailShouldReload;
}

bool AnnotationDialog::Dialog::thumbnailTextShouldReload() const
{
    return _thumbnailTextShouldReload;
}

void AnnotationDialog::Dialog::slotAddTimeInfo()
{
    _addTime->hide();
    _time->show();
}

void AnnotationDialog::Dialog::slotDeleteImage()
{
    Q_ASSERT( _setup != SearchMode );

    MainWindow::DeleteDialog dialog( this );
    DB::ImageInfoPtr info = _origList[_current];
    QStringList strList;
    strList << info->fileName();

    int ret = dialog.exec( strList );
    if ( ret == Rejected )
        return;

    _origList.remove( info );
    _editList.remove( _editList.at( _current ) );
    _thumbnailShouldReload = true;
    MainWindow::DirtyIndicator::markDirty();
    if ( _origList.count() == 0 ) {
        slotOK();
        return;
    }
    if ( _current == (int)_origList.count() ) // we deleted the last image
        _current--;

    load();
}

void AnnotationDialog::Dialog::showHelpDialog( UsageMode type )
{
    QString doNotShowKey;
    QString txt;
    if ( type == SearchMode ) {
        doNotShowKey = QString::fromLatin1( "image_config_search_show_help" );
        txt = i18n( "<p>You have just opened the advanced search dialog; to get the most out of it, "
                    "it is suggested that you read the section in the manual on <a href=\"help:/kphotoalbum/sect-general-image-searches.html\">"
                    "advanced searching</a>.</p>"
                    "<p>This dialog is also used for typing in information about images; you can find "
                    "extra tips on its usage by reading about "
                    "<a href=\"help:/kphotoalbum/chp-typingIn.html\">typing in</a>.</p>" );
    }
    else {
        doNotShowKey = QString::fromLatin1( "image_config_typein_show_help" );
        txt = i18n("<p>You have just opened one of the most important windows in KPhotoAlbum; "
                   "it contains lots of functionality which has been optimized for fast usage.<p>"
                   "<p>It is strongly recommended that you take 5 minutes to read the "
                   "<a href=\"help:/kphotoalbum/chp-typingIn.html\">documentation for this "
                   "dialog</a></p>" );
    }


    KMessageBox::information( this, txt, QString::null, doNotShowKey, KMessageBox::AllowLink );
}

void AnnotationDialog::Dialog::resizeEvent( QResizeEvent* )
{
    Settings::SettingsData::instance()->setWindowGeometry( Settings::ConfigWindow, geometry() );
}

void AnnotationDialog::Dialog::moveEvent( QMoveEvent * )
{
    Settings::SettingsData::instance()->setWindowGeometry( Settings::ConfigWindow, geometry() );

}


void AnnotationDialog::Dialog::setupFocus()
{
    static bool initialized = false;
    if ( initialized )
        return;
    initialized = true;

    QObjectList list = queryList( "QWidget" );
    Q3ValueList<QWidget*> orderedList;

    // Iterate through all widgets in our dialog.
    Q_FOREACH( QObject* obj, list ) {
        QWidget* current = static_cast<QWidget*>( obj );
        if ( /*!current->isVisible() || */current->focusPolicy() == Qt::NoFocus || current->inherits("QAbstractButton") )
            continue;

        int cx = current->mapToGlobal( QPoint(0,0) ).x();
        int cy = current->mapToGlobal( QPoint(0,0) ).y();

        bool inserted = false;
        // Iterate through the ordered list of widgets, and insert the current one, so it is in the right position in the tab chain.
        for( Q3ValueList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
            QWidget* w = *orderedIt;
            int wx = w->mapToGlobal( QPoint(0,0) ).x();
            int wy = w->mapToGlobal( QPoint(0,0) ).y();

            if ( wy > cy ||
                 ( wy == cy && wx >= cx ) ) {
                orderedList.insert( orderedIt, current );
                inserted = true;
                break;
            }
        }
        if (!inserted)
            orderedList.append( current );
    }

    // Now setup tab order.
    QWidget* prev = 0;
    for( Q3ValueList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
        if ( prev )
            setTabOrder( prev, *orderedIt );

        prev = *orderedIt;
    }

    // Finally set focus on the first list select
    for( Q3ValueList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
        if ( (*orderedIt)->objectName().startsWith( QString::fromLatin1("line edit for") ) ) {
            (*orderedIt)->setFocus();
            break;
        }
    }
}

void AnnotationDialog::Dialog::slotResetLayout()
{
    QString fileName =  QString::fromLatin1( "%1/layout.dat" ).arg( Settings::SettingsData::instance()->imageDirectory() );
    QDir().remove(fileName);

    emit deleteMe();
    closeDialog();
}

void AnnotationDialog::Dialog::slotStartDateChanged( const DB::ImageDate& date )
{
    if ( date.start() == date.end() )
        _endDate->setDate( QDate() );
    else
        _endDate->setDate( date.end().date() );
}

void AnnotationDialog::Dialog::loadWindowLayout()
{
    QString fileName =  QString::fromLatin1( "%1/layout.dat" ).arg( Settings::SettingsData::instance()->imageDirectory() );
    if ( !QFileInfo(fileName).exists() )
        return;

    QFile file( fileName );
    file.open( QIODevice::ReadOnly );
    QByteArray data = file.readAll();
    _dockWindow->restoreState(data);
}

void AnnotationDialog::Dialog::setupActions()
{
#ifdef TEMPORARILY_REMOVED
    _actions = new KActionCollection( this );

    new KAction( i18n("Sort Alphabetically"), 0, _optionList.at(0), SLOT( slotSortAlpha() ),
                 _actions, "annotationdialog-sort-alpha" );

    new KAction( i18n("Sort Most Recently Used"), 0, _optionList.at(0), SLOT( slotSortDate() ),
                 _actions, "annotationdialog-sort-MRU" );

    new KAction( i18n("Toggle Sorting"), Qt::CTRL+Qt::Key_T, _optionList.at(0), SLOT( toggleSortType() ),
                 _actions, "annotationdialog-toggle-sort" );

    new KAction( i18n("Toggle Showing Selected Items Only"), Qt::CTRL+Qt::Key_S, &ShowSelectionOnlyManager::instance(), SLOT( toggle() ),
                 _actions, "annotationdialog-toggle-showing-selected-only" );

    new KAction( i18n("Annotate Next"), Qt::Key_PageDown, this, SLOT( slotNext() ),
                 _actions, "annotationdialog-next-image" );

    new KAction( i18n("Annotate Previous"), Qt::Key_PageUp, this, SLOT( slotPrev() ),
                 _actions, "annotationdialog-prev-image" );

    new KAction( i18n("OK dialog"), Qt::CTRL+Qt::Key_Return, this, SLOT( slotOK() ),
                 _actions, "annotationdialog-OK-dialog" );

    new KAction( i18n("Delete"), Qt::CTRL+Qt::Key_Delete, this, SLOT( slotDeleteImage() ),
                 _actions, "annotationdialog-delete-image" );

    new KAction( i18n("Copy tags from previous image"), Qt::CTRL+Qt::Key_Insert, this, SLOT( slotCopyPrevious() ),
                 _actions, "annotationdialog-copy-previous");

    new KAction( i18n("Rotate Left"), 0, this, SLOT( rotateLeft() ), _actions, "annotationdialog-rotate-left" );
    new KAction( i18n("Rotate Right"), 0, this, SLOT( rotateRight() ), _actions, "annotationdialog-rotate-right" );

    connect( _nextBut, SIGNAL( clicked() ), this, SLOT( slotNext() ) );
    connect( _prevBut, SIGNAL( clicked() ), this, SLOT( slotPrev() ) );
    connect( _rotateLeft, SIGNAL( clicked() ), this, SLOT( rotateLeft() ) );
    connect( _rotateRight, SIGNAL( clicked() ), this, SLOT( rotateRight() ) );
#else
    kDebug() << "TEMPORILY REMOVED " << k_funcinfo << endl;
#endif // TEMPORARILY_REMOVED
}

KActionCollection* AnnotationDialog::Dialog::actions()
{
    return _actions;
}

void AnnotationDialog::Dialog::setUpCategoryListBoxForMultiImageSelection( ListSelect* listSel, const DB::ImageInfoList& images )
{
    QPair<StringSet,StringSet> selection = selectionForMultiSelect( listSel, images );
    listSel->setSelection( selection.first, selection.second );
}

QPair<StringSet,StringSet> AnnotationDialog::Dialog::selectionForMultiSelect( ListSelect* listSel, const DB::ImageInfoList& images )
{
    const QString category = listSel->category();
    const StringSet allItems = DB::ImageDB::instance()->categoryCollection()->categoryForName( category )->itemsInclCategories();
    StringSet itemsNotSelectedOnAllImages;
    StringSet itemsOnSomeImages;

    for ( DB::ImageInfoList::ConstIterator imageIt = images.begin(); imageIt != images.end(); ++ imageIt ) {
        const StringSet itemsOnThisImage = (*imageIt)->itemsOfCategory( category );
        itemsNotSelectedOnAllImages += ( allItems - itemsOnThisImage );
        itemsOnSomeImages += itemsOnThisImage;
    }

    const StringSet itemsOnAllImages = allItems - itemsNotSelectedOnAllImages;

    return qMakePair( itemsOnAllImages, itemsOnSomeImages - itemsOnAllImages );
}


#include "Dialog.moc"
