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

#include "imageconfig.h"
#include "listselect.h"
#include <qspinbox.h>
#include <qcombobox.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include "options.h"
#include "imagepreview.h"
#include <qregexp.h>
#include <qtabwidget.h>
#include "viewer.h"
#include <qaccel.h>
#include <kstandarddirs.h>
#include "editor.h"
#include <klocale.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <qpopupmenu.h>
#include <qpoint.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <qdialog.h>

ImageConfig::ImageConfig( QWidget* parent, const char* name )
    : QDialog( parent, name ), _viewer(0)
{
    QVBoxLayout* layout = new QVBoxLayout( this, 6 );
    _dockWindow = new KDockMainWindow( 0 );
    _dockWindow->reparent( this, false, QPoint( 0,0 ) );
    layout->addWidget( _dockWindow );

    // -------------------------------------------------- Label and Date
    // If I make the dateDock a child of 'this', then things seems to break.
    // The datedock isn't shown at all
    KDockWidget* dateDock = _dockWindow->createDockWidget( i18n("Label and Dates"), QPixmap(), 0 );
    _dockWidgets.append( dateDock );
    QWidget* top = new QWidget( dateDock );
    QVBoxLayout* lay2 = new QVBoxLayout( top, 6 );
    dateDock->setWidget( top );

    // Image Label
    QHBoxLayout* lay3 = new QHBoxLayout( lay2, 6 );
    QLabel* label = new QLabel( i18n("Label: " ), top );
    lay3->addWidget( label );
    _imageLabel = new KLineEdit( top );
    lay3->addWidget( _imageLabel );

    // Date
    QHBoxLayout* lay4 = new QHBoxLayout( lay2, 6 );

    label = new QLabel( i18n("From: "), top );
    lay4->addWidget( label );

    _dayStart = new QSpinBox( 0, 31, 1, top );
    _dayStart->setSpecialValueText( QString::fromLatin1( "---" ) );
    lay4->addWidget( _dayStart );

    label = new QLabel( QString::fromLatin1( "-" ), top );
    lay4->addWidget( label );

    _monthStart = new QComboBox( top );
    _monthStart->insertStringList( QStringList() << QString::fromLatin1( "---" )
                                   << i18n( "Jan" ) << i18n( "Feb" ) << i18n( "Mar" ) << i18n( "Apr" )
                                   << i18n( "May" ) << i18n( "June" ) << i18n( "July" ) << i18n( "Aug" )
                                   << i18n( "Sep" ) << i18n( "Oct" ) << i18n( "Nov" ) << i18n( "Dec" ) );
    lay4->addWidget( _monthStart );

    label = new QLabel( QString::fromLatin1( "-" ), top );
    lay4->addWidget( label );

    _yearStart = new QSpinBox( 0, 9999, 1, top );
    _yearStart->setSpecialValueText( QString::fromLatin1( "---" ) );
    lay4->addWidget( _yearStart );

    lay4->addStretch(1);

    label = new QLabel( i18n("To: "), top );
    lay4->addWidget( label );

    _dayEnd = new QSpinBox( 0, 31, 1, top );
    _dayStart->setSpecialValueText( QString::fromLatin1( "---" ) );
    lay4->addWidget( _dayEnd );

    label = new QLabel( QString::fromLatin1( "-" ), top );
    lay4->addWidget( label );

    _monthEnd = new QComboBox( top );
    _monthEnd->insertStringList( QStringList() << QString::fromLatin1( "---" )
                                   << i18n( "Jan" ) << i18n( "Feb" ) << i18n( "Mar" ) << i18n( "Apr" )
                                   << i18n( "May" ) << i18n( "June" ) << i18n( "July" ) << i18n( "Aug" )
                                   << i18n( "Sep" ) << i18n( "Oct" ) << i18n( "Nov" ) << i18n( "Dec" ) );
    lay4->addWidget( _monthEnd );

    label = new QLabel( QString::fromLatin1( "-" ), top );
    lay4->addWidget( label );

    _yearEnd = new QSpinBox( 0, 9999, 1, top );
    _yearEnd->setSpecialValueText( QString::fromLatin1( "---" ) );
    lay4->addWidget( _yearEnd );

    _dockWindow->setView( dateDock );

    // -------------------------------------------------- Image preview
    KDockWidget* previewDock
        = _dockWindow->createDockWidget( i18n("Image Preview"),
                            locate("data", QString::fromLatin1("kimdaba/pics/imagesIcon.png") ));
    _dockWidgets.append( previewDock );
    QWidget* top2 = new QWidget( previewDock );
    QVBoxLayout* lay5 = new QVBoxLayout( top2, 6 );
    previewDock->setWidget( top2 );

    _preview = new ImagePreview( top2 );
    lay5->addWidget( _preview );

    QHBoxLayout* lay6 = new QHBoxLayout( lay5 );
    lay6->addStretch(1);

    _prevBut = new QPushButton( QString::fromLatin1( "<" ), top2 );
    _prevBut->setFixedWidth( 30 );
    lay6->addWidget( _prevBut );

    _nextBut = new QPushButton( QString::fromLatin1( ">" ), top2 );
    _nextBut->setFixedWidth( 30 );
    lay6->addWidget( _nextBut );

    lay6->addStretch(1);

    previewDock->manualDock( dateDock, KDockWidget::DockRight );


    // -------------------------------------------------- The editor
    KDockWidget* descriptionDock = _dockWindow->createDockWidget( i18n("Description"), QPixmap() );
    _dockWidgets.append(descriptionDock);
    _description = new Editor( descriptionDock );
    descriptionDock->setWidget( _description );
    descriptionDock->manualDock( dateDock, KDockWidget::DockBottom, 0 );


    // -------------------------------------------------- Option groups
    KDockWidget* last = dateDock;
    KDockWidget::DockPosition pos = KDockWidget::DockTop;

    Options* opt = Options::instance();
    QStringList grps = opt->optionGroups();
    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        KDockWidget* dockWidget = createListSel( *it );
        dockWidget->manualDock( last, pos );
        last = dockWidget;
        pos = KDockWidget::DockRight;
    }


    // -------------------------------------------------- The buttons.
    QHBoxLayout* lay1 = new QHBoxLayout( layout, 6 );

    //buttonDock->setWidget( buttons );
    //_dockWindow->setView( buttonDock);
    //_dockWindow->setMainDockWidget( buttonDock);

    _revertBut = new QPushButton( i18n("Revert this image"), this );
    lay1->addWidget( _revertBut );

    QPushButton* clearBut = new QPushButton( i18n("Clear form"), this );
    lay1->addWidget( clearBut );

    QPushButton* optionsBut = new QPushButton( i18n("Options..." ), this );
    lay1->addWidget( optionsBut );

    lay1->addStretch(1);

    _okBut = new QPushButton( i18n("OK"), this );
    lay1->addWidget( _okBut );

    QPushButton* cancelBut = new QPushButton( i18n("Cancel"), this );
    lay1->addWidget( cancelBut );

    connect( _revertBut, SIGNAL( clicked() ), this, SLOT( slotRevert() ) );
    connect( _okBut, SIGNAL( clicked() ), this, SLOT( slotOK() ) );
    connect( cancelBut, SIGNAL( clicked() ), this, SLOT( slotCancel() ) );
    connect( clearBut, SIGNAL( clicked() ), this, SLOT(slotClear() ) );
    connect( optionsBut, SIGNAL( clicked() ), this, SLOT( slotOptions() ) );


    // Connect PageUp/PageDown to prev/next
    QAccel* accel = new QAccel( this, "accel for ImageConfig" );
    accel->connectItem( accel->insertItem( Key_PageDown ), this, SLOT( slotNext() ) );
    accel->connectItem( accel->insertItem( Key_PageUp ), this, SLOT( slotPrev() ) );

    _optionList.setAutoDelete( true );
    Options::instance()->loadConfigWindowLayout( this );

    // If I don't explicit show _dockWindow here, then no windows will show up.
    _dockWindow->show();
}


