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

#ifndef EXTERNALPOPUP_H
#define EXTERNALPOPUP_H
#include "DB/ImageInfoList.h"
#include <DB/FileNameList.h>
#include <QMenu>
#include <QPixmap>
#include <Utilities/StringSet.h>
#include <qpair.h>

namespace DB
{
class ImageInfo;
}

namespace MainWindow
{

using Utilities::StringSet;

typedef QSet<QPair<QString, QString>> OfferType;

class ExternalPopup : public QMenu
{
    Q_OBJECT

public:
    explicit ExternalPopup(QWidget *parent);
    void populate(DB::ImageInfoPtr current, const DB::FileNameList &list);

protected slots:
    void slotExecuteService(QAction *);

protected:
    QString mimeType(const DB::FileName &file);
    StringSet mimeTypes(const DB::FileNameList &files);
    OfferType appInfos(const DB::FileNameList &files);

private:
    DB::FileNameList m_list;
    DB::ImageInfoPtr m_currentInfo;
    QMap<QString, StringSet> m_appToMimeTypeMap;
};
}

bool operator<(const QPair<QString, QPixmap> &a, const QPair<QString, QPixmap> &b);

#endif /* EXTERNALPOPUP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
