/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWHANDLER_H
#define VIEWHANDLER_H
#include "ImageDisplay.h"

#include <QMouseEvent>
#include <qpoint.h>

class ImageDisplay;
class QRubberBand;

namespace Viewer
{

class ViewHandler : public QObject
{
    Q_OBJECT
public:
    explicit ViewHandler(ImageDisplay *display);
    bool mousePressEvent(QMouseEvent *e, const QPoint &unTranslatedPos, double scaleFactor);
    bool mouseReleaseEvent(QMouseEvent *e, const QPoint &unTranslatedPos, double scaleFactor);
    bool mouseMoveEvent(QMouseEvent *e, const QPoint &unTranslatedPos, double scaleFactor);
    void hideEvent();

private:
    bool m_scale, m_pan;
    QPoint m_start, m_untranslatedStart, m_last;
    double m_errorX, m_errorY;
    QRubberBand *m_rubberBand;
    ImageDisplay *m_display;
};
}

#endif /* VIEWHANDLER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