void ImageConfig::slotRevert()
{
    if ( _setup == SINGLE )
        load();
}

void ImageConfig::slotPrev()
{
    save();
    if ( _current == 0 )
        return;

    _current--;
    load();
}

void ImageConfig::slotNext()
{
    if ( _current != -1 ) {
        save();
    }
    if ( _current == (int)_origList.count()-1 )
        return;

    _current++;
    load();
}

void ImageConfig::slotOK()
{
    bool change = false;
    if ( _setup == SINGLE )  {
        save();
        for ( uint i = 0; i < _editList.count(); ++i )  {
            change |= (*(_origList.at(i)) != _editList[i]);
            *(_origList.at(i)) = _editList[i];
        }
    }

    else if ( _setup == MULTIPLE ) {
        for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
            (*it)->slotReturn();
        }

        for( ImageInfoListIterator it( _origList ); *it; ++it ) {
            ImageInfo* info = *it;
            if ( _dayStart->value() != 0 ||  _monthStart->currentText() != QString::fromLatin1("---") || _yearStart->value() != 0 )  {
                info->startDate().setDay( _dayStart->value() );
                info->startDate().setMonth( _monthStart->currentItem() );
                info->startDate().setYear( _yearStart->value() );
                change = true;
            }

            if ( _dayEnd->value() != 0 || _monthEnd->currentText() != QString::fromLatin1("---") || _yearEnd->value() != 0 )  {
                info->endDate().setDay( _dayEnd->value() );
                info->endDate().setMonth( _monthEnd->currentItem() );
                info->endDate().setYear( _yearEnd->value() );
                change = true;
            }

            for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
                if ( (*it)->selection().count() != 0 )  {
                    change = true;
                    if ( (*it)->merge() )
                        info->addOption( (*it)->optionGroup(),  (*it)->selection() );
                    else
                        info->setOption( (*it)->optionGroup(),  (*it)->selection() );
                }
            }

            if ( !_imageLabel->text().isEmpty() ) {
                change = true;
                info->setLabel( _imageLabel->text() );
            }

            if ( !_description->text().isEmpty() ) {
                change = true;
                info->setDescription( _description->text() );
            }
        }
    }
    if ( change )
        emit changed();
    _accept = QDialog::Accepted;
    qApp->eventLoop()->exitLoop();
}

