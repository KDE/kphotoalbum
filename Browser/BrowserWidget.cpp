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

#include "BrowserWidget.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <DB/ImageDB.h>
#include <QHeaderView>
#include "ImageViewPage.h"
#include "CategoryPage.h"
#include "TreeFilter.h"
#include <QTreeView>
#include <DB/ImageSearchInfo.h>
#include "OverviewPage.h"
#include "enums.h"

#include <klocale.h>
#include "Settings/SettingsData.h"
#include <qtimer.h>
#include <QHBoxLayout>
#include "Utilities/Util.h"
#include "Utilities/ShowBusyCursor.h"
#include <QStackedWidget>
#include "DB/CategoryCollection.h"

Browser::BrowserWidget* Browser::BrowserWidget::_instance = 0;
bool Browser::BrowserWidget::_isResizing = false;


Browser::BrowserWidget::BrowserWidget( QWidget* parent )
    :QWidget( parent ), _current(-1)
{
    Q_ASSERT( !_instance );
    _instance = this;

    createWidgets();

    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL( categoryCollectionChanged() ), this, SLOT( reload() ) );
    connect( this, SIGNAL( viewChanged() ), this, SLOT( resetIconViewSearch() ) );

    _filterProxy = new TreeFilter(this);
    _filterProxy->setFilterKeyColumn(0);
    _filterProxy->setFilterCaseSensitivity( Qt::CaseInsensitive );
    _filterProxy->setSortRole( ValueRole );

    addAction( new OverviewPage( Breadcrumb::home(), DB::ImageSearchInfo(), this ) );
    QTimer::singleShot( 0, this, SLOT( emitSignals() ) );
}

void Browser::BrowserWidget::forward()
{
    int cur = _current;
    while ( cur < _list.count()-1 ) {
        cur++;
        if ( _list[cur]->showDuringMovement() ) {
            _current = cur;
            break;
        }
    }
    go();
}

void Browser::BrowserWidget::back()
{
    while ( _current > 0 ) {
        _current--;
        if ( currentAction()->showDuringMovement() )
            break;
    }
    go();
}

void Browser::BrowserWidget::go()
{
    switchToViewType( currentAction()->viewType() );
    currentAction()->activate();
    setBranchOpen(QModelIndex(), true);
    adjustTreeViewColumnSize();
    emitSignals();
}

void Browser::BrowserWidget::addSearch( DB::ImageSearchInfo& info )
{
    addAction( new OverviewPage( Breadcrumb::empty(), info, this ) );
}

void Browser::BrowserWidget::addImageView( const DB::FileName& context )
{
    addAction( new ImageViewPage( context, this ) );
}

void Browser::BrowserWidget::addAction( Browser::BrowserPage* action )
{
    while ( (int) _list.count() > _current+1 ) {
        BrowserPage* m = _list.back();
        _list.pop_back();
        delete m;
    }

    _list.append(action);
    _current++;
    go();
}

void Browser::BrowserWidget::emitSignals()
{
    emit canGoBack( _current > 0 );
    emit canGoForward( _current < (int)_list.count()-1 );
    if ( currentAction()->viewer() == ShowBrowser )
        emit showingOverview();

    emit isSearchable( currentAction()->isSearchable() );
    emit isViewChangeable( currentAction()->isViewChangeable() );

    bool isCategoryAction = (dynamic_cast<CategoryPage*>( currentAction() ) != 0);

    if ( isCategoryAction ) {
        DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( currentCategory() );
        Q_ASSERT( category.data() );

        emit currentViewTypeChanged( category->viewType());
    }

    emit pathChanged( createPath() );
    emit viewChanged();
    emit imageCount( DB::ImageDB::instance()->count(currentAction()->searchInfo()).total() );
}

void Browser::BrowserWidget::home()
{
    addAction( new OverviewPage( Breadcrumb::home(), DB::ImageSearchInfo(), this ) );
}

void Browser::BrowserWidget::reload()
{
    currentAction()->activate();
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

    DB::MediaCount counts = DB::ImageDB::instance()->count( info );
    bool loadImages = (counts.total() < Settings::SettingsData::instance()->autoShowThumbnailView());
    if ( Utilities::ctrlKeyDown() ) loadImages = !loadImages;

    if ( loadImages )
        addAction( new ImageViewPage( info, this ) );
    else
        addAction( new OverviewPage( Breadcrumb(value, true) , info, this ) );

    go();
    topLevelWidget()->raise();
    activateWindow();
}

