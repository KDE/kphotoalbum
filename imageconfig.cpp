#include "imageconfig.h"
#include "listselect.h"
#include <qspinbox.h>
#include <qcombobox.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include "imagemanager.h"
#include "options.h"
#include "imagepreview.h"

ImageConfig::ImageConfig( QWidget* parent, const char* name )
    : ImageConfigUI( parent, name )
{
    persons->setLabel( "Persons" );
    keywords->setLabel( "Keywords" );
    locations->setLabel( "Locations" );
    items->setLabel( "Items" );
    _optionList.append(persons);
    _optionList.append(keywords);
    _optionList.append(locations);
    _optionList.append(items);

    Options* options = Options::instance();
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->insertStringList( options->optionValue( (*it)->label() ) );
    }
}


void ImageConfig::slotRevert()
{
    if ( _setup == SINGLE )
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

void ImageConfig::slotOK()
{
    if ( _setup == SINGLE )  {
        save();
        for ( uint i = 0; i < _editList.count(); ++i )  {
            *(_origList.at(i)) = _editList[i];
        }
        Options::instance()->save();
    }

    else if ( _setup == MULTIPLE ) {
        for( ImageInfoListIterator it( _origList ); *it; ++it ) {
            ImageInfo* info = *it;
            if ( dayFrom->value() != 0 )
                info->setDayFrom( dayFrom->value() );
            if ( monthFrom->currentText() != "---" )
                info->setMonthFrom( monthFrom->currentItem() );
            if ( yearFrom->value() != 0 )
                info->setYearFrom( yearFrom->value() );

            if ( dayTo->value() != 0 )
                info->setDayTo( dayTo->value() );
            if ( monthTo->currentText() != "---" )
                info->setMonthTo( monthTo->currentItem() );
            if ( yearTo->value() != 0 )
                info->setYearTo( yearTo->value() );

            if ( quality->currentText() != "---" )
                info->setQuality( quality->currentItem() );

            for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
                if ( (*it)->selection().count() != 0 )  {
                if ( (*it)->merge() )
                    info->addOption( (*it)->label(),  (*it)->selection() );
                else
                    info->setOption( (*it)->label(),  (*it)->selection() );
                }
            }

            if ( !label->text().isEmpty() )
                info->setLabel( label->text() );
            if ( !description->text().isEmpty() )
                info->setDescription( description->text() );
        }
        Options::instance()->save();
    }
}

void ImageConfig::load()
{
    ImageInfo& info = _editList[ _current ];
    yearFrom->setValue( info.yearFrom() );
    monthFrom->setCurrentItem( info.monthFrom() );
    dayFrom->setValue( info.dayFrom() );

    yearTo->setValue( info.yearTo() );
    monthTo->setCurrentItem( info.monthTo() );
    dayTo->setValue( info.dayTo() );

    quality->setCurrentItem( info.quality() );
    label->setText( info.label() );
    description->setText( info.description() );

    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setSelection( info.optionValue( (*it)->label() ) );
    }

    nextBut->setEnabled( _current != (int)_origList.count()-1 );
    prevBut->setEnabled( _current != 0 );
    if ( _preloadImageMap.contains( info.fileName() ) )
         preview->setPixmap( _preloadImageMap[ info.fileName() ] );
    else
        preview->setText( "<qt>Loading<br>preview</qt>" );
    preview->setInfo( &info );
}

void ImageConfig::save()
{
    ImageInfo& info = _editList[ _current ];

    info.setYearFrom( yearFrom->value() );
    info.setMonthFrom( monthFrom->currentItem() );
    info.setDayFrom( dayFrom->value() );

    info.setYearTo( yearTo->value() );
    info.setMonthTo( monthTo->currentItem() );
    info.setDayTo( dayTo->value() );

    info.setQuality( quality->currentItem() );
    info.setLabel( label->text() );
    info.setDescription( description->text() );
    QStringList list = persons->selection();
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        info.setOption( (*it)->label(), (*it)->selection() );
    }
}

void ImageConfig::pixmapLoaded( const QString& fileName, int, int, int, const QPixmap& pixmap )
{
    if ( fileName == _origList.at( _current )->fileName() )
        preview->setPixmap( pixmap );
    _preloadImageMap[ fileName ] = pixmap;
}

int ImageConfig::configure( ImageInfoList list, bool oneAtATime )
{
    _origList = list;
    _editList.clear();
    for( QPtrListIterator<ImageInfo> it( list ); *it; ++it ) {
        _editList.append( *(*it) );
    }
    if ( oneAtATime )
        _setup = SINGLE;
    else
        _setup = MULTIPLE;

    if ( oneAtATime )  {
        quality->setCurrentText( "High" );
        _preloadImageMap.clear();
        for( QPtrListIterator<ImageInfo> it( list ); *it; ++it ) {
            ImageManager::instance()->load( (*it)->fileName(), this, (*it)->angle(), 256, 256, false );
        }
        _current = -1;
        slotNext();
    }
    else {
        preview->setText( "<qt>Multiple images being<br>configured at a time!</qt>" );
        dayFrom->setValue( 0 );
        monthFrom->setCurrentText( "---" );
        yearFrom->setValue( 0 );

        dayTo->setValue( 0 );
        monthTo->setCurrentText( "---" );
        yearTo->setValue( 0 );

        quality->setCurrentText( "---" );

        for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
            (*it)->setSelection( QStringList() );
        }

        label->setText( "" );
        description->setText( "" );

        prevBut->setEnabled( false );
        nextBut->setEnabled( false );
    }

    setup();
    return exec();
}

int ImageConfig::search()
{
    _setup = SEARCH;
    setup();
    return exec();
}

void ImageConfig::setup()
{
    ListSelect::Mode mode;
    if ( _setup == SEARCH )  {
        previewFrame->hide();
        okBut->setText( "Search" );
        revertBut->hide();
        mode = ListSelect::SEARCH;
    }
    else {
        previewFrame->show();
        okBut->setText( "OK" );
        revertBut->setEnabled( _setup == SINGLE );
        revertBut->show();
        mode = ListSelect::INPUT;
    }
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setMode( mode );
        (*it)->setShowMergeCheckbox( _setup == MULTIPLE );
    }
}

bool ImageConfig::match( ImageInfo* info )
{
    bool ok = ( info->quality() == quality->currentItem() );
    return ok;
}


