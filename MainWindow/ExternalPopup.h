/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXTERNALPOPUP_H
#define EXTERNALPOPUP_H
#include <DB/FileNameList.h>
#include <DB/ImageInfoList.h>
#include <Utilities/StringSet.h>

#include <QMenu>
#include <QPixmap>
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
