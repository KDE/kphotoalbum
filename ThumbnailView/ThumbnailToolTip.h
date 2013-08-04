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

#ifndef THUMBNAILTOOLTIP_H
#define THUMBNAILTOOLTIP_H
#include <qtimer.h>
#include <qlabel.h>
#include <QEvent>
#include "ImageManager/ImageClientInterface.h"
#include <DB/FileName.h>
#include "Utilities/ToolTip.h"

namespace DB { class ImageInfo; }

namespace ThumbnailView
{
class ThumbnailWidget;

class ThumbnailToolTip :public Utilities::ToolTip {
    Q_OBJECT

public:
    explicit ThumbnailToolTip( ThumbnailWidget* view );
    virtual void setActive(bool);

private slots:
    void requestToolTip();

private:
    virtual bool eventFilter( QObject*, QEvent* e );
    void placeWindow();

private:
    ThumbnailWidget* _view;
    bool _widthInverse;
    bool _heightInverse;
};

}


#endif /* THUMBNAILTOOLTIP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
