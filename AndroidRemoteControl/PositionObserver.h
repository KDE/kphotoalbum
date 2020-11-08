/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REMOTECONTROL_POSITIONOBSERVER_H
#define REMOTECONTROL_POSITIONOBSERVER_H

class QQuickView;

namespace RemoteControl
{

class PositionObserver
{
public:
    static void setView(QQuickView *view);

    static void setThumbnailOffset(int index);
    static int thumbnailOffset();

    static void setCategoryIconViewOffset(int index);
    static int categoryIconViewOffset();

    static void setCategoryListViewOffset(int index);
    static int categoryListViewOffset();
};

} // namespace RemoteControl

#endif // REMOTECONTROL_POSITIONOBSERVER_H
