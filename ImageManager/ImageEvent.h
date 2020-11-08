/* SPDX-FileCopyrightText: 2003-2011 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEEVENT_H
#define IMAGEEVENT_H

#include <QEvent>
#include <QImage>

namespace ImageManager
{

class ImageRequest;

class ImageEvent : public QEvent
{
public:
    ImageEvent(ImageRequest *request, const QImage &image);
    ImageRequest *loadInfo();
    QImage image();

private:
    ImageRequest *m_request;
    QImage m_image;
};

const int ImageEventID = 1001;

}
#endif // IMAGEEVENT_H
// vi:expandtab:tabstop=4 shiftwidth=4:
