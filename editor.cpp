#include "editor.h"
#include <qlayout.h>
#include <qtextedit.h>
#include <ktrader.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <klibloader.h>
#include <ktexteditor/editinterface.h>

Editor::Editor( QWidget* parent, const char* name )
    :QWidget( parent, name )
{
    _layout = new QVBoxLayout( this );
/*    _edit = new QTextEdit( this );
    layout->addWidget( _edit );*/
    loadPart();
}

bool Editor::loadPart()
{
    KTrader::OfferList offers = KTrader::self()->query( "KTextEditor/Document" );
    if( offers.count() < 1 ) {
        KMessageBox::error(this,i18n("KPAlbum cannot start a text editor component.\n"
                                     "Please check your KDE installation."));
        _doc=0;
        _view=0;
        return false;
    }
    KService::Ptr service = *offers.begin();
    KLibFactory *factory = KLibLoader::self()->factory( service->library().latin1() );
    if( !factory ) {
        KMessageBox::error(this,i18n("KPAlbum cannot start a text editor component.\n"
                                     "Please check your KDE installation."));
        _doc=0;
        _view=0;
        return false;
    }
    _doc = static_cast<KTextEditor::Document *>( factory->create( this, 0, "KTextEditor::Document" ) );

    if( !_doc ) {
        KMessageBox::error(this,i18n("KPAlbum cannot start a text editor component.\n"
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

