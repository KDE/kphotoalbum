#include "RangeWidget.h"
#include <qlabel.h>
#include <qcombobox.h>
#include <qgrid.h>

Exif::RangeWidget::RangeWidget( const QString& text, const QString& searchTag, const ValueList& list, QGrid* parent )
    : QObject( parent ),_searchTag ( searchTag ), _list( list )
{
    QLabel* label = new QLabel( text, parent );
    _from = new QComboBox( parent );
    label = new QLabel( QString::fromLatin1( "to" ), parent );
    _to = new QComboBox( parent );

    Q_ASSERT( list.count() > 2 );
    ValueList::ConstIterator it = list.begin();
    _from->insertItem( QString::fromLatin1( "< %1" ).arg( (*it).text ) );

    for( ; it != list.end(); ++it ) {
        _from->insertItem( (*it).text );
    }

    _from->insertItem( QString::fromLatin1( "> %1" ).arg( list.last().text ) );
    slotUpdateTo( 0 );
    _to->setCurrentItem( _to->count()-1);// set range to be min->max

    connect( _from, SIGNAL( activated( int ) ), this, SLOT( slotUpdateTo( int ) ) );
}

void Exif::RangeWidget::slotUpdateTo( int fromIndex )
{
    _to->clear();

    if ( fromIndex == 0 )
        _to->insertItem( QString::fromLatin1( "< %1" ).arg( _list.first().text ) );
    else
        fromIndex--;

    for ( uint i = fromIndex; i < _list.count(); ++i ) {
        _to->insertItem( _list[i].text );
    }
    _to->insertItem( QString::fromLatin1( "> %1" ).arg( _list.last().text ) );
}

Exif::SearchInfo::Range Exif::RangeWidget::range() const
{
    SearchInfo::Range result( _searchTag );
    result.min = _list.first().value;
    result.max = _list.last().value;
    if ( _from->currentItem() == 0 )
        result.isLowerMin = true;
    else if ( _from->currentItem() == _from->count() -1 )
        result.isLowerMax = true;
    else
        result.min = _list[_from->currentItem()-1].value;


    if ( _to->currentItem() == 0 && _from->currentItem() == 0 )
        result.isUpperMin = true;
    else if ( _to->currentItem() == _to->count() -1 )
        result.isUpperMax = true;
    else
        result.max =  _list[_to->currentItem() + _from->currentItem()-1].value;

    return result;
}

#include "RangeWidget.moc"
