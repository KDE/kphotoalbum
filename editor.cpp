/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
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

#include "editor.h"
#include <qlayout.h>
#include <qtextedit.h>
#include <ktrader.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <klibloader.h>
#include <ktexteditor/editinterface.h>

#include <kparts/componentfactory.h>

Editor::Editor( QWidget* parent, const char* name )
    :QWidget( parent, name )
{
    _layout = new QVBoxLayout( this );
    loadPart();
}

bool Editor::loadPart()
{
    // Don't ask, this is pure magic ;-)
    int errCode;
    _doc = KParts::ComponentFactory::createPartInstanceFromQuery< KTextEditor::Document >( QString::fromLatin1("KTextEditor/Document"), QString::null, this, 0, this, 0,
                                                                                           QStringList(), &errCode );

    if( !_doc ) {
        QString err;
        switch ( errCode ) {
        case KParts::ComponentFactory::ErrNoServiceFound: err = QString::fromLatin1( "NoServiceFound" ); break;
        case KParts::ComponentFactory::ErrServiceProvidesNoLibrary: err = QString::fromLatin1( "ServiceProvidesNoLibrary" ); break;
        case KParts::ComponentFactory::ErrNoLibrary: err = QString::fromLatin1( "NoLibrary" ); break;
        case KParts::ComponentFactory::ErrNoFactory: err = QString::fromLatin1( "NoFactory" ); break;
        case KParts::ComponentFactory::ErrNoComponent: err = QString::fromLatin1( "NoComponent" ); break;
        default: err= QString::fromLatin1( "Unknown" );
        }

        KMessageBox::error(this,i18n("KimDaba cannot start a text editor component.\n"
                                     "Please check your KDE installation\n"
                                     "Error was: %1").arg( err ));
        _doc=0;
        _view=0;
        return false;
    }

    _view = _doc->createView( this, 0 );
    _layout->addWidget(static_cast<QWidget *>(_view));

    return true;
}

QString Editor::text() const
{
    KTextEditor::EditInterface* edit = editInterface( _doc );
    Q_ASSERT( edit );
    return edit->text();
}

void Editor::setText( const QString& txt )
{
    KTextEditor::EditInterface* edit = editInterface( _doc );
    Q_ASSERT( edit );
    edit->setText( txt );
}
