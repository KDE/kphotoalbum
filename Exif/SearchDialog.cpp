/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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
#include <KLocalizedString>
#include <qlayout.h>
#include <QGroupBox>
#include <qcheckbox.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Exif/Database.h"
#include <QGridLayout>
#include <qlabel.h>
#include <qspinbox.h>
#include <QScrollArea>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace Exif;

Exif::SearchDialog::SearchDialog( QWidget* parent )
    : KPageDialog( parent )
{
    setWindowTitle( i18nc("@title:window", "Exif Search") );
    setFaceType( Tabbed );

    QWidget* settings = new QWidget;
    KPageWidgetItem* page = new KPageWidgetItem( settings, i18n("Settings" ) );

    addPage(  page );
    QVBoxLayout* vlay = new QVBoxLayout( settings );

    // Iso, Exposure, Aperture, FNumber
    QHBoxLayout* hlay = new QHBoxLayout;
    vlay->addLayout( hlay );
    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setSpacing( 6 );
    hlay->addLayout( gridLayout );
    hlay->addStretch( 1 );

    makeISO( gridLayout );
    makeExposureTime( gridLayout );
    hlay->addSpacing(30);

    gridLayout = new QGridLayout;
    gridLayout->setSpacing( 6 );
    hlay->addLayout( gridLayout );
    hlay->addStretch( 1 );
    m_apertureValue = makeApertureOrFNumber( i18n( "Aperture Value" ), QString::fromLatin1( "Exif_Photo_ApertureValue" ), gridLayout, 0 );
    m_fNumber = makeApertureOrFNumber( i18n( "F Number" ), QString::fromLatin1( "Exif_Photo_FNumber" ), gridLayout, 1 );

    hlay->addSpacing(30);

    // Focal length
    QHBoxLayout* focalLayout = new QHBoxLayout;
    focalLayout->setSpacing( 6 );
    hlay->addLayout( focalLayout );
    hlay->addStretch( 1 );

    QLabel* label = new QLabel( i18n( "Focal Length" ) );
    focalLayout->addWidget(label);

    m_fromFocalLength = new QSpinBox;
    focalLayout->addWidget(m_fromFocalLength);
    m_fromFocalLength->setRange( 0, 10000 );
    m_fromFocalLength->setSingleStep( 10 );

    label = new QLabel( i18nc("As in 'A range from x to y'","to"));
    focalLayout->addWidget(label);

    m_toFocalLength = new QSpinBox;
    focalLayout->addWidget(m_toFocalLength);
    m_toFocalLength->setRange( 0, 10000 );
    m_toFocalLength->setSingleStep( 10 );

    m_toFocalLength->setValue( 10000 );
    QString suffix = i18nc( "This is millimeter for focal length, like 35mm", "mm" );
    m_fromFocalLength->setSuffix( suffix );
    m_toFocalLength->setSuffix( suffix );

    connect( m_fromFocalLength, SIGNAL(valueChanged(int)), this, SLOT(fromFocalLengthChanged(int)) );
    connect( m_toFocalLength, SIGNAL(valueChanged(int)), this, SLOT(toFocalLengthChanged(int)) );

    // exposure program and Metring mode
    hlay = new QHBoxLayout;
    vlay->addLayout( hlay );
    hlay->addWidget( makeExposureProgram( settings ) );
    hlay->addWidget( makeMeteringMode( settings ) );

    vlay->addStretch( 1 );

    // ------------------------------------------------------------ Camera
    page = new KPageWidgetItem( makeCamera(), i18n("Camera") );
    addPage( page );

    // ------------------------------------------------------------ Lens
    page = new KPageWidgetItem( makeLens(), i18n("Lens") );
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

void Exif::SearchDialog::makeISO( QGridLayout* layout )
{
    Exif::RangeWidget::ValueList list;
    list << Exif::RangeWidget::Value( 100, QString::fromLatin1("100") )
         << Exif::RangeWidget::Value( 200, QString::fromLatin1("200") )
         << Exif::RangeWidget::Value( 400, QString::fromLatin1("400") )
         << Exif::RangeWidget::Value( 800, QString::fromLatin1("800") )
         << Exif::RangeWidget::Value( 1600, QString::fromLatin1("1600") )
         << Exif::RangeWidget::Value( 3200, QString::fromLatin1("3200") )
         << Exif::RangeWidget::Value( 6400, QString::fromLatin1("6400") )
         << Exif::RangeWidget::Value( 12800, QString::fromLatin1("12800") )
         << Exif::RangeWidget::Value( 25600, QString::fromLatin1("25600") )
         << Exif::RangeWidget::Value( 51200, QString::fromLatin1("51200") );

    m_iso = new RangeWidget( i18n("Iso setting" ), QString::fromLatin1( "Exif_Photo_ISOSpeedRatings" ), list, layout, 0 );
}

void Exif::SearchDialog::makeExposureTime( QGridLayout* layout )
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

    m_exposureTime = new RangeWidget( i18n("Exposure time" ), QString::fromLatin1( "Exif_Photo_ExposureTime" ), list, layout, 1 );
}

RangeWidget* Exif::SearchDialog::makeApertureOrFNumber( const QString& text, const QString& key, QGridLayout* layout, int row )
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

    return new RangeWidget( text, key, list, layout, row );
}

#define addSetting(settings,text,num)         \
    {                                         \
        QCheckBox* cb = new QCheckBox( i18n( text ), box ); \
        settings.append( Setting<int>( cb, num ) );         \
        layout->addWidget(cb);                              \
    }

QWidget* Exif::SearchDialog::makeExposureProgram( QWidget* parent )
{
    QGroupBox* box = new QGroupBox( i18n( "Exposure Program" ), parent );
    QVBoxLayout* layout = new QVBoxLayout(box);

    addSetting( m_exposureProgram, "Not defined", 0 );
    addSetting( m_exposureProgram, "Manual", 1 );
    addSetting( m_exposureProgram, "Normal program", 2 );
    addSetting( m_exposureProgram, "Aperture priority", 3 );
    addSetting( m_exposureProgram, "Shutter priority", 4 );
    addSetting( m_exposureProgram, "Creative program (biased toward depth of field)", 5 );
    addSetting( m_exposureProgram, "Action program (biased toward fast shutter speed)", 6 );
    addSetting( m_exposureProgram, "Portrait mode (for closeup photos with the background out of focus)", 7 );
    addSetting( m_exposureProgram, "Landscape mode (for landscape photos with the background in focus)", 8 );
    return box;
}

QWidget* Exif::SearchDialog::makeOrientation( QWidget* parent )
{
    QGroupBox* box = new QGroupBox( i18n( "Orientation" ), parent );
    QVBoxLayout* layout = new QVBoxLayout(box);

    addSetting( m_orientation, "Not rotated", 0);
    addSetting( m_orientation, "Rotated counterclockwise", 6 );
    addSetting( m_orientation, "Rotated clockwise", 8 );
    addSetting( m_orientation, "Rotated 180 degrees", 3 );
    return box;
}

QWidget* Exif::SearchDialog::makeMeteringMode( QWidget* parent )
{
    QGroupBox* box = new QGroupBox( i18n( "Metering Mode" ), parent );
    QVBoxLayout* layout = new QVBoxLayout(box);

    addSetting( m_meteringMode, "Unknown", 0 );
    addSetting( m_meteringMode, "Average", 1 );
    addSetting( m_meteringMode, "CenterWeightedAverage", 2 );
    addSetting( m_meteringMode, "Spot", 3 );
    addSetting( m_meteringMode, "MultiSpot", 4 );
    addSetting( m_meteringMode, "Pattern", 5 );
    addSetting( m_meteringMode, "Partial", 6 );
    addSetting( m_meteringMode, "Other", 255 );
    return box;
}

QWidget* Exif::SearchDialog::makeContrast( QWidget* parent )
{
    QGroupBox* box = new QGroupBox( i18n( "Contrast" ), parent );
    QVBoxLayout* layout = new QVBoxLayout(box);

    addSetting( m_contrast, "Normal", 0 );
    addSetting( m_contrast, "Soft", 1 );
    addSetting( m_contrast, "Hard", 2 );
    return box;
}

QWidget* Exif::SearchDialog::makeSharpness( QWidget* parent )
{
    QGroupBox* box = new QGroupBox( i18n( "Sharpness" ), parent );
    QVBoxLayout* layout = new QVBoxLayout(box);

    addSetting( m_sharpness, "Normal", 0 );
    addSetting( m_sharpness, "Soft", 1 );
    addSetting( m_sharpness, "Hard", 2 );
    return box;
}

QWidget* Exif::SearchDialog::makeSaturation( QWidget* parent )
{
    QGroupBox* box = new QGroupBox( i18n( "Saturation" ), parent );
    QVBoxLayout* layout = new QVBoxLayout(box);

    addSetting( m_saturation, "Normal", 0 );
    addSetting( m_saturation, "Low", 1 );
    addSetting( m_saturation, "High", 2 );
    return box;
}

Exif::SearchInfo Exif::SearchDialog::info()
{
    Exif::SearchInfo result;
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_MeteringMode" ), m_meteringMode.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_ExposureProgram" ), m_exposureProgram.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Image_Orientation" ), m_orientation.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_MeteringMode" ), m_meteringMode.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_Contrast" ), m_contrast.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_Sharpness" ), m_sharpness.selected() );
    result.addSearchKey( QString::fromLatin1( "Exif_Photo_Saturation" ), m_saturation.selected() );
    result.addCamera( m_cameras.selected() );
    result.addLens( m_lenses.selected() );
    result.addRangeKey( m_iso->range() );
    result.addRangeKey( m_exposureTime->range() );
    result.addRangeKey( m_apertureValue->range() );
    result.addRangeKey( m_fNumber->range() );

    SearchInfo::Range focalRange( QString::fromLatin1( "Exif_Photo_FocalLength" ) );
    focalRange.min = m_fromFocalLength->value();
    focalRange.max = m_toFocalLength->value();
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
    std::sort(cameras.begin(),cameras.end());

    for( QList< QPair<QString,QString> >::ConstIterator cameraIt = cameras.constBegin(); cameraIt != cameras.constEnd(); ++cameraIt ) {
        QCheckBox* cb = new QCheckBox( QString::fromUtf8( "%1 - %2" ).arg( (*cameraIt).first.trimmed() ).arg( (*cameraIt).second.trimmed() ) );
        layout->addWidget( cb );
        m_cameras.append( Setting< QPair<QString,QString> >( cb, *cameraIt ) );
    }

    if ( cameras.isEmpty() ) {
        QLabel* label = new QLabel( i18n("No cameras found in the database") );
        layout->addWidget( label );
    }

    return view;
}

QWidget* Exif::SearchDialog::makeLens()
{
    QScrollArea* view = new QScrollArea;
    view->setWidgetResizable(true);

    QWidget* w = new QWidget;
    view->setWidget( w );
    QVBoxLayout* layout = new QVBoxLayout( w );


    QList< QString > lenses = Exif::Database::instance()->lenses();
    std::sort(lenses.begin(),lenses.end());

    if ( lenses.isEmpty() ) {
        QLabel* label = new QLabel( i18n("No lenses found in the database") );
        layout->addWidget( label );
    } else {
        // add option "None" first
        lenses.prepend( i18nc("As in No persons, no locations etc.", "None") );

        for( QList< QString >::ConstIterator lensIt = lenses.constBegin(); lensIt != lenses.constEnd(); ++lensIt ) {
            QCheckBox* cb = new QCheckBox( QString::fromUtf8( "%1" ).arg( (*lensIt).trimmed() ) );
            layout->addWidget( cb );
            m_lenses.append( Setting< QString >( cb, *lensIt ) );
        }
    }

    if (Exif::Database::instance()->DBFileVersionGuaranteed() < 3)
    {
        QLabel* label = new QLabel(
                    i18n("Not all images in the database have lens information. "
                         "<note>Recreate the Exif search database to ensure lens data for all images.</note>") );
        layout->addWidget(label);
    }

    return view;
}

void Exif::SearchDialog::fromFocalLengthChanged( int val )
{
    if ( m_toFocalLength->value() < val )
        m_toFocalLength->setValue( val );
}

void Exif::SearchDialog::toFocalLengthChanged( int val )
{
    if ( m_fromFocalLength->value() > val )
        m_fromFocalLength->setValue( val );
}

// vi:expandtab:tabstop=4 shiftwidth=4:
