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
#include "KeyboardEventHandler.h"
#include "GridResizeInteraction.h"
#include "CellGeometry.h"
#include "enums.h"
#include "DB/CategoryCollection.h"
#include "ThumbnailWidget.h"
#include "MainWindow/DirtyIndicator.h"
#include "DB/ImageDB.h"
#include "ThumbnailModel.h"

ThumbnailView::KeyboardEventHandler::KeyboardEventHandler( ThumbnailFactory* factory )
    : ThumbnailComponent( factory )
{

}

bool ThumbnailView::KeyboardEventHandler::keyPressEvent( QKeyEvent* event )
{
    if ( event->modifiers() == Qt::NoModifier && ( event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z ) ) {
        QString token = event->text().toUpper().left(1);
        bool mustRemoveToken = false;
        bool hadHit          = false;

        const DB::IdList selection = widget()->selection();
        for( DB::IdList::ConstIterator it = selection.begin(); it != selection.end(); ++it ) {
            DB::ImageInfoPtr info = (*it).fetchInfo();
            if ( ! hadHit ) {
                mustRemoveToken = info->hasCategoryInfo( QString::fromLatin1("Tokens"), token );
                hadHit = true;
            }

            if ( mustRemoveToken )
                info->removeCategoryInfo( QString::fromLatin1("Tokens"), token );
            else
                info->addCategoryInfo( QString::fromLatin1("Tokens"), token );

            model()->updateCell( *it );
        }

        DB::ImageDB::instance()->categoryCollection()->categoryForName( QString::fromLatin1("Tokens") )->addItem( token );
        MainWindow::DirtyIndicator::markDirty();
        return true;
    } else if ( event->modifiers() == Qt::NoModifier && ( event->key() >= Qt::Key_0 && event->key() <= Qt::Key_5 ) ) {
        bool ok;
        short rating = event->text().left(1).toShort(&ok, 10);
        if (ok) {
            const DB::IdList selection = widget()->selection();
            for( DB::IdList::ConstIterator it = selection.begin(); it != selection.end(); ++it ) {
                DB::ImageInfoPtr info = (*it).fetchInfo();
                info->setRating(rating * 2);
            }
            MainWindow::DirtyIndicator::markDirty();
        }
        return true;
    }

    if ( event->key() == Qt::Key_Return ) {
        emit showSelection();
        return true;
    }

    return false;
}

/**
   Handle key release event.
   \return true if the event should propagate
*/
bool ThumbnailView::KeyboardEventHandler::keyReleaseEvent( QKeyEvent* event )
{
    if ( widget()->_wheelResizing && event->key() == Qt::Key_Control ) {
        widget()->_gridResizeInteraction.leaveGridResizingMode();
        widget()->_wheelResizing = false;

        return false; // Don't propagate the event - I'm not sure why.
    }
    return true;
}



