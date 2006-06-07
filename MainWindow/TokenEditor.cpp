#include "TokenEditor.h"
#include <qlayout.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include "DB/ImageDB.h"
#include "Settings/SettingsData.h"
#include "DB/CategoryCollection.h"
#include "DB/Category.h"
#include "DB/ImageInfo.h"

using namespace MainWindow;

TokenEditor::TokenEditor( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n( "Remove Tokens" ), Cancel | Ok, Ok, parent, name )
{
    QWidget* top = plainPage();
    QVBoxLayout* vlay = new QVBoxLayout( top, 10 );

    QLabel* label = new QLabel( i18n("Select tokens to remove from all images:"), top );
    vlay->addWidget( label );

    QGridLayout* grid = new QGridLayout( vlay, 6, 5, 10 );

    int index = 0;
    for ( int ch = 'A'; ch <= 'Z'; ch++, index++ ) {
        QChar token = QChar( (char) ch );
        QCheckBox* box = new QCheckBox( token, top );
        grid->addWidget( box, index/5, index % 5 );
        _cbs.append( box );
    }

    QHBoxLayout* hlay = new QHBoxLayout( vlay, 10 );
    hlay->addStretch( 1 );
    KPushButton* selectAll = new KPushButton( i18n("Select All"), top );
    KPushButton* selectNone = new KPushButton( i18n("Select None"), top );
    hlay->addWidget( selectAll );
    hlay->addWidget( selectNone );

    connect( selectAll, SIGNAL( clicked() ), this, SLOT( selectAll() ) );
    connect( selectNone, SIGNAL( clicked() ), this, SLOT( selectNone() ) );
}

void TokenEditor::show()
{
    QStringList tokens = tokensInUse();

    for( QValueList<QCheckBox*>::Iterator it = _cbs.begin(); it != _cbs.end(); ++it ) {
        (*it)->setChecked( false );
        (*it)->setEnabled( tokens.contains( (*it)->text() ) );
    }
    KDialogBase::show();
}

void TokenEditor::selectAll()
{
    for( QValueList<QCheckBox*>::Iterator it = _cbs.begin(); it != _cbs.end(); ++it ) {
        (*it)->setChecked( true );
    }
}

void TokenEditor::selectNone()
{
    for( QValueList<QCheckBox*>::Iterator it = _cbs.begin(); it != _cbs.end(); ++it ) {
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
    QMap<QString,int> map =
        DB::ImageDB::instance()->classify( DB::ImageSearchInfo(), QString::fromLatin1( "Tokens" ), DB::Image | DB::Movie );
    for( QMap<QString,int>::Iterator it = map.begin(); it != map.end(); ++it ) {
        if ( it.data() > 0 )
            res.append( it.key() );
    }
    return res;
}

void TokenEditor::slotOk()
{
    for( QValueList<QCheckBox*>::Iterator it = _cbs.begin(); it != _cbs.end(); ++it ) {
        if ( (*it)->isChecked() && (*it)->isEnabled() ) {
            QString txt = (*it)->text().remove( QString::fromLatin1("&") );
            DB::ImageDB::instance()->categoryCollection()->categoryForName( QString::fromLatin1( "Tokens" ) )->removeItem( txt );
        }
    }
    KDialogBase::slotOk();
}

#include "TokenEditor.moc"
