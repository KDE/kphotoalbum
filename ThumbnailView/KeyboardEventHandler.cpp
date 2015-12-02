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
#include "CellGeometry.h"
#include "enums.h"
#include "DB/CategoryCollection.h"
#include "ThumbnailWidget.h"
#include "MainWindow/DirtyIndicator.h"
#include "DB/ImageDB.h"
#include "ThumbnailModel.h"
#include "VideoThumbnailCycler.h"
#include "Settings/SettingsData.h"

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

        const DB::FileNameList selection = widget()->selection(NoExpandCollapsedStacks);
        DB::CategoryPtr tokensCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory);
        Q_FOREACH( const DB::FileName& fileName, selection ) {
            DB::ImageInfoPtr info = fileName.info();
            if ( ! hadHit ) {
                mustRemoveToken = info->hasCategoryInfo(tokensCategory->name(), token );
                hadHit = true;
            }

            if ( mustRemoveToken )
                info->removeCategoryInfo(tokensCategory->name(), token );
            else
                info->addCategoryInfo(tokensCategory->name(), token );

            model()->updateCell(fileName);
        }

        tokensCategory->addItem(token);
        MainWindow::DirtyIndicator::markDirty();
        return true;
    }

    if ( event->modifiers() == Qt::NoModifier && ( event->key() >= Qt::Key_0 && event->key() <= Qt::Key_5 ) ) {
        bool ok;
        short rating = event->text().left(1).toShort(&ok, 10);
        if (ok) {
            const DB::FileNameList selection = widget()->selection( NoExpandCollapsedStacks );
            Q_FOREACH( const DB::FileName& fileName, selection ) {
                DB::ImageInfoPtr info = fileName.info();
                info->setRating(rating * 2);
            }
            MainWindow::DirtyIndicator::markDirty();
        }
        return true;
    }

    if (event->key() == Qt::Key_Control && widget()->isItemUnderCursorSelected())
        VideoThumbnailCycler::instance()->stopCycle();

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
    if ( widget()->m_wheelResizing && event->key() == Qt::Key_Control ) {
        widget()->m_gridResizeInteraction.leaveGridResizingMode();
        widget()->m_wheelResizing = false;

        return false; // Don't propagate the event - I'm not sure why.
    }

    if (event->key() == Qt::Key_Control)
        VideoThumbnailCycler::instance()->setActive(widget()->mediaIdUnderCursor());

    return true;
}



// vi:expandtab:tabstop=4 shiftwidth=4:
