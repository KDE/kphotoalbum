/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#include <klineedit.h>
#include <kmainwindow.h>
#include <qlabel.h>
#include <QKeyEvent>
#include <QEvent>
#include <klocale.h>
#include <kaction.h>
#include <qapplication.h>
#include <kactioncollection.h>

MainWindow::SearchBar::SearchBar( KMainWindow* parent )
    : KToolBar( parent )
{
    QLabel* label = new QLabel( i18n("Search:") + QString::fromLatin1(" ") );
    addWidget( label );

    _edit = new KLineEdit( this );
    _edit->setClearButtonShown(true);
    label->setBuddy( _edit );

    addWidget( _edit );
    connect( _edit, SIGNAL( textChanged( const QString& ) ), this, SIGNAL( textChanged( const QString& ) ) );
    connect( _edit, SIGNAL( returnPressed() ), this, SIGNAL( returnPressed() ) );

    _edit->installEventFilter( this );
}

bool MainWindow::SearchBar::eventFilter( QObject* , QEvent* e )
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent* ke = static_cast<QKeyEvent*>( e );
        if ( ke->key() == Qt::Key_Up ||
             ke->key() == Qt::Key_Down ||
             ke->key() == Qt::Key_Left ||
             ke->key() == Qt::Key_Right ||
             ke->key() == Qt::Key_PageDown ||
             ke->key() == Qt::Key_PageUp ||
             ke->key() == Qt::Key_Home ||
             ke->key() == Qt::Key_End ) {
            emit keyPressed( ke );
            return true;
        }
        else if ( ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return ) {
            // If I don't interpret return and enter here, but simply rely
            // on QLineEdit itself to emit the signal, then  it will
            // propagate to the main window, and from there be delivered to
            // the central widget.
            emit returnPressed();
            return true;
        }
        else if ( ke->key() == Qt::Key_Escape )
            reset();
    }
    return false;
}

void MainWindow::SearchBar::reset()
{
    _edit->clear();
}

/**
 * This was originally just a call to setEnabled() on the SearchBar itself,
 * but due to a bug in either KDE or Qt, this resulted in the bar never
 * being enabled again after a disable.
 */
void MainWindow::SearchBar::setLineEditEnabled(bool b)
{
    _edit->setEnabled(b);
    _edit->setFocus();
}

#include "SearchBar.moc"
