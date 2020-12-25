/* SPDX-FileCopyrightText: 2007-2010 Jan Kundr �t <jkt@gentoo.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWER_TEXTDISPLAY_H
#define VIEWER_TEXTDISPLAY_H
#include "AbstractDisplay.h"

#include <DB/ImageInfoPtr.h>

class QWidget;
class QLabel;

namespace Viewer
{

class TextDisplay : public Viewer::AbstractDisplay
{
    Q_OBJECT
public:
    explicit TextDisplay(QWidget *parent);
    bool setImage(DB::ImageInfoPtr info, bool forward) override;
    void setText(const QString text);

public slots:
    /* zooming doesn't make sense for textual display */
    void zoomIn() override { }
    void zoomOut() override { }
    void zoomFull() override { }
    void zoomPixelForPixel() override { }

private:
    QLabel *m_text;
};
}

#endif /* VIEWER_TEXTDISPLAY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
