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
    QPoint _dragStartPosition;
    QRect _dragStartGeometry;
    QRect _minMaxCoordinates;
    QRect _actualCoordinates;
    void getMinMaxCoordinates();
    QAction *_lastTagAct;
    QAction *_removeAct;
    QAction *_removeTagAct;
    Dialog *_dialog;
    QPair<QString, QString> _tagData;
};

}

#endif // RESIZABLEFRAME_H
// vi:expandtab:tabstop=4 shiftwidth=4:
