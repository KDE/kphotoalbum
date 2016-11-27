/* Copyright (C) 2016 Tobias Leupold <tobias.leupold@web.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AREATAGSELECTDIALOG_H
#define AREATAGSELECTDIALOG_H

// Qt includes
#include <QDialog>
#include <QString>

// Qt classes
class QKeyEvent;

namespace AnnotationDialog {

class CompletableLineEdit;
class ResizableFrame;
class ListSelect;

/**
 * @brief The AreaTagSelectDialog is the dialog that pops up when
 * a new area is drawn on the ImagePreview.
 */
class AreaTagSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AreaTagSelectDialog(ResizableFrame* area,
                                 ListSelect* ls,
                                 QPixmap& areaImage);

protected:
    void paintEvent(QPaintEvent*);

private slots:
    void slotSetTag(const QString& tag);
private:
    const QString m_category;
    ResizableFrame *m_area;
};

}

#endif // AREATAGSELECTDIALOG_H

// vi:expandtab:tabstop=4 shiftwidth=4:
