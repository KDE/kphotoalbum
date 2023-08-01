// SPDX-FileCopyrightText: 2014-2022 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
    explicit DescriptionEdit(QWidget *parent = nullptr);
    ~DescriptionEdit() override;

Q_SIGNALS:
    void pageUpDownPressed(QKeyEvent *event);

private:
    void keyPressEvent(QKeyEvent *event) override;
};

}

#endif // DESCRIPTIONEDIT_H

// vi:expandtab:tabstop=4 shiftwidth=4:
