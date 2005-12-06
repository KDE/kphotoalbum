#include "Exif/SearchDialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qcheckbox.h>
#include "Exif/Database.h"
#include <qgrid.h>

using namespace Exif;

Exif::SearchDialog::SearchDialog( QWidget* parent, const char* name )
    : KDialogBase( Plain, i18n("EXIF Search"), Cancel | Ok | Help, Ok, parent, name )
{
    QWidget* top = plainPage();
    QVBoxLayout* vlay = new QVBoxLayout( top, 6 );

    QHBoxLayout* hlay = new QHBoxLayout( vlay, 6 );

    QGrid* grid = new QGrid( 4, top );
    grid->setSpacing( 6 );
    hlay->addWidget( grid );
    makeISO( grid );
    makeExposureTime( grid );
    makeApertureValue( grid );

    hlay->addWidget( makeOrientation( top ), 1 );

    hlay = new QHBoxLayout( vlay, 6 );
    hlay->addWidget( makeExposureProgram( top ) );
    hlay->addWidget( makeMeteringMode( top ) );

    hlay = new QHBoxLayout( vlay, 6 );
    hlay->addWidget( makeContrast( top ) );
    hlay->addWidget( makeSharpness( top ) );
    hlay->addWidget( makeSaturation( top ) );
}

void Exif::SearchDialog::makeISO( QGrid* parent )
{
    Exif::RangeWidget::ValueList list;
    list << Exif::RangeWidget::Value( 100, QString::fromLatin1("100") )
         << Exif::RangeWidget::Value( 200, QString::fromLatin1("200") )
         << Exif::RangeWidget::Value( 400, QString::fromLatin1("400") )
         << Exif::RangeWidget::Value( 800, QString::fromLatin1("800") )
         << Exif::RangeWidget::Value( 1600, QString::fromLatin1("1600") );

    _iso = new RangeWidget( i18n("Iso setting" ), QString::fromLatin1( "Exif_Photo_ISOSpeedRatings" ), list, parent );
}

void Exif::SearchDialog::makeExposureTime( QGrid* parent )
{
    QString secs = i18n( "Example 1.6 secs (as in seconds)", "secs." );
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

void Exif::SearchDialog::makeApertureValue( QGrid* parent )
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

    _apertureValue = new RangeWidget( i18n("F Number" ), QString::fromLatin1( "Exif_Photo_ApertureValue" ), list, parent );
}

QWidget* Exif::SearchDialog::makeExposureProgram( QWidget* parent )
{
    QVGroupBox* box = new QVGroupBox( i18n( "Exposure Program" ), parent );
    _exposureProgram.append( IntValueSetting( new QCheckBox( i18n( "Not defined" ), box ), 0 ) );
    _exposureProgram.append( IntValueSetting( new QCheckBox( i18n( "Manual" ), box ), 1 ) );
    _exposureProgram.append( IntValueSetting( new QCheckBox( i18n( "Normal program" ), box ), 2 ) );
    _exposureProgram.append( IntValueSetting( new QCheckBox( i18n( "Aperture priority" ), box ), 3 ) );
    _exposureProgram.append( IntValueSetting( new QCheckBox( i18n( "Shutter priority" ), box ), 4 ) );
    _exposureProgram.append( IntValueSetting( new QCheckBox( i18n( "Creative program (biased toward depth of field)" ), box ), 5 ) );
    _exposureProgram.append( IntValueSetting( new QCheckBox( i18n( "Action program (biased toward fast shutter speed)" ), box ), 6 ) );
    _exposureProgram.append( IntValueSetting( new QCheckBox( i18n( "Portrait mode (for closeup photos with the background out of focus)" ), box ), 7 ) );
    _exposureProgram.append( IntValueSetting( new QCheckBox( i18n( "Landscape mode (for landscape photos with the background in focus)" ), box ), 8 ) );
    return box;
}

QWidget* Exif::SearchDialog::makeOrientation( QWidget* parent )
{
    QVGroupBox* box = new QVGroupBox( i18n( "Orientation" ), parent );
    _orientation.append( IntValueSetting( new QCheckBox( i18n( "Not rotated" ), box ), 0) );
    _orientation.append( IntValueSetting( new QCheckBox( i18n( "Rotated left" ), box ), 6 ) );
    _orientation.append( IntValueSetting( new QCheckBox( i18n( "Rotated right" ), box ), 8 ) );
    _orientation.append( IntValueSetting( new QCheckBox( i18n( "Rotated 180 degrees" ), box ), 3 ) );
    return box;
}

QWidget* Exif::SearchDialog::makeMeteringMode( QWidget* parent )
{
    QVGroupBox* box = new QVGroupBox( i18n( "Metering Mode" ), parent );
    _meteringMode.append( IntValueSetting( new QCheckBox( i18n( "Unknown" ), box ), 0 ) );
    _meteringMode.append( IntValueSetting( new QCheckBox( i18n( "Average" ), box ), 1 ) );
    _meteringMode.append( IntValueSetting( new QCheckBox( i18n( "CenterWeightedAverage" ), box ), 2 ) );
    _meteringMode.append( IntValueSetting( new QCheckBox( i18n( "Spot" ), box ), 3 ) );
    _meteringMode.append( IntValueSetting( new QCheckBox( i18n( "MultiSpot" ), box ), 4 ) );
    _meteringMode.append( IntValueSetting( new QCheckBox( i18n( "Pattern" ), box ), 5 ) );
    _meteringMode.append( IntValueSetting( new QCheckBox( i18n( "Partial" ), box ), 6 ) );
    _meteringMode.append( IntValueSetting( new QCheckBox( i18n( "Other" ), box ), 255 ) );
    return box;
}

QWidget* Exif::SearchDialog::makeContrast( QWidget* parent )
{
    QVGroupBox* box = new QVGroupBox( i18n( "Contrast" ), parent );
    _contrast.append(IntValueSetting( new QCheckBox( i18n( "Normal"), box ), 0 ) );
    _contrast.append(IntValueSetting( new QCheckBox( i18n( "Soft"), box ), 1 ) );
    _contrast.append(IntValueSetting( new QCheckBox( i18n( "Hard"), box ), 2 ) );
    return box;
}

QWidget* Exif::SearchDialog::makeSharpness( QWidget* parent )
{
    QVGroupBox* box = new QVGroupBox( i18n( "Sharpness" ), parent );
    _sharpness.append(IntValueSetting( new QCheckBox( i18n( "Normal"), box ), 0 ) );
    _sharpness.append(IntValueSetting( new QCheckBox( i18n( "Soft"), box ), 1 ) );
    _sharpness.append(IntValueSetting( new QCheckBox( i18n( "Hard"), box ), 2 ) );
    return box;
}

QWidget* Exif::SearchDialog::makeSaturation( QWidget* parent )
{
    QVGroupBox* box = new QVGroupBox( i18n( "Saturation" ), parent );
    _saturation.append(IntValueSetting( new QCheckBox( i18n( "Normal"), box ), 0 ) );
    _saturation.append(IntValueSetting( new QCheckBox( i18n( "Low"), box ), 1 ) );
    _saturation.append(IntValueSetting( new QCheckBox( i18n( "High"), box ), 2 ) );
    return box;
}

QWidget* Exif::SearchDialog::makeCamera( QWidget* parent )
{
    QVGroupBox* box = new QVGroupBox( i18n( "Saturation" ), parent );
    QStringList cameras = availableCameras();
    for( QStringList::Iterator cameraIt = cameras.begin(); cameraIt != cameras.end(); ++cameraIt ) {
        _cameras.append( IntValueSetting( new QCheckBox( *cameraIt, box ), -1 ) );
    }
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
    result.addRangeKey( _iso->range() );
    result.addRangeKey( _exposureTime->range() );
    result.addRangeKey( _apertureValue->range() );
    return result;
}

