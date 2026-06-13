// SPDX-FileCopyrightText: 2012 - 2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2012 - 2025 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef MAINWINDOW_DUPLICATEMERGER_H
#define MAINWINDOW_DUPLICATEMERGER_H

#include <DB/DuplicatesFinder.h>
#include <ImageManager/ImageClientInterface.h>

#include <QDialog>
#include <QMap>
#include <QWidget>

class DuplicateSortFilterProxyModel;
namespace MainWindow
{
class DuplicatesModel;
};
class QItemSelection;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QRadioButton;
class QTableView;

namespace MainWindow
{

/**
 * @brief Implements the merge duplicates dialog.
 *
 * Presents a table of duplicates and a list of files to keep.  When one or
 * more files are selected and added to the list of files to keep, the rows in
 * the duplicates table are hidden.  If a file is removed from the list of
 * files to keep, the corresponding row in the duplicates table is unhidden.
 */
class DuplicateMerger : public QDialog
{
    Q_OBJECT

public:
    explicit DuplicateMerger(const DB::DuplicatesType& duplicates, QWidget *parent = nullptr);
    ~DuplicateMerger() override;

private Q_SLOTS:
    void addToKeepFiles();
    void removeFromKeepFiles();
    void selectNone();
    void go();
    void textChanged(const QString &);
    void duplicatesTableSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void keepersListSelectionChanged();

private:
    QRadioButton *m_trash;
    QRadioButton *m_deleteFromDisk;
    QRadioButton *m_blockFromDB;

    QPushButton *m_addButton;
    QPushButton *m_removeButton;
    QPushButton *m_selectNoneButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    // The duplicates filter.
    QLineEdit *m_lineEdit;

    // The data and sort models for the duplicates table.
    MainWindow::DuplicatesModel *m_model;
    DuplicateSortFilterProxyModel *m_filterProxy;

    // The duplicates table.
    QTableView *m_duplicatesView;

    // The list of files to keep.
    QListWidget *m_keepersList;

    // Maps a filename in m_keepersList to its hidden row in m_duplicatesView.
    QMap<QString, int> m_indexes;
};

} // namespace MainWindow

#endif // MAINWINDOW_DUPLICATEMERGER_H

// vi:expandtab:tabstop=4 shiftwidth=4:
