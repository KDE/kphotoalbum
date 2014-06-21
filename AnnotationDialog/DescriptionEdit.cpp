#include "DescriptionEdit.h"
#include <QKeyEvent>
#include <QDebug>

AnnotationDialog::DescriptionEdit::DescriptionEdit(QWidget *parent) : KTextEdit(parent)
{
}

AnnotationDialog::DescriptionEdit::~DescriptionEdit()
{
}

void AnnotationDialog::DescriptionEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_PageUp or event->key() == Qt::Key_PageDown) {
        emit pageUpDownPressed(event);
    } else {
        QTextEdit::keyPressEvent(event);
    }
}

#include "DescriptionEdit.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
