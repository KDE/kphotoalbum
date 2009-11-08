/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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

#include "Dialog.h"
#include <KAction>
#include <KActionCollection>
#include <QList>
#include <QCloseEvent>
#include <QDir>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QTimeEdit>
#include <QVBoxLayout>
#include <kacceleratormanager.h>
#include <kguiitem.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <ktextedit.h>
#include <QMenu>
#include <qapplication.h>
#include <qcursor.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qpoint.h>

#ifdef HAVE_NEPOMUK
#   include <nepomuk/kratingwidget.h>
#endif

#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "DB/Result.h"
#include "DB/ResultId.h"
#include "ImagePreviewWidget.h"
#include "KDateEdit.h"
#include "ListSelect.h"
#include "MainWindow/DeleteDialog.h"
#include "MainWindow/DirtyIndicator.h"
#include "Settings/SettingsData.h"
#include "ShortCutManager.h"
#include "ShowSelectionOnlyManager.h"
#include "Utilities/ShowBusyCursor.h"
#include "Utilities/Util.h"
#include "Viewer/ViewerWidget.h"
#include "enums.h"

using Utilities::StringSet;

/**
 * \class AnnotationDialog::Dialog
 * \brief QDialog subclass used for tagging images
 */

AnnotationDialog::Dialog::Dialog( QWidget* parent )
    : QDialog( parent ), _ratingChanged(false)
{
    Utilities::ShowBusyCursor dummy;
    ShortCutManager shortCutManager;
    QVBoxLayout* layout = new QVBoxLayout( this );
    _dockWindow = new QMainWindow;
    _dockWindow->setDockNestingEnabled( true );

    layout->addWidget( _dockWindow );

    // -------------------------------------------------- Dock widgets
    QDockWidget* dock = createDock( i18n("Label and Dates"), QString::fromLatin1("Label and Dates"), Qt::TopDockWidgetArea, createDateWidget(shortCutManager) );

    createDock( i18n("Image Preview"), QString::fromLatin1("Image Preview"), Qt::TopDockWidgetArea, createPreviewWidget() );

    _description = new KTextEdit(this);
    _description->setProperty( "WantsFocus", true );

    dock = createDock( i18n("Description"), QString::fromLatin1("description"), Qt::LeftDockWidgetArea, _description );
    shortCutManager.addDock( dock, _description );

    // -------------------------------------------------- Categrories
     QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
     for( QList<DB::CategoryPtr>::ConstIterator categoryIt = categories.constBegin(); categoryIt != categories.constEnd(); ++categoryIt ) {
        ListSelect* sel = createListSel( *categoryIt );
        QDockWidget* dock = createDock( (*categoryIt)->text(), (*categoryIt)->name(), Qt::BottomDockWidgetArea, sel );
        shortCutManager.addDock( dock, sel->lineEdit() );
        if ( (*categoryIt)->isSpecialCategory() )
            dock->hide();
    }

    // -------------------------------------------------- The buttons.
    QHBoxLayout* lay1 = new QHBoxLayout;
    layout->addLayout( lay1 );

    _revertBut = new KPushButton( i18n("Revert This Item"), this );
    KAcceleratorManager::setNoAccel(_revertBut);
    lay1->addWidget( _revertBut );

    _clearBut = new KPushButton( KGuiItem(i18n("Clear Form"),QApplication::isRightToLeft()
                                             ? QString::fromLatin1("clear_left")
                                             : QString::fromLatin1("locationbar_erase")), this );
    KAcceleratorManager::setNoAccel(_clearBut);
    lay1->addWidget( _clearBut );

    KPushButton* optionsBut = new KPushButton( i18n("Options..." ), this );
    KAcceleratorManager::setNoAccel(optionsBut);
    lay1->addWidget( optionsBut );

    lay1->addStretch(1);

    _okBut = new KPushButton( i18n("&Done"), this );
    lay1->addWidget( _okBut );

    _continueLaterBut = new KPushButton( i18n("Continue &Later"), this );
    lay1->addWidget( _continueLaterBut );

    KPushButton* cancelBut = new KPushButton( KStandardGuiItem::cancel(), this );
    lay1->addWidget( cancelBut );

    // It is unfortunately not possible to ask KAcceleratorManager not to setup the OK and cancel keys.
    shortCutManager.addTaken( i18n("&Search") );
    shortCutManager.addTaken( _okBut->text() );
    shortCutManager.addTaken( _continueLaterBut->text());
    shortCutManager.addTaken( cancelBut->text() );

    connect( _revertBut, SIGNAL( clicked() ), this, SLOT( slotRevert() ) );
    connect( _okBut, SIGNAL( clicked() ), this, SLOT( doneTagging() ) );
    connect( _continueLaterBut, SIGNAL( clicked() ), this, SLOT( continueLater() ) );
    connect( cancelBut, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( _clearBut, SIGNAL( clicked() ), this, SLOT(slotClear() ) );
    connect( optionsBut, SIGNAL( clicked() ), this, SLOT( slotOptions() ) );
    
    connect( _preview, SIGNAL( imageRotated( int ) ), this, SLOT( rotate( int ) ) );
    connect( _preview, SIGNAL( indexChanged( int ) ), this, SLOT( slotIndexChanged( int ) ) );
    connect( _preview, SIGNAL( imageDeleted( const DB::ImageInfo& ) ), this, SLOT( slotDeleteImage() ) );
    connect( _preview, SIGNAL( copyPrevClicked() ), this, SLOT( slotCopyPrevious() ) );

    // Disable so no button accept return (which would break with the line edits)
    _revertBut->setAutoDefault( false );
    _okBut->setAutoDefault( false );
    _continueLaterBut->setAutoDefault( false );
    cancelBut->setAutoDefault( false );
    _clearBut->setAutoDefault( false );
    optionsBut->setAutoDefault( false );

    _optionList.setAutoDelete( true );

    _dockWindowCleanState = _dockWindow->saveState();

    loadWindowLayout();
    
    _current = -1;

    setGeometry( Settings::SettingsData::instance()->windowGeometry( Settings::AnnotationDialog ) );

    setupActions();
    shortCutManager.setupShortCuts();
}

QDockWidget* AnnotationDialog::Dialog::createDock( const QString& title, const QString& name,
                                                   Qt::DockWidgetArea location, QWidget* widget )
{
    QDockWidget* dock = new QDockWidget( title );
    KAcceleratorManager::setNoAccel(dock);
    dock->setObjectName( name );
    dock->setAllowedAreas( Qt::AllDockWidgetAreas );
    dock->setWidget( widget );
    _dockWindow->addDockWidget( location, dock );
    _dockWidgets.append( dock );
    return dock;
}

QWidget* AnnotationDialog::Dialog::createDateWidget(ShortCutManager& shortCutManager)
{
    QWidget* top = new QWidget;
    QVBoxLayout* lay2 = new QVBoxLayout( top );

    // Image Label
    QHBoxLayout* lay3 = new QHBoxLayout;
    lay2->addLayout( lay3 );

    QLabel* label = new QLabel( i18n("Label: " ) );
    lay3->addWidget( label );
    _imageLabel = new KLineEdit;
    _imageLabel->setProperty( "WantsFocus", true );
    lay3->addWidget( _imageLabel );
    shortCutManager.addLabel( label );
    label->setBuddy( _imageLabel );


    // Date
    QHBoxLayout* lay4 = new QHBoxLayout;
    lay2->addLayout( lay4 );

    label = new QLabel( i18n("Date: ") );
    lay4->addWidget( label );

    _startDate = new ::AnnotationDialog::KDateEdit( true );
    _startDate->setProperty( "WantsFocus", true );
    lay4->addWidget( _startDate, 1 );
    connect( _startDate, SIGNAL( dateChanged( const DB::ImageDate& ) ), this, SLOT( slotStartDateChanged( const DB::ImageDate& ) ) );
    shortCutManager.addLabel(label );
    label->setBuddy( _startDate);

    label = new QLabel( QString::fromLatin1( "-" ) );
    lay4->addWidget( label );

    _endDate = new ::AnnotationDialog::KDateEdit( false );
    _endDate->setProperty( "WantsFocus", true );
    lay4->addWidget( _endDate, 1 );

    // Time
    QHBoxLayout* lay7 = new QHBoxLayout;
    lay2->addLayout( lay7 );

    label = new QLabel( i18n("Time: ") );
    lay7->addWidget( label );

    _time= new QTimeEdit;
    _time->setProperty( "WantsFocus", true );
    lay7->addWidget( _time );
    lay7->addStretch(1);
    _time->hide();

    _addTime= new KPushButton(i18n("Add Time Info..."));
    _addTime->setProperty( "WantsFocus", true );
    lay7->addWidget( _addTime );
    lay7->addStretch(1);
    _addTime->hide();
    connect(_addTime,SIGNAL(clicked()), this, SLOT(slotAddTimeInfo()));

#ifdef HAVE_NEPOMUK
    _rating = new KRatingWidget( this );
    _rating->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    lay2->addStretch(1);
    lay2->addWidget( _rating, 0, Qt::AlignCenter );
    connect( _rating, SIGNAL( ratingChanged( unsigned int ) ), this, SLOT( slotRatingChanged( unsigned int ) ) );
#endif

    lay2->addStretch(1);

    return top;
}

QWidget* AnnotationDialog::Dialog::createPreviewWidget()
{
    _preview = new ImagePreviewWidget();
    return _preview;
}

void AnnotationDialog::Dialog::slotRevert()
{
    if ( _setup == InputSingleImageConfigMode )
        load();
}

void AnnotationDialog::Dialog::slotIndexChanged( int index )
{
  if ( _setup != InputSingleImageConfigMode )
        return;
    
    if(_current >= 0 )
      writeToInfo();
    
    _current = index;
    
    load();
}

void AnnotationDialog::Dialog::doneTagging()
{
    saveAndClose();
    if ( Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured() ) {
        for( DB::ImageInfoListIterator it = _origList.begin(); it != _origList.end(); ++it ) {
            (*it)->removeCategoryInfo( Settings::SettingsData::instance()->untaggedCategory(),
                                      Settings::SettingsData::instance()->untaggedTag() );
        }
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

#ifdef HAVE_NEPOMUK
    if ( _setup == InputSingleImageConfigMode )
        _rating->setRating( qMax( static_cast<short int>(0), info.rating() ) );
    _ratingChanged = false;
#endif

    for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setSelection( info.itemsOfCategory( (*it)->category() ) );
        (*it)->rePopulate();
    }

    if ( _setup == InputSingleImageConfigMode )
        setWindowTitle( i18n("KPhotoAlbum Annotations (%1/%2)", _current+1, _origList.count() ) );
}

void AnnotationDialog::Dialog::writeToInfo()
{
    for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->slotReturn();
    }

    DB::ImageInfo& info = _editList[ _current ];
    if ( _time->isHidden() ) {
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
    info.setDescription( _description->toPlainText() );
    for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        info.setCategoryInfo( (*it)->category(), (*it)->itemsOn() );
    }

#ifdef HAVE_NEPOMUK
    if ( _ratingChanged ) {
        info.setRating( _rating->rating() );
        _ratingChanged = false;
    }
#endif
}


int AnnotationDialog::Dialog::configure( DB::ImageInfoList list, bool oneAtATime )
{
    if ( Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured() ) {
        DB::ImageDB::instance()->categoryCollection()->categoryForName( Settings::SettingsData::instance()->untaggedCategory() )
            ->addItem(Settings::SettingsData::instance()->untaggedTag() );
    }
    

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
        _current = 0;
        _preview->configure( &_editList, true );
        load();
    }
    else {
        _preview->configure( &_editList, false );
        _startDate->setDate( QDate() );
        _endDate->setDate( QDate() );
        _time->hide();
        _addTime->show();
#ifdef HAVE_NEPOMUK
        _rating->setRating( 0 );
        _ratingChanged = false;
#endif

        for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it )
            setUpCategoryListBoxForMultiImageSelection( *it, list );

        _imageLabel->setText( QString::fromLatin1("") );
        _description->setText( QString::fromLatin1("") );

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

    _preview->setImage(Utilities::locateDataFile(QString::fromLatin1("pics/search.jpg")));

#ifdef HAVE_NEPOMUK
        _rating->setRating( 0 );
        _ratingChanged = false ;
#endif

    showHelpDialog( SearchMode );
    int ok = exec();
    if ( ok == QDialog::Accepted )  {
        const QDateTime start = _startDate->date().isNull() ? QDateTime() : QDateTime(_startDate->date());
        const QDateTime end = _endDate->date().isNull() ? QDateTime() : QDateTime( _endDate->date() );
        _oldSearch = DB::ImageSearchInfo( DB::ImageDate( start, end ),
                                      _imageLabel->text(), _description->toPlainText() );

        for( Q3PtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
            _oldSearch.setCategoryMatchText( (*it)->category(), (*it)->text() );
        }
#ifdef HAVE_NEPOMUK
        //FIXME: for the user to search for 0-rated images, he must first change the rating to anything > 0
        //then change back to 0 .
        if( _ratingChanged)
          _oldSearch.setRating( _rating->rating() );
        
        _ratingChanged = false;
#endif
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
        _continueLaterBut->hide();
        _revertBut->hide();
        _clearBut->show();
        setWindowTitle( i18n("Search") );
        loadInfo( _oldSearch );
    }
    else {
        _okBut->setText( i18n("Done") );
        _continueLaterBut->show();
        _revertBut->setEnabled( _setup == InputSingleImageConfigMode );
        _clearBut->hide();
        _revertBut->show();
        setWindowTitle( i18n("Annotations") );
    }

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
        (*it)->setText( info.categoryMatchText( (*it)->category() ) );
    }

    _imageLabel->setText( info.label() );
    _description->setText( info.description() );
}

void AnnotationDialog::Dialog::slotOptions()
{
    QMenu menu( this );
    QAction* saveCurrent = menu.addAction( i18n("Save Current Window Setup") );
    QAction* reset = menu.addAction( i18n( "Reset layout" ) );
    QAction* res = menu.exec( QCursor::pos() );
    if ( res == saveCurrent )
        slotSaveWindowSetup();
    else if ( res == reset )
        slotResetLayout();
}

int AnnotationDialog::Dialog::exec()
{
    showTornOfWindows();
    show(); // We need to call show before we call setupFocus() otherwise the widget will not yet all have been moved in place.
    setupFocus();
    const int ret = QDialog::exec();
    hideTornOfWindows();
    return ret;
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
    Q_FOREACH( QDockWidget* dock, _dockWidgets ) {
        if ( dock->isFloating() )
            dock->hide();
    }
}

void AnnotationDialog::Dialog::showTornOfWindows()
{
    Q_FOREACH (QDockWidget* dock, _dockWidgets ) {
        if ( dock->isFloating() )
            dock->show();
    }
}


AnnotationDialog::ListSelect* AnnotationDialog::Dialog::createListSel( const DB::CategoryPtr& category )
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
    for( QList<DB::ImageInfo>::Iterator it = _editList.begin(); it != _editList.end(); ++it ) {
        (*it).removeCategoryInfo( category->name(), value );
    }
}

