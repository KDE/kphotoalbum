#include "RangeWidget.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <math.h> // for NAN

Exif::RangeWidget::RangeWidget( const QString& text, const QString& searchTag, const ValueList& list, QWidget* parent, const char* name )
    : QWidget( parent, name ),_searchTag ( searchTag ), _list( list )
{
    QHBoxLayout* lay = new QHBoxLayout( this, 6 );

    QLabel* label = new QLabel( text, this );
    lay->addWidget( label );

    _from = new QComboBox( this );
    lay->addWidget( _from );

    label = new QLabel( QString::fromLatin1( "to" ), this );
    lay->addWidget( label );

    _to = new QComboBox( this );
    lay->addWidget( _to );
    lay->addStretch(1);

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
