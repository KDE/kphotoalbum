#include "datefolder.h"
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qlayout.h>
#include <kdatepicker.h>
#include <qlabel.h>
#include "contentfolder.h"
DateFolder::DateFolder( const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent )
{
    setCount( -1 );
}

QPixmap DateFolder::pixmap()
{
    KIconLoader loader;
    return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "korganizer" ), KIcon::Desktop, 22 );
}

QString DateFolder::text() const
{
    return i18n( "Date" );
}


FolderAction* DateFolder::action( bool /* ctrlDown */ )
{
    DateSearchDialog dialog( _browser );
    if ( dialog.exec() == QDialog::Accepted ) {
        ImageSearchInfo info( _info );
        info.setStartDate( ImageDate( dialog.fromDate() ) );
        info.setEndDate( ImageDate( dialog.toDate() ) );
        return new ContentFolderAction( QString::null, QString::null, info, _browser );
    }
    else
        return 0;
}

DateSearchDialog::DateSearchDialog( QWidget* parent, const char* name )
    :KDialogBase( KDialogBase::Plain, i18n("Date search"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, parent, name ), _toChanged( false )
{
    QWidget* top = plainPage();
    QHBoxLayout* lay1 = new QHBoxLayout( top, 6 );

    QVBoxLayout* lay2 = new QVBoxLayout( lay1, 6 );
    QLabel* label = new QLabel( QString::fromLatin1( "<qt><font size=\"+3\">%1:</font></qt>")
                                .arg(i18n("From") ), top );
    lay2->addWidget( label );
    _from = new KDatePicker( top );

    lay2->addWidget( _from );

    lay1->addSpacing( 100 );

    QVBoxLayout* lay3 = new QVBoxLayout( lay1, 6 );
    label = new QLabel( QString::fromLatin1( "<qt><font size=\"+3\">%1:</font></qt>" )
                        .arg( i18n("To") ), top );
    lay3->addWidget( label );
    _to = new KDatePicker( top );
    lay3->addWidget( _to );

    _from->setDate( QDate( QDate::currentDate().year(), 1, 1 ) );
    _to->setDate( QDate( QDate::currentDate().year()+1, 1, 1 ) );
    connect( _from, SIGNAL( dateChanged( QDate ) ), this, SLOT( fromDateChanged( QDate ) ) );
    connect( _to, SIGNAL( dateChanged( QDate ) ), this, SLOT( toDateChanged() ) );
}

QDate DateSearchDialog::fromDate() const
{
    return _from->date();
}

QDate DateSearchDialog::toDate() const
{
    return _to->date();
}

void DateSearchDialog::fromDateChanged( QDate date )
{
    if ( !_toChanged )
        _to->setDate( date );
}

void DateSearchDialog::toDateChanged()
{
    _toChanged = true;
}

QString DateFolder::countLabel() const
{
    return QString::null;
}

#include "datefolder.moc"