DB::ImageSearchInfo Browser::BrowserWidget::currentContext()
{
    return currentAction()->searchInfo();
}

void Browser::BrowserWidget::slotSmallListView()
{
    changeViewTypeForCurrentView( DB::Category::TreeView );
}

void Browser::BrowserWidget::slotLargeListView()
{
    changeViewTypeForCurrentView( DB::Category::ThumbedTreeView );
}

void Browser::BrowserWidget::slotSmallIconView()
{
    changeViewTypeForCurrentView( DB::Category::IconView );
}

void Browser::BrowserWidget::slotLargeIconView()
{
    changeViewTypeForCurrentView( DB::Category::ThumbedIconView );
}

void Browser::BrowserWidget::changeViewTypeForCurrentView( DB::Category::ViewType type )
{
    Q_ASSERT( _list.size() > 0 );

    DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( currentCategory() );
    Q_ASSERT( category.data() );
    category->setViewType( type );

    switchToViewType( type );
    reload();
}

void Browser::BrowserWidget::setFocus()
{
    _curView->setFocus();
}

QString Browser::BrowserWidget::currentCategory() const
{
    if ( CategoryPage* action = dynamic_cast<CategoryPage*>( currentAction() ) )
        return action->category()->name();
    else
        return QString();
}

void Browser::BrowserWidget::slotLimitToMatch( const QString& str )
{
    _filterProxy->resetCache();
    _filterProxy->setFilterFixedString( str );
    setBranchOpen(QModelIndex(), true);
    adjustTreeViewColumnSize();
}

void Browser::BrowserWidget::resetIconViewSearch()
{
    _filterProxy->resetCache();
    _filterProxy->setFilterRegExp( QString() );
    adjustTreeViewColumnSize();
}

void Browser::BrowserWidget::slotInvokeSeleted()
{
    if ( !_curView->currentIndex().isValid() ) {
        if ( _filterProxy->rowCount( QModelIndex() ) == 0 ) {
            // Absolutely nothing to see here :-)
            return;
        }
        else {
            // Use the first item
            itemClicked( _filterProxy->index( 0,0,QModelIndex() ) );
        }
    }
    else
        itemClicked( _curView->currentIndex() );
}


void Browser::BrowserWidget::itemClicked( const QModelIndex& index )
{
    Utilities::ShowBusyCursor busy;
    BrowserPage* action = currentAction()->activateChild( _filterProxy->mapToSource( index ) );
    if ( action )
        addAction( action );
}


Browser::BrowserPage* Browser::BrowserWidget::currentAction() const
{
    return _list[_current];
}

void Browser::BrowserWidget::setModel( QAbstractItemModel* model)
{
    _filterProxy->setSourceModel( model );
}

void Browser::BrowserWidget::switchToViewType( DB::Category::ViewType type )
{
    if ( _curView ) {
        _curView->setModel(0);
        disconnect( _curView, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)) );
    }

    if ( type == DB::Category::TreeView || type == DB::Category::ThumbedTreeView ) {
        _curView = _treeView;
    }
    else {
        _curView =_listView;
        _filterProxy->invalidate();
        _filterProxy->sort( 0, Qt::AscendingOrder );

        _listView->setViewMode(dynamic_cast<OverviewPage*>(currentAction()) == 0 ?
                               CenteringIconView::NormalIconView : CenteringIconView::CenterView );
    }

    if ( CategoryPage* action = dynamic_cast<CategoryPage*>( currentAction() ) ) {
        const int size = action->category()->thumbnailSize();
        _curView->setIconSize( QSize(size,size) );
//        _curView->setGridSize( QSize( size+10, size+10 ) );
    }


    // Hook up the new view
    _curView->setModel( _filterProxy );
    connect( _curView, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)) );


    _stack->setCurrentWidget( _curView );
    adjustTreeViewColumnSize();
}

void Browser::BrowserWidget::setBranchOpen( const QModelIndex& parent, bool open )
{
    if ( _curView != _treeView )
        return;

    const int count = _filterProxy->rowCount(parent);
    if ( count > 5 )
        open = false;

    _treeView->setExpanded( parent, open );
    for ( int row = 0; row < count; ++row )
        setBranchOpen( _filterProxy->index( row, 0 ,parent ), open );
}


Browser::BreadcrumbList Browser::BrowserWidget::createPath() const
{
    BreadcrumbList result;

    for ( int i = 0; i <= _current; ++i )
        result.append(_list[i]->breadcrumb() );

    return result;
}

void Browser::BrowserWidget::widenToBreadcrumb( const Browser::Breadcrumb& breadcrumb )
{
    while ( currentAction()->breadcrumb() != breadcrumb )
        _current--;
    go();
}

void Browser::BrowserWidget::adjustTreeViewColumnSize()
{
    _treeView->header()->resizeSections(QHeaderView::ResizeToContents);
}


void Browser::BrowserWidget::createWidgets()
{
    _stack = new QStackedWidget;
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget( _stack );

    _listView = new CenteringIconView ( _stack );
    _listView->setIconSize( QSize(100,75) );
    _listView->setSelectionMode( QListView::SingleSelection );
    _listView->setSpacing(10);
    _listView->setUniformItemSizes(true);
    _listView->setResizeMode( QListView::Adjust );
    _stack->addWidget( _listView );


    _treeView = new QTreeView( _stack );

    QPalette pal = _treeView->palette();
    pal.setBrush( QPalette::Base, QApplication::palette().color( QPalette::Background ) );
    _treeView->setPalette( pal );

    _treeView->header()->setStretchLastSection(false);
    _treeView->header()->setSortIndicatorShown(true);
    _treeView->setSortingEnabled(true);
    _treeView->sortByColumn( 0, Qt::AscendingOrder );
    _stack->addWidget( _treeView );

    // Do not give focus to the widgets when they are scrolled with the wheel.
    _listView->setFocusPolicy( Qt::StrongFocus );
    _treeView->setFocusPolicy( Qt::StrongFocus );

    _treeView->installEventFilter( this );
    _treeView->viewport()->installEventFilter( this );
    _listView->installEventFilter( this );
    _listView->viewport()->installEventFilter( this );

    connect( _treeView, SIGNAL( expanded( QModelIndex ) ), SLOT( adjustTreeViewColumnSize() ) );

    _curView = 0;
}

bool Browser::BrowserWidget::eventFilter( QObject* obj, QEvent* event)
{
    if ( event->type() == QEvent::KeyPress ) {
        QKeyEvent* ke = static_cast<QKeyEvent*>( event );
        if ( ke->key() == Qt::Key_Return ) {
            if ( _curView == obj )
                itemClicked( _curView->currentIndex() );
        }
    }

    else if (event->type() == QEvent::MouseButtonPress ||
             event->type() == QEvent::MouseMove ||
             event->type() == QEvent::MouseButtonRelease ) {
        QMouseEvent* me = static_cast<QMouseEvent*>( event );
        if ( me->buttons() & Qt::MidButton || me->button() & Qt::MidButton) {
            handleResizeEvent( me );
            return true;
        }
    }

    return false;
}

void Browser::BrowserWidget::scrollKeyPressed( QKeyEvent* event )
{
    QApplication::sendEvent(_curView, event );
}

void Browser::BrowserWidget::handleResizeEvent( QMouseEvent* event )
{
    static int offset;

    CategoryPage* action = dynamic_cast<CategoryPage*>( currentAction() );
    if ( !action )
        return;

    DB::CategoryPtr category = action->category();

    if ( !action )
        return;

    if ( event->type() ==  QEvent::MouseButtonPress ) {
        _resizePressPos = event->pos();
        offset = category->thumbnailSize();
        _isResizing = true;
    }

    else if ( event->type() == QEvent::MouseMove  ) {
        int distance = (event->pos() - _resizePressPos).x() + (event->pos() - _resizePressPos).y() / 3;
        int size = distance + offset;
        size = qMax( qMin( 512, size ), 32 );
        action->category()->setThumbnailSize( size );

        _curView->setIconSize( QSize(size,size) );
        _filterProxy->invalidate();
        adjustTreeViewColumnSize();
    }
    else if ( event->type() == QEvent::MouseButtonRelease  ) {
        _isResizing = false;
        update();
    }
}

#include "BrowserWidget.moc"

// vi:expandtab:tabstop=4 shiftwidth=4:
