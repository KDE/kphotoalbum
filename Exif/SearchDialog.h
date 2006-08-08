#ifndef EXIFSEARCHDIALOG_H
#define EXIFSEARCHDIALOG_H

#include <kdialogbase.h>
#include "Exif/SearchInfo.h"
#include "Exif/SearchDialogSettings.h"
#include "Exif/RangeWidget.h"

class QSpinBox;
class QCheckBox;

namespace Exif
{

class SearchDialog : public KDialogBase {
    Q_OBJECT

public:
    SearchDialog( QWidget* parent, const char* name = 0 );
    Exif::SearchInfo info();

protected:
    void makeISO( QGrid* parent );
    QWidget* makeExposureProgram( QWidget* parent );
    QWidget* makeOrientation( QWidget* parent );
    QWidget* makeMeteringMode( QWidget* parent );
    QWidget* makeContrast( QWidget* parent );
    QWidget* makeSharpness( QWidget* parent );
    QWidget* makeSaturation( QWidget* parent );
    void makeExposureTime( QGrid* parent );
    RangeWidget* makeApertureOrFNumber( const QString& text, const QString& key, QGrid* parent );
    QWidget* makeCamera( QWidget* parent );
    QStringList availableCameras();

protected slots:
    void fromFocalLengthChanged( int );
    void toFocalLengthChanged( int );

private:
    Exif::RangeWidget* _iso;
    Exif::RangeWidget* _exposureTime;
    Exif::RangeWidget* _apertureValue;
    Exif::RangeWidget* _fNumber;
    Settings<int> _exposureProgram;
    Settings<int> _orientation;
    Settings<int> _meteringMode;
    Settings<int> _contrast;
    Settings<int> _sharpness;
    Settings<int> _saturation;
    Settings< QPair<QString,QString> > _cameras;
    QSpinBox* _fromFocalLength;
    QSpinBox* _toFocalLength;
};

}

#endif /* EXIFSEARCHDIALOG_H */

