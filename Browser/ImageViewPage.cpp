#include "ImageViewPage.h"
#include <ThumbnailView/ThumbnailWidget.h>
#include <DB/ImageDB.h>
#include <MainWindow/Window.h>

 Browser::ImageViewPage::ImageViewPage( const DB::ImageSearchInfo& info, BrowserWidget* browser)
     : BrowserPage( info, browser )
{
}

void Browser::ImageViewPage::activate()
{
    MainWindow::Window::theMainWindow()->showThumbNails( DB::ImageDB::instance()->search( searchInfo() ) );

    if ( !_context.isNull() ) {
        DB::ResultId id = DB::ImageDB::instance()->ID_FOR_FILE( _context );
        ThumbnailView::ThumbnailWidget::theThumbnailView()->setCurrentItem( id );
    }
}

Browser::Viewer Browser::ImageViewPage::viewer()
{
    return Browser::ShowImageViewer;
}

bool Browser::ImageViewPage::isSearchable() const
{
    return false;
}

Browser::ImageViewPage::ImageViewPage( const QString& context, BrowserWidget* browser )
    : BrowserPage(DB::ImageSearchInfo(), browser), _context( context )
{
}
