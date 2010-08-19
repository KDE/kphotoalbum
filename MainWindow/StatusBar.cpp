#include "StatusBar.h"
#include "DB/ImageDB.h"
#include "ImageCounter.h"
#include "Settings/SettingsData.h"
#include <QLabel>
#include "DirtyIndicator.h"
#include <KHBox>
#include <kiconloader.h>

MainWindow::StatusBar::StatusBar()
    : KStatusBar()
{
    setupFixedFonts();
    setupGUI();
}

void MainWindow::StatusBar::setupFixedFonts()
{
    // Avoid flicker in the statusbar when moving over dates from the datebar
    QFont f( font() );
    f.setStyleHint( QFont::TypeWriter );
    f.setFamily( QString::fromLatin1( "courier" ) );
    f.setBold( true );
    setFont( f );
}

void MainWindow::StatusBar::setupGUI()
{
    KHBox* indicators = new KHBox( this );
    _dirtyIndicator = new DirtyIndicator( indicators );
    connect( DB::ImageDB::instance(), SIGNAL( dirty() ), _dirtyIndicator, SLOT( markDirtySlot() ) );


    _lockedIndicator = new QLabel( indicators );

    addPermanentWidget( indicators, 0 );

    _partial = new ImageCounter( this );
    addPermanentWidget( _partial, 0 );

    ImageCounter* total = new ImageCounter( this );
    addPermanentWidget( total, 0 );
    total->setTotal( DB::ImageDB::instance()->totalCount() );
    connect( DB::ImageDB::instance(), SIGNAL( totalChanged( uint ) ), total, SLOT( setTotal( uint ) ) );

    _pathIndicator = new BreadcrumbViewer;
    addWidget( _pathIndicator, 1 );
}

void MainWindow::StatusBar::setLocked( bool locked )
{
    static QPixmap* lockedPix = new QPixmap( SmallIcon( QString::fromLatin1( "object-locked" ) ) );
    _lockedIndicator->setFixedWidth( lockedPix->width() );

    if ( locked )
        _lockedIndicator->setPixmap( *lockedPix );
    else
        _lockedIndicator->setPixmap( QPixmap() );

}
