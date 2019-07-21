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

#include "RangeWidget.h"
#include "SearchDialogSettings.h"
#include "SearchInfo.h"

#include <KPageDialog>

class QSpinBox;
class QGridLayout;

namespace Exif
{

class SearchDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent);
    Exif::SearchInfo info();

protected:
    void makeISO(QGridLayout *layout);
    QWidget *makeExposureProgram(QWidget *parent);
    QWidget *makeOrientation(QWidget *parent);
    QWidget *makeMeteringMode(QWidget *parent);
    QWidget *makeContrast(QWidget *parent);
    QWidget *makeSharpness(QWidget *parent);
    QWidget *makeSaturation(QWidget *parent);
    void makeExposureTime(QGridLayout *layout);
    RangeWidget *makeApertureOrFNumber(const QString &text, const QString &key, QGridLayout *layout, int row);
    QWidget *makeCamera();
    QWidget *makeLens();

protected slots:
    void fromFocalLengthChanged(int);
    void toFocalLengthChanged(int);

private:
    Exif::RangeWidget *m_iso;
    Exif::RangeWidget *m_exposureTime;
    Exif::RangeWidget *m_apertureValue;
    Exif::RangeWidget *m_fNumber;
    Settings<int> m_exposureProgram;
    Settings<int> m_orientation;
    Settings<int> m_meteringMode;
    Settings<int> m_contrast;
    Settings<int> m_sharpness;
    Settings<int> m_saturation;
    Settings<Database::Camera> m_cameras;
    Settings<Database::Lens> m_lenses;
    QSpinBox *m_fromFocalLength;
    QSpinBox *m_toFocalLength;
};
}

#endif /* EXIFSEARCHDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
