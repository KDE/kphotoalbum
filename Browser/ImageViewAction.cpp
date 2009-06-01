#include "ImageViewAction.h"
#include <ThumbnailView/ThumbnailWidget.h>
#include <DB/ImageDB.h>
#include <MainWindow/Window.h>

Browser::ImageViewAction::ImageViewAction( const DB::ImageSearchInfo& info, BrowserWidget* browser)
    : BrowserAction( info, browser )
{
}

void Browser::ImageViewAction::activate()
{
    MainWindow::Window::theMainWindow()->showThumbNails( DB::ImageDB::instance()->search( searchInfo() ) );

    if ( !_context.isNull() ) {
        DB::ResultId id = DB::ImageDB::instance()->ID_FOR_FILE( _context );
        ThumbnailView::ThumbnailWidget::theThumbnailView()->setCurrentItem( id );
    }
}

Browser::Viewer Browser::ImageViewAction::viewer()
{
    return Browser::ShowImageViewer;
}

bool Browser::ImageViewAction::isSearchable() const
{
    return false;
}

Browser::ImageViewAction::ImageViewAction( const QString& context, BrowserWidget* browser )
    : BrowserAction(DB::ImageSearchInfo(), browser), _context( context )
{
}
