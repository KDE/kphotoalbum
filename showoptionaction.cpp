#include "showoptionaction.h"
#include "options.h"
#include <klocale.h>

ShowOptionAction::ShowOptionAction( const QString& optionGroup, QObject* parent, const char* name )
    :QAction( parent, name ), _optionGroup( optionGroup )
{
    setText( i18n( "Show %1" ).arg( optionGroup ) );
    connect( this, SIGNAL( toggled(bool) ), this, SLOT( slotToggled( bool ) ) );
    setToggleAction( true );
    setOn( Options::instance()->showOption( optionGroup ) );
}

void ShowOptionAction::slotToggled( bool b )
{
    emit toggled( _optionGroup, b );
}

#include "showoptionaction.moc"
