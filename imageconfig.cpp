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
#include <qregexp.h>
#include <qtabwidget.h>
#include "viewer.h"
#include <qaccel.h>
#include <kstandarddirs.h>
#include "editor.h"
#include <klocale.h>

ImageConfig::ImageConfig( QWidget* parent, const char* name )
    : ImageConfigUI( parent, name )
{
    // PENDING(blackie) Once this is rewritten to KDialogBase, do delayed initialization,
    // we need to create this instance at startup, see comment in mainview.cpp constructor.
    persons->setLabel( i18n("Persons") );
    keywords->setLabel( i18n("Keywords") );
    locations->setLabel( i18n("Locations") );
    _optionList.append(persons);
    _optionList.append(keywords);
    _optionList.append(locations);

    Options* opt = Options::instance();
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->insertStringList( opt->optionValue( (*it)->label() ) );
        connect( *it, SIGNAL( deleteOption( const QString&, const QString& ) ),
                 this, SIGNAL( deleteOption( const QString&, const QString& ) ) );
        connect( *it, SIGNAL( renameOption( const QString& , const QString& , const QString&  ) ),
                 this, SIGNAL( renameOption( const QString& , const QString& , const QString&  ) ) );
    }

    // Update tab order.
    // PENDING(blackie) Is this still OK?
    setTabOrder( yearEnd,  persons->firstTabWidget() );
    setTabOrder( persons->lastTabWidget(), locations->firstTabWidget() );

    connect( preview,  SIGNAL( doubleClicked() ),  this,  SLOT( displayImage() ) );

    // Connect PageUp/PageDown to prev/next
    QAccel* accel = new QAccel( this, "accel for ImageConfig" );
    accel->connectItem( accel->insertItem( Key_PageDown ), this, SLOT( slotNext() ) );
    accel->connectItem( accel->insertItem( Key_PageUp ), this, SLOT( slotPrev() ) );
}


void ImageConfig::slotRevert()
{
    if ( _setup == SINGLE )
        load();
}

void ImageConfig::slotPrev()
{
    save();
    if ( _current == 0 )
        return;

    _current--;
    load();
}

void ImageConfig::slotNext()
{
    if ( _current != -1 ) {
        save();
    }
    if ( _current == (int)_origList.count()-1 )
        return;

    _current++;
    load();
}

void ImageConfig::slotOK()
{
    bool change = false;
    if ( _setup == SINGLE )  {
        save();
        for ( uint i = 0; i < _editList.count(); ++i )  {
            change |= (*(_origList.at(i)) != _editList[i]);
            *(_origList.at(i)) = _editList[i];
        }
    }

    else if ( _setup == MULTIPLE ) {
        for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
            (*it)->slotReturn();
        }

        for( ImageInfoListIterator it( _origList ); *it; ++it ) {
            ImageInfo* info = *it;
            if ( dayStart->value() != 0 ||  monthStart->currentText() != "---" || yearStart->value() != 0 )  {
                info->startDate().setDay( dayStart->value() );
                info->startDate().setMonth( monthStart->currentItem() );
                info->startDate().setYear( yearStart->value() );
                change = true;
            }

            if ( dayEnd->value() != 0 || monthEnd->currentText() != "---" || yearEnd->value() != 0 )  {
                info->endDate().setDay( dayEnd->value() );
                info->endDate().setMonth( monthEnd->currentItem() );
                info->endDate().setYear( yearEnd->value() );
                change = true;
            }

            for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
                if ( (*it)->selection().count() != 0 )  {
                    change = true;
                    if ( (*it)->merge() )
                        info->addOption( (*it)->label(),  (*it)->selection() );
                    else
                        info->setOption( (*it)->label(),  (*it)->selection() );
                }
            }

            if ( !label->text().isEmpty() ) {
                change = true;
                info->setLabel( label->text() );
            }

            if ( !description->text().isEmpty() ) {
                change = true;
                info->setDescription( description->text() );
            }
        }
    }
    if ( change )
        emit changed();
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

    label->setText( info.label() );
    description->setText( info.description() );

    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->setSelection( info.optionValue( (*it)->label() ) );
    }

    nextBut->setEnabled( _current != (int)_origList.count()-1 );
    prevBut->setEnabled( _current != 0 );

    preview->setInfo( &info );

    Viewer* viewer = Viewer::instance();
    viewer->load( _origList, _current );
    ImageManager::instance()->load( info.fileName( false ), this, info.angle(), 256, 256, false, true, false );
}

void ImageConfig::save()
{
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        (*it)->slotReturn();
    }

    ImageInfo& info = _editList[ _current ];

    info.startDate().setYear( yearStart->value() );
    info.startDate().setMonth( monthStart->currentItem() );
    info.startDate().setDay( dayStart->value() );

    info.endDate().setYear( yearEnd->value() );
    info.endDate().setMonth( monthEnd->currentItem() );
    info.endDate().setDay( dayEnd->value() );

    info.setLabel( label->text() );
    info.setDescription( description->text() );
    QStringList list = persons->selection();
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        info.setOption( (*it)->label(), (*it)->selection() );
    }
}

void ImageConfig::pixmapLoaded( const QString& fileName, int, int, int, const QImage& image )
{
    if ( fileName == _origList.at( _current )->fileName( false ) ) {
        QPixmap pixmap;
        pixmap.convertFromImage( image );
        preview->setPixmap( pixmap );
    }
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
        // PENDING(blackie) We can't just have a QMap as this fills up memory.
        /*_preloadImageMap.clear();
        for( QPtrListIterator<ImageInfo> it( list ); *it; ++it ) {
             ImageManager::instance()->load( (*it)->fileName( false ), this, (*it)->angle(), 256, 256, false, true );
             }*/
        _current = -1;
        slotNext();
    }
    else {
        dayStart->setValue( 0 );
        monthStart->setCurrentText( "---" );
        yearStart->setValue( 0 );

        dayEnd->setValue( 0 );
        monthEnd->setCurrentText( "---" );
        yearEnd->setValue( 0 );

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
    int ok = exec();
    if ( ok == QDialog::Accepted )  {
        _oldSearch = ImageSearchInfo( ImageDate( dayStart->value(), monthStart->currentItem(), yearStart->value()),
                                      ImageDate( dayEnd->value(), monthEnd->currentItem(), yearEnd->value() ),
                                      persons->text(), locations->text(), keywords->text(),
                                      label->text(), description->text() );
    }
    return ok;
}

void ImageConfig::setup()
{
    ListSelect::Mode mode;
    if ( _setup == SEARCH )  {
        okBut->setText( i18n("&Search") );
        revertBut->hide();
        mode = ListSelect::SEARCH;
        setCaption( i18n("Image Search") );
        loadInfo( _oldSearch );
        preview->setPixmap( locate("data", QString::fromLatin1("kimdaba/pics/search.jpg") ) );
        preview->setInfo(0);
        nextBut->setEnabled( false );
        prevBut->setEnabled( false );
    }
    else {
        okBut->setText( i18n("&OK") );
        revertBut->setEnabled( _setup == SINGLE );
        revertBut->show();
        mode = ListSelect::INPUT;
        setCaption( i18n("Image Configuration") );
        if ( _setup == MULTIPLE ) {
            preview->setPixmap( locate("data", QString::fromLatin1("kimdaba/pics/multiconfig.jpg") ) );
            preview->setInfo(0);
        }


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

    ok &= ( b1 || b2 || b3 );


    // -------------------------------------------------- ListSelect
    for( QPtrListIterator<ListSelect> it( _optionList ); *it; ++it ) {
        ok &= (*it)->matches( info );
    }

    // -------------------------------------------------- Label
    ok &= ( label->text().isEmpty() || info->label().find(label->text()) != -1 );

    // -------------------------------------------------- Text
    QString txt = description->text();
    QStringList list = QStringList::split(QRegExp("\\s"), txt );
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        ok &= ( txt.find(*it) != -1 );
    }

    return ok;
}

void ImageConfig::displayImage()
{
    Viewer* viewer = Viewer::instance();
    viewer->show();
    viewer->load( _origList, _current );
}

void ImageConfig::slotClear()
{
    loadInfo( ImageSearchInfo() );
}

void ImageConfig::loadInfo( const ImageSearchInfo& info )
{
    yearStart->setValue( info.startDate().year() );
    monthStart->setCurrentItem( info.startDate().month() );
    dayStart->setValue( info.startDate().day() );

    yearEnd->setValue( info.endDate().year() );
    monthEnd->setCurrentItem( info.endDate().month() );
    dayEnd->setValue( info.endDate().day() );

    persons->setText( info.persons() );
    locations->setText( info.locations() );
    keywords->setText( info.keywords() );

    label->setText( info.label() );
    description->setText( info.description() );
}

#include "imageconfig.moc"
