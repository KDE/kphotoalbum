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

#include "BrowserWidget.h"
#include "Folder.h"
#include <klocale.h>
#include "DB/ImageSearchInfo.h"
#include "Settings/SettingsData.h"
#include "ContentFolder.h"
#include "ImageFolder.h"
#include <qtimer.h>
#include "DB/ImageDB.h"
#include "Utilities/Util.h"
#include <qlistview.h>
#include "Utilities/ShowBusyCursor.h"
#include "BrowserItemFactory.h"
#include <qwidgetstack.h>
#include <qlayout.h>
#include "DB/CategoryCollection.h"
#include "AnnotationDialog/ListViewItemHider.h"
#include "TypeFolderAction.h"
#include "ContentFolderAction.h"

Browser::BrowserWidget* Browser::BrowserWidget::_instance = 0;

Browser::BrowserWidget::BrowserWidget( QWidget* parent, const char* name )
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
    _listView->addColumn( i18n("Images") );
    _listView->addColumn( i18n("Videos") );

    _listView->setSelectionMode( QListView::NoSelection );
    _iconView->setSelectionMode( QIconView::NoSelection );
    connect( _listView, SIGNAL( clicked( QListViewItem* ) ), this, SLOT( select( QListViewItem* ) ) );
    connect( _listView, SIGNAL( returnPressed( QListViewItem* ) ), this, SLOT( select( QListViewItem* ) ) );
    connect( _iconView, SIGNAL( clicked( QIconViewItem* ) ), this, SLOT( select( QIconViewItem* ) ) );
    connect( _iconView, SIGNAL( returnPressed( QIconViewItem* ) ), this, SLOT( select( QIconViewItem* ) ) );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL( categoryCollectionChanged() ), this, SLOT( reload() ) );
    connect( this, SIGNAL( viewChanged() ), this, SLOT( resetIconViewSearch() ) );

    // I got to wait till the event loops runs, so I'm sure that the image database has been loaded.
    QTimer::singleShot( 0, this, SLOT( init() ) );
}

Browser::BrowserWidget::~BrowserWidget()
{
    delete _listViewFactory;
    delete _iconViewFactory;
}


void Browser::BrowserWidget::init()
{
    FolderAction* action = new ContentFolderAction( DB::ImageSearchInfo(), this );
    _list.append( action );
    forward();
}

void Browser::BrowserWidget::select( QListViewItem* item )
{
    if ( !item )
        return;

    BrowserListItem* folder = static_cast<BrowserListItem*>( item );
    FolderAction* action = folder->_folder->action( Utilities::ctrlKeyDown() );
    select( action );
}

void Browser::BrowserWidget::select( QIconViewItem* item )
{
    if ( !item )
        return;

    BrowserIconItem* folder = static_cast<BrowserIconItem*>( item );
    FolderAction* action = folder->_folder->action( Utilities::ctrlKeyDown() );
    select( action );
}

void Browser::BrowserWidget::select( FolderAction* action )
{
    Utilities::ShowBusyCursor dummy;
    if ( action ) {
        addItem( action );
        setupFactory();
        action->action( _currentFactory );
    }
    emit viewChanged();
}

void Browser::BrowserWidget::forward()
{
    _current++;
    go();
}

void Browser::BrowserWidget::back()
{
    _current--;
    go();
}

void Browser::BrowserWidget::go()
{
    setupFactory();
    FolderAction* a = _list[_current-1];
    a->action( _currentFactory );
    emitSignals();
}

void Browser::BrowserWidget::addSearch( DB::ImageSearchInfo& info )
{
    FolderAction* a;
    a = new ImageFolderAction( info, this );
    addItem(a);
    go();
}

void Browser::BrowserWidget::addImageView( const QString& context )
{
    FolderAction* a = new ImageFolderAction( context, this );
    addItem(a);
    go();
}

void Browser::BrowserWidget::addItem( FolderAction* action )
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

void Browser::BrowserWidget::emitSignals()
{
    FolderAction* a = _list[_current-1];
    emit canGoBack( _current > 1 );
    emit canGoForward( _current < (int)_list.count() );
    if ( !a->showsImages() )
        emit showingOverview();
    emit pathChanged( a->path() );
    _listView->setColumnText( 0, a->title() );
    emit showsContentView( a->contentView() );

    if ( a->contentView() && _list.size() > 0 )
        emit currentViewTypeChanged( a->viewType() );
}

void Browser::BrowserWidget::home()
{
    FolderAction* action = new ContentFolderAction( DB::ImageSearchInfo(), this );
    addItem( action );
    go();
}

void Browser::BrowserWidget::reload()
{
    setupFactory();

    if ( _current != 0 ) {
        // _current == 0 when browser hasn't yet been initialized (Which happens through a zero-timer.)
        FolderAction* a = _list[_current-1];
        a->action( _currentFactory );
    }
}

Browser::BrowserWidget* Browser::BrowserWidget::instance()
{
    Q_ASSERT( _instance );
    return _instance;
}

void Browser::BrowserWidget::load( const QString& category, const QString& value )
{
    DB::ImageSearchInfo info;
    info.addAnd( category, value );
    FolderAction* a;

    DB::MediaCount counts = DB::ImageDB::instance()->count( info );
    bool loadImages = (counts.total() < static_cast<uint>(Settings::SettingsData::instance()->autoShowThumbnailView()));
    if ( Utilities::ctrlKeyDown() ) loadImages = !loadImages;

    if ( loadImages )
        a = new ImageFolderAction( info, this );
    else
        a = new ContentFolderAction( info, this );

    addItem( a );
    a->action( _currentFactory );
    topLevelWidget()->raise();
    setActiveWindow();
}

bool Browser::BrowserWidget::allowSort()
{
    return _list[_current-1]->allowSort();
}

DB::ImageSearchInfo Browser::BrowserWidget::currentContext()
{
    return _list[_current-1]->_info;
}

void Browser::BrowserWidget::slotSmallListView()
{
    setViewType( DB::Category::ListView );
}

void Browser::BrowserWidget::slotLargeListView()
{
    setViewType( DB::Category::ThumbedListView );
}

void Browser::BrowserWidget::slotSmallIconView()
{
    setViewType( DB::Category::IconView );
}

void Browser::BrowserWidget::slotLargeIconView()
{
    setViewType( DB::Category::ThumbedIconView );
}

void Browser::BrowserWidget::setViewType( DB::Category::ViewType type )
{
    Q_ASSERT( _list.size() > 0 );

    DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( currentCategory() );
    Q_ASSERT( category.data() );
    category->setViewType( type );
    reload();
}

void Browser::BrowserWidget::clear()
{
    _iconView->clear();
    _listView->clear();
}

void Browser::BrowserWidget::setupFactory()
{
    if ( _list.size() == 0 )
        return;

    FolderAction* a = _list[_current-1];
    DB::Category::ViewType type = a->viewType();

    if ( type == DB::Category::ListView || type == DB::Category::ThumbedListView ) {
        _currentFactory = _listViewFactory;
        _stack->raiseWidget( _listView );
    }
    else {
        _currentFactory = _iconViewFactory;
        _stack->raiseWidget( _iconView );
    }
    setFocus();
}

void Browser::BrowserWidget::setFocus()
{
    _stack->visibleWidget()->setFocus();
}

QString Browser::BrowserWidget::currentCategory() const
{
    FolderAction* a = _list[_current-1];
    if ( TypeFolderAction* action = dynamic_cast<TypeFolderAction*>( a ) )
        return action->category()->name();
    else
        return QString::null;
}

void Browser::BrowserWidget::slotLimitToMatch( const QString& str )
{
    if ( _currentFactory == _iconViewFactory ) {
        // This is a cruel hack to get things working till Qt 4 gets
        // out. I'm sure Qt 4 has the same setVisible feature for icon
        // views as it now has for list view.
        _iconViewFactory->setMatchText( str );
        FolderAction* a = _list[_current-1];
        a->action( _currentFactory );
    }
    else
        AnnotationDialog::ListViewTextMatchHider dummy( str, false, _listView );
}

void Browser::BrowserWidget::resetIconViewSearch()
{
    _iconViewFactory->setMatchText( QString::null );
}

void Browser::BrowserWidget::slotInvokeSeleted()
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


void Browser::BrowserWidget::scrollLine( int direction )
{
    _iconView->scrollBy( 0, 10 * direction );
    _listView->scrollBy( 0, 10 * direction );
}

void Browser::BrowserWidget::scrollPage( int direction )
{
    int dist = direction * (height()-100);
    _iconView->scrollBy( 0, dist );
    _listView->scrollBy( 0, dist );
}

#include "BrowserWidget.moc"
