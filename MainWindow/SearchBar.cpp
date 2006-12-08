/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "SearchBar.h"
#include <klineedit.h>
#include <kmainwindow.h>
#include <qlabel.h>
#include <klocale.h>
#include <kaction.h>
#include <qapplication.h>
#include <kactioncollection.h>

MainWindow::SearchBar::SearchBar( KMainWindow* parent, const char* name )
    : KToolBar( parent, DockTop, false, name, true )
{
    KAction *resetQuickSearch = new KAction( i18n( "Reset Quick Search" ),
                                             QApplication::reverseLayout()
                                             ? QString::fromLatin1("clear_left")
                                             : QString::fromLatin1("locationbar_erase"),
                                             0, this,
                                             SLOT( reset() ),
                                             new KActionCollection(this),
                                             "reset_quicksearch" );
  resetQuickSearch->plug( this );

  QLabel* label = new QLabel( i18n("Search:") + QString::fromLatin1(" "), this );
  insertWidget( -1, -1, label );

  _edit = new KLineEdit( this );
  label->setBuddy( _edit );

  insertWidget( -1, -1, _edit );
  connect( _edit, SIGNAL( textChanged( const QString& ) ), this, SIGNAL( textChanged( const QString& ) ) );
  connect( _edit, SIGNAL( returnPressed() ), this, SIGNAL( returnPressed() ) );

  setStretchableWidget( _edit );
  _edit->installEventFilter( this );
}

void MainWindow::SearchBar::reset()
{
    _edit->clear();
}

bool MainWindow::SearchBar::eventFilter( QObject* , QEvent* e )
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent* ke = static_cast<QKeyEvent*>( e );
        if ( ke->key() == Key_Up )
            emit scrollLine( -1 );
        else if ( ke->key() == Key_Down )
            emit scrollLine( 1 );
        else if ( ke->key() == Key_PageUp )
            emit scrollPage( -1 );
        else if ( ke->key() == Key_PageDown )
            emit scrollPage( 1 );
        else
            return false;
        return true;
    }
    return false;
}

#include "SearchBar.moc"
