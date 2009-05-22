#include "ImageViewAction.h"
#include <DB/ImageDB.h>
#include <MainWindow/Window.h>

Browser::ImageViewAction::ImageViewAction( BrowserWidget* browser, const DB::ImageSearchInfo& info )
    : BrowserAction( browser ), _info(info)
{
}

void Browser::ImageViewAction::activate()
{
    MainWindow::Window::theMainWindow()->showThumbNails( DB::ImageDB::instance()->search( _info ) );

// PENDING(blackie) _context comes from jumpToContext - see ImageFolder.cpp
#ifdef KDAB_TEMPORARILY_REMOVED
    if ( !_context.isNull() ) {
        DB::ResultId id = DB::ImageDB::instance()->ID_FOR_FILE( _context );
        ThumbnailView::ThumbnailWidget::theThumbnailView()->setCurrentItem( id );
    }
#else // KDAB_TEMPORARILY_REMOVED
    qWarning("Code commented out in Browser::OverviewModel::executeImageAction");
#endif //KDAB_TEMPORARILY_REMOVED
}
