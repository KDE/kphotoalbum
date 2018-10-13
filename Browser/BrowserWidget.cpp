/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include <KLocalizedString>
#include "Settings/SettingsData.h"
#include <qtimer.h>
#include <QHBoxLayout>
#include "Utilities/Util.h"
#include "Utilities/ShowBusyCursor.h"
#include <QStackedWidget>
#include "DB/CategoryCollection.h"

#include "TreeCategoryModel.h"

Browser::BrowserWidget* Browser::BrowserWidget::s_instance = nullptr;
bool Browser::BrowserWidget::s_isResizing = false;


Browser::BrowserWidget::BrowserWidget( QWidget* parent )
    :QWidget( parent ), m_current(-1)
{
    Q_ASSERT( !s_instance );
    s_instance = this;

    createWidgets();

    connect( DB::ImageDB::instance()->categoryCollection(), &DB::CategoryCollection::categoryCollectionChanged,
             this, &BrowserWidget::reload);
    connect( this, &BrowserWidget::viewChanged, this, &BrowserWidget::resetIconViewSearch);

    m_filterProxy = new TreeFilter(this);
    m_filterProxy->setFilterKeyColumn(0);
    m_filterProxy->setFilterCaseSensitivity( Qt::CaseInsensitive );
    m_filterProxy->setSortRole( ValueRole );
    m_filterProxy->setSortCaseSensitivity( Qt::CaseInsensitive );

    addAction( new OverviewPage( Breadcrumb::home(), DB::ImageSearchInfo(), this ) );
    QTimer::singleShot( 0, this, SLOT(emitSignals()) );
}

void Browser::BrowserWidget::forward()
{
    int targetIndex = m_current;
    while ( targetIndex < m_list.count()-1 ) {
        targetIndex++;
        if ( m_list[targetIndex]->showDuringMovement() ) {
            break;
        }
    }
    activatePage(targetIndex);
}

void Browser::BrowserWidget::back()
{
    int targetIndex = m_current;
    while ( targetIndex > 0 ) {
        targetIndex--;
        if ( m_list[targetIndex]->showDuringMovement() )
            break;
    }
    activatePage(targetIndex);
}

void Browser::BrowserWidget::activatePage(int pageIndex)
{
    if (pageIndex != m_current) {
        if (currentAction() != 0) {
            currentAction()->deactivate();
        }
        m_current = pageIndex;
        go();
    }
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
    // remove actions which would go forward in the breadcrumbs
    while ( (int) m_list.count() > m_current+1 ) {
        BrowserPage* m = m_list.back();
        m_list.pop_back();
        delete m;
    }

    m_list.append(action);
    activatePage(m_list.count() - 1);
}

void Browser::BrowserWidget::emitSignals()
{
    emit canGoBack( m_current > 0 );
    emit canGoForward( m_current < (int)m_list.count()-1 );
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
    Q_ASSERT( s_instance );
    return s_instance;
}

void Browser::BrowserWidget::load( const QString& category, const QString& value )
{
    DB::ImageSearchInfo info;
    info.addAnd( category, value );

    DB::MediaCount counts = DB::ImageDB::instance()->count( info );
    bool loadImages = (counts.total() < Settings::SettingsData::instance()->autoShowThumbnailView());
    if ( QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier) )
        loadImages = !loadImages;

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
    Q_ASSERT( m_list.size() > 0 );

    DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( currentCategory() );
    Q_ASSERT( category.data() );
    category->setViewType( type );

    switchToViewType( type );
    reload();
}

void Browser::BrowserWidget::setFocus()
{
    m_curView->setFocus();
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
    m_filterProxy->resetCache();
    m_filterProxy->setFilterFixedString( str );
    setBranchOpen(QModelIndex(), true);
    adjustTreeViewColumnSize();
}

void Browser::BrowserWidget::resetIconViewSearch()
{
    m_filterProxy->resetCache();
    m_filterProxy->setFilterRegExp( QString() );
    adjustTreeViewColumnSize();
}

void Browser::BrowserWidget::slotInvokeSeleted()
{
    if ( !m_curView->currentIndex().isValid() ) {
        if ( m_filterProxy->rowCount( QModelIndex() ) == 0 ) {
            // Absolutely nothing to see here :-)
            return;
        }
        else {
            // Use the first item
            itemClicked( m_filterProxy->index( 0,0,QModelIndex() ) );
        }
    }
    else
        itemClicked( m_curView->currentIndex() );
}


void Browser::BrowserWidget::itemClicked( const QModelIndex& index )
{
    Utilities::ShowBusyCursor busy;
    BrowserPage* action = currentAction()->activateChild( m_filterProxy->mapToSource( index ) );
    if ( action )
        addAction( action );
}


Browser::BrowserPage* Browser::BrowserWidget::currentAction() const
{
    return m_current >= 0? m_list[m_current] : 0;
}

void Browser::BrowserWidget::setModel( QAbstractItemModel* model)
{
    m_filterProxy->setSourceModel( model );
    // make sure the view knows about the source model change:
    m_curView->setModel( m_filterProxy );

    if (qobject_cast<TreeCategoryModel*>(model)) {
        // FIXME: The new-style connect here does not work, reload() is not triggered
        //connect(model, &QAbstractItemModel::dataChanged, this, &BrowserWidget::reload);
        // The old-style one triggers reload() correctly
        connect(model, SIGNAL(dataChanged()), this, SLOT(reload()));
    }
}

void Browser::BrowserWidget::switchToViewType( DB::Category::ViewType type )
{
    if ( m_curView ) {
        m_curView->setModel(0);
        disconnect( m_curView, &QAbstractItemView::activated, this, &BrowserWidget::itemClicked);
    }

    if ( type == DB::Category::TreeView || type == DB::Category::ThumbedTreeView ) {
        m_curView = m_treeView;
    }
    else {
        m_curView =m_listView;
        m_filterProxy->invalidate();
        m_filterProxy->sort( 0, Qt::AscendingOrder );

        m_listView->setViewMode(dynamic_cast<OverviewPage*>(currentAction()) == 0 ?
                               CenteringIconView::NormalIconView : CenteringIconView::CenterView );
    }

    if ( CategoryPage* action = dynamic_cast<CategoryPage*>( currentAction() ) ) {
        const int size = action->category()->thumbnailSize();
        m_curView->setIconSize( QSize(size,size) );
//        m_curView->setGridSize( QSize( size+10, size+10 ) );
    }


    // Hook up the new view
    m_curView->setModel( m_filterProxy );
    connect( m_curView, &QAbstractItemView::activated, this, &BrowserWidget::itemClicked);


    m_stack->setCurrentWidget( m_curView );
    adjustTreeViewColumnSize();
}

void Browser::BrowserWidget::setBranchOpen( const QModelIndex& parent, bool open )
{
    if ( m_curView != m_treeView )
        return;

    const int count = m_filterProxy->rowCount(parent);
    if ( count > 5 )
        open = false;

    m_treeView->setExpanded( parent, open );
    for ( int row = 0; row < count; ++row )
        setBranchOpen( m_filterProxy->index( row, 0 ,parent ), open );
}


Browser::BreadcrumbList Browser::BrowserWidget::createPath() const
{
    BreadcrumbList result;

    for ( int i = 0; i <= m_current; ++i )
        result.append(m_list[i]->breadcrumb() );

    return result;
}

void Browser::BrowserWidget::widenToBreadcrumb( const Browser::Breadcrumb& breadcrumb )
{
    while ( currentAction()->breadcrumb() != breadcrumb )
        m_current--;
    go();
}

void Browser::BrowserWidget::adjustTreeViewColumnSize()
{
    m_treeView->header()->resizeSections(QHeaderView::ResizeToContents);
}


void Browser::BrowserWidget::createWidgets()
{
    m_stack = new QStackedWidget;
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget( m_stack );

    m_listView = new CenteringIconView ( m_stack );
    m_listView->setIconSize( QSize(100,75) );
    m_listView->setSelectionMode( QListView::SingleSelection );
    m_listView->setSpacing(10);
    m_listView->setUniformItemSizes(true);
    m_listView->setResizeMode( QListView::Adjust );
    m_stack->addWidget( m_listView );


    m_treeView = new QTreeView( m_stack );

    m_treeView->setDragEnabled(true);
    m_treeView->setAcceptDrops(true);
    m_treeView->setDropIndicatorShown(true);
    m_treeView->setDefaultDropAction( Qt::MoveAction );

    QPalette pal = m_treeView->palette();
    pal.setBrush( QPalette::Base, QApplication::palette().color( QPalette::Background ) );
    m_treeView->setPalette( pal );

    m_treeView->header()->setStretchLastSection(false);
    m_treeView->header()->setSortIndicatorShown(true);
    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn( 0, Qt::AscendingOrder );
    m_stack->addWidget( m_treeView );

    // Do not give focus to the widgets when they are scrolled with the wheel.
    m_listView->setFocusPolicy( Qt::StrongFocus );
    m_treeView->setFocusPolicy( Qt::StrongFocus );

    m_treeView->installEventFilter( this );
    m_treeView->viewport()->installEventFilter( this );
    m_listView->installEventFilter( this );
    m_listView->viewport()->installEventFilter( this );

    connect( m_treeView, &QTreeView::expanded, this, &BrowserWidget::adjustTreeViewColumnSize);

    m_curView = nullptr;
}

bool Browser::BrowserWidget::eventFilter( QObject* /* obj */, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress ||
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
    QApplication::sendEvent(m_curView, event );
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
        m_resizePressPos = event->pos();
        offset = category->thumbnailSize();
        s_isResizing = true;
    }

    else if ( event->type() == QEvent::MouseMove  ) {
        int distance = (event->pos() - m_resizePressPos).x() + (event->pos() - m_resizePressPos).y() / 3;
        int size = distance + offset;
        size = qMax( qMin( 512, size ), 32 );
        action->category()->setThumbnailSize( size );

        m_curView->setIconSize( QSize(size,size) );
        m_filterProxy->invalidate();
        adjustTreeViewColumnSize();
    }
    else if ( event->type() == QEvent::MouseButtonRelease  ) {
        s_isResizing = false;
        update();
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