void AnnotationDialog::Dialog::slotRenameOption( DB::Category* category, const QString& oldValue, const QString& newValue )
{
    for( QList<DB::ImageInfo>::Iterator it = _editList.begin(); it != _editList.end(); ++it ) {
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
        for ( int i = 0; i < _editList.count(); ++i )  {
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
        changed |= ( !_description->toPlainText().isEmpty() );
        changed |= _ratingChanged;
    }
    return changed;
}

void AnnotationDialog::Dialog::rotate( int angle )
{
    _thumbnailShouldReload = true;
    if ( _setup == InputMultiImageConfigMode ) {
        // In doneTagging the preview will be queried for its angle.
    }
    else {
        DB::ImageInfo& info = _editList[ _current ];
        info.rotate(angle);
    }
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
    
    if( _setup == InputMultiImageConfigMode )  //TODO: probably delete here should mean remove from selection
      return;
    
    MainWindow::DeleteDialog dialog( this );
    DB::ImageInfoPtr info = _origList[_current];

    DB::ResultId idToDelete = DB::ImageDB::instance()->ID_FOR_FILE(info->fileName(DB::AbsolutePath));
    const DB::Result deleteList = DB::Result(idToDelete);

    int ret = dialog.exec( deleteList );
    if ( ret == Rejected )
        return;

    _origList.remove( info );
    _editList.removeAll( _editList.at( _current ) );
    _thumbnailShouldReload = true;
    MainWindow::DirtyIndicator::markDirty();
    if ( _origList.count() == 0 ) {
        doneTagging();
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
                   "it contains lots of functionality which has been optimized for fast usage.</p>"
                   "<p>It is strongly recommended that you take 5 minutes to read the "
                   "<a href=\"help:/kphotoalbum/chp-typingIn.html\">documentation for this "
                   "dialog</a></p>" );
    }


    KMessageBox::information( this, txt, QString::null, doNotShowKey, KMessageBox::AllowLink );
}

void AnnotationDialog::Dialog::resizeEvent( QResizeEvent* )
{
    Settings::SettingsData::instance()->setWindowGeometry( Settings::AnnotationDialog, geometry() );
}

void AnnotationDialog::Dialog::moveEvent( QMoveEvent * )
{
    Settings::SettingsData::instance()->setWindowGeometry( Settings::AnnotationDialog, geometry() );

}


void AnnotationDialog::Dialog::setupFocus()
{
    QList<QWidget*> list = findChildren<QWidget*>();
    QList<QWidget*> orderedList;

    // Iterate through all widgets in our dialog.
    Q_FOREACH( QObject* obj, list ) {
        QWidget* current = static_cast<QWidget*>( obj );
        if ( !current->property("WantsFocus").isValid() )
            continue;

        int cx = current->mapToGlobal( QPoint(0,0) ).x();
        int cy = current->mapToGlobal( QPoint(0,0) ).y();

        bool inserted = false;
        // Iterate through the ordered list of widgets, and insert the current one, so it is in the right position in the tab chain.
        for( QList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
            const QWidget* w = *orderedIt;
            int wx = w->mapToGlobal( QPoint(0,0) ).x();
            int wy = w->mapToGlobal( QPoint(0,0) ).y();

            if ( wy > cy || ( wy == cy && wx >= cx ) ) {
                orderedList.insert( orderedIt, current );
                inserted = true;
                break;
            }
        }
        if (!inserted)
            orderedList.append( current );
    }


    // now setup tab order.
    QWidget* prev = 0;
    for( QList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
        if ( prev ) {
            setTabOrder( prev, *orderedIt );
        }
        prev = *orderedIt;
    }

    // Finally set focus on the first list select
    for( QList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
        if ( (*orderedIt)->property("FocusCandidate").isValid() && (*orderedIt)->isVisible() ) {
            (*orderedIt)->setFocus();
            break;
        }
    }
}

