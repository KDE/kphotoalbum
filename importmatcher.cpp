/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "importmatcher.h"
#include <qcheckbox.h>
#include <qlayout.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include "options.h"
#include <qgrid.h>
#include <qlabel.h>
#include <klocale.h>

ImportMatcher::ImportMatcher( const QString& otherOptionGroup, const QString& myOptionGroup,
                              const QStringList& otherOptionList, const QStringList& myOptionList,
                              bool allowNew, QWidget* parent, const char* name )
    : QScrollView( parent, name ), _otherOptionGroup( otherOptionGroup ), _myOptionGroup( myOptionGroup )
{
    setResizePolicy( AutoOneFit );

    QWidget* top = new QWidget( viewport() );
    QVBoxLayout* layout = new QVBoxLayout( top, 6 );
    QWidget* grid = new QWidget( top, "grid" );
    layout->addWidget( grid );
    layout->addStretch(1);

    QGridLayout* gridLay = new QGridLayout( grid, 2, 1, 0, 6 );
    gridLay->setColStretch( 1, 1 );
    addChild( top );

    QLabel* label = new QLabel( i18n("Key in file"), grid );
    gridLay->addWidget( label, 0,0 );
    QColor col = label->paletteBackgroundColor();
    label->setPaletteBackgroundColor( label->paletteForegroundColor() );
    label->setPaletteForegroundColor( col );
    label->setAlignment( AlignCenter );

    label = new QLabel( i18n("Key in your data base"), grid );
    gridLay->addWidget( label, 0, 1 );
    label->setPaletteBackgroundColor( label->paletteForegroundColor() );
    label->setPaletteForegroundColor( col );
    label->setAlignment( AlignCenter );

    int row = 1;
    for( QStringList::ConstIterator it = otherOptionList.begin(); it != otherOptionList.end(); ++it ) {
        OptionMatch* match = new OptionMatch( allowNew, *it, myOptionList, grid, gridLay, row++ );
        _matchers.append( match );
    }
}

OptionMatch::OptionMatch( bool allowNew, const QString& option, QStringList options, QWidget* parent, QGridLayout* grid, int row )
{
    _checkbox = new QCheckBox( option, parent );
    _text = option; // We can't just use QCheckBox::text() as Qt adds accelerators.
    _checkbox->setChecked( true );
    grid->addWidget( _checkbox, row, 0 );

    _combobox = new QComboBox( allowNew, parent, "combo box" );

    options.sort();
    _combobox->insertStringList( options );
    QObject::connect( _checkbox, SIGNAL( toggled( bool ) ), _combobox, SLOT( setEnabled( bool ) ) );
    grid->addWidget( _combobox, row, 1 );

    if ( options.contains( option ) ) {
        _combobox->setCurrentText( option );
    }
    else {
        QString match = QString::null;
        for( QStringList::ConstIterator it = options.begin(); it != options.end(); ++it ) {
            if ( (*it).contains( option ) || option.contains( *it ) ) {
                if ( match == QString::null )
                    match = *it;
                else {
                    match = QString::null;
                    break;
                }
            }
        }
        if ( match != QString::null ) {
            _combobox->setCurrentText( match );
        }
        else {
            if ( allowNew )
                _combobox->setCurrentText( option );
            else
                _checkbox->setChecked( false );
        }
        _checkbox->setPaletteForegroundColor( Qt::red );
    }
}

#include "importmatcher.moc"
