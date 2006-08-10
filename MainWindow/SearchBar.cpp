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
