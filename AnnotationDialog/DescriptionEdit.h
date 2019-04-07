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
    ~DescriptionEdit() override;

signals:
    void pageUpDownPressed(QKeyEvent *event);

private:
    void keyPressEvent(QKeyEvent *event) override;

};

}

#endif // DESCRIPTIONEDIT_H

// vi:expandtab:tabstop=4 shiftwidth=4:
