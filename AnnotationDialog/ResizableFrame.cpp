// SPDX-FileCopyrightText: 2014-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2017 Jonathan Riddell <jr@jriddell.org>
// SPDX-FileCopyrightText: 2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2014-2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

// The basic resizable QFrame has been shamelessly stolen from
// http://qt-project.org/forums/viewthread/24104
// Big thanks to Mr. kripton :-)

#include "ResizableFrame.h"

// Local includes
#include "AreaTagSelectDialog.h"
#include "ImagePreview.h"
#include "ImagePreviewWidget.h"
#include "Logging.h"

// Qt includes
#include <QApplication>
#include <QDockWidget>
#include <QList>
#include <QLoggingCategory>
#include <QMenu>
#include <QMouseEvent>
#include <QTimer>

// KDE includes
#include <KLocalizedString>
#include <KMessageBox>

namespace
{
constexpr int SCALE_TOP = 0b00000001;
constexpr int SCALE_BOTTOM = 0b00000010;
constexpr int SCALE_RIGHT = 0b00000100;
constexpr int SCALE_LEFT = 0b00001000;
constexpr int MOVE = 0b10000000;
}

AnnotationDialog::ResizableFrame::ResizableFrame(QWidget *parent)
    : QFrame(parent)
    , m_dialog(nullptr)
{
    m_preview = dynamic_cast<ImagePreview *>(parent);
    Q_ASSERT(m_preview);
    m_previewWidget = dynamic_cast<ImagePreviewWidget *>(m_preview->parentWidget());
    Q_ASSERT(m_previewWidget);

    setFrameShape(QFrame::Box);
    setMouseTracking(true);

    m_removeAct = new QAction(
        i18nc("area of an image; rectangle that is overlayed upon the image",
              "Remove area"),
        this);
    connect(m_removeAct, &QAction::triggered, this, &ResizableFrame::remove);

    m_removeTagAct = new QAction(this);
    connect(m_removeTagAct, &QAction::triggered, this, &ResizableFrame::removeTag);
}

AnnotationDialog::ResizableFrame::~ResizableFrame()
{
}

void AnnotationDialog::ResizableFrame::setActualCoordinates(QRect actualCoordinates)
{
    m_actualCoordinates = actualCoordinates;
}

QRect AnnotationDialog::ResizableFrame::actualCoordinates() const
{
    return m_actualCoordinates;
}

void AnnotationDialog::ResizableFrame::getMinMaxCoordinates()
{
    // Get the maximal area to drag or resize the frame
    m_minMaxCoordinates = m_preview->minMaxAreaPreview();
    // Add one pixel (width of the frame)
    m_minMaxCoordinates.setWidth(m_minMaxCoordinates.width() + 1);
    m_minMaxCoordinates.setHeight(m_minMaxCoordinates.height() + 1);
}

void AnnotationDialog::ResizableFrame::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->position().toPoint();
        m_dragStartGeometry = geometry();

        // Just in case this will be a drag/resize and not just a click
        getMinMaxCoordinates();
    }
}

void AnnotationDialog::ResizableFrame::mouseMoveEvent(QMouseEvent *event)
{
    const auto eventPos = event->position().toPoint();
    static int moveAction = 0;
    if (!(event->buttons() & Qt::LeftButton)) {
        // No drag, just change the cursor and return
        if (eventPos.x() <= 3 && eventPos.y() <= 3) {
            moveAction = SCALE_TOP | SCALE_LEFT;
            setCursor(Qt::SizeFDiagCursor);
        } else if (eventPos.x() <= 3 && eventPos.y() >= height() - 3) {
            moveAction = SCALE_BOTTOM | SCALE_LEFT;
            setCursor(Qt::SizeBDiagCursor);
        } else if (eventPos.x() >= width() - 3 && eventPos.y() <= 3) {
            moveAction = SCALE_TOP | SCALE_RIGHT;
            setCursor(Qt::SizeBDiagCursor);
        } else if (eventPos.x() >= width() - 3 && eventPos.y() >= height() - 3) {
            moveAction = SCALE_BOTTOM | SCALE_RIGHT;
            setCursor(Qt::SizeFDiagCursor);
        } else if (eventPos.x() <= 3) {
            moveAction = SCALE_LEFT;
            setCursor(Qt::SizeHorCursor);
        } else if (eventPos.x() >= width() - 3) {
            moveAction = SCALE_RIGHT;
            setCursor(Qt::SizeHorCursor);
        } else if (eventPos.y() <= 3) {
            moveAction = SCALE_TOP;
            setCursor(Qt::SizeVerCursor);
        } else if (eventPos.y() >= height() - 3) {
            moveAction = SCALE_BOTTOM;
            setCursor(Qt::SizeVerCursor);
        } else {
            moveAction = MOVE;
            setCursor(Qt::SizeAllCursor);
        }
        return;
    }

    int x;
    int y;
    int w;
    int h;

    h = height();

    if (moveAction & MOVE) {
        x = m_dragStartGeometry.left() - (m_dragStartPosition.x() - eventPos.x());
        y = m_dragStartGeometry.top() - (m_dragStartPosition.y() - eventPos.y());
        w = width();

        // Be sure not to move out of the preview
        if (x < m_minMaxCoordinates.left()) {
            x = m_minMaxCoordinates.left();
        }
        if (y < m_minMaxCoordinates.top()) {
            y = m_minMaxCoordinates.top();
        }
        if (x + w > m_minMaxCoordinates.width()) {
            x = m_minMaxCoordinates.width() - w;
        }
        if (y + h > m_minMaxCoordinates.height()) {
            y = m_minMaxCoordinates.height() - h;
        }
    } else {
        // initialize with the "missing" values when only one direction is manipulated:
        x = m_dragStartGeometry.left();
        y = m_dragStartGeometry.top();
        w = m_dragStartGeometry.width();

        if (moveAction & SCALE_TOP) {
            y = m_dragStartGeometry.top() - (m_dragStartPosition.y() - eventPos.y());

            if (y >= geometry().y() + geometry().height()) {
                y = m_dragStartGeometry.top() + m_dragStartGeometry.height();
                moveAction ^= SCALE_BOTTOM | SCALE_TOP;
            }

            if (y < m_minMaxCoordinates.top()) {
                y = m_minMaxCoordinates.top();
                h = m_dragStartGeometry.top() + m_dragStartGeometry.height() - m_minMaxCoordinates.y();
            } else {
                h = height() + (m_dragStartPosition.y() - eventPos.y());
            }
        } else if (moveAction & SCALE_BOTTOM) {
            y = m_dragStartGeometry.top();
            h = eventPos.y();

            if (h <= 0) {
                h = 0;
                m_dragStartPosition.setY(0);
                moveAction ^= SCALE_BOTTOM | SCALE_TOP;
            }

            if (y + h > m_minMaxCoordinates.height()) {
                h = m_minMaxCoordinates.height() - y;
            }
        }

        if (moveAction & SCALE_RIGHT) {
            x = m_dragStartGeometry.left();
            w = eventPos.x();

            if (w <= 0) {
                w = 0;
                m_dragStartPosition.setX(0);
                moveAction ^= SCALE_RIGHT | SCALE_LEFT;
            }

            if (x + w > m_minMaxCoordinates.width()) {
                w = m_minMaxCoordinates.width() - x;
            }
        } else if (moveAction & SCALE_LEFT) {
            x = m_dragStartGeometry.left() - (m_dragStartPosition.x() - eventPos.x());

            if (x >= geometry().left() + geometry().width()) {
                x = m_dragStartGeometry.left() + m_dragStartGeometry.width();
                moveAction ^= SCALE_RIGHT | SCALE_LEFT;
            }

            if (x < m_minMaxCoordinates.left()) {
                x = m_minMaxCoordinates.left();
                w = m_dragStartGeometry.x() + m_dragStartGeometry.width() - m_minMaxCoordinates.x();
            } else {
                w = m_dragStartGeometry.width() + (m_dragStartPosition.x() - eventPos.x());
            }
        }
    }
    setGeometry(x, y, w, h);
    m_dragStartGeometry = geometry();

    if (!m_tagData.first.isEmpty()) {
        // If we change an area with an associated tag:
        // tell the Dialog we made a change that should be saved (set the dirty marker)
        m_dialog->areaChanged();
    }
}

void AnnotationDialog::ResizableFrame::checkGeometry()
{
    // If this is called when the area is created, we don't have it yet
    getMinMaxCoordinates();

    // First cache the current geometry
    int x;
    int y;
    int w;
    int h;
    x = geometry().x();
    y = geometry().y();
    w = geometry().width();
    h = geometry().height();

    // Be sure no non-visible area is created by resizing to a height or width of 0
    // A height and width of 3 is the minimum to have more than a line
    if (geometry().height() < 3) {
        y = y - 1;
        h = 3;
    }
    if (geometry().width() < 3) {
        x = x - 1;
        w = 3;
    }

    // Probably, the above tweaking moved the area out of the preview area
    if (x < m_minMaxCoordinates.left()) {
        x = m_minMaxCoordinates.left();
    }
    if (y < m_minMaxCoordinates.top()) {
        y = m_minMaxCoordinates.top();
    }
    if (x + w > m_minMaxCoordinates.width()) {
        x = m_minMaxCoordinates.width() - w;
    }
    if (y + h > m_minMaxCoordinates.height()) {
        y = m_minMaxCoordinates.height() - h;
    }

    // If anything has been changed, set the updated geometry
    if (geometry().x() != x
        || geometry().y() != y
        || geometry().width() != w
        || geometry().height() != h) {

        setGeometry(x, y, w, h);
    }
}

void AnnotationDialog::ResizableFrame::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        checkGeometry();
        setActualCoordinates(m_preview->areaPreviewToActual(geometry()));
    }
}

void AnnotationDialog::ResizableFrame::contextMenuEvent(QContextMenuEvent *)
{
    showContextMenu();
}

void AnnotationDialog::ResizableFrame::repolish()
{
    style()->unpolish(this);
    style()->polish(this);
    update();
}

QAction *AnnotationDialog::ResizableFrame::createAssociateTagAction(
    const QPair<QString, QString> &tag,
    QString prefix)
{
    QString actionText;
    if (!prefix.isEmpty()) {
        actionText = i18nc("%1 is a prefix like 'Associate with', "
                           "%2 is the tag name and %3 is the tag's category",
                           "%1 %2 (%3)",
                           prefix, tag.second, tag.first);
    } else {
        actionText = i18nc("%1 is the tag name and %2 is the tag's category",
                           "%1 (%2)",
                           tag.second, tag.first);
    }

    QAction *action = new QAction(actionText, this);
    QStringList data;
    data << tag.first << tag.second;
    action->setData(data);

    return action;
}

bool AnnotationDialog::ResizableFrame::associated() const
{
    return !m_tagData.first.isEmpty();
}

void AnnotationDialog::ResizableFrame::associateTag()
{
    QAction *action = qobject_cast<QAction *>(sender());
    Q_ASSERT(action != nullptr);
    associateTag(action);
}

void AnnotationDialog::ResizableFrame::associateTag(QAction *action)
{
    setTagData(action->data().toStringList()[0], action->data().toStringList()[1]);
}

void AnnotationDialog::ResizableFrame::setTagData(QString category, QString tag, ChangeOrigin changeOrigin)
{
    QPair<QString, QString> selectedData = QPair<QString, QString>(category, tag);

    // check existing areas for consistency
    const auto areas = m_dialog->areas();
    for (ResizableFrame *area : areas) {
        if (area->isTidied()) {
            continue;
        }

        if (area->tagData() == selectedData) {
            if (KMessageBox::Cancel == KMessageBox::warningContinueCancel(m_preview, i18n("<p>%1 has already been tagged in another area on this image.</p>"
                                                                                          "<p>If you continue, the previous tag will be removed...</p>",
                                                                                          tag),
                                                                          i18n("Replace existing area?"))) {
                // don't execute setTagData
                return;
            }
            // replace existing tag
            area->removeTagData();
        }
    }
    // Add the data to this area
    m_tagData = selectedData;

    // Update the tool tip
    setToolTip(tag + QString::fromUtf8(" (") + category + QString::fromUtf8(")"));

    repolish();
    qCDebug(AnnotationDialogLog) << "ResizeableFrame is now associated to" << m_tagData;

    // Remove the associated tag from the tag candidate list
    m_dialog->removeTagFromCandidateList(m_tagData.first, m_tagData.second);

    if (changeOrigin != AutomatedChange) {
        // Tell the dialog an area has been changed
        m_dialog->areaChanged();
    }
}

void AnnotationDialog::ResizableFrame::removeTag()
{
    // Deselect the tag
    m_dialog->listSelectForCategory(m_tagData.first)->deselectTag(m_tagData.second);
    // Delete the tag data from this area
    removeTagData();
}

void AnnotationDialog::ResizableFrame::removeTagData()
{
    // Delete the data
    m_tagData.first.clear();
    m_tagData.second.clear();
    setToolTip(QString());
    repolish();
    qCDebug(AnnotationDialogLog) << "ResizeableFrame is now unassociated";

    // Tell the dialog an area has been changed
    m_dialog->areaChanged();
}

void AnnotationDialog::ResizableFrame::remove()
{
    if (!m_tagData.first.isEmpty()) {
        // Deselect the tag
        m_dialog->listSelectForCategory(m_tagData.first)->deselectTag(m_tagData.second);
    }

    // Delete the area
    this->deleteLater();
}

void AnnotationDialog::ResizableFrame::showContextMenu()
{
    // Display a dialog where a tag can be selected directly
    QString category = m_previewWidget->defaultPositionableCategory();
    // this is not a memory leak: AreaTagSelectDialog is a regular parented dialog
    AreaTagSelectDialog *tagMenu = new AreaTagSelectDialog(
        this,
        m_dialog->listSelectForCategory(category),
        m_preview->grabAreaImage(geometry()),
        m_dialog);

    tagMenu->show();
    tagMenu->moveToArea(mapToGlobal(QPoint(0, 0)));
    tagMenu->exec();
}

void AnnotationDialog::ResizableFrame::setDialog(Dialog *dialog)
{
    m_dialog = dialog;
}

QPair<QString, QString> AnnotationDialog::ResizableFrame::tagData() const
{
    return m_tagData;
}

QPair<QString, QString> AnnotationDialog::ResizableFrame::proposedTagData() const
{
    return m_proposedTagData;
}

void AnnotationDialog::ResizableFrame::removeProposedTagData()
{
    m_proposedTagData = QPair<QString, QString>();
    setToolTip(QString());
}

void AnnotationDialog::ResizableFrame::addTagActions(QMenu *menu)
{
    // Let's see if we already have an associated tag
    if (!m_tagData.first.isEmpty()) {
        m_removeTagAct->setText(
            i18nc("As in: remove tag %1 in category %2 [from this marked area of the image]",
                  "Remove tag %1 (%2)",
                  m_tagData.second,
                  m_tagData.first));
        menu->addAction(m_removeTagAct);

    } else {
        // Handle the last selected positionable tag (if we have one)
        QPair<QString, QString> lastSelectedPositionableTag = m_dialog->lastSelectedPositionableTag();
        if (!lastSelectedPositionableTag.first.isEmpty()) {
            QAction *associateLastSelectedTagAction = createAssociateTagAction(
                lastSelectedPositionableTag,
                i18n("Associate with"));
            connect(associateLastSelectedTagAction, &QAction::triggered,
                    this, QOverload<>::of(&ResizableFrame::associateTag));
            menu->addAction(associateLastSelectedTagAction);
        }

        // Handle all positionable tag candidates

        QList<QPair<QString, QString>> positionableTagCandidates = m_dialog->positionableTagCandidates();
        // If we have a last selected positionable tag: remove it
        const auto lastIndex = positionableTagCandidates.indexOf(lastSelectedPositionableTag);
        if (lastIndex >= 0)
            positionableTagCandidates.removeAt(lastIndex);

        // If we still have candidates:
        if (positionableTagCandidates.length() > 0) {
            if (positionableTagCandidates.length() == 1
                && lastSelectedPositionableTag.first.isEmpty()) {

                // Add a single action
                QAction *associateOnlyCandidateAction = createAssociateTagAction(
                    positionableTagCandidates[0],
                    i18nc("As in: associate [this marked area of the image] with one of the "
                          "following choices/menu items",
                          "Associate with"));
                connect(associateOnlyCandidateAction, &QAction::triggered,
                        this, QOverload<>::of(&ResizableFrame::associateTag));
                menu->addAction(associateOnlyCandidateAction);
            } else {
                // Create a new menu for all other tags
                QMenu *submenu = menu->addMenu(
                    i18nc("As in: associate [this marked area of the image] with one of the "
                          "following choices/menu items",
                          "Associate with"));

                for (const QPair<QString, QString> &tag : positionableTagCandidates) {
                    submenu->addAction(createAssociateTagAction(tag));
                }

                connect(submenu, &QMenu::triggered, this, QOverload<QAction *>::of(&ResizableFrame::associateTag));
            }
        }
    }

    QAction *sep = menu->addSeparator();
    // clicking the separator should not dismiss the menu:
    sep->setEnabled(false);

    // Append the "Remove area" action
    menu->addAction(m_removeAct);
}

void AnnotationDialog::ResizableFrame::markTidied()
{
    m_tidied = true;
}

bool AnnotationDialog::ResizableFrame::isTidied() const
{
    return m_tidied;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ResizableFrame.cpp"
