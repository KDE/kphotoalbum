/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
