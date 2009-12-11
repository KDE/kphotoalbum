/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "SearchDialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <q3vgroupbox.h>
#include <qcheckbox.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Exif/Database.h"
#include <q3grid.h>

#include <qlabel.h>
#include <qspinbox.h>
#include <QScrollArea>

using namespace Exif;

Exif::SearchDialog::SearchDialog( QWidget* parent )
    : KPageDialog( parent )
{
    setWindowTitle( i18n("EXIF Search") );
    setButtons( Cancel | Ok | Help );
    setFaceType( Tabbed );

    QWidget* settings = new QWidget;
    KPageWidgetItem* page = new KPageWidgetItem( settings, i18n("Settings" ) );

    addPage(  page );
    QVBoxLayout* vlay = new QVBoxLayout( settings );

    // Iso, Exposure, Aperture, FNumber
    QHBoxLayout* hlay = new QHBoxLayout;
    vlay->addLayout( hlay );
    Q3Grid* grid = new Q3Grid( 4, settings );
    grid->setSpacing( 6 );
    hlay->addWidget( grid );
    hlay->addStretch( 1 );

    makeISO( grid );
    makeExposureTime( grid );

    grid = new Q3Grid( 4, settings );
    grid->setSpacing( 6 );
    hlay->addWidget( grid );
    hlay->addStretch( 1 );
    _apertureValue = makeApertureOrFNumber( i18n( "Aperture Value" ), QString::fromLatin1( "Exif_Photo_ApertureValue" ), grid );
    _fNumber = makeApertureOrFNumber( i18n( "F Number" ), QString::fromLatin1( "Exif_Photo_FNumber" ), grid );

    // Focal length
    grid = new Q3Grid( 4, settings );
    grid->setSpacing( 6 );
    hlay->addWidget( grid );
    hlay->addStretch( 1 );
    QLabel* label = new QLabel( i18n( "Focal Length" ), grid );
    _fromFocalLength = new QSpinBox( grid );
    _fromFocalLength->setRange( 0, 10000 );
    _fromFocalLength->setSingleStep( 10 );
    label = new QLabel( i18n("to"), grid );
    _toFocalLength = new QSpinBox( grid );
    _toFocalLength->setRange( 0, 10000 );
    _toFocalLength->setSingleStep( 10 );

    _toFocalLength->setValue( 10000 );
    QString suffix = i18nc( "This is millimeter for focal length, like 35mm", "mm" );
    _fromFocalLength->setSuffix( suffix );
    _toFocalLength->setSuffix( suffix );

    connect( _fromFocalLength, SIGNAL( valueChanged( int ) ), this, SLOT( fromFocalLengthChanged( int ) ) );
    connect( _toFocalLength, SIGNAL( valueChanged( int ) ), this, SLOT( toFocalLengthChanged( int ) ) );

    // exposure program and Metring mode
    hlay = new QHBoxLayout;
    vlay->addLayout( hlay );
    hlay->addWidget( makeExposureProgram( settings ) );
    hlay->addWidget( makeMeteringMode( settings ) );

    vlay->addStretch( 1 );

    // ------------------------------------------------------------ Camera
    page = new KPageWidgetItem( makeCamera(), i18n("Camera") );
    addPage( page );

    // ------------------------------------------------------------ Misc
    QWidget* misc = new QWidget;
    addPage( new KPageWidgetItem( misc, i18n("Miscellaneous") ) );
    vlay = new QVBoxLayout( misc );
    vlay->addWidget( makeOrientation( misc ), 1 );

    hlay = new QHBoxLayout;
    vlay->addLayout( hlay );
    hlay->addWidget( makeContrast( misc ) );
    hlay->addWidget( makeSharpness( misc ) );
    hlay->addWidget( makeSaturation( misc ) );
    vlay->addStretch( 1 );
}

void Exif::SearchDialog::makeISO( Q3Grid* parent )
{
    Exif::RangeWidget::ValueList list;
    list << Exif::RangeWidget::Value( 100, QString::fromLatin1("100") )
         << Exif::RangeWidget::Value( 200, QString::fromLatin1("200") )
         << Exif::RangeWidget::Value( 400, QString::fromLatin1("400") )
         << Exif::RangeWidget::Value( 800, QString::fromLatin1("800") )
         << Exif::RangeWidget::Value( 1600, QString::fromLatin1("1600") );

    _iso = new RangeWidget( i18n("Iso setting" ), QString::fromLatin1( "Exif_Photo_ISOSpeedRatings" ), list, parent );
}

void Exif::SearchDialog::makeExposureTime( Q3Grid* parent )
{
    QString secs = i18nc( "Example 1.6 secs (as in seconds)", "secs." );
    Exif::RangeWidget::ValueList list;
    list
        << Exif::RangeWidget::Value( 1.0/4000, QString::fromLatin1( "1/4000" ) )
        << Exif::RangeWidget::Value( 1.0/3200, QString::fromLatin1( "1/3200" ) )
        << Exif::RangeWidget::Value( 1.0/2500,  QString::fromLatin1( "1/2500" ) )
        << Exif::RangeWidget::Value( 1.0/2000, QString::fromLatin1( "1/2000" ) )
        << Exif::RangeWidget::Value( 1.0/1600,  QString::fromLatin1( "1/1600" ) )
        << Exif::RangeWidget::Value( 1.0/1250, QString::fromLatin1( "1/1250" ) )
        << Exif::RangeWidget::Value( 1.0/1000,  QString::fromLatin1( "1/1000" ) )
        << Exif::RangeWidget::Value( 1.0/800, QString::fromLatin1( "1/800" ) )
        << Exif::RangeWidget::Value( 1.0/640, QString::fromLatin1( "1/640" ) )
        << Exif::RangeWidget::Value( 1.0/500, QString::fromLatin1( "1/500" ) )
        << Exif::RangeWidget::Value( 1.0/400, QString::fromLatin1( "1/400" ) )
        << Exif::RangeWidget::Value( 1.0/320, QString::fromLatin1( "1/320" ) )
        << Exif::RangeWidget::Value( 1.0/250, QString::fromLatin1( "1/250" ) )
        << Exif::RangeWidget::Value( 1.0/200, QString::fromLatin1( "1/200" ) )
        << Exif::RangeWidget::Value( 1.0/160, QString::fromLatin1( "1/160" ) )
        << Exif::RangeWidget::Value( 1.0/125, QString::fromLatin1( "1/125" ) )
        << Exif::RangeWidget::Value( 1.0/100, QString::fromLatin1( "1/100" ) )
        << Exif::RangeWidget::Value( 1.0/80, QString::fromLatin1( "1/80" ) )
        << Exif::RangeWidget::Value( 1.0/60, QString::fromLatin1( "1/60" ) )
        << Exif::RangeWidget::Value( 1.0/50, QString::fromLatin1( "1/50" ) )
        << Exif::RangeWidget::Value( 1.0/40, QString::fromLatin1( "1/40" ) )
        << Exif::RangeWidget::Value( 1.0/30, QString::fromLatin1( "1/30" ) )
        << Exif::RangeWidget::Value( 1.0/25, QString::fromLatin1( "1/25" ) )
        << Exif::RangeWidget::Value( 1.0/20, QString::fromLatin1( "1/20" ) )
        << Exif::RangeWidget::Value( 1.0/15, QString::fromLatin1( "1/15" ) )
        << Exif::RangeWidget::Value( 1.0/13, QString::fromLatin1( "1/13" ) )
        << Exif::RangeWidget::Value( 1.0/10, QString::fromLatin1( "1/10" ) )
        << Exif::RangeWidget::Value( 1.0/8, QString::fromLatin1( "1/8" ) )
        << Exif::RangeWidget::Value( 1.0/6, QString::fromLatin1( "1/6" ) )
        << Exif::RangeWidget::Value( 1.0/5, QString::fromLatin1( "1/5" ) )
        << Exif::RangeWidget::Value( 1.0/4, QString::fromLatin1( "1/4" ) )
        << Exif::RangeWidget::Value( 0.3, QString::fromLatin1( "0.3 %1" ).arg( secs ) )
        << Exif::RangeWidget::Value( 0.4, QString::fromLatin1( "0.4 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 0.5, QString::fromLatin1( "0.5 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 0.6, QString::fromLatin1( "0.6 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 0.8, QString::fromLatin1( "0.8 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 1, i18n( "1 second" ) )
        << Exif::RangeWidget::Value( 1.3, QString::fromLatin1( "1.3 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 1.6, QString::fromLatin1( "1.6 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 2, QString::fromLatin1( "2 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 2.5, QString::fromLatin1( "2.5 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 3.2, QString::fromLatin1( "3.2 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 4, QString::fromLatin1( "4 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 5, QString::fromLatin1( "5 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 6, QString::fromLatin1( "6 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 8, QString::fromLatin1( "8 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 10, QString::fromLatin1( "10 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 13, QString::fromLatin1( "13 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 15, QString::fromLatin1( "15 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 20, QString::fromLatin1( "20 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 25, QString::fromLatin1( "25 %1").arg(secs ) )
        << Exif::RangeWidget::Value( 30, QString::fromLatin1( "30 %1").arg(secs ) );

    _exposureTime = new RangeWidget( i18n("Exposure time" ), QString::fromLatin1( "Exif_Photo_ExposureTime" ), list, parent );
}

RangeWidget* Exif::SearchDialog::makeApertureOrFNumber( const QString& text, const QString& key, Q3Grid* parent )
{
    Exif::RangeWidget::ValueList list;
    list
        << Exif::RangeWidget::Value( 1.4, QString::fromLatin1( "1.4" ) )
        << Exif::RangeWidget::Value( 1.8, QString::fromLatin1( "1.8" ) )
        << Exif::RangeWidget::Value( 2.0, QString::fromLatin1( "2.0" ) )
        << Exif::RangeWidget::Value( 2.2, QString::fromLatin1( "2.2" ) )
        << Exif::RangeWidget::Value( 2.5, QString::fromLatin1( "2.5" ) )
        << Exif::RangeWidget::Value( 2.8, QString::fromLatin1( "2.8" ) )
        << Exif::RangeWidget::Value( 3.2, QString::fromLatin1( "3.2" ) )
        << Exif::RangeWidget::Value( 3.5, QString::fromLatin1( "3.5" ) )
        << Exif::RangeWidget::Value( 4.0, QString::fromLatin1( "4.0" ) )
        << Exif::RangeWidget::Value( 4.5, QString::fromLatin1( "4.5" ) )
        << Exif::RangeWidget::Value( 5.0, QString::fromLatin1( "5.0" ) )
        << Exif::RangeWidget::Value( 5.6, QString::fromLatin1( "5.6" ) )
        << Exif::RangeWidget::Value( 6.3, QString::fromLatin1( "6.3" ) )
        << Exif::RangeWidget::Value( 7.1, QString::fromLatin1( "7.1" ) )
        << Exif::RangeWidget::Value( 8.0, QString::fromLatin1( "8.0" ) )
        << Exif::RangeWidget::Value( 9.0, QString::fromLatin1( "9.0" ) )
        << Exif::RangeWidget::Value( 10, QString::fromLatin1( "10" ) )
        << Exif::RangeWidget::Value( 11, QString::fromLatin1( "11" ) )
        << Exif::RangeWidget::Value( 13, QString::fromLatin1( "13" ) )
        << Exif::RangeWidget::Value( 14, QString::fromLatin1( "14" ) )
        << Exif::RangeWidget::Value( 16, QString::fromLatin1( "16" ) )
        << Exif::RangeWidget::Value( 18, QString::fromLatin1( "18" ) )
        << Exif::RangeWidget::Value( 20, QString::fromLatin1( "20" ) )
        << Exif::RangeWidget::Value( 22, QString::fromLatin1( "22" ) )
        << Exif::RangeWidget::Value( 25, QString::fromLatin1( "25" ) )
        << Exif::RangeWidget::Value( 29, QString::fromLatin1( "29" ) )
        << Exif::RangeWidget::Value( 32, QString::fromLatin1( "32" ) )
        << Exif::RangeWidget::Value( 36, QString::fromLatin1( "36" ) )
        << Exif::RangeWidget::Value( 40, QString::fromLatin1( "40" ) )
        << Exif::RangeWidget::Value( 45, QString::fromLatin1( "45" ) );

    return new RangeWidget( text, key, list, parent );
}

QWidget* Exif::SearchDialog::makeExposureProgram( QWidget* parent )
{
    Q3VGroupBox* box = new Q3VGroupBox( i18n( "Exposure Program" ), parent );
    _exposureProgram.append( Setting<int>( new QCheckBox( i18n( "Not defined" ), box ), 0 ) );
    _exposureProgram.append( Setting<int>( new QCheckBox( i18n( "Manual" ), box ), 1 ) );
    _exposureProgram.append( Setting<int>( new QCheckBox( i18n( "Normal program" ), box ), 2 ) );
    _exposureProgram.append( Setting<int>( new QCheckBox( i18n( "Aperture priority" ), box ), 3 ) );
    _exposureProgram.append( Setting<int>( new QCheckBox( i18n( "Shutter priority" ), box ), 4 ) );
    _exposureProgram.append( Setting<int>( new QCheckBox( i18n( "Creative program (biased toward depth of field)" ), box ), 5 ) );
    _exposureProgram.append( Setting<int>( new QCheckBox( i18n( "Action program (biased toward fast shutter speed)" ), box ), 6 ) );
    _exposureProgram.append( Setting<int>( new QCheckBox( i18n( "Portrait mode (for closeup photos with the background out of focus)" ), box ), 7 ) );
    _exposureProgram.append( Setting<int>( new QCheckBox( i18n( "Landscape mode (for landscape photos with the background in focus)" ), box ), 8 ) );
    return box;
}

QWidget* Exif::SearchDialog::makeOrientation( QWidget* parent )
{
    Q3VGroupBox* box = new Q3VGroupBox( i18n( "Orientation" ), parent );
    _orientation.append( Setting<int>( new QCheckBox( i18n( "Not rotated" ), box ), 0) );
    _orientation.append( Setting<int>( new QCheckBox( i18n( "Rotated counterclockwise" ), box ), 6 ) );
    _orientation.append( Setting<int>( new QCheckBox( i18n( "Rotated clockwise" ), box ), 8 ) );
    _orientation.append( Setting<int>( new QCheckBox( i18n( "Rotated 180 degrees" ), box ), 3 ) );
    return box;
}

QWidget* Exif::SearchDialog::makeMeteringMode( QWidget* parent )
{
    Q3VGroupBox* box = new Q3VGroupBox( i18n( "Metering Mode" ), parent );
    _meteringMode.append( Setting<int>( new QCheckBox( i18n( "Unknown" ), box ), 0 ) );
    _meteringMode.append( Setting<int>( new QCheckBox( i18n( "Average" ), box ), 1 ) );
    _meteringMode.append( Setting<int>( new QCheckBox( i18n( "CenterWeightedAverage" ), box ), 2 ) );
    _meteringMode.append( Setting<int>( new QCheckBox( i18n( "Spot" ), box ), 3 ) );
    _meteringMode.append( Setting<int>( new QCheckBox( i18n( "MultiSpot" ), box ), 4 ) );
    _meteringMode.append( Setting<int>( new QCheckBox( i18n( "Pattern" ), box ), 5 ) );
    _meteringMode.append( Setting<int>( new QCheckBox( i18n( "Partial" ), box ), 6 ) );
    _meteringMode.append( Setting<int>( new QCheckBox( i18n( "Other" ), box ), 255 ) );
    return box;
}

QWidget* Exif::SearchDialog::makeContrast( QWidget* parent )
{
    Q3VGroupBox* box = new Q3VGroupBox( i18n( "Contrast" ), parent );
    _contrast.append(Setting<int>( new QCheckBox( i18n( "Normal"), box ), 0 ) );
    _contrast.append(Setting<int>( new QCheckBox( i18n( "Soft"), box ), 1 ) );
    _contrast.append(Setting<int>( new QCheckBox( i18n( "Hard"), box ), 2 ) );
    return box;
}

QWidget* Exif::SearchDialog::makeSharpness( QWidget* parent )
{
    Q3VGroupBox* box = new Q3VGroupBox( i18n( "Sharpness" ), parent );
    _sharpness.append(Setting<int>( new QCheckBox( i18n( "Normal"), box ), 0 ) );
    _sharpness.append(Setting<int>( new QCheckBox( i18n( "Soft"), box ), 1 ) );
    _sharpness.append(Setting<int>( new QCheckBox( i18n( "Hard"), box ), 2 ) );
    return box;
}

QWidget* Exif::SearchDialog::makeSaturation( QWidget* parent )
{
    Q3VGroupBox* box = new Q3VGroupBox( i18n( "Saturation" ), parent );
    _saturation.append(Setting<int>( new QCheckBox( i18n( "Normal"), box ), 0 ) );
    _saturation.append(Setting<int>( new QCheckBox( i18n( "Low"), box ), 1 ) );
    _saturation.append(Setting<int>( new QCheckBox( i18n( "High"), box ), 2 ) );
    return box;
}

QStringList Exif::SearchDialog::availableCameras()
{
    return (QStringList() << QString::fromLatin1("Camera 1") << QString::fromLatin1("Camera 2"));
}

Exif::SearchInfo Exif::SearchDialog::info()
{
    Exif::SearchInfo result;
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_MeteringMode" ), _meteringMode.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_ExposureProgram" ), _exposureProgram.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Image_Orientation" ), _orientation.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_MeteringMode" ), _meteringMode.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_Contrast" ), _contrast.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_Sharpness" ), _sharpness.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_Saturation" ), _saturation.selected() );
    result.addCamera( _cameras.selected() );
    result.addRangeKey( _iso->range() );
    result.addRangeKey( _exposureTime->range() );
    result.addRangeKey( _apertureValue->range() );
    result.addRangeKey( _fNumber->range() );

    SearchInfo::Range focalRange( QString::fromLatin1( "Exif_Photo_FocalLength" ) );
    focalRange.min = _fromFocalLength->value();
    focalRange.max = _toFocalLength->value();
    result.addRangeKey( focalRange );
    return result;
}

QWidget* Exif::SearchDialog::makeCamera()
{
    QScrollArea* view = new QScrollArea;
    view->setWidgetResizable(true);

    QWidget* w = new QWidget;
    view->setWidget( w );
    QVBoxLayout* layout = new QVBoxLayout( w );


    QList< QPair<QString, QString> > cameras = Exif::Database::instance()->cameras();
    qSort( cameras );

    for( QList< QPair<QString,QString> >::ConstIterator cameraIt = cameras.constBegin(); cameraIt != cameras.constEnd(); ++cameraIt ) {
        QCheckBox* cb = new QCheckBox( QString::fromLatin1( "%1 - %2" ).arg( (*cameraIt).first.trimmed() ).arg( (*cameraIt).second.trimmed() ) );
        layout->addWidget( cb );
        _cameras.append( Setting< QPair<QString,QString> >( cb, *cameraIt ) );
    }

    if ( cameras.isEmpty() ) {
        QLabel* label = new QLabel( i18n("No cameras found in the database") );
        layout->addWidget( label );
    }

    return view;
}

void Exif::SearchDialog::fromFocalLengthChanged( int val )
{
    if ( _toFocalLength->value() < val )
        _toFocalLength->setValue( val );
}

void Exif::SearchDialog::toFocalLengthChanged( int val )
{
    if ( _fromFocalLength->value() > val )
        _fromFocalLength->setValue( val );
}


#include "SearchDialog.moc"
