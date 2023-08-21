// SPDX-FileCopyrightText: 2003 - 2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef ABSTRACTDISPLAY_H
#define ABSTRACTDISPLAY_H

#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>
#include <qwidget.h>

namespace Viewer
{
class AbstractDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit AbstractDisplay(QWidget *parent);
    bool setImage(DB::ImageInfoPtr info, bool forward);

public Q_SLOTS:
    virtual void zoomIn() {};
    virtual void zoomOut() {};
    virtual void zoomFull() {};
    virtual void zoomPixelForPixel() {};
    virtual void stop() = 0;
    virtual void rotate(const DB::ImageInfoPtr &info) = 0;

protected:
    virtual bool setImageImpl(DB::ImageInfoPtr info, bool forward) = 0;
    DB::ImageInfoPtr m_info; ///< The pointer to the currently displayed image (may be null)
};

}

#endif /* ABSTRACTDISPLAY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