void ImageConfig::load()
{
    ImageInfo& info = _editList[ _current ];
    _yearStart->setValue( info.startDate().year() );
    _monthStart->setCurrentItem( info.startDate().month() );
    _dayStart->setValue( info.startDate().day() );

    _yearEnd->setValue( info.endDate().year() );
    _monthEnd->setCurrentItem( info.endDate().month() );
    _dayEnd->setValue( info.endDate().day() );

    _imageLabel->setText( info.label() );
    _description->setText( info.description() );

    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setSelection( info.optionValue( (*it)->optionGroup() ) );
    }

    _nextBut->setEnabled( _current != (int)_origList.count()-1 );
    _prevBut->setEnabled( _current != 0 );

    _preview->setInfo( &info );

    if ( _viewer )
        _viewer->load( _origList, _current );
}

void ImageConfig::save()
{
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->slotReturn();
    }

    ImageInfo& info = _editList[ _current ];

    info.startDate().setYear( _yearStart->value() );
    info.startDate().setMonth( _monthStart->currentItem() );
    info.startDate().setDay( _dayStart->value() );

    info.endDate().setYear( _yearEnd->value() );
    info.endDate().setMonth( _monthEnd->currentItem() );
    info.endDate().setDay( _dayEnd->value() );

    info.setLabel( _imageLabel->text() );
    info.setDescription( _description->text() );
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        info.setOption( (*it)->optionGroup(), (*it)->selection() );
    }
}


