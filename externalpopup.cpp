#include "externalpopup.h"
#include "imageinfo.h"
#include <ktrader.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <kservice.h>
#include <kurl.h>
#include <krun.h>
#include <klocale.h>

void ExternalPopup::populate( ImageInfo* current, const ImageInfoList& imageList )
{
    _list = imageList;
    _currentInfo = current;
    clear();
    KTrader::OfferList offers = KTrader::self()->query( QString::fromLatin1("image/jpeg"), QString::fromLatin1("Type == 'Application'"));

    QStringList list = QStringList() << i18n("Current Image") << i18n("All Image in Viewer");

    bool first = true;
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QLabel* label = new QLabel( *it, this );
        label->setAlignment( Qt::AlignHCenter );
        label->setLineWidth(2);
        label->setFrameStyle( QFrame::Raised | QFrame::StyledPanel );

        bool multiple = (_list.count() > 1);
        bool enabled = (first && _currentInfo ) || (!first && multiple);

        int id = insertItem( label );
        setItemEnabled( id, enabled );
        for(KTrader::OfferList::Iterator it = offers.begin(); it != offers.end(); ++it) {
            id = insertItem( (*it)->pixmap(KIcon::Toolbar), (*it)->name() );
            setItemEnabled( id, enabled );
        }
        first = false;
    }
}

void ExternalPopup::slotExecuteService( int id )
{
    QString name = text( id );
    KTrader::OfferList offers = KTrader::self()->query( QString::fromLatin1("image/jpeg"), QString::fromLatin1("Type == 'Application' and Name == '%1'").arg(name));
    Q_ASSERT( offers.count() == 1 );
    KService::Ptr ptr = offers.first();
    KURL::List lst;
    if ( (uint) indexOf(id) > count() / 2 ) {
        for( ImageInfoListIterator it( _list ); *it; ++it ) {
            lst.append( QString::fromLatin1("file://%1").arg( (*it)->fileName() ) );
        }
    }
    else {
        lst.append( QString::fromLatin1("file://%1").arg(_currentInfo->fileName()));
    }

    KRun::run(*ptr, lst);
}

ExternalPopup::ExternalPopup( QWidget* parent, const char* name )
    :QPopupMenu( parent, name )
{
    connect( this, SIGNAL( activated( int ) ), this, SLOT( slotExecuteService( int ) ) );
}

#include "externalpopup.moc"