void AnnotationDialog::Dialog::slotResetLayout()
{
    _dockWindow->restoreState(_dockWindowCleanState);
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
    _actions = new KActionCollection( this );

    KAction* action = 0;
    action = _actions->addAction( QString::fromLatin1("annotationdialog-sort-alpha"), _optionList.at(0), SLOT( slotSortAlpha() ) );
    action->setText( i18n("Sort Alphabetically") );
    action->setShortcut(Qt::CTRL+Qt::Key_F4);


    action = _actions->addAction( QString::fromLatin1("annotationdialog-sort-MRU"), _optionList.at(0), SLOT( slotSortDate() ) );
    action->setText( i18n("Sort Most Recently Used") );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-toggle-sort"),  _optionList.at(0), SLOT( toggleSortType() ) );
    action->setText( i18n("Toggle Sorting") );
    action->setShortcut( Qt::CTRL+Qt::Key_T );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-toggle-showing-selected-only"),
                                  &ShowSelectionOnlyManager::instance(), SLOT( toggle() ) );
    action->setText( i18n("Toggle Showing Selected Items Only") );
    action->setShortcut( Qt::CTRL+Qt::Key_S );


    action = _actions->addAction( QString::fromLatin1("annotationdialog-next-image"),  _preview, SLOT( slotNext() ) );
    action->setText(  i18n("Annotate Next") );
    action->setShortcut(  Qt::Key_PageDown );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-prev-image"),  _preview, SLOT( slotPrev() ) );
    action->setText(  i18n("Annotate Previous") );
    action->setShortcut(  Qt::Key_PageUp );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-OK-dialog"),  this, SLOT( doneTagging() ) );
    action->setText(  i18n("OK dialog") );
    action->setShortcut(  Qt::CTRL+Qt::Key_Return );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-delete-image"),  this, SLOT( slotDeleteImage() ) );
    action->setText(  i18n("Delete") );
    action->setShortcut(  Qt::CTRL+Qt::Key_Delete );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-copy-previous"),  this, SLOT( slotCopyPrevious() ) );
    action->setText(  i18n("Copy tags from previous image") );
    action->setShortcut(  Qt::ALT+Qt::Key_Insert );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-rotate-left"),  _preview, SLOT( rotateLeft() ) );
    action->setText(  i18n("Rotate counterclockwise") );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-rotate-right"),  _preview, SLOT( rotateRight() ) );
    action->setText(  i18n("Rotate clockwise") );

    foreach (QAction* action, _actions->actions()) {
      action->setShortcutContext(Qt::WindowShortcut);
      addAction(action);
  }
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
    const StringSet allItems = DB::ImageDB::instance()->categoryCollection()->categoryForName( category )->itemsInclCategories().toSet();
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

void AnnotationDialog::Dialog::slotRatingChanged( unsigned int )
{
    _ratingChanged = true;
}

void AnnotationDialog::Dialog::continueLater()
{
    saveAndClose();
}

void AnnotationDialog::Dialog::saveAndClose()
{
    if (_origList.isEmpty()) {
        // all images are deleted.
        QDialog::accept();
        return;
    }

    // I need to check for the changes first, as the case for _setup == InputSingleImageConfigMode, saves to the _origList,
    // and we can thus not check for changes anymore
    const bool anyChanges = hasChanges();

    if ( _setup == InputSingleImageConfigMode )  {
        writeToInfo();
        for ( int i = 0; i < _editList.count(); ++i )  {
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
            if ( !_startDate->date().isNull() )
                info->setDate( DB::ImageDate( _startDate->date(), _endDate->date(), _time->time() ) );

            for( Q3PtrListIterator<ListSelect> listSelectIt( _optionList ); *listSelectIt; ++listSelectIt ) {
                info->addCategoryInfo( (*listSelectIt)->category(), (*listSelectIt)->itemsOn() );
                info->removeCategoryInfo( (*listSelectIt)->category(), (*listSelectIt)->itemsOff() );
            }

            if ( !_imageLabel->text().isEmpty() ) {
                info->setLabel( _imageLabel->text() );
            }


            if ( !_description->toPlainText().isEmpty() ) {
                info->setDescription( _description->toPlainText() );
            }

#ifdef HAVE_NEPOMUK
            if( _ratingChanged)
            {
              info->setRating( _rating->rating() );
            }
#endif

            info->delaySavingChanges(false);
        }
#ifdef HAVE_NEPOMUK
        _ratingChanged = false;
#endif
    }
    _accept = QDialog::Accepted;

    // I shouldn't emit changed before I've actually commited the changes, otherwise the listeners will act on the old data.
    if ( anyChanges ) {
        MainWindow::DirtyIndicator::markDirty();
        _thumbnailTextShouldReload = true;
    }

    QDialog::accept();
}

#include "Dialog.moc"
