/* Copyright (C) 2014 Tobias Leupold <tobias.leupold@web.de>

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
// Initially shamelessly stolen from http://qt-project.org/forums/viewthread/24104
// Big thanks to Mr. kripton :-)

#include "ResizableFrame.h"
#include "ImagePreview.h"
#include <QMouseEvent>
#include <QMenu>
#include <klocale.h>
#include <QDebug>
#include <QApplication>
#include <QList>

static const int SCALE_TOP    = 0b00000001;
static const int SCALE_BOTTOM = 0b00000010;
static const int SCALE_RIGHT  = 0b00000100;
static const int SCALE_LEFT   = 0b00001000;
static const int MOVE         = 0b10000000;

AnnotationDialog::ResizableFrame::ResizableFrame(QWidget *parent) :
    QFrame(parent)
{
    setFrameShape(QFrame::Box);
    setMouseTracking(true);
    setStyleSheet(QString::fromLatin1(
        "AnnotationDialog--ResizableFrame { color: rgb(255,0,0); }"
        "AnnotationDialog--ResizableFrame:hover { background-color: rgb(255,255,255,30); }"
    ));

    _removeAct = new QAction( i18nc("area of an image; rectangle that is overlayed upon the image", "Remove area"), this);
    connect( _removeAct, SIGNAL(triggered()), this, SLOT(remove()) );

    _lastTagAct = new QAction(this);
    // Can this also be done without the associateLastSelectedTag() helper function?
    connect( _lastTagAct, SIGNAL(triggered()), this, SLOT(associateLastSelectedTag()) );

    _removeTagAct = new QAction(this);
    connect( _removeTagAct, SIGNAL(triggered()), this, SLOT(removeTag()) );
}

AnnotationDialog::ResizableFrame::~ResizableFrame()
{
}

void AnnotationDialog::ResizableFrame::setActualCoordinates(QRect actualCoordinates)
{
    _actualCoordinates = actualCoordinates;
}

QRect AnnotationDialog::ResizableFrame::actualCoordinates() const
{
    return _actualCoordinates;
}

void AnnotationDialog::ResizableFrame::getMinMaxCoordinates()
{
    // Get the maximal area to drag or resize the frame
    ImagePreview *parent = dynamic_cast<ImagePreview *>(parentWidget());
    if (parent) {
        _minMaxCoordinates = parent->minMaxAreaPreview();
        // Add one pixel (width of the frame)
        _minMaxCoordinates.setWidth(_minMaxCoordinates.width() + 1);
        _minMaxCoordinates.setHeight(_minMaxCoordinates.height() + 1);
    }
}

void AnnotationDialog::ResizableFrame::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        _dragStartPosition = event->pos();
        _dragStartGeometry = geometry();

        // Just in case this will be a drag/resize and not just a click
        getMinMaxCoordinates();
    }
}

void AnnotationDialog::ResizableFrame::mouseMoveEvent(QMouseEvent *event)
{
    static int moveAction = 0;
    if (! (event->buttons() & Qt::LeftButton)) {
        // No drag, just change the cursor and return
        if (event->x() <= 3 && event->y() <= 3) {
            moveAction = SCALE_TOP | SCALE_LEFT;
            setCursor(Qt::SizeFDiagCursor);
        } else if (event->x() <= 3 && event->y() >= height() - 3) {
            moveAction = SCALE_BOTTOM | SCALE_LEFT;
            setCursor(Qt::SizeBDiagCursor);
        } else if (event->x() >= width() - 3 && event->y() <= 3) {
            moveAction = SCALE_TOP | SCALE_RIGHT;
            setCursor(Qt::SizeBDiagCursor);
        } else if (event->x() >= width() - 3 && event->y() >= height() - 3) {
            moveAction = SCALE_BOTTOM | SCALE_RIGHT;
            setCursor(Qt::SizeFDiagCursor);
        } else if (event->x() <= 3) {
            moveAction = SCALE_LEFT;
            setCursor(Qt::SizeHorCursor);
        } else if (event->x() >= width() - 3) {
            moveAction = SCALE_RIGHT;
            setCursor(Qt::SizeHorCursor);
        } else if (event->y() <= 3) {
            moveAction = SCALE_TOP;
            setCursor(Qt::SizeVerCursor);
        } else if (event->y() >= height() - 3) {
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
        x = _dragStartGeometry.left() - (_dragStartPosition.x() - event->x());
        y = _dragStartGeometry.top() - (_dragStartPosition.y() - event->y());
        w = width();

        // Be sure not to move out of the preview
        if (x < _minMaxCoordinates.left()) {
            x = _minMaxCoordinates.left();
        }
        if (y < _minMaxCoordinates.top()) {
            y = _minMaxCoordinates.top();
        }
        if (x + w > _minMaxCoordinates.width()) {
            x = _minMaxCoordinates.width() - w;
        }
        if (y + h > _minMaxCoordinates.height()) {
            y = _minMaxCoordinates.height() - h;
        }
    } else {
        // initialize with the "missing" values when only one direction is manipulated:
        x = _dragStartGeometry.left();
        y = _dragStartGeometry.top();
        w = _dragStartGeometry.width();

        if (moveAction & SCALE_TOP) {
            y = _dragStartGeometry.top() - (_dragStartPosition.y() - event->y());

            if (y >= geometry().y() + geometry().height()) {
                h = 0;
                y = _dragStartGeometry.top() + _dragStartGeometry.height();
                moveAction ^= SCALE_BOTTOM | SCALE_TOP;
            }

            if(y < _minMaxCoordinates.top()) {
                y = _minMaxCoordinates.top();
                h = _dragStartGeometry.top() + _dragStartGeometry.height() - _minMaxCoordinates.y();
            } else {
                h = height() + (_dragStartPosition.y() - event->y());
            }
        } else if (moveAction & SCALE_BOTTOM) {
            y = _dragStartGeometry.top();
            h = event->y();

            if (h <= 0) {
                h = 0;
                _dragStartPosition.setY(0);
                moveAction ^= SCALE_BOTTOM | SCALE_TOP;
            }

            if (y + h > _minMaxCoordinates.height()) {
                h = _minMaxCoordinates.height() - y;
            }
        }

        if (moveAction & SCALE_RIGHT) {
            x = _dragStartGeometry.left();
            w = event->x();

            if (w <= 0) {
                w = 0;
                _dragStartPosition.setX(0);
                moveAction ^= SCALE_RIGHT | SCALE_LEFT;
            }

            if (x + w > _minMaxCoordinates.width()) {
                w = _minMaxCoordinates.width() - x;
            }
        } else if (moveAction & SCALE_LEFT) {
            x = _dragStartGeometry.left() - (_dragStartPosition.x() - event->x());

            if (x >= geometry().left() + geometry().width()) {
                w = 0;
                x = _dragStartGeometry.left() + _dragStartGeometry.width();
                moveAction ^= SCALE_RIGHT | SCALE_LEFT;
            }

            if (x < _minMaxCoordinates.left()) {
                x = _minMaxCoordinates.left();
                w = _dragStartGeometry.x() + _dragStartGeometry.width() - _minMaxCoordinates.x();
            } else {
                w = _dragStartGeometry.width() + (_dragStartPosition.x() - event->x());
            }
        }
    }
    setGeometry(x, y, w, h);
    _dragStartGeometry = geometry();
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
    if (x < _minMaxCoordinates.left()) {
        x = _minMaxCoordinates.left();
    }
    if (y < _minMaxCoordinates.top()) {
        y = _minMaxCoordinates.top();
    }
    if (x + w > _minMaxCoordinates.width()) {
        x = _minMaxCoordinates.width() - w;
    }
    if (y + h > _minMaxCoordinates.height()) {
        y = _minMaxCoordinates.height() - h;
    }

    // If anything has been changed, set the updated geometry
    if (geometry().x() != x or geometry().y() != y or
        geometry().width() != w or geometry().height() != h) {
        setGeometry(x, y, w, h);
    }
}

void AnnotationDialog::ResizableFrame::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        checkGeometry();

        ImagePreview *parent = dynamic_cast<ImagePreview *>(parentWidget());
        if (parent) {
            setActualCoordinates(parent->areaPreviewToActual(geometry()));
        }
    }
}

void AnnotationDialog::ResizableFrame::contextMenuEvent(QContextMenuEvent *event)
{
    // Create the context menu
    QMenu *menu = new QMenu(this);

    // Let's see if we already have an associated tag
    if (! _tagData.first.isEmpty()) {
        _removeTagAct->setText(
                i18nc( "As in: remove tag %1 in category %2 [from this marked area of the image]"
                     , "Remove tag %1 (%2)"
                     , _tagData.second, _tagData.first ) );
        menu->addAction(_removeTagAct);
    } else {
        // Handle the last selected positionable tag

        QPair<QString, QString> lastSelectedPositionableTag = _dialog->lastSelectedPositionableTag();

        if (! lastSelectedPositionableTag.first.isEmpty()) {
            _lastTagAct->setText(
                    i18nc( "As in: associate [this marked area of the image] with tag %1 in category %2"
                         , "Associate with %1 (%2)"
                         , lastSelectedPositionableTag.second, _dialog->localizedCategory(lastSelectedPositionableTag.first)
                        ) );

            QStringList data;
            data << lastSelectedPositionableTag.first << lastSelectedPositionableTag.second;
            _lastTagAct->setData(data);

            menu->addAction(_lastTagAct);
        }

        // Handle all positionable tag candidates

        QList<QPair<QString, QString>> positionableTagCandidates = _dialog->positionableTagCandidates();

        // If we have a last selected positionable tag: remove it
        positionableTagCandidates.removeAt(positionableTagCandidates.indexOf(lastSelectedPositionableTag));

        // If we still have other candidates: add a respective sub-menu
        if (positionableTagCandidates.length() > 0) {
            // Create a new menu for all other tags
            QMenu *submenu = menu->addMenu(
                    i18nc( "As in: associate [this marked area of the image] with one of the following choices/menu items", "Associate with")
                    );

            for (const QPair<QString, QString> &tag : positionableTagCandidates) {
                QAction *action = new QAction(
                    tag.second + QString::fromLatin1(" (") +
                    _dialog->localizedCategory(tag.first) +
                    QString::fromLatin1(")"), this
                );

                QStringList data;
                data << tag.first << tag.second;
                action->setData(data);

                submenu->addAction(action);
            }
            connect( submenu, SIGNAL(triggered(QAction*)), this, SLOT(associateTag(QAction*)) );
        }
    }

    // Append the "Remove area" action
    menu->addAction(_removeAct);
    menu->exec(event->globalPos());

    // Clean up the menu
    delete menu;
}

void AnnotationDialog::ResizableFrame::associateLastSelectedTag()
{
    associateTag(_lastTagAct);
}

void AnnotationDialog::ResizableFrame::associateTag(QAction *action)
{
    setTagData(action->data().toStringList()[0], action->data().toStringList()[1]);
}

void AnnotationDialog::ResizableFrame::setTagData(QString category, QString tag)
{
    // Add the data to this area
    _tagData.first = category;
    _tagData.second = tag;
    setToolTip(
        _tagData.second + QString::fromLatin1(" (") +
        _dialog->localizedCategory(_tagData.first) +
        QString::fromLatin1(")")
    );

    // Set the color to "associated"
    setStyleSheet(QString::fromLatin1("AnnotationDialog--ResizableFrame { color: rgb(0,255,0); }"));

    // Remove the associated tag from the tag candidate list
    _dialog->removeTagFromCandidateList(_tagData.first, _tagData.second);
}

void AnnotationDialog::ResizableFrame::removeTag()
{
    // Add the tag to the positionable candidate list again
    _dialog->addTagToCandidateList(_tagData.first, _tagData.second);
    // Delete the tag data from this area
    removeTagData();
}

void AnnotationDialog::ResizableFrame::removeTagData()
{
    // Delete the data
    _tagData.first.clear();
    _tagData.second.clear();
    setToolTip(QString());

    // Set the color to "un-associated"
    setStyleSheet(QString::fromLatin1("AnnotationDialog--ResizableFrame { color: rgb(255,0,0); }"));
}

void AnnotationDialog::ResizableFrame::remove()
{
    if (! _tagData.first.isEmpty()) {
        // Re-add the associated tag to the candidate list
        removeTag();
    }

    // Delete the area
    this->deleteLater();
}

void AnnotationDialog::ResizableFrame::checkShowContextMenu()
{
    // Don't show the context menu when we don't have a last selected positionable tag
    if (_dialog->lastSelectedPositionableTag().first.isEmpty()) {
        return;
    }

    // Show the context menu at the lower right corner of the newly created area
    QContextMenuEvent *event = new QContextMenuEvent(
        QContextMenuEvent::Mouse, QPoint(0, 0), QCursor::pos(), Qt::NoModifier
    );
    QApplication::postEvent(this, event);
}

void AnnotationDialog::ResizableFrame::setDialog(Dialog *dialog)
{
    _dialog = dialog;
}

QPair<QString, QString> AnnotationDialog::ResizableFrame::tagData() const
{
    return _tagData;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
