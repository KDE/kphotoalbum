#include "metainfo.h"
#include "imageconfig.h"
#include "listselect.h"
#include <qspinbox.h>
#include <qcombobox.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include "imagemanager.h"

ImageConfig::ImageConfig( QWidget* parent, const char* name )
    : ImageConfigUI( parent, name )
{
    persons->setLabel( "Persons" );
    keywords->setLabel("Keywords");

    MetaInfo* minfo = MetaInfo::instance();
    persons->insertStringList( minfo->items( "persons" ) );
    keywords->insertStringList( minfo->items( "keywords" ) );
}


void ImageConfig::slotRevert()
{
    load();
}

void ImageConfig::slotPrev()
{
    save();
    _current--;
    load();
}

void ImageConfig::slotNext()
{
    if ( _current != -1 ) {
        save();
    }
    _current++;
    load();
}

void ImageConfig::slotDone()
{
    save();
}

void ImageConfig::load()
{
    ImageInfo* info = _list.at( _current );
    year->setValue( info->year() );
    month->setCurrentItem( info->month() );
    day->setValue( info->day() );
    hour->setValue( info->hour() );
    minute->setValue( info->minute() );
    quality->setCurrentItem( info->quality() );
    label->setText( info->label() );
    description->setText( info->description() );
    persons->setSelection( info->optionValue( persons->label() ) );
    keywords->setSelection( info->optionValue( keywords->label() ) );

    nextBut->setEnabled( _current != (int)_list.count()-1 );
    prevBut->setEnabled( _current != 0 );
    if ( _preloadImageMap.contains( info->fileName() ) )
         preview->setPixmap( _preloadImageMap[ info->fileName() ] );
    else
        preview->setText( "<qt>Loading<br>preview</qt>" );
}

void ImageConfig::save()
{
    ImageInfo* info = _list.at( _current );
    info->setDate( year->value(),  month->currentItem(), day->value() );
    info->setTime( hour->value(), minute->value() );
    info->setQuality( quality->currentItem() );
    info->setLabel( label->text() );
    info->setDescription( description->text() );
    QStringList list = persons->selection();
    info->setOption( persons->label(),  persons->selection() );
    info->setOption( keywords->label(),  keywords->selection() );
}

void ImageConfig::setImageInfo( ImageInfoList list )
{
    _list = list;
    _current = -1;
    slotNext();
    _preloadImageMap.clear();
    for( QPtrListIterator<ImageInfo> it( list ); *it; ++it ) {
        ImageManager::instance()->load( (*it)->fileName(), this, 256, 256, false );
    }
}

void ImageConfig::pixmapLoaded( const QString& fileName, int, int, const QPixmap& pixmap )
{
    if ( fileName == _list.at( _current )->fileName() )
        preview->setPixmap( pixmap );
    _preloadImageMap[ fileName ] = pixmap;
}


