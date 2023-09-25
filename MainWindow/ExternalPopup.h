// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2020-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXTERNALPOPUP_H
#define EXTERNALPOPUP_H
#include <DB/ImageInfoList.h>
#include <kpabase/FileNameList.h>
#include <kpabase/StringSet.h>

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

class ExternalPopup : public QMenu
{
    Q_OBJECT

public:
    explicit ExternalPopup(QWidget *parent);
    void populate(DB::ImageInfoPtr current, const DB::FileNameList &list);

protected:
    QString mimeType(const DB::FileName &file);
    Utilities::StringSet mimeTypes(const DB::FileNameList &files);
    /**
     * @brief appInfos gets a list of KService entries for the list of files.
     * The result list only contains entries that are valid for all files and is sorted by preference.
     * Note that "sorted by preference" means sorted by one of the mime types, so if the user has vastly different preferences for different mime types they might still end up with a somewhat random order.
     * @param files
     * @return
     */
    KService::List appInfos(const DB::FileNameList &files);

private:
    enum class PopupAction { OpenCurrent,
                             OpenAllSelected,
                             CopyAndOpenAllSelected };
    void runService(KService::Ptr servicel, QList<QUrl> urls);
    QList<QUrl> relevantUrls(PopupAction which);

    DB::FileNameList m_list;
    DB::ImageInfoPtr m_currentInfo;
    QMap<QString, Utilities::StringSet> m_appToMimeTypeMap;
};
}

bool operator<(const QPair<QString, QPixmap> &a, const QPair<QString, QPixmap> &b);

#endif /* EXTERNALPOPUP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
