// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ThumbnailDND.h"

#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"

#include <Browser/BrowserWidget.h>
#include <DB/ImageDB.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <QMimeData>
#include <QTimer>
#include <kwidgetsaddons_version.h>

ThumbnailView::ThumbnailDND::ThumbnailDND(ThumbnailFactory *factory)
    : ThumbnailComponent(factory)
{
}

void ThumbnailView::ThumbnailDND::contentsDragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls() && widget()->m_selectionInteraction.isDragging())
        event->accept();
    else {
        event->ignore();
        return;
    }

    removeDropIndications();

    const DB::FileName fileName = widget()->mediaIdUnderCursor();
    if (fileName.isNull()) {
        // cursor not in drop zone (e.g. empty space right/below of the thumbnails)
        return;
    }

    const QRect rect = widget()->visualRect(widget()->indexUnderCursor());

    if ((event->pos().y() < 10))
        widget()->scrollTo(widget()->indexUnderCursor(), QAbstractItemView::PositionAtCenter);
    if ((event->pos().y() > widget()->viewport()->visibleRegion().cbegin()->height() - 10))
        widget()->scrollTo(widget()->indexUnderCursor(), QAbstractItemView::PositionAtCenter);
    const bool isLeftHalfOfItem = (event->pos().x() - rect.x() < rect.width() / 2);
    if (isLeftHalfOfItem) {
        model()->setLeftDropItem(fileName);
        const int index = model()->indexOf(fileName) - 1;
        if (index != -1)
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

void ThumbnailView::ThumbnailDND::contentsDragLeaveEvent(QDragLeaveEvent *)
{
    removeDropIndications();
}

void ThumbnailView::ThumbnailDND::contentsDropEvent(QDropEvent *event)
{
    if (model()->leftDropItem().isNull() && model()->rightDropItem().isNull()) {
        // drop outside drop zone
        removeDropIndications();
        event->ignore();
    } else {
        QTimer::singleShot(0, this, SLOT(realDropEvent()));
    }
}

/**
 * Do the real work for the drop event.
 * We can't bring up the dialog in the contentsDropEvent, as Qt is still in drag and drop mode with a different cursor etc.
 * That's why we use a QTimer to get this call back executed.
 */
void ThumbnailView::ThumbnailDND::realDropEvent()
{
    const QString question = i18n("<p><b>Really reorder thumbnails?</b></p>"
                                  "<p>By dragging images around in the thumbnail viewer, you actually reorder them. "
                                  "This is very useful where you do not know the exact date for the images. On the other hand, "
                                  "if the images have valid timestamps, you should use "
                                  "<b>Maintenance -&gt; Sort All By Date and Time</b> or "
                                  "<b>View -&gt; Sort Selected By Date and Time</b>.</p>");

    const QString title = i18n("Reorder Thumbnails");
    const QString dontAskAgainName = QString::fromLatin1("reorder_images");
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
    const auto answer = KMessageBox::questionTwoActions(widget(),
                                                        question,
                                                        title,
                                                        KStandardGuiItem::ok(),
                                                        KStandardGuiItem::cancel(), dontAskAgainName);
    if (answer == KMessageBox::ButtonCode::PrimaryAction) {
#else
    const auto answer = KMessageBox::questionYesNo(widget(), question, title, KStandardGuiItem::yes(), KStandardGuiItem::no(), dontAskAgainName);
    if (answer == KMessageBox::Yes) {
#endif
        // expand selection so that stacks are always selected as a whole:
        const DB::FileNameList selected = widget()->selection(IncludeAllStacks);

        // protect against self drop
        if (selected.indexOf(model()->leftDropItem()) == -1 && selected.indexOf(model()->rightDropItem()) == -1) {
            if (model()->rightDropItem().isNull()) {
                // We dropped onto the first image.
                DB::ImageDB::instance()->reorder(model()->leftDropItem(), selected, false);
            } else
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
    model()->setLeftDropItem(DB::FileName());
    model()->setRightDropItem(DB::FileName());

    model()->updateCell(left);
    model()->updateCell(right);
}

void ThumbnailView::ThumbnailDND::contentsDragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls() && widget()->m_selectionInteraction.isDragging())
        event->accept();
    else
        event->ignore();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ThumbnailDND.cpp"
