/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include "SearchBar.h"
#include <KLocalizedString>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <kactioncollection.h>
#include <kmainwindow.h>
#include <qapplication.h>
#include <qlabel.h>

MainWindow::SearchBar::SearchBar(KMainWindow *parent)
    : KToolBar(parent)
{
    QLabel *label = new QLabel(i18nc("@label:textbox label on the search bar", "Search:") + QString::fromLatin1(" "));
    addWidget(label);

    m_edit = new QLineEdit(this);
    m_edit->setClearButtonEnabled(true);
    label->setBuddy(m_edit);

    addWidget(m_edit);
    connect(m_edit, &QLineEdit::textChanged, this, &SearchBar::textChanged);
    connect(m_edit, &QLineEdit::returnPressed, this, &SearchBar::returnPressed);

    m_edit->installEventFilter(this);
}

bool MainWindow::SearchBar::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down || ke->key() == Qt::Key_Left || ke->key() == Qt::Key_Right || ke->key() == Qt::Key_PageDown || ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_Home || ke->key() == Qt::Key_End) {
            emit keyPressed(ke);
            return true;
        } else if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) {
            // If I don't interpret return and enter here, but simply rely
            // on QLineEdit itself to emit the signal, then  it will
            // propagate to the main window, and from there be delivered to
            // the central widget.
            emit returnPressed();
            return true;
        } else if (ke->key() == Qt::Key_Escape)
            reset();
    }
    return false;
}

void MainWindow::SearchBar::reset()
{
    m_edit->clear();
}

/**
 * This was originally just a call to setEnabled() on the SearchBar itself,
 * but due to a bug in either KDE or Qt, this resulted in the bar never
 * being enabled again after a disable.
 */
void MainWindow::SearchBar::setLineEditEnabled(bool b)
{
    m_edit->setEnabled(b);
    m_edit->setFocus();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
