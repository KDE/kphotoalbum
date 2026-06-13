// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef MAINWINDOW_DUPLICATESMODEL_H
#define MAINWINDOW_DUPLICATESMODEL_H

#include <kpabase/FileNameList.h>
#include <DB/DuplicatesFinder.h>
#include <ImageManager/ImageClientInterface.h>

#include <QAbstractTableModel>
#include <QMap>

namespace MainWindow
{

/**
 * @brief The data model for the duplicates table view.
 *
 * Each row contains two or more duplicates (ie. files with the same MD5 sum).
 * Any row in the table can contain more than two duplicates, and the table
 * rows do not necessarily contain the same number of cells.
 */
class DuplicatesModel : public QAbstractTableModel, ImageManager::ImageClientInterface
{
public:
    explicit DuplicatesModel(const DB::DuplicatesType& duplicates, QObject *parent = nullptr);

    ~DuplicatesModel() override;

    int rowCount(const QModelIndex &index = QModelIndex()) const override
    {
        if (index.isValid())
            return 0;
        else
            return m_files.count();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        // One column for the pixmap and a column for each duplicate.
        return 1 + m_maxDuplicates;
    }

    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (index.column() == 0) {
            // Don't allow selection in column zero (the pixmap column) but
            // keep it enabled so the pixmaps aren't grayscaled.
            return Qt::ItemIsEnabled;
        }

        return QAbstractTableModel::flags(index);
    }

    unsigned getTotalCount();

    void addDuplicates(const DB::FileNameList &files);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;

private:
    QList<DB::FileNameList> m_files;
    QMap<QString, QPixmap> m_pixmaps;
    int m_maxDuplicates;
};

} // namespace MainWindow

#endif // MAINWINDOW_DUPLICATESMODEL_H

// vi:expandtab:tabstop=4 shiftwidth=4:
