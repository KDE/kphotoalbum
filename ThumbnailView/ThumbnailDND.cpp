#include "ThumbnailDND.h"
#include "Browser/BrowserWidget.h"
#include "DB/ImageDB.h"
#include <KMessageBox>
#include <klocale.h>
#include "ThumbnailWidget.h"
#include <QDragEnterEvent>
#include <QDragMoveEvent>

ThumbnailView::ThumbnailDND::ThumbnailDND( ThumbnailFactory* factory )
    : ThumbnailComponent( factory )
{
}

void ThumbnailView::ThumbnailDND::contentsDragMoveEvent( QDragMoveEvent* event )
{
    if ( event->provides( "text/uri-list" ) && widget()->_selectionInteraction.isDragging() )
        event->accept();
    else {
        event->ignore();
        return;
    }

    int row = widget()->rowAt( event->pos().y() );
    int col = widget()->columnAt( event->pos().x() );
    DB::ResultId id = model()->mediaIdInCell( row, col );

    removeDropIndications();

    QRect rect = widget()->cellGeometry( row, col );
    bool left = ( event->pos().x() - rect.x() < rect.width()/2 );
    if ( left ) {
        model()->setLeftDropItem(id);
        int index = model()->indexOf(id) - 1;
        if ( index != -1 )
            model()->setRightDropItem( model()->_displayList.at(index) );
    }

    else {
        model()->setRightDropItem(id);
        const int index = model()->indexOf(id) + 1;
        if (index != model()->_displayList.size())
            model()->setLeftDropItem( model()->_displayList.at(index) );
    }

    widget()->updateCell( model()->leftDropItem() );
    widget()->updateCell( model()->rightDropItem() );
}

void ThumbnailView::ThumbnailDND::contentsDragLeaveEvent( QDragLeaveEvent* )
{
    removeDropIndications();
}

void ThumbnailView::ThumbnailDND::contentsDropEvent( QDropEvent* )
{
    QTimer::singleShot( 0, this, SLOT( realDropEvent() ) );
}

/**
 * Do the real work for the drop event.
 * We can't bring up the dialog in the contentsDropEvent, as Qt is still in drag and drop mode with a different cursor etc.
 * That's why we use a QTimer to get this call back executed.
 */
void ThumbnailView::ThumbnailDND::realDropEvent()
{
    QString msg =
        i18n( "<p><b>Really reorder thumbnails?</b></p>"
              "<p>By dragging images around in the thumbnail viewer, you actually reorder them. "
              "This is very useful in case you don't know the exact date for the images. On the other hand, "
              "if the images themself have valid timestamps, you should use "
              "<b>Images -&gt; Sort Selected By Date and Time</b></p>" );

    if ( KMessageBox::questionYesNo( widget(), msg, i18n("Reorder Thumbnails") , KStandardGuiItem::yes(), KStandardGuiItem::no(),
                                     QString::fromLatin1( "reorder_images" ) ) == KMessageBox::Yes ) {

        // protect against self drop
        if ( !model()->_selectedFiles.contains( model()->leftDropItem() ) && ! model()->_selectedFiles.contains( model()->rightDropItem() ) ) {
            const DB::Result selected = model()->selection();
            if ( model()->rightDropItem().isNull() ) {
                // We dropped onto the first image.
                DB::ImageDB::instance()->reorder( model()->leftDropItem(), selected, false );
            }
            else
                DB::ImageDB::instance()->reorder( model()->rightDropItem(), selected, true );

            Browser::BrowserWidget::instance()->reload();
        }
    }
    removeDropIndications();
}

void ThumbnailView::ThumbnailDND::removeDropIndications()
{
    DB::ResultId left = model()->leftDropItem();
    DB::ResultId right = model()->rightDropItem();
    model()->setLeftDropItem( DB::ResultId::null );
    model()->setRightDropItem( DB::ResultId::null );

    widget()->updateCell( left );
    widget()->updateCell( right );
}

void ThumbnailView::ThumbnailDND::contentsDragEnterEvent( QDragEnterEvent * event )
{
    if ( event->provides( "text/uri-list" ) && widget()->_selectionInteraction.isDragging() )
        event->accept();
    else
        event->ignore();
}
