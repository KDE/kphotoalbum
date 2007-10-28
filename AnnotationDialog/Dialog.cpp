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
#include "Settings/SettingsData.h"
#include "ImagePreview.h"
#include "Viewer/ViewerWidget.h"
#include <qaccel.h>
#include <kstandarddirs.h>
#include "Editor.h"
#include <klocale.h>
#include <qlayout.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <qpopupmenu.h>
#include <qpoint.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <qdialog.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kiconloader.h>
#include "Utilities/ShowBusyCursor.h"
#include <ktimewidget.h>
#include "KDateEdit.h"
#include "MainWindow/DeleteDialog.h"
#include <kguiitem.h>
#include <qobjectlist.h>
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

using Utilities::StringSet;

AnnotationDialog::Dialog::Dialog( QWidget* parent, const char* name )
    : QDialog( parent, name ), _viewer(0)
{
    Utilities::ShowBusyCursor dummy;
    QVBoxLayout* layout = new QVBoxLayout( this, 6 );
    _dockWindow = new KDockMainWindow( 0 );

    _dockWindow->reparent( this, false, QPoint( 0,0 ) );
    layout->addWidget( _dockWindow );

    // -------------------------------------------------- Label and Date
    // If I make the dateDock a child of 'this', then things seems to break.
    // The datedock isn't shown at all
    KDockWidget* dateDock = _dockWindow->createDockWidget( QString::fromLatin1("Label and Dates"), QPixmap(), _dockWindow,
                                                           i18n("Label and Dates") );

    _dockWidgets.append( dateDock );
    QWidget* top = new QWidget( dateDock );
    QVBoxLayout* lay2 = new QVBoxLayout( top, 6 );
    dateDock->setWidget( top );

    // Image Label
    QHBoxLayout* lay3 = new QHBoxLayout( lay2, 6 );
    QLabel* label = new QLabel( i18n("Label: " ), top, "label" );
    lay3->addWidget( label );
    _imageLabel = new KLineEdit( top, "label line edit" );
    label->setBuddy( _imageLabel );
    lay3->addWidget( _imageLabel );


    // Date
    QHBoxLayout* lay4 = new QHBoxLayout( lay2, 6 );

    label = new QLabel( i18n("Date: "), top, "date label" );
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
    QHBoxLayout* lay7 = new QHBoxLayout( lay2, 6 );
    label = new QLabel( i18n("Time: "), top);
    lay7->addWidget( label );

    _time= new KTimeWidget(top);
    lay7->addWidget( _time );
    lay7->addStretch(1);
    _time->hide();

    _addTime= new QPushButton(i18n("Add Time Info..."),top);
    lay7->addWidget( _addTime );
    lay7->addStretch(1);
    _addTime->hide();
    connect(_addTime,SIGNAL(clicked()), this, SLOT(slotAddTimeInfo()));

    _dockWindow->setView( dateDock );

    // -------------------------------------------------- Image preview
    KDockWidget* previewDock
        = _dockWindow->createDockWidget( QString::fromLatin1("Image Preview"),
                                         locate("data", QString::fromLatin1("kphotoalbum/pics/imagesIcon.png") ),
                                         _dockWindow, i18n("Image Preview") );
    _dockWidgets.append( previewDock );
    QWidget* top2 = new QWidget( previewDock );
    QVBoxLayout* lay5 = new QVBoxLayout( top2, 6 );
    previewDock->setWidget( top2 );

    _preview = new ImagePreview( top2 );
    lay5->addWidget( _preview );

    QHBoxLayout* lay6 = new QHBoxLayout( lay5 );
    lay6->addStretch(1);

    _prevBut = new QPushButton( top2 );
    _prevBut->setIconSet( KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "1leftarrow" ), KIcon::Desktop, 22 ) );
    _prevBut->setFixedWidth( 40 );
    lay6->addWidget( _prevBut );
    QToolTip::add( _prevBut, i18n("Annotate previous image") );

    _nextBut = new QPushButton( top2 );
    _nextBut->setIconSet( KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "1rightarrow" ), KIcon::Desktop, 22 ) );
    _nextBut->setFixedWidth( 40 );
    lay6->addWidget( _nextBut );
    QToolTip::add( _nextBut, i18n("Annotate next image") );

    lay6->addStretch(1);

    _rotateLeft = new QPushButton( top2 );
    lay6->addWidget( _rotateLeft );
    _rotateLeft->setIconSet( KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "rotate_ccw" ), KIcon::Desktop, 22 ) );
    _rotateLeft->setFixedWidth( 40 );
    QToolTip::add( _rotateLeft, i18n("Rotate counterclockwise") );

    _rotateRight = new QPushButton( top2 );
    lay6->addWidget( _rotateRight );
    _rotateRight->setIconSet( KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "rotate_cw" ), KIcon::Desktop, 22 ) );
    _rotateRight->setFixedWidth( 40 );
    QToolTip::add( _rotateRight, i18n("Rotate clockwise") );

    _copyPreviousBut = new QPushButton( top2 );
    lay6->addWidget( _copyPreviousBut );
    _copyPreviousBut->setIconSet( KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "legalmoves" ), KIcon::Desktop, 22 ) );
    _copyPreviousBut->setFixedWidth( 40 );
    connect( _copyPreviousBut, SIGNAL( clicked() ), this, SLOT( slotCopyPrevious() ) );
    QToolTip::add( _copyPreviousBut, i18n("Copy tags from previously tagged image") );
    
    lay6->addStretch( 1 );
    _delBut = new QPushButton( top2 );
    _delBut->setPixmap( KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "editdelete" ), KIcon::Desktop, 22 ) );
    lay6->addWidget( _delBut );
    connect( _delBut, SIGNAL( clicked() ), this, SLOT( slotDeleteImage() ) );
    QToolTip::add( _delBut, i18n("Delete image") );

    lay6->addStretch(1);

    previewDock->manualDock( dateDock, KDockWidget::DockRight, 80  );


    // -------------------------------------------------- The editor
    KDockWidget* descriptionDock = _dockWindow->createDockWidget( QString::fromLatin1("Description"), QPixmap(), _dockWindow,
                                                                  i18n("Description") );

    _dockWidgets.append(descriptionDock);
    _description = new Editor( descriptionDock, "_description" );
    descriptionDock->setWidget( _description );
    descriptionDock->manualDock( dateDock, KDockWidget::DockBottom, 20 );

    // -------------------------------------------------- Categrories
    KDockWidget* last = descriptionDock;
    KDockWidget::DockPosition pos = KDockWidget::DockBottom;

    QValueList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( QValueList<DB::CategoryPtr>::ConstIterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
        KDockWidget* dockWidget = createListSel( *categoryIt );
        dockWidget->manualDock( last, pos );
        last = dockWidget;
        pos = KDockWidget::DockRight;
    }


    // -------------------------------------------------- The buttons.
    QHBoxLayout* lay1 = new QHBoxLayout( layout, 6 );

    _revertBut = new QPushButton( i18n("Revert This Item"), this );
    lay1->addWidget( _revertBut );

    QPushButton* clearBut = new KPushButton( KGuiItem(i18n("Clear Form"),QApplication::reverseLayout()
                                             ? QString::fromLatin1("clear_left")
                                             : QString::fromLatin1("locationbar_erase")), this );
    lay1->addWidget( clearBut );

    QPushButton* optionsBut = new QPushButton( i18n("Options" ), this );
    lay1->addWidget( optionsBut );

    lay1->addStretch(1);

    _okBut = new KPushButton( KStdGuiItem::ok(), this );
    lay1->addWidget( _okBut );

    QPushButton* cancelBut = new KPushButton( KStdGuiItem::cancel(), this );
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
        for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
            (*it)->slotReturn();
        }

        for( DB::ImageInfoListConstIterator it = _origList.constBegin(); it != _origList.constEnd(); ++it ) {
            DB::ImageInfoPtr info = *it;
            info->delaySavingChanges(true);
            info->rotate( _preview->angle() );
            if ( !_startDate->date().isNull() )
                info->setDate( DB::ImageDate( _startDate->date(), _endDate->date(), _time->time() ) );

            for( QPtrListIterator<ListSelect> listSelectIt( _optionList ); *listSelectIt; ++listSelectIt ) {
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
    qApp->eventLoop()->exitLoop();

    // I shouldn't emit changed before I've actually commited the changes, otherwise the listeners will act on the old data.
    if ( anyChanges ) {
        MainWindow::DirtyIndicator::markDirty();
        _thumbnailTextShouldReload = true;
    }
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

    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
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

    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setSelection( info.itemsOfCategory( (*it)->category() ) );
    }

    _nextBut->setEnabled( _current != (int)_origList.count()-1 );
    _prevBut->setEnabled( _current != 0 );
    _copyPreviousBut->setEnabled( _current != 0 );

    _preview->setImage( info );

    if ( _viewer )
        _viewer->load( Utilities::infoListToStringList(_origList), _current );

    if ( _setup == InputSingleImageConfigMode )
        setCaption( i18n("KPhotoAlbum Annotations (%1/%2)").arg( _current+1 ).arg( _origList.count() ) );
}

