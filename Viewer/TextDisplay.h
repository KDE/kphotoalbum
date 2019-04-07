/* Copyright (C) 2007-2010 Jan Kundr�t <jkt@gentoo.org>

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

#ifndef VIEWER_TEXTDISPLAY_H
#define VIEWER_TEXTDISPLAY_H
#include "AbstractDisplay.h"
#include "DB/ImageInfoPtr.h"

class QWidget;
class QLabel;

namespace Viewer
{

class TextDisplay :public Viewer::AbstractDisplay {
Q_OBJECT
public:
    explicit TextDisplay( QWidget* parent );
    bool setImage( DB::ImageInfoPtr info, bool forward ) override;
    void setText( const QString text );

public slots:
    /* zooming doesn't make sense for textual display */
    void zoomIn() override {}
    void zoomOut() override {}
    void zoomFull() override {}
    void zoomPixelForPixel() override {}

private:
    QLabel *m_text;
};

}

#endif /* VIEWER_TEXTDISPLAY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