void ImageConfig::configure( ImageInfoList list, bool oneAtATime )
{
    _origList = list;
    _editList.clear();
    for( QPtrListIterator<ImageInfo> it( list ); *it; ++it ) {
        _editList.append( *(*it) );
    }
    if ( oneAtATime )
        _setup = SINGLE;
    else
        _setup = MULTIPLE;

    if ( oneAtATime )  {
        _current = -1;
        slotNext();
    }
    else {
        _dayStart->setValue( 0 );
        _monthStart->setCurrentText( QString::fromLatin1("---") );
        _yearStart->setValue( 0 );

        _dayEnd->setValue( 0 );
        _monthEnd->setCurrentText( QString::fromLatin1("---") );
        _yearEnd->setValue( 0 );

        for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
            (*it)->setSelection( QStringList() );
        }

        _imageLabel->setText( QString::fromLatin1("") );
        _description->setText( QString::fromLatin1("") );

        _prevBut->setEnabled( false );
        _nextBut->setEnabled( false );
    }

    setup();
    exec();
}

ImageSearchInfo ImageConfig::search( ImageSearchInfo* search  )
{
    _setup = SEARCH;
    if ( search )
        _oldSearch = *search;

    setup();
    int ok = exec();
    if ( ok == QDialog::Accepted )  {
        _oldSearch = ImageSearchInfo( ImageDate( _dayStart->value(), _monthStart->currentItem(),
                                                 _yearStart->value()),
                                      ImageDate( _dayEnd->value(), _monthEnd->currentItem(),
                                                 _yearEnd->value() ),
                                      _imageLabel->text(), _description->text() );

        for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
            _oldSearch.setOption( (*it)->optionGroup(), (*it)->text() );
        }

        return _oldSearch;
    }
    else
        return ImageSearchInfo();
}

void ImageConfig::setup()
{
    ListSelect::Mode mode;
    if ( _setup == SEARCH )  {
        _okBut->setText( i18n("&Search") );
        _revertBut->hide();
        mode = ListSelect::SEARCH;
        setCaption( i18n("Image Search") );
        loadInfo( _oldSearch );
        _preview->setPixmap( locate("data", QString::fromLatin1("kimdaba/pics/search.jpg") ) );
        _preview->setInfo(0);
        _nextBut->setEnabled( false );
        _prevBut->setEnabled( false );
    }
    else {
        _okBut->setText( i18n("&OK") );
        _revertBut->setEnabled( _setup == SINGLE );
        _revertBut->show();
        mode = ListSelect::INPUT;
        setCaption( i18n("Image Configuration") );
        if ( _setup == MULTIPLE ) {
            _preview->setPixmap( locate("data", QString::fromLatin1("kimdaba/pics/multiconfig.jpg") ) );
            _preview->setInfo(0);
        }


    }
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setMode( mode );
        (*it)->setShowMergeCheckbox( _setup == MULTIPLE );
    }
}


void ImageConfig::displayImage()
{
    if ( !_viewer ) {
        _viewer = new Viewer( this );
        connect( _viewer, SIGNAL( destroyed() ), this, SLOT( viewerDestroyed() ) );
        _viewer->resize( 400, 300 );
    }

    _viewer->show();
    _viewer->load( _origList, _current );
    topLevelWidget()->raise();
    setActiveWindow();
}

void ImageConfig::slotClear()
{
    loadInfo( ImageSearchInfo() );
}

void ImageConfig::loadInfo( const ImageSearchInfo& info )
{
    _yearStart->setValue( info.startDate().year() );
    _monthStart->setCurrentItem( info.startDate().month() );
    _dayStart->setValue( info.startDate().day() );

    _yearEnd->setValue( info.endDate().year() );
    _monthEnd->setCurrentItem( info.endDate().month() );
    _dayEnd->setValue( info.endDate().day() );

    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setText( info.option( (*it)->optionGroup() ) );
    }

    _imageLabel->setText( info.label() );
    _description->setText( info.description() );
}

