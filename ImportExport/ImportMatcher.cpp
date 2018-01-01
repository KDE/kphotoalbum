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

#include "ImportMatcher.h"
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <QGridLayout>
#include <QVBoxLayout>
#include "ImportSettings.h"
#include <KLocalizedString>
using namespace ImportExport;

ImportMatcher::ImportMatcher( const QString& otherCategory, const QString& myCategory,
                              const QStringList& otherItems, const QStringList& myItems,
                              bool allowNew, QWidget* parent )
    : QScrollArea( parent ), m_otherCategory( otherCategory ), m_myCategory( myCategory )
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
        m_matchers.append( match );
    }
}

CategoryMatch::CategoryMatch( bool allowNew, const QString& kimFileItem, QStringList myItems, QWidget* parent, QGridLayout* grid, int row )
{
    m_checkbox = new QCheckBox( kimFileItem, parent );
    m_text = kimFileItem; // We can't just use QCheckBox::text() as Qt adds accelerators.
    m_checkbox->setChecked( true );
    grid->addWidget( m_checkbox, row, 0 );

    m_combobox = new QComboBox;
    m_combobox->setEditable( allowNew );

    myItems.sort();
    m_combobox->addItems( myItems );
    QObject::connect(m_checkbox, &QCheckBox::toggled, m_combobox, &QComboBox::setEnabled);
    grid->addWidget( m_combobox, row, 1 );

    if ( myItems.contains( kimFileItem ) ) {
        m_combobox->setCurrentIndex( myItems.indexOf(kimFileItem) );
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
            // there was a single substring match
            m_combobox->setCurrentIndex( myItems.indexOf(match) );
        }
        else {
            // Either none or multiple items matches
            if ( allowNew ) {
                m_combobox->addItem(kimFileItem);
                m_combobox->setCurrentIndex( m_combobox->count()-1 );
            }
            else
                m_checkbox->setChecked( false );
        }
        QPalette pal = m_checkbox->palette();
        pal.setColor( QPalette::ButtonText, Qt::red );
        m_checkbox->setPalette( pal );
    }
}

ImportExport::CategoryMatchSetting ImportExport::ImportMatcher::settings()
{
    CategoryMatchSetting res( m_myCategory, m_otherCategory );
    for ( CategoryMatch* match : m_matchers ) {
        if ( match->m_checkbox->isChecked() )
            res.add( match->m_combobox->currentText(),match->m_text );
    }
    return res;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
