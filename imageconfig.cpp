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
            if ( dayStart->value() != 0 )
                info->startDate().setDay( dayStart->value() );
            if ( monthStart->currentText() != "---" )
                info->startDate().setMonth( monthStart->currentItem() );
            if ( yearStart->value() != 0 )
                info->startDate().setYear( yearStart->value() );

            if ( dayEnd->value() != 0 )
                info->endDate().setDay( dayEnd->value() );
            if ( monthEnd->currentText() != "---" )
                info->endDate().setMonth( monthEnd->currentItem() );
            if ( yearEnd->value() != 0 )
                info->endDate().setYear( yearEnd->value() );

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
    yearStart->setValue( info.startDate().year() );
    monthStart->setCurrentItem( info.startDate().month() );
    dayStart->setValue( info.startDate().day() );

    yearEnd->setValue( info.endDate().year() );
    monthEnd->setCurrentItem( info.endDate().month() );
    dayEnd->setValue( info.endDate().day() );

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

    info.startDate().setYear( yearStart->value() );
    info.startDate().setMonth( monthStart->currentItem() );
    info.startDate().setDay( dayStart->value() );

    info.endDate().setYear( yearEnd->value() );
    info.endDate().setMonth( monthEnd->currentItem() );
    info.endDate().setDay( dayEnd->value() );

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
        dayStart->setValue( 0 );
        monthStart->setCurrentText( "---" );
        yearStart->setValue( 0 );

        dayEnd->setValue( 0 );
        monthEnd->setCurrentText( "---" );
        yearEnd->setValue( 0 );

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
        qualityTo->resize( 100, 100);

        qualityToLabel->show();
        qualityTo->show();
        qualityTo->updateGeometry();
    }
    else {
        previewFrame->show();
        okBut->setText( "OK" );
        revertBut->setEnabled( _setup == SINGLE );
        revertBut->show();
        mode = ListSelect::INPUT;
        qualityToLabel->hide();
        qualityTo->hide();
    }
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setMode( mode );
        (*it)->setShowMergeCheckbox( _setup == MULTIPLE );
    }
}

bool ImageConfig::match( ImageInfo* info )
{
    bool ok = true;

    // Date
    // the search date matches the actual date if:
    // actual.start <= search.start <= actuel.end or
    // actual.start <= search.end <=actuel.end or
    // search.start <= actual.start and actual.end <= search.end
    ImageDate searchStart = ImageDate( dayStart->value(),  monthStart->currentItem(), yearStart->value() );
    ImageDate searchEnd = ImageDate( dayEnd->value(),  monthEnd->currentItem(), yearEnd->value() );
    ImageDate tmp;
    if ( !searchEnd.isNull() && searchEnd <= searchStart )  {
        tmp = searchEnd;
        searchEnd = searchStart;
        searchStart = tmp;
    }

    if ( searchEnd.isNull() )
        searchEnd = searchStart;

    ImageDate actualStart = info->startDate();
    ImageDate actualEnd = info->endDate();
    if ( !actualEnd.isNull() && actualEnd <= actualStart )  {
        tmp = actualStart;
        actualStart = actualEnd;
        actualEnd = tmp;
    }
    if ( actualEnd.isNull() )
        actualEnd = actualStart;

    bool b1 =( actualStart <= searchStart && searchStart <= actualEnd );
    bool b2 =( actualStart <= searchEnd && searchEnd <= actualEnd );
    bool b3 = ( searchStart <= actualStart && actualEnd <= searchEnd );
    bool b4 = actualStart <= searchEnd;
    bool b5 = searchEnd <= actualEnd;

    qDebug( QString("%1,%2,%3,%4,%5,%6,%7").arg(searchStart).arg(searchEnd).arg(actualStart).arg(actualEnd).arg(b4).arg(b5).arg(b3).latin1());


    ok &= ( b1 || b2 || b3 );


    // -------------------------------------------------- Quality
    int v1 = quality->currentItem();
    int v2 = qualityTo->currentItem();
    if ( v1 != 0 || v2 != 0 )  {
        int min, max;
        if ( v1 == 0 )  {
            min = v2;
            max = v2;
        }
        else if ( v2 == 0 )  {
            min = v1;
            max = v1;
        }
        else {
            min = QMIN( v1, v2 );
            max = QMAX( v1, v2 );
        }

        ok &= info->quality() >= min;
        ok &= info->quality() <= max;
    }

    return ok;
}