void AnnotationDialog::Dialog::writeToInfo()
{
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
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
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
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


        for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it )
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
        _oldSearch = DB::ImageSearchInfo( DB::ImageDate( _startDate->date(), _endDate->date() ),
                                      _imageLabel->text(), _description->text() );

        for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
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
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->populate();
    }

    if ( _setup == SearchMode )  {
        _okBut->setGuiItem( KGuiItem(i18n("&Search"), QString::fromLatin1("find")) );
        _revertBut->hide();
        setCaption( i18n("Search") );
        loadInfo( _oldSearch );
        _preview->setImage(Utilities::locateDataFile(QString::fromLatin1("pics/search.jpg")));
        _nextBut->setEnabled( false );
        _prevBut->setEnabled( false );
        _rotateLeft->setEnabled( false );
        _rotateRight->setEnabled( false );
    }
    else {
        _okBut->setGuiItem( KStdGuiItem::ok() );
        _revertBut->setEnabled( _setup == InputSingleImageConfigMode );
        _revertBut->show();
        setCaption( i18n("Annotations") );
        if ( _setup == InputMultiImageConfigMode ) {
            _preview->setImage(Utilities::locateDataFile(QString::fromLatin1("pics/multiconfig.jpg")));
        }
        _rotateLeft->setEnabled( true );
        _rotateRight->setEnabled( true );
    }

    _delBut->setEnabled( _setup == InputSingleImageConfigMode );
    _copyPreviousBut->setEnabled( _setup == InputSingleImageConfigMode );

    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it )
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

    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
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
    QPopupMenu menu( this, "context popup menu");
    menu.insertItem( i18n("Show/Hide Windows"),  _dockWindow->dockHideShowMenu());
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
    show();
    setupFocus();
    showTornOfWindows();
    qApp->installEventFilter( this );
    qApp->eventLoop()->enterLoop();

    // Executed when the window is gone.
    qApp->removeEventFilter( this );
    hide();
    hideTornOfWindows();
    return _accept;
}

void AnnotationDialog::Dialog::slotSaveWindowSetup()
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"), QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );
    QDomElement top = doc.createElement( QString::fromLatin1( "WindowLayout" ) );
    doc.appendChild( top );

    _dockWindow->writeDockConfig( top );
    QCString xml = doc.toCString();
    QFile file( QString::fromLatin1( "%1/layout.xml" ).arg( Settings::SettingsData::instance()->imageDirectory() ) );
    file.open( IO_WriteOnly );
    file.writeBlock( xml.data(), xml.size()-1 );
    file.close();
}

void AnnotationDialog::Dialog::closeEvent( QCloseEvent* e )
{
    e->ignore();
    reject();
}

void AnnotationDialog::Dialog::hideTornOfWindows()
{
    _tornOfWindows.clear();
    for( QValueList<KDockWidget*>::Iterator it = _dockWidgets.begin(); it != _dockWidgets.end(); ++it ) {
        if ( (*it)->isTopLevel() && (*it)->isShown() ) {
            (*it)->hide();
            _tornOfWindows.append( *it );
        }
    }
}

void AnnotationDialog::Dialog::showTornOfWindows()
{
    for( QValueList<KDockWidget*>::Iterator it = _tornOfWindows.begin(); it != _tornOfWindows.end(); ++it ) {
        (*it)->show();
    }
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
    if ( w->topLevelWidget()->className() == QCString( "MainWindow" ) || w->topLevelWidget()->className() == QCString( "Viewer" )) {
        if ( isMinimized() )
            showNormal();
        raise();
        return true;
    }

    return false;
}


KDockWidget* AnnotationDialog::Dialog::createListSel( const DB::CategoryPtr& category )
{
    KDockWidget* dockWidget = _dockWindow->createDockWidget( category->text(), category->icon(), _dockWindow, category->text() );
    _dockWidgets.append( dockWidget );
    ListSelect* sel = new ListSelect( category, dockWidget );
    _optionList.append( sel );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL( itemRemoved( DB::Category*, const QString& ) ),
             this, SLOT( slotDeleteOption( DB::Category*, const QString& ) ) );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL( itemRenamed( DB::Category* , const QString& , const QString&  ) ),
             this, SLOT( slotRenameOption( DB::Category* , const QString& , const QString&  ) ) );

    dockWidget->setWidget( sel );

    return dockWidget;
}

void AnnotationDialog::Dialog::slotDeleteOption( DB::Category* category, const QString& value )
{
    for( QValueListIterator<DB::ImageInfo> it = _editList.begin(); it != _editList.end(); ++it ) {
        (*it).removeCategoryInfo( category->name(), value );
    }
}

void AnnotationDialog::Dialog::slotRenameOption( DB::Category* category, const QString& oldValue, const QString& newValue )
{
    for( QValueListIterator<DB::ImageInfo> it = _editList.begin(); it != _editList.end(); ++it ) {
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
    qApp->eventLoop()->exitLoop();
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

        for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
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

    QObjectList* list = queryList( "QWidget" );
    QValueList<QWidget*> orderedList;

    // Iterate through all widgets in our dialog.
    for ( QObjectListIt inputIt( *list ); *inputIt; ++inputIt ) {
        QWidget* current = static_cast<QWidget*>( *inputIt );
        if ( !current->isVisible() || current->focusPolicy() == NoFocus || current->inherits("QPushButton") )
            continue;
        int cx = current->mapToGlobal( QPoint(0,0) ).x();
        int cy = current->mapToGlobal( QPoint(0,0) ).y();

        bool inserted = false;
        // Iterate through the ordered list of widgets, and insert the current one, so it is in the right position in the tab chain.
        for( QValueList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
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
    for( QValueList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
        if ( prev )
            setTabOrder( prev, *orderedIt );

        prev = *orderedIt;
    }
    delete list;

    // Finally set focus on the first list select
    for( QValueList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
        if ( QString::fromLatin1((*orderedIt)->name()).startsWith( QString::fromLatin1("line edit for") ) ) {
            (*orderedIt)->setFocus();
            break;
        }
    }
}

void AnnotationDialog::Dialog::slotResetLayout()
{
    QString dest =  QString::fromLatin1( "%1/layout.xml" ).arg( Settings::SettingsData::instance()->imageDirectory() );
    QString src =locate( "data", QString::fromLatin1( "kphotoalbum/default-layout.xml" ) );
    Utilities::copy( src,dest );

    deleteLater();
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
    QString fileName =  QString::fromLatin1( "%1/layout.xml" ).arg( Settings::SettingsData::instance()->imageDirectory() );
    if ( !QFileInfo(fileName).exists() )
        fileName =locate( "data", QString::fromLatin1( "kphotoalbum/default-layout.xml" ) );

    QFile file( fileName );
    file.open( IO_ReadOnly );
    QTextStream stream( &file );
    stream.setEncoding( QTextStream::UnicodeUTF8 );
    QString content = stream.read();

    QMap<QString,QString> map = DB::Category::standardCategories();
    for ( QMap<QString,QString>::ConstIterator it = map.begin(); it != map.end(); ++it )
        content.replace( it.key(), it.data() );

    QDomDocument doc;
    doc.setContent( content );
    QDomElement elm = doc.documentElement();
    _dockWindow->readDockConfig( elm );
}

void AnnotationDialog::Dialog::setupActions()
{
    _actions = new KActionCollection( this, "viewer", KGlobal::instance() );

    new KAction( i18n("Sort Alphabetically"), 0, _optionList.at(0), SLOT( slotSortAlpha() ),
                 _actions, "annotationdialog-sort-alpha" );

    new KAction( i18n("Sort Most Recently Used"), 0, _optionList.at(0), SLOT( slotSortDate() ),
                 _actions, "annotationdialog-sort-MRU" );

    new KAction( i18n("Toggle Sorting"), CTRL+Key_T, _optionList.at(0), SLOT( toggleSortType() ),
                 _actions, "annotationdialog-toggle-sort" );

    new KAction( i18n("Toggle Showing Selected Items Only"), CTRL+Key_S, &ShowSelectionOnlyManager::instance(), SLOT( toggle() ),
                 _actions, "annotationdialog-toggle-showing-selected-only" );

    new KAction( i18n("Annotate Next"), Key_PageDown, this, SLOT( slotNext() ),
                 _actions, "annotationdialog-next-image" );

    new KAction( i18n("Annotate Previous"), Key_PageUp, this, SLOT( slotPrev() ),
                 _actions, "annotationdialog-prev-image" );

    new KAction( i18n("OK dialog"), CTRL+Key_Return, this, SLOT( slotOK() ),
                 _actions, "annotationdialog-OK-dialog" );

    new KAction( i18n("Delete"), CTRL+Key_Delete, this, SLOT( slotDeleteImage() ),
                 _actions, "annotationdialog-delete-image" );

    new KAction( i18n("Copy tags from previous image"), CTRL+Key_Insert, this, SLOT( slotCopyPrevious() ),
                 _actions, "annotationdialog-copy-previous");

    new KAction( i18n("Rotate counterclockwise"), 0, this, SLOT( rotateLeft() ), _actions, "annotationdialog-rotate-left" );
    new KAction( i18n("Rotate clockwise"), 0, this, SLOT( rotateRight() ), _actions, "annotationdialog-rotate-right" );

    connect( _nextBut, SIGNAL( clicked() ), this, SLOT( slotNext() ) );
    connect( _prevBut, SIGNAL( clicked() ), this, SLOT( slotPrev() ) );
    connect( _rotateLeft, SIGNAL( clicked() ), this, SLOT( rotateLeft() ) );
    connect( _rotateRight, SIGNAL( clicked() ), this, SLOT( rotateRight() ) );
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
