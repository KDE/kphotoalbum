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

#include "browser.h"
#include <qiconview.h>
#include "folder.h"
#include <klocale.h>
#include "imagesearchinfo.h"
#include "options.h"
#include "contentfolder.h"
#include "imagefolder.h"
#include <qtimer.h>
#include "imagedb.h"
#include "util.h"

Browser* Browser::_instance = 0;

Browser::Browser( QWidget* parent, const char* name )
    :QIconView( parent, name ), _current(0)
{
    Q_ASSERT( !_instance );
    _instance = this;
    // I got to wait till the event loops runs, so I'm sure that the image database has been loaded.
    QTimer::singleShot( 0, this, SLOT( init() ) );
}

void Browser::init()
{
    FolderAction* action = new ContentFolderAction( QString::null, QString::null, ImageSearchInfo(), this );
    _list.append( action );
    forward();

    setSelectionMode( NoSelection );
    setItemsMovable( false );

    connect( this, SIGNAL( clicked( QIconViewItem* ) ), this, SLOT( select( QIconViewItem* ) ) );
    connect( this, SIGNAL( returnPressed( QIconViewItem* ) ), this, SLOT( select( QIconViewItem* ) ) );
    connect( Options::instance(), SIGNAL( optionGroupsChanged() ), this, SLOT( reload() ) );
}

void Browser::select( QIconViewItem* item )
{
    if ( !item )
        return;

    Folder* f = static_cast<Folder*>( item );
    FolderAction* action = f->action( Util::ctrlKeyDown() );
    if ( action ) {
        addItem( action );
        action->action();
    }
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
    FolderAction* a = _list[_current-1];
    a->action();
    emitSignals();
}

void Browser::addSearch( ImageSearchInfo& info )
{
    FolderAction* a;
    if ( ImageDB::instance()->count( info ) > Options::instance()->maxImages() )
        a = new ContentFolderAction( QString::null, QString::null, info, this );
    else
        a = new ImageFolderAction( info, -1, -1, this );
    addItem(a);
    go();
}

void Browser::addItem( FolderAction* action )
{
    while ( (int) _list.count() > _current )
        _list.pop_back();

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
}

void Browser::home()
{
    FolderAction* action = new ContentFolderAction( QString::null, QString::null, ImageSearchInfo(), this );
    addItem( action );
    go();
}

void Browser::reload()
{
    FolderAction* a = _list[_current-1];
    if ( !a->showsImages() )
        a->action();
}

Browser* Browser::theBrowser()
{
    Q_ASSERT( _instance );
    return _instance;
}

void Browser::load( const QString& optionGroup, const QString& value )
{
    ImageSearchInfo info;
    info.addAnd( optionGroup, value );
    FolderAction* a;
    if (  Util::ctrlKeyDown() && ImageDB::instance()->count( info ) < Options::instance()->maxImages() )
        a = new ImageFolderAction( info, -1, -1, this );
    else {
        a = new ContentFolderAction( optionGroup, value, info, this );
    }

    addItem( a );
    a->action();
    topLevelWidget()->raise();
    setActiveWindow();
}

#include "browser.moc"
