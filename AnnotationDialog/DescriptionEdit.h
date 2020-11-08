/* SPDX-FileCopyrightText: 2014 Tobias Leupold <tobias.leupold@web.de>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DESCRIPTIONEDIT_H
#define DESCRIPTIONEDIT_H

#include <KTextEdit>

class QKeyEvent;

namespace AnnotationDialog
{

class DescriptionEdit : public KTextEdit
{
    Q_OBJECT

public:
    explicit DescriptionEdit(QWidget *parent = 0);
    ~DescriptionEdit() override;

signals:
    void pageUpDownPressed(QKeyEvent *event);

private:
    void keyPressEvent(QKeyEvent *event) override;
};

}

#endif // DESCRIPTIONEDIT_H

// vi:expandtab:tabstop=4 shiftwidth=4:
