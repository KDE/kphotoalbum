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
    _doc = KParts::ComponentFactory::createPartInstanceFromQuery< KTextEditor::Document >( QString::fromLatin1("KTextEditor/Document"), QString::null, this, 0, this, 0 );

    if( !_doc ) {
        KMessageBox::error(this,i18n("KimDaba cannot start a text editor component.\n"
                                     "Please check your KDE installation."));
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

