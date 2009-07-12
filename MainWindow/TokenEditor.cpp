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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "TokenEditor.h"
#include <qlayout.h>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QList>
#include <klocale.h>
#include <kpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include "DB/ImageDB.h"
#include "DB/CategoryCollection.h"
#include "DB/Category.h"
#include "DB/ImageSearchInfo.h"

using namespace MainWindow;

TokenEditor::TokenEditor( QWidget* parent )
    :KDialog( parent )
{
    setWindowTitle( i18n( "Remove Tokens" ) );
    setButtons( Cancel | Ok );

    QWidget* top = new QWidget;
    QVBoxLayout* vlay = new QVBoxLayout( top );
    setMainWidget( top );

    QLabel* label = new QLabel( i18n("Select tokens to remove from all images and videos:") );
    vlay->addWidget( label );

    QGridLayout* grid = new QGridLayout;
    vlay->addLayout( grid );

    int index = 0;
    for ( int ch = 'A'; ch <= 'Z'; ch++, index++ ) {
        QChar token = QChar::fromLatin1( (char) ch );
        QCheckBox* box = new QCheckBox( token );
        grid->addWidget( box, index/5, index % 5 );
        _cbs.append( box );
    }

    QHBoxLayout* hlay = new QHBoxLayout;
    vlay->addLayout( hlay );

    hlay->addStretch( 1 );
    KPushButton* selectAll = new KPushButton( i18n("Select All") );
    KPushButton* selectNone = new KPushButton( i18n("Select None") );
    hlay->addWidget( selectAll );
    hlay->addWidget( selectNone );

    connect( selectAll, SIGNAL( clicked() ), this, SLOT( selectAll() ) );
    connect( selectNone, SIGNAL( clicked() ), this, SLOT( selectNone() ) );
}

void TokenEditor::show()
{
    QStringList tokens = tokensInUse();

     for( QList<QCheckBox*>::Iterator it = _cbs.begin(); it != _cbs.end(); ++it ) {
        (*it)->setChecked( false );
        QString txt = (*it)->text().remove( QString::fromLatin1("&") );
        (*it)->setEnabled( tokens.contains( txt ) );
    }
    KDialog::show();
}

void TokenEditor::selectAll()
{
     for( QList<QCheckBox*>::Iterator it = _cbs.begin(); it != _cbs.end(); ++it ) {
        (*it)->setChecked( true );
    }
}

void TokenEditor::selectNone()
{
     for( QList<QCheckBox*>::Iterator it = _cbs.begin(); it != _cbs.end(); ++it ) {
        (*it)->setChecked( false );
    }
}


/**
   I would love to use Settings::optionValue, but that method does not
    forget about an item once it has seen it, which is really what it should
    do anyway, otherwise it would be way to expensive in use.
*/
QStringList TokenEditor::tokensInUse()
{
    QStringList res;
    QMap<QString,uint> map =
        DB::ImageDB::instance()->classify( DB::ImageSearchInfo(), QString::fromLatin1( "Tokens" ), DB::anyMediaType );
    for( QMap<QString,uint>::Iterator it = map.begin(); it != map.end(); ++it ) {
        if ( it.value() > 0 )
            res.append( it.key() );
    }
    return res;
}

void TokenEditor::accept()
{
     for( QList<QCheckBox*>::Iterator it = _cbs.begin(); it != _cbs.end(); ++it ) {
        if ( (*it)->isChecked() && (*it)->isEnabled() ) {
            QString txt = (*it)->text().remove( QString::fromLatin1("&") );
            DB::ImageDB::instance()->categoryCollection()->categoryForName( QString::fromLatin1( "Tokens" ) )->removeItem( txt );
        }
    }
    KDialog::accept();
}

#include "TokenEditor.moc"
