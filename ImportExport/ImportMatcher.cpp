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

#include "ImportMatcher.h"
#include <qcheckbox.h>
#include <qlayout.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <QGridLayout>
#include <QVBoxLayout>
#include <klocale.h>
using namespace ImportExport;

ImportMatcher::ImportMatcher( const QString& otherCategory, const QString& myCategory,
                              const QStringList& otherItems, const QStringList& myItems,
                              bool allowNew, QWidget* parent, const char* name )
    : Q3ScrollView( parent, name ), _otherCategory( otherCategory ), _myCategory( myCategory )
{
    setResizePolicy( AutoOneFit );

    QWidget* top = new QWidget( viewport() );
    QVBoxLayout* layout = new QVBoxLayout( top );
    QWidget* grid = new QWidget;
    layout->addWidget( grid );
    layout->addStretch(1);

    QGridLayout* gridLay = new QGridLayout(grid);
    gridLay->setColumnStretch( 1, 1 );
    addChild( top );

    QLabel* label = new QLabel( i18n("Key in file"), grid );
    gridLay->addWidget( label, 0,0 );

    QPalette pal = label->palette();
    QColor col = pal.color( QPalette::Background);
    pal.setColor( QPalette::Background, pal.color( QPalette::Foreground ) );
    pal.setColor( QPalette::Foreground, col );
    label->setPalette( pal );
    label->setAlignment( Qt::AlignCenter );

    label = new QLabel( i18n("Key in your database"), grid );
    gridLay->addWidget( label, 0, 1 );
    pal.setColor( QPalette::Background, pal.color( QPalette::Foreground ) );
    pal.setColor( QPalette::Foreground, col );
    label->setPalette( pal );
    label->setAlignment( Qt::AlignCenter );

    int row = 1;
    for( QStringList::ConstIterator it = otherItems.begin(); it != otherItems.end(); ++it ) {
        CategoryMatch* match = new CategoryMatch( allowNew, *it, myItems, grid, gridLay, row++ );
        _matchers.append( match );
    }
}

CategoryMatch::CategoryMatch( bool allowNew, const QString& category, QStringList items, QWidget* parent, QGridLayout* grid, int row )
{
    _checkbox = new QCheckBox( category, parent );
    _text = category; // We can't just use QCheckBox::text() as Qt adds accelerators.
    _checkbox->setChecked( true );
    grid->addWidget( _checkbox, row, 0 );

    _combobox = new QComboBox;
    _combobox->setEditable( allowNew );

    items.sort();
    _combobox->addItems( items );
    QObject::connect( _checkbox, SIGNAL( toggled( bool ) ), _combobox, SLOT( setEnabled( bool ) ) );
    grid->addWidget( _combobox, row, 1 );

    if ( items.contains( category ) ) {
        _combobox->setCurrentIndex( items.indexOf(category) );
    }
    else {
        QString match = QString::null;
        for( QStringList::ConstIterator it = items.begin(); it != items.end(); ++it ) {
            if ( (*it).contains( category ) || category.contains( *it ) ) {
                if ( match == QString::null )
                    match = *it;
                else {
                    match = QString::null;
                    break;
                }
            }
        }
        if ( match != QString::null ) {
            _combobox->setCurrentIndex( items.indexOf(match) );
        }
        else {
            if ( allowNew )
                _combobox->setCurrentIndex( items.indexOf(category) );
            else
                _checkbox->setChecked( false );
        }
        QPalette pal = _checkbox->palette();
        pal.setColor( QPalette::Foreground, Qt::red );
        _checkbox->setPalette( pal );
    }
}

#include "ImportMatcher.moc"
