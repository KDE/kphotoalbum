#include "Exif/SearchDialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qcheckbox.h>
#include "Exif/Database.h"

using namespace Exif;

QValueList<int> Settings::selected()
{
    QValueList<int> result;
    for( QValueList<IntValueSetting>::Iterator it = begin(); it != end(); ++it ) {
        if ( (*it).cb->isChecked() )
            result.append( (*it).value );
    }
    return result;
}


Exif::SearchDialog::SearchDialog( QWidget* parent, const char* name )
    : KDialogBase( Plain, i18n("EXIF Search"), Cancel | Ok | Help, Ok, parent, name )
{
    QWidget* top = plainPage();
    QVBoxLayout* layout = new QVBoxLayout( top, 10 );

    layout->addWidget( makeISO( top ) );
    layout->addWidget( makeExposureProgram( top ) );
    layout->addWidget( makeOrientation( top ));
    layout->addWidget( makeMeteringMode( top ) );
    layout->addWidget( makeContrast( top ) );
    layout->addWidget( makeSharpness( top ) );
    layout->addWidget( makeSaturation( top ) );
    layout->addWidget( makeExposureTime( top ) );
    layout->addWidget( makeFNumber( top ) );
}

QWidget* Exif::SearchDialog::makeISO( QWidget* parent )
{
    QVGroupBox* box = new QVGroupBox( i18n("ISO"), parent );
    for ( int i = 100; i <= 1600; i*=2 ) {
        _iso.append( IntValueSetting( new QCheckBox( QString::number( i ), box ), i ) );
    }
    return box;
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

QWidget* Exif::SearchDialog::makeExposureTime( QWidget* parent )
{
    QVGroupBox* box = new QVGroupBox( i18n( "Exposure time" ), parent );
    RationalList list = Exif::Database::instance()->rationalValue( QString::fromLatin1( "Exif_Photo_ExposureTime" ) );
    for( RationalList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        new QCheckBox( QString::fromLatin1("%1/%2").arg( (*it).first ).arg( (*it).second ), box );
    }
    return box;
}

QWidget* Exif::SearchDialog::makeFNumber( QWidget* parent )
{
    QVGroupBox* box = new QVGroupBox( i18n( "F Number" ), parent );
    RationalList list = Exif::Database::instance()->rationalValue( QString::fromLatin1( "Exif_Photo_FNumber" ) );
    for( RationalList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        new QCheckBox( QString::fromLatin1("%1/%2").arg( (*it).first ).arg( (*it).second ), box );
    }
    return box;

}

Exif::SearchInfo Exif::SearchDialog::info()
{
    Exif::SearchInfo result;
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_MeteringMode" ), _meteringMode.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_ISOSpeedRatings" ), _iso.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_ExposureTime" ), _exposureProgram.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Image_Orientation" ), _orientation.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_MeteringMode" ), _meteringMode.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_Contrast" ), _contrast.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_Sharpness" ), _sharpness.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_Saturation" ), _saturation.selected() );

    return result;
}

