// SPDX-FileCopyrightText: 2007-2022 Jan Kundrát <jkt@gentoo.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

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
    void setText(const QString text);
    bool canRotate() override;

public Q_SLOTS:
    /* zooming doesn't make sense for textual display */
    void zoomIn() override { }
    void zoomOut() override { }
    void zoomFull() override { }
    void zoomPixelForPixel() override { }
    void stop() override { }
    void rotate(const DB::ImageInfoPtr & /*info*/) override { }

protected:
    bool setImageImpl(DB::ImageInfoPtr info, bool forward) override;

private:
    QLabel *m_text;
};
}

#endif /* VIEWER_TEXTDISPLAY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
