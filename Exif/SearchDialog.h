/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
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
    RangeWidget *makeFNumber(const QString &text, const QString &key, QGridLayout *layout, int row);
    QWidget *makeCamera();
    QWidget *makeLens();

protected slots:
    void fromFocalLengthChanged(int);
    void toFocalLengthChanged(int);

private:
    Exif::RangeWidget *m_iso;
    Exif::RangeWidget *m_exposureTime;
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
