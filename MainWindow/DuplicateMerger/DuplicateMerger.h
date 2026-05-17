// SPDX-FileCopyrightText: 2012 - 2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2012 - 2025 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef MAINWINDOW_DUPLICATEMERGER_H
#define MAINWINDOW_DUPLICATEMERGER_H

#include <DB/DuplicatesFinder.h>
#include <kpabase/FileNameList.h>
#include <ImageManager/ImageClientInterface.h>
#include <Utilities/DeleteFiles.h>

#include <QAbstractTableModel>
#include <QDialog>
#include <QMap>
#include <QWidget>

class DuplicateSortFilterProxyModel;
class DuplicatesTableView;
class QItemSelection;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QRadioButton;

namespace MainWindow
{

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

    unsigned getTotalCount();

    void addDuplicates(const DB::FileNameList &files);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;

private:
    QList<DB::FileNameList> m_files;
    QMap<QString, QPixmap> m_pixmaps;
    int m_maxDuplicates;
};


class DuplicateMerger : public QDialog
{
    Q_OBJECT

public:
    explicit DuplicateMerger(const DB::DuplicatesType& duplicates, QWidget *parent = nullptr);
    ~DuplicateMerger() override;

private Q_SLOTS:
    void addToKeepFiles();
    void removeFromKeepFiles();
    void enableAddToKeepFiles(const QModelIndex &parent, int first, int last);
    void selectNone();
    void go();
    void textChanged(const QString &);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QRadioButton *m_trash;
    QRadioButton *m_deleteFromDisk;
    QRadioButton *m_blockFromDB;

    QPushButton *m_addButton;
    QPushButton *m_removeButton;
    QPushButton *m_selectNoneButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    QLineEdit *m_lineEdit;

    DuplicatesModel *m_model;
    DuplicateSortFilterProxyModel *m_filterProxy;

    DuplicatesTableView *m_duplicatesView;
    QListWidget *m_keepersList;

    // Maps a filename to keep to its row in m_duplicatesView.
    QMap<QString, int> m_indexes;
};

} // namespace MainWindow

#endif // MAINWINDOW_DUPLICATEMERGER_H

// vi:expandtab:tabstop=4 shiftwidth=4:
