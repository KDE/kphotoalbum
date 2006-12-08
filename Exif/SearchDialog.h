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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
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

