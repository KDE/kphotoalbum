/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
class QGridLayout;
class QLabel;
class QGroupBox;
class KComboBox;
class QTreeWidgetItem;
class QTreeWidget;

namespace DB
{
class ImageSearchInfo;
}
namespace MainWindow
{

class StatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDialog(QWidget *parent);
    QSize sizeHint() const override;
    void show();

private:
    QTreeWidgetItem *addRow(const QString &title, int noOfTags, int tagCount, int imageCount, QTreeWidgetItem *parent);
    void highlightTotalRow(QTreeWidgetItem *item);
    QGroupBox *createAnnotatedGroupBox();
    void populateSubTree(const DB::ImageSearchInfo &info, int imageCount, QTreeWidgetItem *top);

private slots:
    void categoryChanged(int);
    void populate();

private:
    QTreeWidget *m_treeWidget;
    KComboBox *m_category;
    QLabel *m_tagLabel;
    KComboBox *m_tag;
    QGridLayout *m_boxLayout;
};

}

#endif /* STATISTICSDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
