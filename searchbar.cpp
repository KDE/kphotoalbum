#include "searchbar.h"
#include <klineedit.h>
#include <kmainwindow.h>
#include <qlabel.h>
#include <klocale.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kaction.h>
#include <qapplication.h>
#include <kstdaction.h>
#include <kactioncollection.h>

SearchBar::SearchBar( KMainWindow* parent, const char* name )
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
  insertWidget( -1, -1, _edit );
  connect( _edit, SIGNAL( textChanged( const QString& ) ), this, SIGNAL( textChanged( const QString& ) ) );
  connect( _edit, SIGNAL( returnPressed() ), this, SIGNAL( returnPressed() ) );

  setStretchableWidget( _edit );
}

void SearchBar::reset()
{
    _edit->clear();
}

#include "searchbar.moc"
