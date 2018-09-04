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
#include "ThumbnailDND.h"

#include <QMimeData>
#include <QTimer>

#include <KMessageBox>
#include <KLocalizedString>

#include <Browser/BrowserWidget.h>
#include <DB/ImageDB.h>
#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"

ThumbnailView::ThumbnailDND::ThumbnailDND( ThumbnailFactory* factory )
    : ThumbnailComponent( factory )
{
}

void ThumbnailView::ThumbnailDND::contentsDragMoveEvent( QDragMoveEvent* event )
{
    if ( event->mimeData()->hasUrls() && widget()->m_selectionInteraction.isDragging() )
        event->accept();
    else {
        event->ignore();
        return;
    }

    removeDropIndications();

    const DB::FileName fileName = widget()->mediaIdUnderCursor();
    if (fileName.isNull())
    {
        // cursor not in drop zone (e.g. empty space right/below of the thumbnails)
        return;
    }

    const QRect rect = widget()->visualRect( widget()->indexUnderCursor() );

    if ( ( event->pos().y() < 10 ) )
        widget()->scrollTo( widget()->indexUnderCursor(), QAbstractItemView::PositionAtCenter );
    if ( ( event->pos().y() > widget()->viewport()->visibleRegion().cbegin()->height() - 10 ) )
        widget()->scrollTo( widget()->indexUnderCursor(), QAbstractItemView::PositionAtCenter );
    bool left = ( event->pos().x() - rect.x() < rect.width()/2 );
    if ( left ) {
        model()->setLeftDropItem(fileName);
        const int index = model()->indexOf(fileName) - 1;
        if ( index != -1 )
            model()->setRightDropItem(model()->imageAt(index));
    }

    else {
        model()->setRightDropItem(fileName);
        const int index = model()->indexOf(fileName) + 1;
        if (index != model()->imageCount())
            model()->setLeftDropItem(model()->imageAt(index));
    }

    model()->updateCell(model()->leftDropItem());
    model()->updateCell(model()->rightDropItem());
}

void ThumbnailView::ThumbnailDND::contentsDragLeaveEvent( QDragLeaveEvent* )
{
    removeDropIndications();
}

void ThumbnailView::ThumbnailDND::contentsDropEvent(QDropEvent *event)
{
    if (model()->leftDropItem().isNull() && model()->rightDropItem().isNull())
    {
        // drop outside drop zone
        removeDropIndications();
        event->ignore();
    } else {
        QTimer::singleShot( 0, this, SLOT(realDropEvent()) );
    }
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
              "This is very useful where you do not know the exact date for the images. On the other hand, "
              "if the images have valid timestamps, you should use "
              "<b>Maintenance -&gt; Sort All By Date and Time</b> or "
              "<b>View -&gt; Sort Selected By Date and Time</b>.</p>" );

    if ( KMessageBox::questionYesNo( widget(), msg, i18n("Reorder Thumbnails") , KStandardGuiItem::yes(), KStandardGuiItem::no(),
                                     QString::fromLatin1( "reorder_images" ) ) == KMessageBox::Yes )
    {
        // expand selection so that stacks are always selected as a whole:
        const DB::FileNameList selected = widget()->selection(IncludeAllStacks);

        // protect against self drop
        if ( selected.indexOf(model()->leftDropItem()) == -1 && selected.indexOf(model()->rightDropItem()) == -1 ) {
            if ( model()->rightDropItem().isNull() ) {
                // We dropped onto the first image.
                DB::ImageDB::instance()->reorder(model()->leftDropItem(), selected, false);
            }
            else
                DB::ImageDB::instance()->reorder(model()->rightDropItem(), selected, true);

            Browser::BrowserWidget::instance()->reload();
        }
    }
    removeDropIndications();
}

void ThumbnailView::ThumbnailDND::removeDropIndications()
{
    const DB::FileName left = model()->leftDropItem();
    const DB::FileName right = model()->rightDropItem();
    model()->setLeftDropItem( DB::FileName() );
    model()->setRightDropItem( DB::FileName() );

    if (!left.isNull())
        model()->updateCell(left);
    if (!right.isNull())
        model()->updateCell(right);
}

void ThumbnailView::ThumbnailDND::contentsDragEnterEvent( QDragEnterEvent * event )
{
    if ( event->mimeData()->hasUrls() && widget()->m_selectionInteraction.isDragging() )
        event->accept();
    else
        event->ignore();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
