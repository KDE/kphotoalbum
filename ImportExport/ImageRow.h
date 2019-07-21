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
