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

// The basic resizable QFrame has been shamelessly stolen from
// http://qt-project.org/forums/viewthread/24104
// Big thanks to Mr. kripton :-)

#ifndef RESIZABLEFRAME_H
#define RESIZABLEFRAME_H

// Qt includes
#include <QFrame>
#include <QTreeWidgetItem>

// Local includes
#include "enums.h"
#include "Dialog.h"
#include "ListSelect.h"
#include "config-kpa-kface.h"

class QMouseEvent;

namespace AnnotationDialog {

#ifdef HAVE_KFACE
class ProposedFaceDialog;
#endif

class ResizableFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ResizableFrame(QWidget* parent = 0);
    ~ResizableFrame();

    void setActualCoordinates(QRect actualCoordinates);
    QRect actualCoordinates() const;

    void checkGeometry();
    void showContextMenu();

    void setDialog(Dialog* dialog);
    QPair<QString, QString> tagData() const;
    void removeTagData();
    void setTagData(QString category, QString tag, ChangeOrigin changeOrigin = ManualChange);
    void setProposedTagData(QPair<QString, QString> tagData);
    QPair<QString, QString> proposedTagData() const;
    void removeProposedTagData();

    /**
     * @brief Add the context menu actions to a QMenu.
     * @sa AreaTagSelectDialog
     * @param w
     */
    void addTagActions(QMenu *menu);
#ifdef HAVE_KFACE
    /**
     * If the face has been detected by the face detector, this method is called.
     * In this case, the marking is considered "good enough" for the recognition
     * database to be trained on this face.
     *
     * When a user manually marks a person, this should not be called.
     */
    void markAsFace();
    void proposedFaceDialogRemoved();

public slots:
    void acceptTag();
#endif

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent *);
#ifdef HAVE_KFACE
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);

protected slots:
    void checkUnderMouse();
#endif

private slots:
    void associateTag();
    void associateTag(QAction* action);
    void remove();
    void removeTag();
#ifdef HAVE_KFACE
    void updateRecognitionDatabase();
    void recognize();
#endif

private: // Functions
    void getMinMaxCoordinates();
    QAction* createAssociateTagAction(
        const QPair<QString, QString>& tag, QString prefix = QString()
    );

private: // Variables
    QPoint m_dragStartPosition;
    QRect m_dragStartGeometry;
    QRect m_minMaxCoordinates;
    QRect m_actualCoordinates;
    QAction* m_removeAct;
    QAction* m_removeTagAct;
    Dialog* m_dialog;
    QPair<QString, QString> m_tagData;
    QPair<QString, QString> m_proposedTagData;
    ImagePreview* m_preview;
    ImagePreviewWidget* m_previewWidget;
#ifdef HAVE_KFACE
    QAction* m_updateRecognitionDatabaseAct;
    QAction* m_recognizeAct;
    bool m_changed;
    bool m_trained;
    bool m_detectedFace;
    ProposedFaceDialog* m_proposedFaceDialog;
#endif
};

}

#endif // RESIZABLEFRAME_H

// vi:expandtab:tabstop=4 shiftwidth=4:
