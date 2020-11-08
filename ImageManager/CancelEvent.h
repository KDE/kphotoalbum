/* SPDX-FileCopyrightText: 2003-2011 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CANCELEVENT_H
#define CANCELEVENT_H

#include <QEvent>

namespace ImageManager
{

class ImageRequest;

const int CANCELEVENTID = 1002;

class CancelEvent : public QEvent
{
public:
    explicit CancelEvent(ImageRequest *request);
    ~CancelEvent() override;
    ImageRequest *request() const;

private:
    ImageRequest *m_request;
};

}
#endif // CANCELEVENT_H
// vi:expandtab:tabstop=4 shiftwidth=4:
