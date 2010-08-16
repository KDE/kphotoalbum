/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef EXIFSEARCHDIALOG_H
#define EXIFSEARCHDIALOG_H

#include "Exif/SearchInfo.h"
#include "Exif/SearchDialogSettings.h"
#include "Exif/RangeWidget.h"
#include <KPageDialog>

class QSpinBox;

namespace Exif
{

class SearchDialog : public KPageDialog {
    Q_OBJECT

public:
    SearchDialog( QWidget* parent );
    Exif::SearchInfo info();

protected:
    void makeISO( Q3Grid* parent );
    QWidget* makeExposureProgram( QWidget* parent );
    QWidget* makeOrientation( QWidget* parent );
    QWidget* makeMeteringMode( QWidget* parent );
    QWidget* makeContrast( QWidget* parent );
    QWidget* makeSharpness( QWidget* parent );
    QWidget* makeSaturation( QWidget* parent );
    void makeExposureTime( Q3Grid* parent );
    RangeWidget* makeApertureOrFNumber( const QString& text, const QString& key, Q3Grid* parent );
    QWidget* makeCamera();
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
    Settings<Database::Camera> _cameras;
    QSpinBox* _fromFocalLength;
    QSpinBox* _toFocalLength;
};

}

#endif /* EXIFSEARCHDIALOG_H */

