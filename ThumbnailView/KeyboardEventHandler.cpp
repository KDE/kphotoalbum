/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "KeyboardEventHandler.h"

#include "CellGeometry.h"
#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"
#include "VideoThumbnailCycler.h"
#include "enums.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <MainWindow/DirtyIndicator.h>
#include <Settings/SettingsData.h>

ThumbnailView::KeyboardEventHandler::KeyboardEventHandler(ThumbnailFactory *factory)
    : ThumbnailComponent(factory)
{
}

bool ThumbnailView::KeyboardEventHandler::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Escape) {
        if (model()->isFiltered()) {
            model()->clearFilter();
            return true;
        }
    }
    // tokens
    if (event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z) {
        const QString token = event->text().toUpper().left(1);
        if (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::ShiftModifier) {
            // toggle tokens
            bool mustRemoveToken = false;
            bool hadHit = false;

            const DB::FileNameList selection = widget()->selection(event->modifiers() == Qt::NoModifier ? NoExpandCollapsedStacks : IncludeAllStacks);
            const DB::CategoryPtr tokensCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory);
            for (const DB::FileName &fileName : selection) {
                DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
                if (!hadHit) {
                    mustRemoveToken = info->hasCategoryInfo(tokensCategory->name(), token);
                    hadHit = true;
                }

                if (mustRemoveToken)
                    info->removeCategoryInfo(tokensCategory->name(), token);
                else
                    info->addCategoryInfo(tokensCategory->name(), token);

                model()->updateCell(fileName);
            }

            tokensCategory->addItem(token);
            MainWindow::DirtyIndicator::markDirty();
            return true;
        }
        if (event->modifiers() == (Qt::AltModifier | Qt::ShiftModifier)) {
            // filter view
            const QString tokensCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory)->name();
            model()->toggleCategoryFilter(tokensCategory, token);
            return true;
        }
    }

    // rating
    if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_5) {
        bool ok;
        const short rating = 2 * event->text().left(1).toShort(&ok, 10);
        if (ok) {
            if (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::ShiftModifier) {
                // set rating
                const DB::FileNameList selection = widget()->selection(event->modifiers() == Qt::NoModifier ? NoExpandCollapsedStacks : IncludeAllStacks);
                for (const DB::FileName &fileName : selection) {
                    DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
                    info->setRating(rating);
                }
                MainWindow::DirtyIndicator::markDirty();
                return true;
            }
        }
    }

    if (event->key() == Qt::Key_Control && widget()->isItemUnderCursorSelected())
        VideoThumbnailCycler::instance()->stopCycle();

    if (event->key() == Qt::Key_Return) {
        emit showSelection();
        return true;
    }

    return false;
}

/**
   Handle key release event.
   \return true if the event should propagate
*/
bool ThumbnailView::KeyboardEventHandler::keyReleaseEvent(QKeyEvent *event)
{
    if (widget()->m_wheelResizing && event->key() == Qt::Key_Control) {
        widget()->m_gridResizeInteraction.leaveGridResizingMode();
        widget()->m_wheelResizing = false;

        return false; // Don't propagate the event - I'm not sure why.
    }

    if (event->key() == Qt::Key_Control)
        VideoThumbnailCycler::instance()->setActive(widget()->mediaIdUnderCursor());

    return true;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
