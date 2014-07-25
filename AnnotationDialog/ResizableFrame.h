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

#ifndef RESIZABLEFRAME_H
#define RESIZABLEFRAME_H

#include <QFrame>
#include "Dialog.h"
#include <QTreeWidgetItem>
#include "ListSelect.h"

class QMouseEvent;

namespace AnnotationDialog {

class ResizableFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ResizableFrame(QWidget *parent = 0);
    ~ResizableFrame();
    void setActualCoordinates(QRect actualCoordinates);
    QRect actualCoordinates() const;
    void checkGeometry();
    void checkShowContextMenu();
    void setDialog(Dialog *dialog);
    QPair<QString, QString> tagData() const;
    void removeTagData();
    void setTagData(QString category, QString tag);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void associateLastSelectedTag();
    void associateTag(QAction *action);
    void remove();
    void removeTag();

private:
    QPoint m_dragStartPosition;
    QRect m_dragStartGeometry;
    QRect m_minMaxCoordinates;
    QRect m_actualCoordinates;
    void getMinMaxCoordinates();
    QAction *m_lastTagAct;
    QAction *m_removeAct;
    QAction *m_removeTagAct;
    Dialog *m_dialog;
    QPair<QString, QString> m_tagData;
};

}

#endif // RESIZABLEFRAME_H
// vi:expandtab:tabstop=4 shiftwidth=4:
