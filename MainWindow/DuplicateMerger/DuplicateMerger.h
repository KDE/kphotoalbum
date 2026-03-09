// SPDX-FileCopyrightText: 2012 - 2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2012 - 2025 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef MAINWINDOW_DUPLICATEMERGER_H
#define MAINWINDOW_DUPLICATEMERGER_H

#include <DB/MD5.h>
#include <kpabase/FileNameList.h>
#include <ImageManager/ImageClientInterface.h>
#include <Utilities/DeleteFiles.h>

#include <QAbstractTableModel>
#include <QDialog>
#include <QMap>
#include <QWidget>

class QItemSelection;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSortFilterProxyModel;
class QTableView;

namespace MainWindow
{

class DuplicateMatch;

class DuplicatesModel : public QAbstractTableModel, ImageManager::ImageClientInterface
{
public:
    explicit DuplicatesModel(QObject *parent = nullptr);

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
    explicit DuplicateMerger(QWidget *parent = nullptr);
    ~DuplicateMerger() override;

private Q_SLOTS:
    void selectNone();
    void go();
    void updateSelectionCount(qsizetype selectionCount = 0);
    void textChanged(const QString &);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    void findDuplicates();
    void tellThatNoDuplicatesWereFound();

    QMap<DB::MD5, DB::FileNameList> m_matches;

    QWidget *m_container;
    QRadioButton *m_trash;
    QRadioButton *m_deleteFromDisk;
    QRadioButton *m_blockFromDB;
    QLabel *m_selectionCount;

    QPushButton *m_selectNoneButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    QLineEdit *m_lineEdit;
    QSortFilterProxyModel *m_filterProxy;

    DuplicatesModel *m_model;

    QTableView *m_tableView;
    QLabel *m_previewWidget;
};

} // namespace MainWindow

#endif // MAINWINDOW_DUPLICATEMERGER_H

// vi:expandtab:tabstop=4 shiftwidth=4:
