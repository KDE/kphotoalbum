// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CATEGORYIMAGEPOPUP_H
#define CATEGORYIMAGEPOPUP_H

#include <DB/ImageDB.h>

#include <QImage>
#include <QMenu>

namespace MainWindow
{

class CategoryImagePopup : public QMenu
{
    Q_OBJECT

public:
    explicit CategoryImagePopup(QWidget *parent);
    void populate(const QImage &image, const DB::FileName &imageName);

protected Q_SLOTS:
    void slotExecuteService(QAction *);
    void makeCategoryImage();

private:
    QImage m_image;
    DB::ImageInfoPtr m_imageInfo;
};
}

#endif /* CATEGORYIMAGEPOPUP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