void ImageConfig::viewerDestroyed()
{
    _viewer = 0;
}

void ImageConfig::slotOptions()
{
    QPopupMenu menu;
    menu.insertItem( i18n("Show/Hide windows"),  _dockWindow->dockHideShowMenu());
    menu.insertItem( i18n("Save current window setup"), this, SLOT( slotSaveWindowSetup() ) );
    menu.exec( QCursor::pos() );
}

int ImageConfig::exec()
{
    show();
    showTornOfWindows();
    qApp->installEventFilter( this );
    qApp->eventLoop()->enterLoop();

    // Executed when the window is gone.
    qApp->removeEventFilter( this );
    hide();
    hideTornOfWindows();
    return _accept;
}

void ImageConfig::slotCancel()
{
    _accept = QDialog::Rejected;
    qApp->eventLoop()->exitLoop();
}

void ImageConfig::slotSaveWindowSetup()
{
    Options::instance()->saveConfigWindowLayout( this );
}

void ImageConfig::closeEvent( QCloseEvent* e )
{
    e->ignore();
    slotCancel();
}

void ImageConfig::hideTornOfWindows()
{
    _tornOfWindows.clear();
    for( QValueList<KDockWidget*>::Iterator it = _dockWidgets.begin(); it != _dockWidgets.end(); ++it ) {
        if ( (*it)->isTopLevel() && (*it)->isShown() ) {
            (*it)->hide();
            _tornOfWindows.append( *it );
        }
    }
}

void ImageConfig::showTornOfWindows()
{
    for( QValueList<KDockWidget*>::Iterator it = _tornOfWindows.begin(); it != _tornOfWindows.end(); ++it ) {
        (*it)->show();
    }
}

bool ImageConfig::eventFilter( QObject* watched, QEvent* event )
{
    if ( !watched->isWidgetType() )
        return false;

    QWidget* w = static_cast<QWidget*>( watched );

    if ( event->type() != QEvent::MouseButtonPress &&
         event->type() != QEvent::MouseButtonRelease &&
         event->type() != QEvent::MouseButtonDblClick &&
         event->type() != QEvent::KeyPress &&
         event->type() != QEvent::KeyRelease &&
         event->type() != QEvent::ContextMenu )
        return false;

    // Initially I used an allow list, but combo boxes pop up menu's did for example not work then.
    if ( w->topLevelWidget()->className() == QCString( "MainView" ) || w->topLevelWidget()->className() == QCString( "Viewer" )) {
        if ( isMinimized() )
            showNormal();
        raise();
        return true;
    }

    return false;
}


KDockWidget* ImageConfig::createListSel( const QString& optionGroup )
{
    KDockWidget* dockWidget = _dockWindow->createDockWidget( optionGroup, Options::instance()->iconForOptionGroup(optionGroup),
                                                0L, optionGroup );
    _dockWidgets.append( dockWidget );
    ListSelect* sel = new ListSelect( optionGroup, dockWidget );
    _optionList.append( sel );
    connect( sel, SIGNAL( deleteOption( const QString&, const QString& ) ),
             this, SIGNAL( deleteOption( const QString&, const QString& ) ) );
    connect( sel, SIGNAL( renameOption( const QString& , const QString& , const QString&  ) ),
             this, SIGNAL( renameOption( const QString& , const QString& , const QString&  ) ) );

    dockWidget->setWidget( sel );
    return dockWidget;
}

void ImageConfig::writeDockConfig( QDomElement& doc )
{
    _dockWindow->writeDockConfig( doc );
}

void ImageConfig::readDockConfig( QDomElement& doc )
{
    _dockWindow->readDockConfig( doc );
}

#include "imageconfig.moc"
