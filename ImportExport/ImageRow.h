/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef IMAGEROW_H
#define IMAGEROW_H

#include <DB/ImageInfoPtr.h>

#include <QObject>

class QCheckBox;
namespace ImportExport
{
class ImportDialog;
class KimFileReader;

/**
 * This class represent a single row on the ImageDialog's "select widgets to import" page.
 */
class ImageRow : public QObject
{
    Q_OBJECT
public:
    ImageRow(DB::ImageInfoPtr info, ImportDialog *import, KimFileReader *kimFileReader, QWidget *parent);
    QCheckBox *m_checkbox;
    DB::ImageInfoPtr m_info;
    ImportDialog *m_import;
    KimFileReader *m_kimFileReader;

public slots:
    void showImage();
};

}

#endif /* IMAGEROW_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
