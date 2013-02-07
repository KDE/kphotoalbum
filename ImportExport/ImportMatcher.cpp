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

#include "ImportMatcher.h"
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <QGridLayout>
#include <QVBoxLayout>
#include "ImportSettings.h"
#include <klocale.h>
using namespace ImportExport;

ImportMatcher::ImportMatcher( const QString& otherCategory, const QString& myCategory,
                              const QStringList& otherItems, const QStringList& myItems,
                              bool allowNew, QWidget* parent )
    : QScrollArea( parent ), _otherCategory( otherCategory ), _myCategory( myCategory )
{
    setWidgetResizable(true);
    QWidget* top = new QWidget( viewport() );
    QVBoxLayout* layout = new QVBoxLayout( top );
    QWidget* grid = new QWidget;
    layout->addWidget( grid );
    layout->addStretch(1);

    QGridLayout* gridLay = new QGridLayout(grid);
    gridLay->setColumnStretch( 1, 1 );
    setWidget( top );

    QLabel* label = new QLabel( i18n("Key in file"), grid );
    gridLay->addWidget( label, 0,0 );

    QPalette pal = label->palette();
    QColor col = pal.color( QPalette::Background);
    label->setAutoFillBackground(true);
    pal.setColor( QPalette::Background, pal.color( QPalette::Foreground ) );
    pal.setColor( QPalette::Foreground, col );
    label->setPalette( pal );
    label->setAlignment( Qt::AlignCenter );

    label = new QLabel( i18n("Key in your database"), grid );
    label->setAutoFillBackground(true);
    gridLay->addWidget( label, 0, 1 );
    label->setPalette( pal );
    label->setAlignment( Qt::AlignCenter );

    int row = 1;
    for( QStringList::ConstIterator it = otherItems.begin(); it != otherItems.end(); ++it ) {
        CategoryMatch* match = new CategoryMatch( allowNew, *it, myItems, grid, gridLay, row++ );
        _matchers.append( match );
    }
}

CategoryMatch::CategoryMatch( bool allowNew, const QString& kimFileItem, QStringList myItems, QWidget* parent, QGridLayout* grid, int row )
{
    _checkbox = new QCheckBox( kimFileItem, parent );
    _text = kimFileItem; // We can't just use QCheckBox::text() as Qt adds accelerators.
    _checkbox->setChecked( true );
    grid->addWidget( _checkbox, row, 0 );

    _combobox = new QComboBox;
    _combobox->setEditable( allowNew );

    myItems.sort();
    _combobox->addItems( myItems );
    QObject::connect( _checkbox, SIGNAL( toggled( bool ) ), _combobox, SLOT( setEnabled( bool ) ) );
    grid->addWidget( _combobox, row, 1 );

    if ( myItems.contains( kimFileItem ) ) {
        _combobox->setCurrentIndex( myItems.indexOf(kimFileItem) );
    }
    else {
        // This item was not in my database
        QString match;
        for( QStringList::ConstIterator it = myItems.constBegin(); it != myItems.constEnd(); ++it ) {
            if ( (*it).contains( kimFileItem ) || kimFileItem.contains( *it ) ) {
                // Either my item was a substring of the kim item or the other way around (Jesper is a substring of Jesper Pedersen)
                if ( match.isEmpty() )
                    match = *it;
                else {
                    match.clear();
                    break;
                }
            }
        }
        if ( ! match.isEmpty() ) {
            // there was a single substring matach
            _combobox->setCurrentIndex( myItems.indexOf(match) );
        }
        else {
            // Either none or multiple items matches
            if ( allowNew ) {
                _combobox->addItem(kimFileItem);
                _combobox->setCurrentIndex( _combobox->count()-1 );
            }
            else
                _checkbox->setChecked( false );
        }
        QPalette pal = _checkbox->palette();
        pal.setColor( QPalette::ButtonText, Qt::red );
        _checkbox->setPalette( pal );
    }
}

ImportExport::CategoryMatchSetting ImportExport::ImportMatcher::settings()
{
    CategoryMatchSetting res( _myCategory, _otherCategory );
    Q_FOREACH( CategoryMatch* match, _matchers ) {
        if ( match->_checkbox->isChecked() )
            res.add( match->_combobox->currentText(),match->_text );
    }
    return res;
}

#include "ImportMatcher.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
