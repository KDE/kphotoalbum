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
    keywords->setLabel( "Keywords" );
    locations->setLabel( "Locations" );
    _optionList.append(persons);
    _optionList.append(keywords);
    _optionList.append(locations);

    MetaInfo* minfo = MetaInfo::instance();
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->insertStringList( minfo->items( (*it)->label() ) );
    }
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
    if ( _oneAtATime )
        save();
    else {
        for( ImageInfoListIterator it( _list ); *it; ++it ) {
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
    }
}

void ImageConfig::load()
{
    ImageInfo* info = _list.at( _current );
    yearFrom->setValue( info->yearFrom() );
    monthFrom->setCurrentItem( info->monthFrom() );
    dayFrom->setValue( info->dayFrom() );

    yearTo->setValue( info->yearTo() );
    monthTo->setCurrentItem( info->monthTo() );
    dayTo->setValue( info->dayTo() );

    quality->setCurrentItem( info->quality() );
    label->setText( info->label() );
    description->setText( info->description() );

    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setSelection( info->optionValue( (*it)->label() ) );
    }

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

    info->setYearFrom( yearFrom->value() );
    info->setMonthFrom( monthFrom->currentItem() );
    info->setDayFrom( dayFrom->value() );

    info->setYearTo( yearTo->value() );
    info->setMonthTo( monthTo->currentItem() );
    info->setDayTo( dayTo->value() );

    info->setQuality( quality->currentItem() );
    info->setLabel( label->text() );
    info->setDescription( description->text() );
    QStringList list = persons->selection();
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        info->setOption( (*it)->label(), (*it)->selection() );
    }
}

void ImageConfig::pixmapLoaded( const QString& fileName, int, int, const QPixmap& pixmap )
{
    if ( fileName == _list.at( _current )->fileName() )
        preview->setPixmap( pixmap );
    _preloadImageMap[ fileName ] = pixmap;
}

int ImageConfig::exec( ImageInfoList list, bool oneAtATime )
{
    _list = list;
    _oneAtATime = oneAtATime;

    quality->clear();
    QStringList qualityItems;
    qualityItems << "Low" << "Medium" << "High";
    if ( !oneAtATime )
        qualityItems << "---";
    quality->insertStringList( qualityItems );

    if ( oneAtATime )  {
        quality->setCurrentText( "High" );
        revert->setText( "Revert edits for this image" );
        _preloadImageMap.clear();
        for( QPtrListIterator<ImageInfo> it( list ); *it; ++it ) {
            ImageManager::instance()->load( (*it)->fileName(), this, 256, 256, false );
        }
        _current = -1;
        slotNext();
    }
    else {
        preview->setText( "<qt>Multiple images being<br>configured at a time!</qt>" );
        revert->setText( "Revert edits" );
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

    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setShowMergeCheckbox( !oneAtATime );
    }

    return QDialog::exec();
}


