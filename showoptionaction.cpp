/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "showoptionaction.h"
#include "options.h"
#include <klocale.h>

ShowOptionAction::ShowOptionAction( const QString& optionGroup, QObject* parent, const char* name )
    :KToggleAction( parent, name ), _optionGroup( optionGroup )
{
    setText( i18n( "Show %1" ).arg( optionGroup ) );
    connect( this, SIGNAL( toggled(bool) ), this, SLOT( slotToggled( bool ) ) );
    setChecked( Options::instance()->showOption( optionGroup ) );
}

void ShowOptionAction::slotToggled( bool b )
{
    emit toggled( _optionGroup, b );
}

#include "showoptionaction.moc"
