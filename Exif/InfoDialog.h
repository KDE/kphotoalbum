/* SPDX-FileCopyrightText: 2003-2016 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <ImageManager/ImageClientInterface.h>
#include <kpabase/FileName.h>

#include <QDialog>

class QComboBox;
class QLineEdit;
class QLabel;
class QKeyEvent;
class QResizeEvent;

namespace DB
{

class Id;

}

namespace Exif
{

class Grid;
class MetaDataDisplay;

class InfoDialog : public QDialog, public ImageManager::ImageClientInterface
{
    Q_OBJECT

public:
    InfoDialog(const DB::FileName &fileName, QWidget *parent);
    void setImage(const DB::FileName &fileName);

    QSize sizeHint() const override;
    void enterEvent(QEvent *) override;

    // ImageManager::ImageClient interface.
    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;

private:
    QLineEdit *m_searchBox;
    QLabel *m_pix;
    QComboBox *m_iptcCharset;
    Grid *m_grid;
    QLabel *m_fileNameLabel;
    MetaDataDisplay *m_metaDataDisplay;

};

}

#endif /* INFODIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
