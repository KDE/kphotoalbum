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

#include "browser.h"
#include "folder.h"
#include <klocale.h>
#include "imagesearchinfo.h"
#include "options.h"
#include "contentfolder.h"
#include "imagefolder.h"
#include <qtimer.h>
#include "imagedb.h"
#include "util.h"
#include <qlistview.h>
#include "showbusycursor.h"
#include "browseritemfactory.h"
#include <qwidgetstack.h>
#include <qlayout.h>
#include <qlabel.h>
#include "categorycollection.h"

Browser* Browser::_instance = 0;

Browser::Browser( QWidget* parent, const char* name )
    :QWidget( parent, name ), _current(0)
{
    Q_ASSERT( !_instance );
    _instance = this;

    _stack = new QWidgetStack( this, "_stack" );
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->addWidget( _stack );

    _listView = new QListView ( _stack, "_listView" );
    _stack->addWidget( _listView );

    _iconView = new QIconView( _stack, "_iconView" );
    _stack->addWidget( _iconView );

    _listViewFactory = new BrowserListViewItemFactory( _listView );
    _iconViewFactory = new BrowserIconViewItemFactory( _iconView );

    _listView->addColumn( i18n("What") );
    _listView->addColumn( i18n("Count") );

    _listView->setSelectionMode( QListView::NoSelection );
    _iconView->setSelectionMode( QIconView::NoSelection );
    connect( _listView, SIGNAL( clicked( QListViewItem* ) ), this, SLOT( select( QListViewItem* ) ) );
    connect( _listView, SIGNAL( returnPressed( QListViewItem* ) ), this, SLOT( select( QListViewItem* ) ) );
    connect( _iconView, SIGNAL( clicked( QIconViewItem* ) ), this, SLOT( select( QIconViewItem* ) ) );
    connect( _iconView, SIGNAL( returnPressed( QIconViewItem* ) ), this, SLOT( select( QIconViewItem* ) ) );
    connect( ImageDB::instance()->categoryCollection(), SIGNAL( categoryCollectionChanged() ), this, SLOT( reload() ) );
    connect( this, SIGNAL( viewChanged() ), this, SLOT( resetIconViewSearch() ) );

    // I got to wait till the event loops runs, so I'm sure that the image database has been loaded.
    QTimer::singleShot( 0, this, SLOT( init() ) );
}

Browser::~Browser()
{
    delete _listViewFactory;
    delete _iconViewFactory;
}


void Browser::init()
{
    FolderAction* action = new ContentFolderAction( QString::null, QString::null, ImageSearchInfo(), this );
    _list.append( action );
    forward();
}

void Browser::select( QListViewItem* item )
{
    if ( !item )
        return;

    BrowserListItem* folder = static_cast<BrowserListItem*>( item );
    FolderAction* action = folder->_folder->action( Util::ctrlKeyDown() );
    select( action );
}

void Browser::select( QIconViewItem* item )
{
    if ( !item )
        return;

    BrowserIconItem* folder = static_cast<BrowserIconItem*>( item );
    FolderAction* action = folder->_folder->action( Util::ctrlKeyDown() );
    select( action );
}

void Browser::select( FolderAction* action )
{
    ShowBusyCursor dummy;
    if ( action ) {
        addItem( action );
        setupFactory();
        action->action( _currentFactory );
    }
    emit viewChanged();
}

void Browser::forward()
{
    _current++;
    go();
}

void Browser::back()
{
    _current--;
    go();
}

void Browser::go()
{
    setupFactory();
    FolderAction* a = _list[_current-1];
    a->action( _currentFactory );
    emitSignals();
}

void Browser::addSearch( ImageSearchInfo& info )
{
    FolderAction* a;
    a = new ImageFolderAction( info, this );
    addItem(a);
    go();
}

void Browser::addImageView( const QString& context )
{
    FolderAction* a = new ImageFolderAction( context, this );
    addItem(a);
    go();
}

void Browser::addItem( FolderAction* action )
{
    while ( (int) _list.count() > _current ) {
        FolderAction* a = _list.back();
        _list.pop_back();
        delete a;
    }

    _list.append(action);
    _current++;
    emitSignals();
}

void Browser::emitSignals()
{
    FolderAction* a = _list[_current-1];
    emit canGoBack( _current > 1 );
    emit canGoForward( _current < (int)_list.count() );
    if ( !a->showsImages() )
        emit showingOverview();
    emit pathChanged( a->path() );
    _listView->setColumnText( 0, a->title() );
    emit showsContentView( a->contentView() );

    if ( a->contentView() && _list.size() > 0 ) {
        QString grp = a->category();
        Q_ASSERT( !grp.isNull() );
        Category::ViewSize size = ImageDB::instance()->categoryCollection()->categoryForName( grp )->viewSize();
        Category::ViewType type = ImageDB::instance()->categoryCollection()->categoryForName( grp )->viewType();
        emit currentSizeAndTypeChanged( size, type );
    }
}

void Browser::home()
{
    FolderAction* action = new ContentFolderAction( QString::null, QString::null, ImageSearchInfo(), this );
    addItem( action );
    go();
}

void Browser::reload()
{
    setupFactory();

    if ( _current != 0 ) {
        // _current == 0 when browser hasn't yet been initialized (Which happens through a zero-timer.)
        FolderAction* a = _list[_current-1];
        a->action( _currentFactory );
    }
}

Browser* Browser::instance()
{
    Q_ASSERT( _instance );
    return _instance;
}

void Browser::load( const QString& category, const QString& value )
{
    ImageSearchInfo info;
    info.addAnd( category, value );
    FolderAction* a;

    bool loadImages = ImageDB::instance()->count( info ) < Options::instance()->autoShowThumbnailView();
    if ( Util::ctrlKeyDown() ) loadImages = !loadImages;

    if ( loadImages )
        a = new ImageFolderAction( info, this );
    else
        a = new ContentFolderAction( category, value, info, this );

    addItem( a );
    a->action( _currentFactory );
    topLevelWidget()->raise();
    setActiveWindow();
}

bool Browser::allowSort()
{
    return _list[_current-1]->allowSort();
}

ImageSearchInfo Browser::currentContext()
{
    return _list[_current-1]->_info;
}

void Browser::slotSmallListView()
{
    setSizeAndType( Category::ListView,Category::Small );
}

void Browser::slotLargeListView()
{
    setSizeAndType( Category::ListView,Category::Large );
}

void Browser::slotSmallIconView()
{
    setSizeAndType( Category::IconView,Category::Small );
}

void Browser::slotLargeIconView()
{
    setSizeAndType( Category::IconView,Category::Large );
}

void Browser::setSizeAndType( Category::ViewType type, Category::ViewSize size )
{
    Q_ASSERT( _list.size() > 0 );

    FolderAction* a = _list[_current-1];
    QString grp = a->category();
    Q_ASSERT( !grp.isNull() );

    ImageDB::instance()->categoryCollection()->categoryForName( grp )->setViewType( type );
    ImageDB::instance()->categoryCollection()->categoryForName( grp )->setViewSize( size );
    reload();
}

void Browser::clear()
{
    _iconView->clear();
    _listView->clear();
}

void Browser::setupFactory()
{
    Category::ViewType type = Category::ListView;
    if ( _list.size() == 0 )
        return;

    FolderAction* a = _list[_current-1];
    QString category = a->category();

    if ( !category.isNull() )
        type = ImageDB::instance()->categoryCollection()->categoryForName( category )->viewType();

    if ( type == Category::ListView ) {
        _currentFactory = _listViewFactory;
        _stack->raiseWidget( _listView );
    }
    else {
        _currentFactory = _iconViewFactory;
        _stack->raiseWidget( _iconView );
    }
    setFocus();
}

void Browser::setFocus()
{
    _stack->visibleWidget()->setFocus();
}

QString Browser::currentCategory() const
{
    FolderAction* a = _list[_current-1];
    return a->category();
}

void Browser::slotLimitToMatch( const QString& str )
{
    if ( _currentFactory == _iconViewFactory ) {
        // This is a cruel hack to get things working till Qt 4 gets
        // out. I'm sure Qt 4 has the same setVisible feature for icon
        // views as it now has for list view.
        _iconViewFactory->setMatchText( str );
        FolderAction* a = _list[_current-1];
        a->action( _currentFactory );
    }
    else {
        for ( QListViewItem* item = _listView->firstChild(); item; item = item->nextSibling() ) {
            item->setVisible( item->text(0).lower().contains( str.lower() ) );
        }
    }
}

void Browser::resetIconViewSearch()
{
    _iconViewFactory->setMatchText( QString::null );
}

void Browser::slotInvokeSeleted()
{
    if ( _currentFactory == _iconViewFactory ) {
        QIconViewItem* item = _iconView->currentItem();
        if ( !item )
            item = _iconView->firstItem();

        select( item );
    }

    else {
        QListViewItem* item = _listView->currentItem();
        if ( !item || !item->isVisible() ) {
            for ( QListViewItem* it = _listView->firstChild(); it; it = it->nextSibling() ) {
                if ( it->isVisible() ) {
                    item = it;
                    break;
                }
            }
        }
        select( item );
    }
}


#include "browser.moc"
