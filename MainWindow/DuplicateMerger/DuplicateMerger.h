/* SPDX-FileCopyrightText: 2012-2016 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MAINWINDOW_DUPLICATEMERGER_H
#define MAINWINDOW_DUPLICATEMERGER_H

#include <DB/MD5.h>
#include <kpabase/FileNameList.h>

#include <QDialog>
#include <QMap>
#include <QWidget>

class QVBoxLayout;
class QRadioButton;
class QLabel;
class QPushButton;

namespace MainWindow
{

class DuplicateMatch;

class DuplicateMerger : public QDialog
{
    Q_OBJECT

public:
    explicit DuplicateMerger(QWidget *parent = nullptr);
    ~DuplicateMerger() override;

private slots:
    void selectAll();
    void selectNone();
    void go();
    void updateSelectionCount();

private:
    void findDuplicates();
    void addRow(const DB::MD5 &);
    void selectAll(bool b);
    void tellThatNoDuplicatesWereFound();

    QMap<DB::MD5, DB::FileNameList> m_matches;

    QWidget *m_container;
    QVBoxLayout *m_scrollLayout;
    QList<DuplicateMatch *> m_selectors;
    QRadioButton *m_trash;
    QRadioButton *m_deleteFromDisk;
    QLabel *m_selectionCount;

    QPushButton *m_selectAllButton;
    QPushButton *m_selectNoneButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

} // namespace MainWindow

#endif // MAINWINDOW_DUPLICATEMERGER_H

// vi:expandtab:tabstop=4 shiftwidth=4:
