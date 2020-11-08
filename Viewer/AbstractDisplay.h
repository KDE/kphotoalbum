/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACTDISPLAY_H
#define ABSTRACTDISPLAY_H

#include <DB/ImageInfoPtr.h>

#include <qwidget.h>

namespace Viewer
{
class AbstractDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit AbstractDisplay(QWidget *parent);
    virtual bool setImage(DB::ImageInfoPtr info, bool forward) = 0;

public slots:
    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;
    virtual void zoomFull() = 0;
    virtual void zoomPixelForPixel() = 0;

protected:
    DB::ImageInfoPtr m_info;
};

}

#endif /* ABSTRACTDISPLAY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
