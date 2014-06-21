#ifndef DESCRIPTIONEDIT_H
#define DESCRIPTIONEDIT_H

#include <KTextEdit>

class QKeyEvent;

namespace AnnotationDialog {

class DescriptionEdit : public KTextEdit
{
    Q_OBJECT

public:
    explicit DescriptionEdit(QWidget *parent = 0);
    ~DescriptionEdit();

signals:
    void pageUpDownPressed(QKeyEvent *event);

private:
    void keyPressEvent(QKeyEvent *event);

};

}

#endif // DESCRIPTIONEDIT_H

// vi:expandtab:tabstop=4 shiftwidth=4:
