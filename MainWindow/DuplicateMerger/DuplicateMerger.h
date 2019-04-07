/* Copyright 2012-2016 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAINWINDOW_DUPLICATEMERGER_H
#define MAINWINDOW_DUPLICATEMERGER_H

#include <QWidget>
#include <QDialog>
#include <QMap>

#include <DB/MD5.h>
#include <DB/FileNameList.h>

class QVBoxLayout;
class QRadioButton;
class QLabel;
class QPushButton;

namespace MainWindow {

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
    void addRow(const DB::MD5&);
    void selectAll(bool b);
    void tellThatNoDuplicatesWereFound();

    QMap<DB::MD5, DB::FileNameList> m_matches;

    QWidget* m_container;
    QVBoxLayout* m_scrollLayout;
    QList<DuplicateMatch*> m_selectors;
    QRadioButton* m_trash;
    QRadioButton *m_deleteFromDisk;
    QLabel* m_selectionCount;

    QPushButton* m_selectAllButton;
    QPushButton* m_selectNoneButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
};

} // namespace MainWindow

#endif // MAINWINDOW_DUPLICATEMERGER_H

// vi:expandtab:tabstop=4 shiftwidth=4:
