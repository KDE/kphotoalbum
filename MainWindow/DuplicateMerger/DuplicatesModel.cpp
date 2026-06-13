// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "DuplicatesModel.h"

#include <DB/ImageDB.h>
#include <DB/MD5.h>
#include <ImageManager/AsyncLoader.h>
#include <kpabase/FileNameList.h>
#include <kpabase/Logging.h>

#include <KLocalizedString>
#include <QDebug>

#include <utility>

// Width and height of the thumbnail preview pixmaps.
const auto THUMBNAIL_HEIGHT = 100;
const auto THUMBNAIL_WIDTH = 100;

namespace MainWindow
{
DuplicatesModel::DuplicatesModel(const DB::DuplicatesType& duplicates, QObject* parent)
    : QAbstractTableModel(parent)
    , m_maxDuplicates(0)
    , m_thumbnailSize(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT)
{
    // This is used to sort the rows in the duplicates table by relative
    // pathname of the oldest image in each set of duplicates.
    QMap<QString, DB::MD5> displayOrderMap;

    for (QMap<DB::MD5, DB::FileNameList>::const_iterator it = duplicates.constBegin();
         it != duplicates.constEnd(); ++it) {
        if (it.value().count() > 1) {
            displayOrderMap.insert(it.value().first().relative(), it.key());
        }
    }

    for (DB::MD5 md5 : displayOrderMap.values()) {
        addDuplicates(duplicates[md5]);
    }
}

DuplicatesModel::~DuplicatesModel()
{
}

void DuplicatesModel::addDuplicates(const DB::FileNameList &files)
{
    m_files << files;
    if (files.size() > m_maxDuplicates) {
        m_maxDuplicates = files.size();
    }

    const auto &originalFileName = files.first();
    const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(originalFileName);
    const int angle = info->angle();
    ImageManager::ImageRequest *request = new ImageManager::ImageRequest(originalFileName, m_thumbnailSize, angle, this);
    ImageManager::AsyncLoader::instance()->load(request);
}

void DuplicatesModel::pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image)
{
    m_pixmaps[request->databaseFileName().relative()] = QPixmap::fromImage(image);
    qCDebug(ImageManagerLog) << "Loaded pixmap for" << request->databaseFileName().relative();
}

QVariant DuplicatesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int row = index.row();
    const int column = index.column();

    if (row < m_files.count() && column < columnCount()) {
        if (role == Qt::DisplayRole) {
            // Column zero is the pixmap.
            if (column > 0) {
                if (column <= m_files[row].size()) {
                    return m_files[row][column-1].relative();
                }
            }
        }
        else if (role == Qt::DecorationRole) {
            if (column == 0) {
                const auto &originalFileName = m_files[row].first().relative();
                return m_pixmaps[originalFileName];
            }
        }
        else if (role == Qt::SizeHintRole) {
            return m_thumbnailSize;
        }
    }

    return QVariant();
}

QVariant DuplicatesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    if (section == 0) {
        return i18nc("@title:column Image preview", "Preview");
    } else {
        return i18nc("@title:column Duplicate file", "Duplicate %1", section);
    }
}
} // namespace MainWindow

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_DuplicatesModel.cpp"
