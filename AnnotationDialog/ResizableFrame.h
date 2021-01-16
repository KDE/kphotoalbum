/* SPDX-FileCopyrightText: 2014-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

// The basic resizable QFrame has been shamelessly stolen from
// http://qt-project.org/forums/viewthread/24104
// Big thanks to Mr. kripton :-)

#ifndef RESIZABLEFRAME_H
#define RESIZABLEFRAME_H

#include "Dialog.h"
#include "ListSelect.h"

#include <kpabase/enums.h>

#include <QFrame>
#include <QTreeWidgetItem>

class QMouseEvent;

namespace AnnotationDialog
{

/**
 * @brief The ResizableFrame class represents a positionable tag in the annotation dialog.
 * It has two basic states: associated to a tag, and unassociated.
 *
 * An AreaTagSelectDialog provides the context menu to allow associating the ResizableFrame with a tag,
 * as well as removing the tag, or removing the area completely.
 *
 * If an area is removed, the associated tag is usually removed from the image as well.
 *
 * ## Styling
 * The frame is styled based on this state (see property \c associated).
 * The following styles are expected to be set for a proper appearance of ResizableFrame:
 *  - `AnnotationDialog--ResizableFrame`
 *  - `AnnotationDialog--ResizableFrame:hover`
 *  - `AnnotationDialog--ResizableFrame[associated="true"]`
 */
class ResizableFrame : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool associated READ associated)

public:
    explicit ResizableFrame(QWidget *parent = 0);
    ~ResizableFrame() override;

    void setActualCoordinates(QRect actualCoordinates);
    QRect actualCoordinates() const;

    void checkGeometry();
    void showContextMenu();

    void setDialog(Dialog *dialog);
    QPair<QString, QString> tagData() const;
    void removeTagData();
    void setTagData(QString category, QString tag, ChangeOrigin changeOrigin = ManualChange);
    QPair<QString, QString> proposedTagData() const;
    void removeProposedTagData();

    /**
     * @brief Add the context menu actions to a QMenu.
     * @sa AreaTagSelectDialog
     * @param menu
     */
    void addTagActions(QMenu *menu);
    void markTidied();
    bool isTidied() const;

    /**
     * @brief associated
     * @return \c true, if a tag is associated with this area, \c false otherwise.
     */
    bool associated() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *) override;
    /**
     * @brief repolish tells the widget to reevaluate its style.
     * This required when the style is dynamically changed because a property changed.
     */
    void repolish();

private slots:
    void associateTag();
    void associateTag(QAction *action);
    void remove();
    void removeTag();

private: // Functions
    void getMinMaxCoordinates();
    QAction *createAssociateTagAction(
        const QPair<QString, QString> &tag, QString prefix = QString());

private: // Variables
    QPoint m_dragStartPosition;
    QRect m_dragStartGeometry;
    QRect m_minMaxCoordinates;
    QRect m_actualCoordinates;
    QAction *m_removeAct;
    QAction *m_removeTagAct;
    Dialog *m_dialog;
    QPair<QString, QString> m_tagData;
    QPair<QString, QString> m_proposedTagData;
    ImagePreview *m_preview;
    ImagePreviewWidget *m_previewWidget;
    bool m_tidied = false;
};

}

#endif // RESIZABLEFRAME_H

// vi:expandtab:tabstop=4 shiftwidth=4:
