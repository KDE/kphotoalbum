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

#ifndef PROPOSEDFACEDIALOG_H
#define PROPOSEDFACEDIALOG_H

// Qt includes
#include <QDialog>

// Local includes
#include "ResizableFrame.h"

// Qt classes
class QEvent;

namespace AnnotationDialog {

class ProposedFaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProposedFaceDialog(QWidget* parent = 0);
    ~ProposedFaceDialog();

public slots:
    void checkUnderMouse();

protected:
    void leaveEvent(QEvent*);
    void paintEvent(QPaintEvent*);

private slots:
    void acceptTag();
    void declineTag();

private: // Functions
    void removeMe();

private: // Variables
    ResizableFrame* m_area;
};

}

#endif // PROPOSEDFACEDIALOG_H

// vi:expandtab:tabstop=4 shiftwidth=4:
