/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MINIVIEWER_H
#define MINIVIEWER_H

#include <DB/ImageInfoPtr.h>

#include <qdialog.h>
#include <qimage.h>

class QCloseEvent;
class QLabel;

namespace DB
{
class ImageInfo;
}

namespace ImportExport
{

class MiniViewer : public QDialog
{
    Q_OBJECT

public:
    static void show(QImage img, DB::ImageInfoPtr info, QWidget *parent = nullptr);
    void closeEvent(QCloseEvent *event) override;

protected slots:
    void slotClose();

private:
    explicit MiniViewer(QWidget *parent = nullptr);
    static MiniViewer *s_instance;
    QLabel *m_pixmap;
};

}

#endif /* MINIVIEWER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
