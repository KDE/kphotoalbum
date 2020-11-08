/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "VideoUtil.h"

#include <QFileInfo>
#include <QSet>
#include <QString>

const QSet<QString> &Utilities::supportedVideoExtensions()
{
    static QSet<QString> videoExtensions;
    if (videoExtensions.empty()) {
        videoExtensions.insert(QString::fromLatin1("3gp"));
        videoExtensions.insert(QString::fromLatin1("avi"));
        videoExtensions.insert(QString::fromLatin1("mp4"));
        videoExtensions.insert(QString::fromLatin1("m4v"));
        videoExtensions.insert(QString::fromLatin1("mpeg"));
        videoExtensions.insert(QString::fromLatin1("mpg"));
        videoExtensions.insert(QString::fromLatin1("qt"));
        videoExtensions.insert(QString::fromLatin1("mov"));
        videoExtensions.insert(QString::fromLatin1("moov"));
        videoExtensions.insert(QString::fromLatin1("qtvr"));
        videoExtensions.insert(QString::fromLatin1("rv"));
        videoExtensions.insert(QString::fromLatin1("3g2"));
        videoExtensions.insert(QString::fromLatin1("fli"));
        videoExtensions.insert(QString::fromLatin1("flc"));
        videoExtensions.insert(QString::fromLatin1("mkv"));
        videoExtensions.insert(QString::fromLatin1("mng"));
        videoExtensions.insert(QString::fromLatin1("asf"));
        videoExtensions.insert(QString::fromLatin1("asx"));
        videoExtensions.insert(QString::fromLatin1("wmp"));
        videoExtensions.insert(QString::fromLatin1("wmv"));
        videoExtensions.insert(QString::fromLatin1("ogm"));
        videoExtensions.insert(QString::fromLatin1("rm"));
        videoExtensions.insert(QString::fromLatin1("flv"));
        videoExtensions.insert(QString::fromLatin1("webm"));
        videoExtensions.insert(QString::fromLatin1("mts"));
        videoExtensions.insert(QString::fromLatin1("ogg"));
        videoExtensions.insert(QString::fromLatin1("ogv"));
        videoExtensions.insert(QString::fromLatin1("m2ts"));
    }
    return videoExtensions;
}
bool Utilities::isVideo(const DB::FileName &fileName)
{
    QFileInfo fi(fileName.relative());
    QString ext = fi.suffix().toLower();
    return supportedVideoExtensions().contains(ext);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
