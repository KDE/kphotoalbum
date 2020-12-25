// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXTERNALPOPUP_H
#define EXTERNALPOPUP_H
#include <DB/FileNameList.h>
#include <DB/ImageInfoList.h>
#include <Utilities/StringSet.h>

#include <QMenu>
#include <QPixmap>
#include <qpair.h>

#include <KService>

namespace DB
{
class ImageInfo;
}

namespace MainWindow
{

using Utilities::StringSet;

typedef QSet<KService::Ptr> OfferType;

class ExternalPopup : public QMenu
{
    Q_OBJECT

public:
    explicit ExternalPopup(QWidget *parent);
    void populate(DB::ImageInfoPtr current, const DB::FileNameList &list);

protected:
    QString mimeType(const DB::FileName &file);
    StringSet mimeTypes(const DB::FileNameList &files);
    OfferType appInfos(const DB::FileNameList &files);

private:
    enum class PopupAction { OpenCurrent,
                             OpenAllSelected,
                             CopyAndOpenAllSelected };
    void runService(KService::Ptr servicel, QList<QUrl> urls);
    QList<QUrl> relevantUrls(PopupAction which, KService::Ptr service);

    DB::FileNameList m_list;
    DB::ImageInfoPtr m_currentInfo;
    QMap<QString, StringSet> m_appToMimeTypeMap;
};
}

bool operator<(const QPair<QString, QPixmap> &a, const QPair<QString, QPixmap> &b);

#endif /* EXTERNALPOPUP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
